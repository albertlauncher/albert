// Copyright (c) 2022-2024 Manuel Schneider

#include "iconprovider.h"
#include "logging.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QIconEngine>
#include <QMetaEnum>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QUrlQuery>
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include "iconlookup.h"
#endif
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace albert;
using namespace std;

static const QString &explicit_qrc_scheme      = u"qrc:"_s;
static const QString &file_scheme              = u"file:"_s;
static const QString &generative_scheme        = u"gen:?"_s;
static const QString &implicit_qrc_scheme      = u":"_s;
static const QString &qfileiconprovider_scheme = u"qfip:"_s;
static const QString &qstandardpixmap_scheme   = u"qsp:"_s;
static const QString &xdg_icon_lookup_scheme   = u"xdg:"_s;
static const QString &compose_lookup_scheme    = u"comp:?"_s;
static const QString &mask_lookup_scheme       = u"mask:?"_s;

/// Returns a pixmap from a file path.
/// The size of the pixmap may be smaller but never larger than the requested size.
static QPixmap pixmapFromFilePath(const QString &path, const QSize &requestedSize)
{
    if (auto pm = QPixmap(path);
        pm.width() > requestedSize.width() || pm.height() > requestedSize.height())
        return pm.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    else
        return pm;
}

static QIcon standardIconFromName(const QString &enumerator_name)
{
    // https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
    auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
    for (int i = 0; i < meta_enum.keyCount(); ++i)
        if (enumerator_name == meta_enum.key(i))
            return qApp->style()->standardIcon(static_cast<QStyle::StandardPixmap>(meta_enum.value(i)));
    WARN << "No such StandardPixmap found:" << enumerator_name;
    return {};
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
QString detail::xdgIconLookup(const QString &name)
{
    // https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    return XDG::IconLookup::iconPath(name);
}
#endif

QIcon detail::fileIcon(const QString &path)
{
    // https://doc.qt.io/qt-6/qfileiconprovider.html
    static QFileIconProvider qfip;
    return qfip.icon(QFileInfo(path));
}

static void drawGenericIcon(QPainter *p, const QRect &rect, const QColor &bgcolor, const QColor &fgcolor, const QString &text, float scalar)
{
    p->setRenderHint(QPainter::Antialiasing);

    if (bgcolor.isValid())
    {
        p->setBrush(bgcolor);
        p->setPen(Qt::NoPen);
        p->drawEllipse(rect);
    }

    if (!text.isEmpty())
    {
        p->setPen(fgcolor);

        QFont font = p->font();
        font.setPixelSize(static_cast<int>(round(rect.height() * scalar)));
        p->setFont(font);

        p->drawText(rect, Qt::AlignCenter, text);
    }
}

QPixmap detail::genericPixmap(int size, const QColor &bgcolor, const QColor &fgcolor, const QString &text, float scalar)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    drawGenericIcon(&p, pm.rect(), bgcolor, fgcolor, text, scalar);
    return pm;
}

struct GenericIconEngine : public QIconEngine
{
    QColor bgcolor_;
    QColor fgcolor_;
    QString text_;
    float scalar_;

    GenericIconEngine(const QUrlQuery &urlquery):
        bgcolor_(urlquery.queryItemValue(QStringLiteral("background"))),
        fgcolor_(urlquery.queryItemValue(QStringLiteral("foreground"))),
        text_(urlquery.queryItemValue(QStringLiteral("text")))
    {
        if (!bgcolor_.isValid())
            bgcolor_ = Qt::transparent;

        if (!fgcolor_.isValid())
            fgcolor_ = Qt::black;

        bool ok;
        scalar_ = urlquery.queryItemValue(QStringLiteral("fontscalar")).toFloat(&ok);
        if (!ok)
            scalar_ = 1.;
    }

    GenericIconEngine(const QColor& bgcolor, const QColor& fgcolor, const QString& text, float scalar):
        bgcolor_(bgcolor), fgcolor_(fgcolor), text_(text), scalar_(scalar) {}

    virtual void paint(QPainter *p, const QRect &rect, QIcon::Mode = {}, QIcon::State = {}) override
    { drawGenericIcon(p, rect, bgcolor_, fgcolor_, text_, scalar_); }

