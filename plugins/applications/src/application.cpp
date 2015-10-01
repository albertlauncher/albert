// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDirIterator>
#include <memory>
#include <map>
using std::map;
#include "application.h"
#include "desktopaction.h"
#include "albertapp.h"


/** ***************************************************************************/
QString Applications::Application::name() const {
    return _name;
}



/** ***************************************************************************/
QString Applications::Application::info() const {
    return _altName;
}



/** ***************************************************************************/
QIcon Applications::Application::icon() const {
    return _icon;
}



/** ***************************************************************************/
void Applications::Application::activate() {
    // Standard action
    qApp->hideWidget();
    CommandAction(_exec).activate();
    ++_usage;
}



/** ***************************************************************************/
bool Applications::Application::hasChildren() const {
    return true;
}




/** ***************************************************************************/
vector<shared_ptr<A2Item> > Applications::Application::children() {
    return _actions;
}



/** ***************************************************************************/
std::vector<QString> Applications::Application::aliases() const {
    return std::vector<QString>({_name, _altName, _exec.section(" ",0,0)});
}



/** ***************************************************************************/
bool Applications::Application::readDesktopEntry() {
    // TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
    map<QString,map<QString,QString>> values;

    // Read the file
    QFile desktopFile(_path);
    if (desktopFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
        QTextStream stream(&desktopFile);
        QString line;
        QString key;
        QString value;
        QString currentGroup;
        while (stream.readLineInto(&line)) {
            line = line.trimmed();

            if (line.startsWith('#') || line.isEmpty())
                continue;

            // Check for groups
            if (line.startsWith("[")){
                currentGroup = line.mid(1,line.size()-2);
                continue;
            }

            key = line.section('=', 0,0).trimmed();
            value = line.section('=', 1, -1).trimmed();
            values[currentGroup][key]=value;
        }
        desktopFile.close();
    } else return false;


    if (values["Desktop Entry"]["NoDisplay"] == "true"
            || values["Desktop Entry"]["Term"] == "true")
        return false;


    // Try to get the (localized name)
    QString locale = QLocale().name();
    QString shortLocale = locale.left(2);
    if (values["Desktop Entry"].count(QString("Name[%1]").arg(locale)))
        _name = values["Desktop Entry"][QString("Name[%1]").arg(locale)];
    else if (values["Desktop Entry"].count(QString("Name[%1]").arg(shortLocale)))
        _name = values["Desktop Entry"][QString("Name[%1]").arg(shortLocale)];
    else if (values["Desktop Entry"].count("Name"))
        _name = values["Desktop Entry"]["Name"];
    else return false;


    // Try to get the command
    if (values["Desktop Entry"].count("Exec"))
        _exec = values["Desktop Entry"]["Exec"];
    else return false;
    _exec.replace("%c", _name);
    _exec.remove(QRegExp("%.")); // Todo standard conform http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html


    // Try to get the icon
    if (values["Desktop Entry"].count("Icon"))
        _icon = getIcon(values["Desktop Entry"]["Icon"]);
    else {
        qWarning() << "No icon specified in " << _path;
        _icon = QIcon::fromTheme("exec");
    }


    // Try to get any [localized] secondary information comment
    if (values["Desktop Entry"].count(QString("Comment[%1]").arg(locale)))
        _altName = values["Desktop Entry"][QString("Comment[%1]").arg(locale)];
    else if (values["Desktop Entry"].count(QString("Comment[%1]").arg(shortLocale)))
        _altName = values["Desktop Entry"][QString("Comment[%1]").arg(shortLocale)];
    else if (values["Desktop Entry"].count("Comment"))
        _altName = values["Desktop Entry"]["Comment"];
    else if (values["Desktop Entry"].count(QString("GenericName[%1]").arg(locale)))
        _altName = values["Desktop Entry"][QString("GenericName[%1]").arg(locale)];
    else if (values["Desktop Entry"].count(QString("GenericName[%1]").arg(shortLocale)))
        _altName = values["Desktop Entry"][QString("GenericName[%1]").arg(shortLocale)];
    else if (values["Desktop Entry"].count("GenericName"))
        _altName = values["Desktop Entry"]["GenericName"];
    else
        _altName = _exec;


    // Default action
    _actions.push_back(std::make_shared<DesktopAction>(this, QString("Run %1").arg(_name), _exec, _icon));

    // Root actions
    QStringList graphicalSudos({"gksu", "kdesu"});
    for (const QString &s : graphicalSudos){
        QProcess p;
        p.start("which", {s});
        p.waitForFinished(-1);
        if (p.exitCode() == 0)
            _actions.push_back(std::make_shared<DesktopAction>(this,
                                                               QString("Run %1 as root").arg(_name),
                                                               QString("%1 %2").arg(s, _exec),
                                                               _icon));
    }


    // Desktop entry actions
    if (values["Desktop Entry"].count("Actions")){
        QString actionsString = values["Desktop Entry"]["Actions"];
        QStringList actionStrings = actionsString.split(';',QString::SkipEmptyParts);
        QString name;
        QString exec;
        QString group;
        for (const QString &actionString: actionStrings){
            // Get the name
            group = QString("Desktop Action %1").arg(actionString);
            if (!values[group].count("Name")) continue;
            name = values[group]["Name"];

            // Get the command
            group = QString("Desktop Action %1").arg(actionString);
            if (!values[group].count("Exec")) continue;
            exec = values[group]["Exec"];

            // Try to get an icon
            group = QString("Desktop Action %1").arg(actionString);
            if (values[group].count("Icon"))
                _actions.push_back(std::make_shared<DesktopAction>(this, name, exec, getIcon(values[group]["Icon"])));
            else
                _actions.push_back(std::make_shared<DesktopAction>(this, name, exec, _icon));

        }
    }
    return true;
}



/** ***************************************************************************/
QIcon Applications::Application::getIcon(const QString &iconName) {

    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    // http://standards.freedesktop.org/desktop-entry-spec/latest/

    /*
     * Icon to display in file manager, menus, etc. If the name is an absolute
     * path, the given file will be used. If the name is not an absolute path,
     * the algorithm described in the Icon Theme Specification will be used to
     * locate the icon. oh funny qt-bug h8 u
     */

    if (iconName.startsWith('/'))
        // If it is a full path
        return QIcon(iconName);
    else if (QIcon::hasThemeIcon(iconName))
        // If it is in the theme
        return QIcon::fromTheme(iconName);
    else {
        QIcon result;
        // Try hicolor
        QString currentTheme = QIcon::themeName(); // missing fallback (qt-bug)
        QIcon::setThemeName("hicolor");
        if (QIcon::hasThemeIcon(iconName)) {
            result = QIcon::fromTheme(iconName);
            QIcon::setThemeName(currentTheme);
        } else {
            QIcon::setThemeName(currentTheme);
            // if it is in the pixmaps
            QDirIterator it("/usr/share/pixmaps", QDir::Files, QDirIterator::Subdirectories);
            bool found = false;
            while (it.hasNext()) {
                it.next();
                QFileInfo fi = it.fileInfo();
                if (fi.isFile() && (fi.fileName() == iconName || fi.baseName() == iconName)) {
                    result = QIcon(fi.canonicalFilePath());
                    found = true;
                    break;
                }
            }
            if (!found) {
                // If it is still not found use a generic one
                qWarning() << "Unknown icon:" << iconName;
                result = QIcon::fromTheme("exec");
            }
        }
        return result;
    }
}
