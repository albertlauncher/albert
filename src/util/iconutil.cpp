// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "icons.h"
#include "iconutil.h"
#include "logging.h"
#include "qiconengineadapter.h"
#include <QPainter>
#include <memory>
using namespace albert;
using namespace std;

// ---------------------------------------------------------------------------------------------------------------------

QSize albert::Icon::actualSize(const QSize &device_independent_size, qreal)
{
    return device_independent_size;
}

QPixmap Icon::pixmap(const QSize &device_independent_size, qreal device_pixel_ratio)
{
    const auto actual_device_independent_size = actualSize(device_independent_size, device_pixel_ratio);

    QPixmap pm(actual_device_independent_size * device_pixel_ratio);
    pm.setDevicePixelRatio(device_pixel_ratio);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    paint(&p, QRect(QPoint(0,0), actual_device_independent_size));

    return pm;
}

bool Icon::isNull() { return false; }

QString Icon::cacheKey() { return toUrl(); }


// ---------------------------------------------------------------------------------------------------------------------

static inline bool checkUrlScheme(const QString &url, const QString &scheme)
{
    return url.size() > scheme.size() + 1  // ":"
           && url[scheme.size()] == u':'
           && url.startsWith(scheme);
}

static unique_ptr<Icon> dispatch(const QString &url)
{
    if (checkUrlScheme(url, FileTypeIcon::scheme))
        return FileTypeIcon::fromUrl(url);

    else if (checkUrlScheme(url, StandardIcon::scheme))
        return StandardIcon::fromUrl(url);

    else if (checkUrlScheme(url, ThemeIcon::scheme))
        return ThemeIcon::fromUrl(url);

    else if (checkUrlScheme(url, ImageIcon::scheme))
        return ImageIcon::fromUrl(url);

    else if (checkUrlScheme(url, GraphemeIcon::scheme))
        return GraphemeIcon::fromUrl(url);

    else if (checkUrlScheme(url, RectIcon::scheme))
        return RectIcon::fromUrl(url);

    else if (checkUrlScheme(url, ComposedIcon::scheme))
        return ComposedIcon::fromUrl(url);

    else if (checkUrlScheme(url, IconifiedIcon::scheme))
        return IconifiedIcon::fromUrl(url);

    else if (checkUrlScheme(url, ImageIcon::qrc_scheme))
        return make_unique<ImageIcon>(url.mid(ImageIcon::qrc_scheme.size()));  // keep ":"

    else  // takes care of qresource files (:bla) as well as regular files (/foo/bar)
        return make_unique<ImageIcon>(url);

    return {};
}

QIcon albert::qIcon(unique_ptr<Icon> icon) {
    if (icon && !icon->isNull())
        return QIcon(new QIconEngineAdapter(::move(icon)));
    return {};
}

unique_ptr<Icon> albert::iconFromUrl(const QString &url)
{
    if (auto engine = dispatch(url); engine && !engine->isNull())
        return engine;
    return {};
}

unique_ptr<Icon> albert::iconFromUrls(const QStringList &urls)
{
    for (const auto &url : urls)
        if (auto engine = iconFromUrl(url); engine && !engine->isNull())
            return engine;

    WARN << "Failed getting icon for:" << urls;
    return {};
}


// ---------------------------------------------------------------------------------------------------------------------

unique_ptr<Icon> albert::makeImageIcon(const QString &path)
{ return make_unique<ImageIcon>(path); }

unique_ptr<Icon> albert::makeImageIcon(const filesystem::path &path)
{ return make_unique<ImageIcon>(path); }


// ---------------------------------------------------------------------------------------------------------------------

unique_ptr<Icon> albert::makeFileTypeIcon(const QString &path)
{ return make_unique<FileTypeIcon>(path); }

unique_ptr<Icon> albert::makeFileTypeIcon(const filesystem::path &path)
{ return make_unique<FileTypeIcon>(path); }


// ---------------------------------------------------------------------------------------------------------------------

unique_ptr<Icon> albert::makeStandardIcon(StandardIconType type)
{ return make_unique<StandardIcon>(type); }


// ---------------------------------------------------------------------------------------------------------------------

unique_ptr<Icon> albert::makeThemeIcon(const QString &icon_name)
{ return make_unique<ThemeIcon>(icon_name); }


// ---------------------------------------------------------------------------------------------------------------------

const QBrush &albert::graphemeIconDefaultColor()
{ return GraphemeIcon::default_color; }

double albert::graphemeIconDefaultScalar()
{ return GraphemeIcon::default_scalar; }

unique_ptr<Icon> albert::makeGraphemeIcon(const QString &grapheme, double scalar, const QBrush &color)
{ return make_unique<GraphemeIcon>(grapheme, scalar, color); }


// ---------------------------------------------------------------------------------------------------------------------

const QBrush &albert::rectIconDefaultColor()
{ return RectIcon::default_color; }

const QBrush &albert::rectIconDefaultBorderColor()
{ return RectIcon::default_border_color; }

double albert::rectIconDefaultRadius()
{ return RectIcon::default_radius; }

int albert::rectIconDefaultBorderWidth()
{ return RectIcon::default_border_width; }

unique_ptr<Icon> albert::makeRectIcon(const QBrush &color, double radius, int border_width, const QBrush &border_color)
{ return make_unique<RectIcon>(color, radius, border_width, border_color); }


// ---------------------------------------------------------------------------------------------------------------------

const QBrush &albert::iconifiedIconDefaultColor()
{ return IconifiedIcon::default_color; }

double albert::iconifiedIconDefaultBorderRadius()
{ return IconifiedIcon::default_border_radius; }

int albert::iconifiedIconDefaultBorderWidth()
{ return IconifiedIcon::default_border_width; }

const QBrush &albert::iconifiedIconDefaultBorderColor()
{ return IconifiedIcon::default_border_color; }

unique_ptr<Icon> albert::makeIconifiedIcon(unique_ptr<Icon> src,
                                           const QBrush &color,
                                           double border_radius,
                                           int border_width,
                                           const QBrush &border_color)
{ return make_unique<IconifiedIcon>(::move(src), color, border_radius, border_width, border_color); }


// ---------------------------------------------------------------------------------------------------------------------

double albert::composedIconDefaultSize()
{ return ComposedIcon::default_size; }

double albert::composedIconDefaultPos1()
{ return ComposedIcon::default_pos1; }

double albert::composedIconDefaultPos2()
{ return ComposedIcon::default_pos2; }

unique_ptr<Icon> albert::makeComposedIcon(unique_ptr<Icon> src1,
                                          unique_ptr<Icon> src2,
                                          double size1,
                                          double size2,
                                          double x1,
                                          double y1,
                                          double x2,
                                          double y2)
{ return make_unique<ComposedIcon>(::move(src1), ::move(src2), size1, size2, x1, y1, x2, y2); }
