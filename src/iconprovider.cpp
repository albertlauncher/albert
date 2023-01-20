// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "iconprovider.h"
#include "xdg/iconlookup.h"
#include <QApplication>
#include <QIcon>
#include <QMetaEnum>
#include <QStyle>
#include <QStyle>

static const char * XDG_SCHEME = "xdg:";  // https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
static const size_t XDG_SCHEME_LEN = strlen(XDG_SCHEME);
static const char * QFIP_SCHEME = "qfip:";  // https://doc.qt.io/qt-6/qfileiconprovider.html
static const size_t QFIP_SCHEME_LEN = strlen(QFIP_SCHEME);
static const char * QSP_SCHEME = "qsp:";  // https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
static const size_t QSP_SCHEME_LEN = strlen(QSP_SCHEME);

QIcon IconProvider::getIcon(const QStringList &urls) const
{
    for (const auto &url : urls)
        if (auto icon = getIcon(url); !icon.isNull())
            return icon;
    return {};
}

QIcon IconProvider::getIcon(const QString &url) const
{
    if (url.startsWith(XDG_SCHEME))
        return QIcon(XDG::IconLookup::iconPath(url.mid((qsizetype)XDG_SCHEME_LEN)));

    if (url.startsWith(QFIP_SCHEME))
        return file_icon_provider.icon(QFileInfo(url.mid((qsizetype)QFIP_SCHEME_LEN)));

    if (url.startsWith(QSP_SCHEME)){
        auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
        auto name = url.mid((qsizetype)QSP_SCHEME_LEN);
        for (int i = 0; i < meta_enum.keyCount(); ++i)
            if (name == meta_enum.key(i))
                return QApplication::style()->standardIcon(static_cast<QStyle::StandardPixmap>(meta_enum.value(i)));
        WARN << "No such StandardPixmap found:" << name;
        return QIcon();
    }
    else
        return QIcon(url);
}
