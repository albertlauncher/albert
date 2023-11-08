// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/iconprovider.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QMetaEnum>
#include <QPainter>
#include <QRegularExpression>
#include <QString>
#include <QStyle>
#include <QUrl>
#include <QUrlQuery>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include "platform/Linux/xdg/iconlookup.h"
#endif
using namespace albert;
using namespace std;


static QPixmap genericPixmap(int size, const QColor& bgcolor, const QColor& fgcolor, const QString& text, float scalar)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (bgcolor.isValid()){
        QColor borderColor = bgcolor;
        auto borderWidth = size/6;
        borderColor.setAlpha(128);
        painter.setBrush(bgcolor);
        painter.setPen(QPen(borderColor, borderWidth));
        painter.drawEllipse(borderWidth / 2, borderWidth / 2, size - borderWidth, size - borderWidth);
    }

    if (!text.isEmpty())
    {
        painter.setPen(fgcolor);

        QFont font = painter.font();
        font.setPixelSize((double)size * scalar);
        painter.setFont(font);

        QRect textRect(0, 0, size, size);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }

    return pixmap;
}


class IconProvider::Private
{
    QFileIconProvider file_icon_provider;
public:
    mutable std::unordered_map<QString, QPixmap> pixmap_cache;
    mutable std::shared_mutex mutex_;

    QPixmap getRawPixmap(const QString &urlstr, const QSize &requestedSize) const
    {
        // https://doc.qt.io/qt-6/qresource.html
        if (QUrl url(urlstr); url.scheme() == QStringLiteral("qrc") || urlstr.startsWith(':'))
            return QPixmap(urlstr);

        // https://doc.qt.io/qt-6/qfileiconprovider.html
        else if (url.scheme() == QStringLiteral("qfip"))
            return file_icon_provider.icon(QFileInfo(url.toString(QUrl::RemoveScheme))).pixmap(requestedSize);

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
        // https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
        else if (url.scheme() == QStringLiteral("xdg"))
            return QPixmap(XDG::IconLookup::iconPath(url.toString(QUrl::RemoveScheme)));
#endif

        // https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
        else if (url.scheme() == QStringLiteral("qsp")){
            auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
            auto name = url.toString(QUrl::RemoveScheme);
            for (int i = 0; i < meta_enum.keyCount(); ++i)
                if (name == meta_enum.key(i))
                    return qApp->style()->standardIcon(static_cast<QStyle::StandardPixmap>(meta_enum.value(i))).pixmap(requestedSize);
            WARN << "No such StandardPixmap found:" << name;
        }

        else if (url.isLocalFile())
            return QPixmap(url.toLocalFile());

        else if (url.scheme() == QStringLiteral("gen")){
            auto urlquery = QUrlQuery(url);

            QColor background(urlquery.queryItemValue(QStringLiteral("background")));

            QColor foreground(urlquery.queryItemValue(QStringLiteral("foreground")));
            if (!foreground.isValid())
                foreground = QApplication::palette().color(QPalette::Text);

            auto text = urlquery.queryItemValue(QStringLiteral("text"));

            bool ok;
            auto scalar = urlquery.queryItemValue(QStringLiteral("fontscalar")).toFloat(&ok);
            if (!ok)
                scalar = 1.0f;

            return genericPixmap(requestedSize.width(), background, foreground, text, scalar);
        }

        return {};
    }
};

IconProvider::IconProvider() : d(new Private) {}
IconProvider::~IconProvider() = default;

QPixmap IconProvider::getPixmap(const QStringList &urls, QSize *size, const QSize &requestedSize) const
{
    for (const auto &url : urls)
        if (auto pm = getPixmap(url, size, requestedSize); !pm.isNull())
            return pm;
    WARN << "No icons found for" << urls;
    return {};
}

QPixmap IconProvider::getPixmap(const QString &urlstr, QSize *size, const QSize &requestedSize) const
{
    auto cache_key = QString("%1%2%3").arg(urlstr).arg(requestedSize.width(), requestedSize.height());
    try {
        std::shared_lock lock(d->mutex_);
        return d->pixmap_cache.at(cache_key);
    } catch (const out_of_range &) {
        auto pm = d->getRawPixmap(urlstr, requestedSize);

        if (!pm.isNull() && pm.size() != requestedSize)
            pm = pm.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        *size = pm.size();

        std::unique_lock lock(d->mutex_);
        d->pixmap_cache.emplace(cache_key, pm);

        return pm;
    }
}

void IconProvider::clearCache()
{
    std::unique_lock lock(d->mutex_);
    d->pixmap_cache.clear();
}




//QIcon IconProvider::getIcon(const QStringList &urls) const
//{
//    for (const auto &url : urls)
//        if (auto icon = getIcon(url); !icon.isNull())
//            return icon;
//    return {};
//}

//QIcon IconProvider::getIcon(const QUrl &url) const
//{
//    if (url.scheme() == XDG_SCHEME)
//        return QIcon(XDG::IconLookup::iconPath(url.toString(QUrl::RemoveScheme)));

//    else if (url.scheme() == QFIP_SCHEME)
//        return file_icon_provider->icon(QFileInfo(url.toString(QUrl::RemoveScheme)));

//    else if (url.scheme() == QSP_SCHEME){
//        auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
//        auto name = url.toString(QUrl::RemoveScheme);
//        for (int i = 0; i < meta_enum.keyCount(); ++i)
//            if (name == meta_enum.key(i))
//                return QApplication::style()->standardIcon(static_cast<QStyle::StandardPixmap>(meta_enum.value(i)));
//        WARN << "No such StandardPixmap found:" << name;
//        return QIcon();

//    } else if (auto qurl = QUrl(url); qurl.isLocalFile()){
//        return QIcon(qurl.toLocalFile());

//    } else
//        return {};
//}

