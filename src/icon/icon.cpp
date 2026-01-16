// SPDX-FileCopyrightText: 2022-2026 Manuel Schneider

#include "icon.h"
#include "composedicon.h"
#include "filetypeicon.h"
#include "graphemeicon.h"
#include "iconifiedicon.h"
#include "imageicon.h"
#include "logging.h"
#include "qiconengineadapter.h"
#include "recticon.h"
#include "standardicon.h"
#include "themeicon.h"
#include <QPainter>
#include <memory>
using namespace albert;
using namespace std;

Icon::~Icon() = default;

QSize Icon::actualSize(const QSize &device_independent_size, qreal)
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
    if (checkUrlScheme(url, FileTypeIcon::scheme()))
        return FileTypeIcon::fromUrl(url);

    else if (checkUrlScheme(url, StandardIcon::scheme()))
        return StandardIcon::fromUrl(url);

    else if (checkUrlScheme(url, ThemeIcon::scheme()))
        return ThemeIcon::fromUrl(url);

    else if (checkUrlScheme(url, ImageIcon::fileScheme()))
        return ImageIcon::fromUrl(url);

    else if (checkUrlScheme(url, GraphemeIcon::scheme()))
        return GraphemeIcon::fromUrl(url);

    else if (checkUrlScheme(url, RectIcon::scheme()))
        return RectIcon::fromUrl(url);

    else if (checkUrlScheme(url, ComposedIcon::scheme()))
        return ComposedIcon::fromUrl(url);

    else if (checkUrlScheme(url, IconifiedIcon::scheme()))
        return IconifiedIcon::fromUrl(url);

    else if (checkUrlScheme(url, ImageIcon::qrcScheme()))
        return make_unique<ImageIcon>(url.mid(ImageIcon::qrcScheme().size()));  // keep ":"

    else  // takes care of qresource files (:bla) as well as regular files (/foo/bar)
        return make_unique<ImageIcon>(url);

    return {};
}

//--------------------------------------------------------------------------------------------------

QIcon Icon::qIcon(unique_ptr<Icon> icon) {
    if (icon && !icon->isNull())
        return QIcon(new QIconEngineAdapter(::move(icon)));
    return {};
}

unique_ptr<Icon> Icon::iconFromUrl(const QString &url)
{
    if (auto engine = dispatch(url); engine && !engine->isNull())
        return engine;
    return {};
}

unique_ptr<Icon> Icon::iconFromUrls(const QStringList &urls)
{
    for (const auto &url : urls)
        if (auto engine = iconFromUrl(url); engine && !engine->isNull())
            return engine;

    WARN << "Failed getting icon for:" << urls;
    return {};
}

//--------------------------------------------------------------------------------------------------

unique_ptr<Icon> Icon::image(const QString &path) { return make_unique<ImageIcon>(path); }

unique_ptr<Icon> Icon::image(const filesystem::path &path) { return make_unique<ImageIcon>(path); }

// -------------------------------------------------------------------------------------------------

unique_ptr<Icon> Icon::fileType(const QString &path) { return make_unique<FileTypeIcon>(path); }

unique_ptr<Icon> Icon::fileType(const filesystem::path &path)
{ return make_unique<FileTypeIcon>(path); }

// -------------------------------------------------------------------------------------------------

unique_ptr<Icon> Icon::theme(const QString &icon_name) { return make_unique<ThemeIcon>(icon_name); }

// -------------------------------------------------------------------------------------------------

unique_ptr<Icon> Icon::standard(StandardIconType type) { return make_unique<StandardIcon>(type); }

// -------------------------------------------------------------------------------------------------


QBrush Icon::graphemeDefaultBrush() { return GraphemeIcon::defaultBrush(); }

unique_ptr<Icon> Icon::grapheme(const QString &grapheme, double scalar)
{ return make_unique<GraphemeIcon>(grapheme, scalar); }

unique_ptr<Icon> Icon::grapheme(const QString &grapheme, double scalar, const QBrush &brush)
{ return make_unique<GraphemeIcon>(grapheme, scalar, brush); }

// ---------------------------------------------------------------------------------------------------------------------

const QBrush &Icon::iconifiedDefaultBackgroundBrush()
{ return IconifiedIcon::default_background_brush; }

const QBrush &Icon::iconifiedDefaultBorderBrush()
{ return IconifiedIcon::default_border_brush; }

unique_ptr<Icon> Icon::iconified(unique_ptr<Icon> icon,
                                 const QBrush &background_brush,
                                 double border_radius,
                                 int border_width,
                                 const QBrush &border_brush)
{ return make_unique<IconifiedIcon>(::move(icon), background_brush, border_radius, border_width, border_brush); }

// ---------------------------------------------------------------------------------------------------------------------

unique_ptr<Icon> Icon::composed(unique_ptr<Icon> icon1,
                                unique_ptr<Icon> icon2,
                                double size1,
                                double size2,
                                double x1,
                                double y1,
                                double x2,
                                double y2)
{ return make_unique<ComposedIcon>(::move(icon1), ::move(icon2), size1, size2, x1, y1, x2, y2); }
