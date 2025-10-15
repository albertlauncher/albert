// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "icons.h"
#include "iconutil.h"
#include "logging.h"
#include "networkutil.h"
#include "systemutil.h"
#include <QApplication>
#include <QFile>
#include <QFileIconProvider>
#include <QMetaEnum>
#include <QPainter>
#include <QStyle>
#include <QUrlQuery>
#include <memory>
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include "iconlookup.h"
#endif
static QFileIconProvider qfip;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

// ---------------------------------------------------------------------------------------------------------------------

QIconIcon::QIconIcon(const QIcon &icon) : icon_(icon) {}

QSize QIconIcon::actualSize(const QSize &device_independent_size, qreal device_pixel_ratio)
{
    //
    // Docs: QIcon::actualSize:
    //
    // > Returns the actual size of the icon for the requested size, mode, and state. The result might be smaller than
    // > requested, but never larger. The returned size is in device-independent pixels (This is relevant for high-dpi
    // > pixmaps.)
    //
    // Tests show it returns device dependent sizes.
    //
    return icon_.actualSize(device_independent_size * device_pixel_ratio) / device_pixel_ratio;
}

QPixmap QIconIcon::pixmap(const QSize &device_independent_size, qreal device_pixel_ratio)
{
    return icon_.pixmap(device_independent_size, device_pixel_ratio);
}

void QIconIcon::paint(QPainter *painter, const QRect &rect) { icon_.paint(painter, rect); }

bool QIconIcon::isNull() { return icon_.isNull(); }

// ---------------------------------------------------------------------------------------------------------------------

// TODO use imageloader

const QString ImageIcon::scheme     = u"file"_s;
const QString ImageIcon::qrc_scheme = u"qrc"_s;

ImageIcon::ImageIcon(const QString &path)
    : QIconIcon(QFile::exists(path) ? QIcon(path) : QIcon())  // QIcon produces non null icons from non existing files
    , path_(path)
{}

ImageIcon::ImageIcon(const std::filesystem::path &path)
    : ImageIcon(toQString(path))
{}

std::unique_ptr<Icon> ImageIcon::clone() const { return make_unique<ImageIcon>(*this); }

QString ImageIcon::toUrl() const { return u"%1:%2"_s.arg(scheme, path_); }

std::unique_ptr<ImageIcon> ImageIcon::fromUrl(const QString &url)
{ return make_unique<ImageIcon>(url.mid(scheme.size()+1)); }  // ":"


// ---------------------------------------------------------------------------------------------------------------------

const QString FileTypeIcon::scheme = u"qfip"_s;

FileTypeIcon::FileTypeIcon(const QString &path)
    : QIconIcon(qfip.icon(QFileInfo(path)))
    , path_(path)
{}

FileTypeIcon::FileTypeIcon(const std::filesystem::path &path)
    : FileTypeIcon(toQString(path))
{}

std::unique_ptr<Icon> FileTypeIcon::clone() const { return make_unique<FileTypeIcon>(*this); }

QString FileTypeIcon::toUrl() const { return u"%1:%2"_s.arg(scheme, path_); }

std::unique_ptr<FileTypeIcon> FileTypeIcon::fromUrl(const QString &url)
{ return make_unique<FileTypeIcon>(url.mid(scheme.size() + 1 )); }  // ":"


// ---------------------------------------------------------------------------------------------------------------------

// https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum

const QString StandardIcon::scheme = u"qsp"_s;

const QMetaEnum StandardIcon::meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();

StandardIcon::StandardIcon(int standard_icon_enum_value)
    : QIconIcon(qApp->style()->standardIcon((QStyle::StandardPixmap)standard_icon_enum_value))
    , standard_icon_enum_value_(standard_icon_enum_value)
{}

std::unique_ptr<Icon> StandardIcon::clone() const { return make_unique<StandardIcon>(*this); }

QString StandardIcon::toUrl() const { return u"%1:%2"_s.arg(scheme, meta_enum.key(standard_icon_enum_value_)); }

std::unique_ptr<StandardIcon> StandardIcon::fromUrl(const QString &url)
{
    const auto enum_key = url.mid(scheme.size() + 1 );  // ":"

    for (int i = 0; i < meta_enum.keyCount(); ++i)
        if (enum_key == meta_enum.key(i))
            return make_unique<StandardIcon>(meta_enum.value(i));

    WARN << "No such QStyle::StandardPixmap key found:" << enum_key;
    return nullptr;
}


// ---------------------------------------------------------------------------------------------------------------------

const QString ThemeIcon::scheme = u"xdg"_s;

ThemeIcon::ThemeIcon(const QString &name)
    : QIconIcon(
        // https://bugreports.qt.io/browse/QTBUG-135159
        // https://codereview.qt-project.org/c/qt/qtbase/+/634907
#if QT_VERSION > 0x060800
        QIcon::fromTheme(name)
#else
        QIcon(XDG::IconLookup::iconPath(name))
#endif
    )
    , name_(name)
{}

std::unique_ptr<Icon> ThemeIcon::clone() const { return make_unique<ThemeIcon>(*this); }

QString ThemeIcon::toUrl() const { return u"%1:%2"_s.arg(scheme, name_); }

std::unique_ptr<ThemeIcon> ThemeIcon::fromUrl(const QString &url)
{ return make_unique<ThemeIcon>(url.mid(scheme.size() + 1 )); }  // ":"


// ---------------------------------------------------------------------------------------------------------------------

const QString GraphemeIcon::scheme = u"grapheme"_s;

const double GraphemeIcon::default_scalar = 1.0;

const QBrush GraphemeIcon::default_color = Qt::black;

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

