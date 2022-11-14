// Copyright (c) 2022 Manuel Schneider

#include "iconprovider.h"

static const char * XDG_SCHEME = "xdg:";
static const size_t XDG_SCHEME_LEN = strlen(XDG_SCHEME);
static const char * QFIP_SCHEME = "qfip:";
static const size_t QFIP_SCHEME_LEN = strlen(QFIP_SCHEME);


QIcon IconProvider::getIcon(const QString &url)
{
    QIcon icon;
    if (url.startsWith(XDG_SCHEME)){
        if (icon = QIcon::fromTheme(url.mid((qsizetype)XDG_SCHEME_LEN)); !icon.isNull())
            return icon;
        else return {};
    }
    else if (url.startsWith(QFIP_SCHEME)){
        if (icon = file_icon_provider.icon(QFileInfo(url.mid((qsizetype)QFIP_SCHEME_LEN))); !icon.isNull())
            return icon;
        else return {};
    }
    else return QIcon(url);
}
