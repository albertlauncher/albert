// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QPixmap>
#include <QSize>
#include <QStringList>

namespace albert{

/// Generic icon provider.
///
/// URLs supported by the icon provider to create the icons displayed.
/// 'file:<path>' is interpreted as path to a local image file.
/// 'qfip:<path>' uses QFileIconProvider to get the icon for thefile.
/// 'xdg:<icon-name>' performs freedesktop icon theme specification lookup (linux only).
/// ':<path>' is a QResource path.
///
/// @note Icons are cached for a minute
class ALBERT_EXPORT IconProvider
{
public:
    IconProvider();
    ~IconProvider();

    QPixmap getPixmap(const QStringList &urls, QSize *size, const QSize &requestedSize) const;
    QPixmap getPixmap(const QString &url, QSize *size, const QSize &requestedSize) const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

}

