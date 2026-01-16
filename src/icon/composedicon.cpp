// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "composedicon.h"
#include "networkutil.h"
#include <QPainter>
#include <QUrlQuery>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

double ComposedIcon::default_size = 0.7;
double ComposedIcon::default_pos1 = 0.0;
double ComposedIcon::default_pos2 = 1.0;

ComposedIcon::ComposedIcon(unique_ptr<Icon> src1,
                           unique_ptr<Icon> src2,
                           double size1,
                           double size2,
                           double x1,
                           double y1,
                           double x2,
                           double y2) :
    src1_(::move(src1)),
    src2_(::move(src2)),
    size1_(size1),
    size2_(size2),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2)
{}

void ComposedIcon::paint(QPainter *p, const QRect &rect)
{
    if (isNull())
        return;

    const auto extent = min(rect.width(), rect.height());

    // Add .5 to round instead of floor
    const int extent1 = int(extent * size1_ + .5);
    const int extent2 = int(extent * size2_ + .5);

    const auto dpr = p->device()->devicePixelRatio();

    const auto size1 = src1_->actualSize(QSize(extent1, extent1), dpr);
    const auto size2 = src2_->actualSize(QSize(extent2, extent2), dpr);

    const auto r1 = QRect(int((rect.width() - size1.width()) * x1_),
                          int((rect.height() - size1.height()) * y1_),
                          size1.width(),
                          size1.height());

    const auto r2 = QRect(int((rect.width() - size2.width()) * x2_),
                          int((rect.height() - size2.height()) * y2_),
                          size2.width(),
                          size2.height());

    src1_->paint(p, r1);
    src2_->paint(p, r2);
}

bool ComposedIcon::isNull() { return !src1_ || src1_->isNull() || !src2_ || src2_->isNull(); }

unique_ptr<Icon> ComposedIcon::clone() const
{
    return make_unique<ComposedIcon>(src1_->clone(),
                                     src2_->clone(),
                                     size1_,
                                     size2_,
                                     x1_,
                                     y1_,
                                     x2_,
                                     y2_);
}

QString ComposedIcon::toUrl() const
{
    QString url = u"%1:?src1=%2&src2=%3"_s.arg(scheme(),
                                               percentEncoded(src1_->toUrl()),
                                               percentEncoded(src2_->toUrl()));
    if (size1_ != default_size)
        url += u"&size1="_s + QString::number(size1_);
    if (size2_ != default_size)
        url += u"&size2="_s + QString::number(size2_);
    if (x1_ != default_pos1)
        url += u"&x1="_s + QString::number(x1_);
    if (y1_ != default_pos1)
        url += u"&y1="_s + QString::number(y1_);
    if (x2_ != default_pos2)
        url += u"&x2="_s + QString::number(x2_);
    if (y2_ != default_pos2)
        url += u"&y2="_s + QString::number(y2_);
    return url;
}

unique_ptr<ComposedIcon> ComposedIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme().size() + 2));  // ":?"

    auto src1 = iconFromUrl(percentDecoded(url_query.queryItemValue(u"src1"_s)));
    if (!src1 || src1->isNull())
        return {};

    auto src2 = iconFromUrl(percentDecoded(url_query.queryItemValue(u"src2"_s)));
    if (!src2 || src2->isNull())
        return {};

    const auto size1s = url_query.queryItemValue(u"size1"_s);
    const auto size1 = size1s.isEmpty() ? default_size : size1s.toDouble();

    const auto size2s = url_query.queryItemValue(u"size2"_s);
    const auto size2 = size2s.isEmpty() ? default_size : size2s.toDouble();

    const auto x1s = url_query.queryItemValue(u"x1"_s);
    const auto x1 = x1s.isEmpty() ? default_pos1 : x1s.toDouble();

    const auto y1s = url_query.queryItemValue(u"y1"_s);
    const auto y1 = y1s.isEmpty() ? default_pos1 : y1s.toDouble();

    const auto x2s = url_query.queryItemValue(u"x2"_s);
    const auto x2 = x2s.isEmpty() ? default_pos2 : x2s.toDouble();

    const auto y2s = url_query.queryItemValue(u"y2"_s);
    const auto y2 = y2s.isEmpty() ? default_pos2 : y2s.toDouble();

    return make_unique<ComposedIcon>(::move(src1), ::move(src2), size1, size2, x1, y1, x2, y2);
}

QString ComposedIcon::scheme() { return u"comp"_s; }
