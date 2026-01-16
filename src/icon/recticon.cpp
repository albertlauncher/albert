// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "recticon.h"
#include <QPainter>
#include <QUrlQuery>
#include <memory>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

RectIcon::RectIcon(const QBrush &color, double radius, int border_width, const QBrush &border_color) :
    color_(color),
    radius_(radius),
    border_width_(border_width),
    border_color_(border_color)
{}

void RectIcon::paint(QPainter *p, const QRect &rect)
{
    p->save();

    p->setRenderHint(QPainter::RenderHint::Antialiasing, true);
    p->setPen(Qt::NoPen);

    const auto radius = radius_ * rect.width()/2;

    if (radius == 0)
    {
        if (border_width_ == 0)
        {
            p->setBrush(color_);
            p->drawRect(rect);
        }
        else
        {
            p->setBrush(border_color_);
            p->drawRect(rect);
            const auto inner_rect = rect.marginsRemoved({border_width_, border_width_, border_width_, border_width_});
            p->setBrush(color_);
            p->setCompositionMode(QPainter::CompositionMode_Source);
            p->drawRect(inner_rect);
        }
    }
    else
    {
        p->setRenderHint(QPainter::RenderHint::Antialiasing, true);
        if (border_width_ == 0)
        {
            p->setBrush(color_);
            p->drawRoundedRect(rect, radius-border_width_, radius-border_width_);
        }
        else
        {
            p->setBrush(border_color_);
            p->drawRoundedRect(rect, radius, radius);
            const auto inner_rect = rect.marginsRemoved({border_width_, border_width_, border_width_, border_width_});
            p->setBrush(color_);
            p->setCompositionMode(QPainter::CompositionMode_Source);
            p->drawRoundedRect(inner_rect, radius-border_width_, radius-border_width_);

        }
    }

    p->restore();
}

bool RectIcon::isNull() { return false; }

unique_ptr<Icon> RectIcon::clone() const { return make_unique<RectIcon>(*this); }

QString RectIcon::toUrl() const
{
    QString url = u"%1:color=%2"_s.arg(scheme(), color_.color().name(QColor::HexArgb));
    if (radius_ != defaultRadius())
        url += u"&radius="_s + QString::number(radius_);
    if (border_width_ != defaultBorderWidth())
        url += u"&border_width="_s + QString::number(border_width_);
    if (border_color_ != defaultBorderColor())
        url += u"&border_color="_s + border_color_.color().name(QColor::HexArgb);
    return url;
}

unique_ptr<RectIcon> RectIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme().size() + 2));  // ":?"

    QColor color(url_query.queryItemValue(u"color"_s));

    QColor border_color(url_query.queryItemValue(u"border_color"_s));

    bool ok;
    int border_width{url_query.queryItemValue(u"border_width"_s).toInt(&ok)};
    if (!ok)
        border_width = defaultBorderWidth();

    double radius{url_query.queryItemValue(u"radius"_s).toDouble(&ok)};
    if (!ok)
        radius = defaultRadius();

    return make_unique<RectIcon>(color.isValid() ? color : defaultColor(),
                                 radius,
                                 border_width,
                                 border_color.isValid() ? border_color : defaultBorderColor());
}

QString RectIcon::scheme() { return u"rect"_s; }

QBrush RectIcon::defaultColor() { return QBrush(Qt::black); }

double RectIcon::defaultRadius() { return 1.0; }

int RectIcon::defaultBorderWidth() { return 0; }

QBrush RectIcon::defaultBorderColor() { return QBrush(Qt::black); }