    virtual QPixmap pixmap(const QSize &size, QIcon::Mode = {}, QIcon::State = {}) override
    { return genericPixmap(size.width(), bgcolor_, fgcolor_, text_, scalar_); }

    virtual QIconEngine *clone() const override
    { return new GenericIconEngine(*this); }

};

QPixmap detail::pixmapFromUrl(const QString &url, const QSize &requestedSize)
{
    if (url.startsWith(implicit_qrc_scheme))
        return pixmapFromFilePath(url, requestedSize);  // intended, colon has to remain

    else if (url.startsWith(explicit_qrc_scheme))
        return pixmapFromFilePath(url.mid(explicit_qrc_scheme.size()-1), requestedSize);  // intended, colon has to remain

    else if (url.startsWith(qfileiconprovider_scheme))
        return fileIcon(url.mid(qfileiconprovider_scheme.size())).pixmap(requestedSize, 1.);

    else if (url.startsWith(xdg_icon_lookup_scheme))
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        // return pixmapFromFilePath(xdgIconLookup(url.mid(xdg_icon_lookup_scheme.size())), requestedSize);
        return QIcon::fromTheme(url.mid(xdg_icon_lookup_scheme.size())).pixmap(requestedSize, 1.);
#else
        return {};
#endif

    else if (url.startsWith(qstandardpixmap_scheme))
    {
        auto icon = standardIconFromName(url.mid(qstandardpixmap_scheme.size()));
        if (icon.isNull())
            return {};

        auto pm = icon.pixmap(requestedSize, 1.);
        if (!pm.isNull() && (pm.width() > requestedSize.width() || pm.height() > requestedSize.height()))
            pm = pm.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        return pm;
    }

    else if (url.startsWith(file_scheme))
        return pixmapFromFilePath(url.mid(file_scheme.size()), requestedSize);

    else if (url.startsWith(generative_scheme))
    {
        QUrlQuery urlquery(url.mid(generative_scheme.size()));
        QColor bgcolor{urlquery.queryItemValue(QStringLiteral("background"))};
        if (!bgcolor.isValid())
            bgcolor = Qt::transparent;

        QColor fgcolor{urlquery.queryItemValue(QStringLiteral("foreground"))};
        if (!fgcolor.isValid())
            fgcolor = Qt::black;

        QString text{urlquery.queryItemValue(QStringLiteral("text"))};

        bool ok;
        float scalar{urlquery.queryItemValue(QStringLiteral("fontscalar")).toFloat(&ok)};
        if (!ok)
            scalar = 1.;

        return genericPixmap(requestedSize.height(), bgcolor, fgcolor, text, scalar);
    }

    else if (url.startsWith(mask_lookup_scheme))
    {
        QUrlQuery urlquery(url.mid(mask_lookup_scheme.size()));
        const auto radius_divisor = urlquery.queryItemValue(QStringLiteral("radius")).toInt();
        const auto src_url = urlquery.queryItemValue(QStringLiteral("src")).toLocal8Bit();
        const auto src_pm = pixmapFromUrl(QUrl::fromPercentEncoding(src_url), requestedSize);
        const auto src_img = src_pm.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

        QPixmap pm(requestedSize);
        pm.fill(Qt::transparent);

        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);

        const auto radius = requestedSize.width() / radius_divisor;

        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(pm.rect(), radius, radius);

        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.drawImage(pm.rect(), src_img);

        return pm;
    }

    else if (url.startsWith(compose_lookup_scheme))
    {
        QUrlQuery urlquery(url.mid(compose_lookup_scheme.size()));

        double size1 = 0.7, size2 = 0.7;

        if (const auto sizestr1 = urlquery.queryItemValue(u"size1"_s);
            !sizestr1.isEmpty())
            size1 = sizestr1.toDouble();

        if (const auto sizestr2 = urlquery.queryItemValue(u"size2"_s);
            !sizestr2.isEmpty())
            size2 = sizestr2.toDouble();

        const auto src1 = pixmapFromUrl(QUrl::fromPercentEncoding(
            urlquery.queryItemValue(u"src1"_s).toLocal8Bit()), requestedSize * size1);
        if (src1.isNull())
            return {};

        const auto src2 = pixmapFromUrl(QUrl::fromPercentEncoding(
            urlquery.queryItemValue(u"src2"_s).toLocal8Bit()), requestedSize * size2);
        if (src2.isNull())
            return {};

        struct Helper
        {
            static optional<QPoint> parsePosition(const QString &pos, const QPixmap &src, const QPixmap &dst)
            {
                if (pos == u"tl"_s)
                    return QPoint{0, 0};
                else if (pos == u"tc"_s)
                    return QPoint{dst.height()/2 - src.height()/2, 0};
                else if (pos == u"tr"_s)
                        return QPoint{dst.height() - src.height(), 0};
                else if (pos == u"cl"_s)
                    return QPoint{0, dst.width()/2 - src.width()/2};
                else if (pos == u"cc"_s)
                    return QPoint{dst.height()/2 - src.height()/2, dst.width()/2 - src.width()/2};
                else if (pos == u"cr"_s)
                        return QPoint{dst.height() - src.height(), dst.width()/2 - src.width()/2};
                else if (pos == u"bl"_s)
                    return QPoint{0, dst.width() - src.width()};
                else if (pos == u"bc"_s)
                    return QPoint{dst.height()/2 - src.height()/2, dst.width() - src.width()};
                else if (pos == u"br"_s)
                    return QPoint{dst.height() - src.height(), dst.width() - src.width()};
                else
                    return {};
            }
        };

        QPixmap pm(requestedSize);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);

        if (const auto pos = Helper::parsePosition(urlquery.queryItemValue(u"pos1"_s), src1, pm);
            pos)
            p.drawPixmap(*pos, src1);
        else
            p.drawPixmap(0, 0, src1); // top left

        if (const auto pos = Helper::parsePosition(urlquery.queryItemValue(u"pos2"_s), src2, pm);
            pos)
            p.drawPixmap(*pos, src2);
        else
            p.drawPixmap(pm.height() - src2.height(), pm.width() - src2.width(), src2);  // bottom right

        return pm;
    }

    // Implicitly check for file existence
    return pixmapFromFilePath(url, requestedSize);
}

