// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QFileIconProvider>
#include <QIcon>
#include <QStringList>
#include <map>


class IconProvider
{
public:

    QIcon getIcon(const QString &url);

private:

    QFileIconProvider file_icon_provider;

};
