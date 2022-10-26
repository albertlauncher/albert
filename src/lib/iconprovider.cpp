// Copyright (c) 2022 Manuel Schneider

#include "iconprovider.h"


const char * XDG_SCHEME = "xdg:";
const size_t XDG_SCHEME_LEN = strlen(XDG_SCHEME);

const char * QFIP_SCHEME = "qfip:";
const size_t QFIP_SCHEME_LEN = strlen(QFIP_SCHEME);


QIcon IconProvider::getIcon(const QString &url)
{
    QIcon icon;
    if (url.startsWith(XDG_SCHEME)){
        if (icon = QIcon::fromTheme(url.mid(XDG_SCHEME_LEN)); !icon.isNull())
            return icon;
        else return QIcon();
    }
    else if (url.startsWith(QFIP_SCHEME)){
        if (icon = file_icon_provider.icon(QFileInfo(url.mid(QFIP_SCHEME_LEN))); !icon.isNull())
            return icon;
        else return QIcon();
    }
    else return QIcon(url);
}