QPixmap detail::pixmapFromUrls(const QStringList &urls, const QSize &requestedSize)
{
    for (const auto &url : urls)
        if (auto pm = pixmapFromUrl(url, requestedSize); !pm.isNull())
            return pm;
    return {};
}

QIcon detail::iconFromUrl(const QString &url)
{
    if (url.startsWith(implicit_qrc_scheme))
        return QIcon(url); // intended, colon has to remain

    else if (url.startsWith(explicit_qrc_scheme))
        return QIcon(url.mid(explicit_qrc_scheme.size()-1));  // intended, colon has to remain

    else if (url.startsWith(qfileiconprovider_scheme))
        return fileIcon(url.mid(qfileiconprovider_scheme.size()));

    else if (url.startsWith(xdg_icon_lookup_scheme))
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        // return QIcon(xdgIconLookup(url.mid(xdg_icon_lookup_scheme.size())));
        return QIcon::fromTheme(url.mid(xdg_icon_lookup_scheme.size()));
#else
        return {};
#endif

    else if (url.startsWith(qstandardpixmap_scheme))
        return standardIconFromName(url.mid(qstandardpixmap_scheme.size()));

    else if (url.startsWith(file_scheme))
        return QIcon(url.mid(file_scheme.size()));

    else if (url.startsWith(generative_scheme))
        return QIcon(new GenericIconEngine(QUrlQuery(url.mid(generative_scheme.size()))));

    // QIcon produces non null icons from non existing files, check before
    else if (QFile::exists(url))
        return QIcon(url);

    else return {};
}

QIcon detail::iconFromUrls(const QStringList &urls)
{
    for (const auto &url : urls)
        if (auto icon = iconFromUrl(url); !icon.isNull())
            return icon;
    return {};
}

QIcon detail::genericIcon(const QColor &bgcolor, const QColor &fgcolor, const QString &text, float scalar)
{
    return QIcon(new GenericIconEngine(bgcolor, fgcolor, text, scalar));
}
