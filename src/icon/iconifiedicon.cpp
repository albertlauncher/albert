// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "iconifiedicon.h"
#include "networkutil.h"
#include <QPainter>
#include <QUrlQuery>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;
static QRadialGradient makeBackgroundGradient(auto s, auto e)
{
    QRadialGradient gradient(.5, .5, .8, .5, .0);
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0.0, QColor(s, s, s));
    gradient.setColorAt(1.0, QColor(e, e, e));
    return gradient;
}

static QLinearGradient makeBorderGradient(auto s, auto e)
{
    QLinearGradient gradient(.0, .0, .0, 1.);
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0.0, QColor(s, s, s));
    gradient.setColorAt(1.0, QColor(e, e, e));
    return gradient;
}

double IconifiedIcon::default_border_radius = 1.0;
int IconifiedIcon::default_border_width = 1;
QBrush IconifiedIcon::default_background_brush = makeBackgroundGradient(255, 224);
QBrush IconifiedIcon::default_border_brush = makeBorderGradient(224, 192);

IconifiedIcon::IconifiedIcon(unique_ptr<Icon> icon,
                             const QBrush &background_brush,
                             double border_radius,
                             int border_width,
                             const QBrush &border_brush) :
    src_(::move(icon)),
    color_(background_brush),
    border_radius_(border_radius),
    border_width_(border_width),
    border_brush_(border_brush)
{}

QSize IconifiedIcon::actualSize(const QSize &device_independent_size, double device_pixel_ratio)
{
    const auto dst_extent = min(device_independent_size.width(), device_independent_size.height());

    const auto max_content_extent = dst_extent - 2 * border_width_;  // excl. border

    const auto src_size = src_->actualSize({max_content_extent, max_content_extent}, device_pixel_ratio);

    const auto src_extent = max(src_size.width(), src_size.height());  // excl. border

    const auto final_extent = min(src_extent + 2 * border_width_, dst_extent);  // incl. border

    return {final_extent, final_extent};
}

void IconifiedIcon::paint(QPainter *p, const QRect &rect)
{
    p->save();

    const auto dpr = p->device()->devicePixelRatioF();

    const auto size = actualSize(rect.size(), dpr);

    const auto final_rect = QRect(rect.topLeft() + QPoint(rect.width() - size.width(),
                                                          rect.height() - size.height()) / 2,
                                  size);

    QImage img(size * dpr, QImage::Format_ARGB32);
    img.setDevicePixelRatio(dpr);
    img.fill(Qt::transparent);

    QPainter imgp(&img);
    imgp.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    const QRect img_content_rect = QRect(border_width_, border_width_,
                                         size.width() - 2 * border_width_,
                                         size.height() - 2 * border_width_);

    // Draw backgound circle
    imgp.setPen(Qt::NoPen);
    imgp.setBrush(color_);
    imgp.drawRoundedRect(img_content_rect, 100 * border_radius_, 100 * border_radius_, Qt::RelativeSize);

    // Draw pixmap
    imgp.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    src_->paint(&imgp, img_content_rect);

    // Draw border circle
    if (border_width_ > 0)
    {
        QPen pen(border_brush_, border_width_);
        imgp.setCompositionMode(QPainter::CompositionMode_SourceOver);
        imgp.setPen(pen);
        imgp.setBrush(Qt::NoBrush);
        // const auto m = border_width_/2 + 1/dpr; // compensate for antialiasing
        // imgp.drawEllipse(img_rect.marginsRemoved({m, m, m, m}));
        imgp.drawRoundedRect(img_content_rect, 100 * border_radius_, 100 * border_radius_, Qt::RelativeSize);
    }

    imgp.end();

    p->drawImage(final_rect, img, img.rect());

    p->restore();
}

bool IconifiedIcon::isNull() { return !src_ || src_->isNull(); }

unique_ptr<Icon> IconifiedIcon::clone() const
{
    return make_unique<IconifiedIcon>(src_->clone(),
                                      color_,
                                      border_width_,
                                      border_radius_,
                                      border_brush_);
}

QString IconifiedIcon::toUrl() const
{
    QString url = u"%1:?src=%2"_s.arg(scheme(), percentEncoded(src_->toUrl()));
    if (color_ != default_background_brush)
        url += u"&color="_s + color_.color().name(QColor::HexArgb);
    if (border_width_ != default_border_width)
        url += u"&border_width="_s + QString::number(border_width_);
    if (border_radius_ != default_border_width)
        url += u"&border_radius="_s + QString::number(border_radius_);
    if (border_brush_ != default_border_brush)
        url += u"&border_color="_s + border_brush_.color().name(QColor::HexArgb);
    return url;
}


unique_ptr<IconifiedIcon> IconifiedIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme().size() + 2));  // ":?"

    auto src = iconFromUrl(percentDecoded(url_query.queryItemValue(u"src"_s)));

    QColor color(url_query.queryItemValue(u"color"_s));

    QColor border_color(url_query.queryItemValue(u"border_color"_s));

    bool ok;
    int border_width{url_query.queryItemValue(u"border_width"_s).toInt(&ok)};
    if (!ok)
        border_width = default_border_width;

    double border_radius{url_query.queryItemValue(u"border_radius"_s).toDouble(&ok)};
    if (!ok)
        border_radius = default_border_radius;

    return make_unique<IconifiedIcon>(::move(src),
                                      color.isValid() ? color : default_background_brush,
                                      border_width,
                                      border_radius,
                                      border_color.isValid() ? border_color : default_border_brush);
}

QString IconifiedIcon::scheme() { return u"icon"_s; }
