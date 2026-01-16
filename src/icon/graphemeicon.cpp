// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "graphemeicon.h"
#include "networkutil.h"
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QUrlQuery>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

GraphemeIcon::GraphemeIcon(const QString &grapheme, double scalar, const QBrush &color) :
    grapheme_(grapheme),
    scalar_(scalar),
    color_(color)
{}

void GraphemeIcon::paint(QPainter *p, const QRect &rect)
{
    if (grapheme_.isEmpty())
        return;

    p->save();

    QFont font = p->font();
    // rough initial estimate to skip the first iterations. asc ~= 4 * desc, plus some buffer
    font.setPixelSize(int(rect.height() * 5 / 6 ));
    p->setFont(font);
    auto br = p->boundingRect(rect, Qt::AlignCenter, grapheme_);

    while (rect.width() < br.width() || rect.height() < br.height())
    {
        font.setPixelSize(font.pixelSize() - 1);
        p->setFont(font);
        br = p->boundingRect(rect, Qt::AlignCenter, grapheme_);
    }

    if (scalar_ != 1.0)
    {
        font.setPixelSize(int(font.pixelSize() * scalar_));
        p->setFont(font);
    }

    p->setPen(QPen(color_, 1/p->device()->devicePixelRatioF()));
    p->drawText(rect.toRectF(), Qt::AlignCenter, grapheme_);
    // p->drawRect(p->boundingRect(rect, Qt::AlignCenter, grapheme_));

    p->restore();

}

bool GraphemeIcon::isNull() { return grapheme_.isEmpty(); }

unique_ptr<Icon> GraphemeIcon::clone() const { return make_unique<GraphemeIcon>(*this); }

QString GraphemeIcon::toUrl() const
{
    QString url = u"%1:?grapheme=%2"_s.arg(scheme(), percentEncoded(grapheme_));
    if (scalar_ != 1.0)
        url += u"&scalar="_s + QString::number(scalar_);
    if (color_ != defaultBrush())
        url += u"&color="_s + color_.color().name(QColor::HexArgb);
    return url;
}

unique_ptr<GraphemeIcon> GraphemeIcon::fromUrl(const QString &url)
{
    QUrlQuery urlquery(url.mid(scheme().size() + 2));  // ":?"

    QString text{urlquery.queryItemValue(u"grapheme"_s)};

    bool ok;
    double scalar{urlquery.queryItemValue(u"scalar"_s).toDouble(&ok)};
    if (!ok)
        scalar = 1.0;

    QColor color(urlquery.queryItemValue(u"color"_s));

    return make_unique<GraphemeIcon>(text, scalar, color.isValid() ? color : defaultBrush());
}

QString GraphemeIcon::scheme() { return u"grapheme"_s; }

QBrush GraphemeIcon::defaultBrush(){ return QApplication::palette().color(QPalette::WindowText); }
