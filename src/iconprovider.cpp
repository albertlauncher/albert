// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/iconprovider.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QIconEngine>
#include <QMetaEnum>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QUrlQuery>
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include "platform/Linux/xdg/iconlookup.h"
#endif
using namespace albert;
using namespace std;

static const QString explicit_qrc_scheme = "qrc:";
static const QString file_scheme = "file:";
static const QString generative_scheme = "gen:?";
static const QString implicit_qrc_scheme = ":";
static const QString qfileiconprovider_scheme = "qfip:";
static const QString qstandardpixmap_scheme = "qsp:";
static const QString xdg_icon_lookup_scheme = "xdg:";

static QPixmap pixmapFromFilePath(const QString &path, const QSize &requestedSize)
{
    // https://doc.qt.io/qt-6/qresource.html
    auto pm = QPixmap(path);
    if (!pm.isNull()
        && (pm.width() > requestedSize.width()
            || pm.height() > requestedSize.height()))
        pm = pm.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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

static QPixmap standardPixmapFromName(const QString &enumerator_name)
{
    // https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
    auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
    for (int i = 0; i < meta_enum.keyCount(); ++i)
        if (enumerator_name == meta_enum.key(i))
            return qApp->style()->standardPixmap(static_cast<QStyle::StandardPixmap>(meta_enum.value(i)));
    WARN << "No such StandardPixmap found:" << enumerator_name;
    return {};
}

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
QString albert::xdgIconLookup(const QString &name)
{
    // https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    return XDG::IconLookup::iconPath(name);
}
#endif

QIcon albert::fileIcon(const QString &path)
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
        font.setPixelSize(rect.height() * scalar);
        p->setFont(font);

        p->drawText(rect, Qt::AlignCenter, text);
    }
}

QPixmap albert::genericPixmap(int size, const QColor &bgcolor, const QColor &fgcolor, const QString &text, float scalar)
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

QPixmap albert::pixmapFromUrl(const QString &url, const QSize &requestedSize)
{
    if (url.startsWith(implicit_qrc_scheme))
        return pixmapFromFilePath(url, requestedSize);  // intended, colon has to remain

    else if (url.startsWith(explicit_qrc_scheme))
        return pixmapFromFilePath(url.mid(explicit_qrc_scheme.size()-1), requestedSize);  // intended, colon has to remain

    else if (url.startsWith(qfileiconprovider_scheme))
        return fileIcon(url.mid(qfileiconprovider_scheme.size())).pixmap(requestedSize, 1.);

    else if (url.startsWith(xdg_icon_lookup_scheme))
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
        return pixmapFromFilePath(xdgIconLookup(url.mid(xdg_icon_lookup_scheme.size())), requestedSize);
#else
        return {};
#endif

    else if (url.startsWith(qstandardpixmap_scheme))
    {
        auto pm = standardPixmapFromName(url.mid(qstandardpixmap_scheme.size()));
        if (!pm.isNull()
            && (pm.width() > requestedSize.width()
                || pm.height() > requestedSize.height()))
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

    // Implicitly check for file existence
    return pixmapFromFilePath(url, requestedSize);
}

QPixmap albert::pixmapFromUrls(const QStringList &urls, const QSize &requestedSize)
{
    for (const auto &url : urls)
        if (auto pm = pixmapFromUrl(url, requestedSize); !pm.isNull())
            return pm;
    return {};
}

QIcon albert::iconFromUrl(const QString &url)
{
    if (url.startsWith(implicit_qrc_scheme))
        return QIcon(url); // intended, colon has to remain

    else if (url.startsWith(explicit_qrc_scheme))
        return QIcon(url.mid(explicit_qrc_scheme.size()-1));  // intended, colon has to remain

    else if (url.startsWith(qfileiconprovider_scheme))
        return fileIcon(url.mid(qfileiconprovider_scheme.size()));

    else if (url.startsWith(xdg_icon_lookup_scheme))
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
        return QIcon(xdgIconLookup(url.mid(xdg_icon_lookup_scheme.size())));
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

QIcon albert::iconFromUrls(const QStringList &urls)
{
    for (const auto &url : urls)
        if (auto icon = iconFromUrl(url); !icon.isNull())
            return icon;
    return {};
}

QIcon genericIcon(const QColor &bgcolor, const QColor &fgcolor, const QString &text, float scalar)
{
    return QIcon(new GenericIconEngine(bgcolor, fgcolor, text, scalar));
}
