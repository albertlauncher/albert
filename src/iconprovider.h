// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QFileIconProvider>
#include <QIcon>
#include <QStringList>
#include <map>


class IconProvider
{
public:

    QIcon getIcon(const QStringList &urls) const;
    QIcon getIcon(const QString &url) const;

private:

    QFileIconProvider file_icon_provider;

};
