// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/iconprovider.h"
#include "xdg/iconlookup.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QMetaEnum>
#include <QStringLiteral>
#include <QStyle>
#include <QTimer>
#include <QUrl>
#include <unordered_map>
using namespace albert;
using namespace std;

class IconProvider::Private
{
public:
    QFileIconProvider file_icon_provider;
    mutable std::unordered_map<QString, QPixmap> pixmap_cache;
    mutable QTimer pixmap_cache_timer;

    QPixmap getPixmapNoCache(const QString &urlstr, QSize *size, const QSize &requestedSize) const
    {
        if (urlstr.startsWith(':')){
            // https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
            if (auto pm = QPixmap(urlstr); !pm.isNull()){
                *size = pm.size();
                return pm;
            }

        } else if (QUrl url(urlstr); url.scheme() == QStringLiteral("qfip")){
            // https://doc.qt.io/qt-6/qfileiconprovider.html
            if (auto pm = file_icon_provider.icon(QFileInfo(url.toString(QUrl::RemoveScheme))).pixmap(requestedSize); !pm.isNull()){
                *size = pm.size();
                return pm;
            }

#if defined Q_OS_LINUX or defined Q_OS_FREEBSD
        } else if (url.scheme() == QStringLiteral("xdg")){
            // https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
            if (auto pm = QPixmap(XDG::IconLookup::iconPath(url.toString(QUrl::RemoveScheme))); !pm.isNull()){
                *size = pm.size();
                return pm;
            }
#endif
        } else if (url.scheme() == QStringLiteral("qsp")){
            // https://doc.qt.io/qt-6/qresource.html
            auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
            auto name = url.toString(QUrl::RemoveScheme);
            for (int i = 0; i < meta_enum.keyCount(); ++i)
                if (name == meta_enum.key(i)){
                    if (auto pm = qApp->style()->standardIcon(static_cast<QStyle::StandardPixmap>(meta_enum.value(i))).pixmap(requestedSize); !pm.isNull()){
                        *size = pm.size();
                        return pm;
                    }
                }
            WARN << "No such StandardPixmap found:" << name;

        } else if (url.isLocalFile()){
            if (auto pm = QPixmap(url.toLocalFile()); !pm.isNull()){
                *size = pm.size();
                return pm;
            }
        }

        return {};
    }
};

IconProvider::IconProvider() : d(new Private)
{
    d->pixmap_cache_timer.setSingleShot(true);
    d->pixmap_cache_timer.setInterval(60000); // 1 minute
    QObject::connect(&d->pixmap_cache_timer, &QTimer::timeout, [this](){
        DEBG << "Clearing icon lookup cache.";
        d->pixmap_cache.clear();
    });
}

IconProvider::~IconProvider() = default;


QPixmap IconProvider::getPixmap(const QStringList &urls, QSize *size, const QSize &requestedSize) const
{
    for (const auto &url : urls)
        if (auto pm = getPixmap(url, size, requestedSize); !pm.isNull())
            return pm;
    return {};
}

QPixmap IconProvider::getPixmap(const QString &urlstr, QSize *size, const QSize &requestedSize) const
{
    d->pixmap_cache_timer.start();
    try {
        return d->pixmap_cache.at(urlstr);
    } catch (const out_of_range &) {
        return d->pixmap_cache.emplace(urlstr, d->getPixmapNoCache(urlstr, size, requestedSize)).first->second;
    }
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

