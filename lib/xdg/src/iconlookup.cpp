// Copyright (C) 2014-2018 Manuel Schneider

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QStandardPaths>
#include <QSettings>
#include <QString>
#include "themefileparser.h"
#include "iconlookup.h"


namespace  {
    QStringList icon_extensions = {"png", "svg", "xpm"};
}


/** ***************************************************************************/
QString XDG::IconLookup::iconPath(QString iconName, QString themeName){
    return instance()->themeIconPath(iconName, themeName);
}



/** ***************************************************************************/
QString XDG::IconLookup::iconPath(std::initializer_list<QString> iconNames, QString themeName) {
    for ( const QString &iconName : iconNames ) {
        QString result = instance()->themeIconPath(iconName, themeName);
        if ( !result.isEmpty() )
            return result;
    }
    return QString();
}



/** ***************************************************************************/
XDG::IconLookup::IconLookup()
{
    /*
     * Icons and themes are looked for in a set of directories. By default,
     * apps should look in $HOME/.icons (for backwards compatibility), in
     * $XDG_DATA_DIRS/icons and in /usr/share/pixmaps (in that order).
     */

    QString path = QDir::home().filePath(".icons");
    if (QFile::exists(path))
        iconDirs_.append(path);

    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)){
        path = QDir(basedir).filePath("icons");
        if (QFile::exists(path))
            iconDirs_.append(path);
    }

    path = "/usr/share/pixmaps";
    if (QFile::exists(path))
        iconDirs_.append(path);
}



/** ***************************************************************************/
XDG::IconLookup *XDG::IconLookup::instance()
{
    static IconLookup *instance_ = nullptr;
    if (!instance_){
        qInfo() << "Systems icon theme is:" << QIcon::themeName();
        instance_ = new IconLookup();
    }
    return instance_;
}



/** ***************************************************************************/
QString XDG::IconLookup::themeIconPath(QString iconName, QString themeName){

    // if we have an absolute path, just return it
    if ( iconName[0]=='/' ){
        if ( QFile::exists(iconName) )
            return iconName;
        else
            return QString();
    }

    // check if it has an extension and strip it
    for (const QString &ext : icon_extensions)
        if (iconName.endsWith(QString(".").append(ext)))
            iconName.chop(4);

    // Check cache
    QString iconPath = iconCache_.value(iconName);
    if (!iconPath.isNull())
        return iconPath;

    // Lookup themefile
    QStringList checkedThemes;
    iconPath = doRecursiveIconLookup(iconName, themeName, &checkedThemes);
    if (!iconPath.isNull()){
        iconCache_.insert(iconName, iconPath);
        return iconPath;
    }

    // Lookup in hicolor
    if (!checkedThemes.contains("hicolor")){
        iconPath = doRecursiveIconLookup(iconName, "hicolor", &checkedThemes);
        if (!iconPath.isNull()){
            iconCache_.insert(iconName, iconPath);
            return iconPath;
        }
    }

    // Now search unsorted
    for (const QString &iconDir : iconDirs_){
        for (const QString &ext : icon_extensions){
            QString filename = QString("%1/%2.%3").arg(iconDir, iconName, ext);
            if (QFile(filename).exists()){
                iconCache_.insert(iconName, filename);
                return filename;
            }
        }
    }

    // Nothing found, save though to avoid repeated expensive lookups
    return iconCache_.insert(iconName, QString("")).value();
}



/** ***************************************************************************/
QString XDG::IconLookup::doRecursiveIconLookup(const QString &iconName, const QString &themeName, QStringList *checked){

    // Exlude multiple scans
    if (checked->contains(themeName))
        return QString();
    checked->append(themeName);

    // Check if theme exists
    QString themeFile = lookupThemeFile(themeName);
    if (themeFile.isNull())
        return QString();

    // Check if icon exists
    QString iconPath;
    iconPath = doIconLookup(iconName, themeFile);
    if (!iconPath.isNull())
        return iconPath;

    // Check its parents too
    for (const QString &parent : ThemeFileParser(themeFile).inherits()){
        iconPath = doRecursiveIconLookup(iconName, parent, checked);
        if (!iconPath.isNull())
            return iconPath;
    }

    return QString();
}



/** ***************************************************************************/
QString XDG::IconLookup::doIconLookup(const QString &iconName, const QString &themeFile) {

    ThemeFileParser themeFileParser(themeFile);
    QDir themeDir = QFileInfo(themeFile).dir();
    QString themeName = themeDir.dirName();

    // Get the sizes of the dirs
    std::vector<std::pair<QString, int>> dirsAndSizes;
    for (const QString &subdir : themeFileParser.directories())
        dirsAndSizes.push_back(std::make_pair(subdir, themeFileParser.size(subdir)));

    // Sort them by size
    std::sort(dirsAndSizes.begin(), dirsAndSizes.end(),
              [](std::pair<QString, int>  a, std::pair<QString, int> b) {
                  return a.second > b.second;
              });

    // Well now search for a file beginning with the greatest
    QString filename;
    QFile file;
    for (const auto &dirAndSize : dirsAndSizes){
        for (const QString &iconDir : iconDirs_){
            for (const QString &ext : icon_extensions){
                filename = QString("%1/%2/%3/%4.%5").arg(iconDir, themeName, dirAndSize.first, iconName, ext);
                if (file.exists(filename)){
                    return filename;
                }
            }
        }
    }

    return QString();
}


/** ***************************************************************************/
QString XDG::IconLookup::lookupThemeFile(const QString &themeName)
{
    // Lookup themefile
    for (const QString &iconDir : iconDirs_){
        QString indexFile = QString("%1/%2/index.theme").arg(iconDir, themeName);
        if (QFile(indexFile).exists())
            return indexFile;
    }
    return QString();
}
