// Copyright (C) 2014-2024 Manuel Schneider

#pragma once
#include <QSize>
#include <QStringList>
#include <map>

namespace XDG {

class IconLookup
{
public:

    /**
     * @brief iconPath Does XDG icon lookup for the given icon name
     * @param iconName The icon name to lookup
     * @param themeName The theme to use, use current theme if empty
     * @return If an icon was found the path to the icon, else an empty string
     */
    static QString iconPath(QString iconName, QSize size = QSize(), QString themeName = QString());

private:

    IconLookup();
    static IconLookup *instance();

    QString themeIconPath(QString iconName, QString themeName = QString());
    QString doRecursiveIconLookup(const QString &iconName, const QString &theme, QStringList *checked);
    QString doIconLookup(const QString &iconName, const QString &themeFile);
    QString lookupThemeFile(const QString &themeName);

    QStringList iconDirs_;
    std::map<QString, QString> iconCache_;
};

}