std::unique_ptr<Icon> GraphemeIcon::clone() const { return make_unique<GraphemeIcon>(*this); }

QString GraphemeIcon::toUrl() const
{
    QString url = u"%1:?grapheme=%2"_s.arg(scheme, percentEncoded(grapheme_));
    if (scalar_ != default_scalar)
        url += u"&scalar="_s + QString::number(scalar_);
    if (color_ != default_color)
        url += u"&color="_s + color_.color().name(QColor::HexArgb);
    return url;
}

std::unique_ptr<GraphemeIcon> GraphemeIcon::fromUrl(const QString &url)
{
    QUrlQuery urlquery(url.mid(scheme.size() + 2));  // ":?"

    QString text{urlquery.queryItemValue(u"grapheme"_s)};

    bool ok;
    double scalar{urlquery.queryItemValue(u"scalar"_s).toDouble(&ok)};
    if (!ok)
        scalar = default_scalar;

    QColor color(urlquery.queryItemValue(u"color"_s));

    return make_unique<GraphemeIcon>(text, scalar, color.isValid() ? color : default_color);
}


// ---------------------------------------------------------------------------------------------------------------------

const QString RectIcon::scheme = u"rect"_s;

const QBrush RectIcon::default_color = QBrush(Qt::black);

const double RectIcon::default_radius = 1.0;

const int RectIcon::default_border_width = 0;

const QBrush RectIcon::default_border_color = QBrush(Qt::black);

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

std::unique_ptr<Icon> RectIcon::clone() const { return make_unique<RectIcon>(*this); }

QString RectIcon::toUrl() const
{
    QString url = u"%1:color=%2"_s.arg(scheme, color_.color().name(QColor::HexArgb));
    if (radius_ != default_radius)
        url += u"&radius="_s + QString::number(radius_);
    if (border_width_ != default_border_width)
        url += u"&border_width="_s + QString::number(border_width_);
    if (border_color_ != default_border_color)
        url += u"&border_color="_s + border_color_.color().name(QColor::HexArgb);
    return url;
}

std::unique_ptr<RectIcon> RectIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme.size() + 2));  // ":?"

    QColor color(url_query.queryItemValue(u"color"_s));

    QColor border_color(url_query.queryItemValue(u"border_color"_s));

    bool ok;
    int border_width{url_query.queryItemValue(u"border_width"_s).toInt(&ok)};
    if (!ok)
        border_width = default_border_width;

    double radius{url_query.queryItemValue(u"radius"_s).toDouble(&ok)};
    if (!ok)
        radius = default_radius;

    return make_unique<RectIcon>(color.isValid() ? color : default_color,
                                 radius,
                                 border_width,
                                 border_color.isValid() ? border_color : default_border_color);
}


// ---------------------------------------------------------------------------------------------------------------------

const QString ComposedIcon::scheme = u"comp"_s;
const double ComposedIcon::default_size = 0.7;
const double ComposedIcon::default_pos1 = 0.0;
const double ComposedIcon::default_pos2 = 1.0;

ComposedIcon::ComposedIcon(std::unique_ptr<Icon> src1,
                           std::unique_ptr<Icon> src2,
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

std::unique_ptr<Icon> ComposedIcon::clone() const
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
    QString url = u"%1:?src1=%2&src2=%3"_s.arg(scheme,
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

std::unique_ptr<ComposedIcon> ComposedIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme.size() + 2));  // ":?"

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


// ---------------------------------------------------------------------------------------------------------------------

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

const QString IconifiedIcon::scheme = u"icon"_s;

const QBrush IconifiedIcon::default_color = makeBackgroundGradient(255, 224);

const int IconifiedIcon::default_border_width = 1.0;

const double IconifiedIcon::default_border_radius = 1.0;

const QBrush IconifiedIcon::default_border_color = makeBorderGradient(224, 192);

IconifiedIcon::IconifiedIcon(std::unique_ptr<Icon> src,
                             const QBrush &background_color,
                             double border_radius,
                             int border_width,
                             const QBrush &border_color) :
    src_(::move(src)),
    background_color_(background_color),
    border_radius_(border_radius),
    border_width_(border_width),
    border_color_(border_color)
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
    imgp.setBrush(background_color_);
    imgp.drawRoundedRect(img_content_rect, 100 * border_radius_, 100 * border_radius_, Qt::RelativeSize);

    // Draw pixmap
    imgp.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    src_->paint(&imgp, img_content_rect);

    // Draw border circle
    if (border_width_ > 0)
    {
        QPen pen(border_color_, border_width_);
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

std::unique_ptr<Icon> IconifiedIcon::clone() const
{
    return make_unique<IconifiedIcon>(src_->clone(),
                                      background_color_,
                                      border_width_,
                                      border_radius_,
                                      border_color_);
}

QString IconifiedIcon::toUrl() const
{
    QString url = u"%1:?src=%2"_s.arg(scheme, percentEncoded(src_->toUrl()));
    if (background_color_ != default_color)
        url += u"&color="_s + background_color_.color().name(QColor::HexArgb);
    if (border_width_ != default_border_width)
        url += u"&border_width="_s + QString::number(border_width_);
    if (border_radius_ != default_border_radius)
        url += u"&border_radius="_s + QString::number(border_radius_);
    if (border_color_ != default_border_color)
        url += u"&border_color="_s + border_color_.color().name(QColor::HexArgb);
    return url;
}

std::unique_ptr<IconifiedIcon> IconifiedIcon::fromUrl(const QString &url)
{
    QUrlQuery url_query(url.mid(scheme.size() + 2));  // ":?"

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
                                      color.isValid() ? color : default_color,
                                      border_width,
                                      border_radius,
                                      border_color.isValid() ? border_color : default_border_color);
}
