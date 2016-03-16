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


QString Applications::Application::terminal;

/** ***************************************************************************/
QString Applications::Application::text() const {
    return _name;
}



/** ***************************************************************************/
QString Applications::Application::subtext() const {
    return (_altName.isNull()) ? _exec : _altName;
}



/** ***************************************************************************/
QIcon Applications::Application::icon() const {
    return _icon;
}



/** ***************************************************************************/
void Applications::Application::activate() {
    qApp->hideWidget();

    // QTBUG-51678

    // General escape rule has been applied on parse time
    // Apply exec escape rule
    QStringList arguments = execValueEscape( (_term) ? terminal.arg(quoteString(_exec)) : _exec);
    QString program = arguments.takeFirst();

    // TODO: Apply code fiels expansion (currently done at parese time)

    // Run the application
    QProcess::startDetached(program, arguments);
    ++_usage;
}



/** ***************************************************************************/
bool Applications::Application::hasChildren() const {
    return true;
}




/** ***************************************************************************/
vector<shared_ptr<AlbertItem> > Applications::Application::children() {
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
        QString key;
        QString value;
        QString currentGroup;
        for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine()) {
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


    if (values["Desktop Entry"]["NoDisplay"] == "true")
        return false;


    // Try to get the (localized name)
    QString locale = QLocale().name();
    QString shortLocale = locale.left(2);
    if (values["Desktop Entry"].count(QString("Name[%1]").arg(locale)))
        _name = escapeString(values["Desktop Entry"][QString("Name[%1]").arg(locale)]);
    else if (values["Desktop Entry"].count(QString("Name[%1]").arg(shortLocale)))
        _name = escapeString(values["Desktop Entry"][QString("Name[%1]").arg(shortLocale)]);
    else if (values["Desktop Entry"].count("Name"))
        _name = escapeString(values["Desktop Entry"]["Name"]);
    else return false;



    /*
     * The Exec Key - pretty complicated stuff
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
     */
    if (values["Desktop Entry"].count("Exec"))
        _exec = escapeString(values["Desktop Entry"]["Exec"]);
    else return false;

    _exec.replace("%f", ""); // Unhandled TODO
    _exec.replace("%F", ""); // Unhandled TODO
    _exec.replace("%u", ""); // Unhandled TODO
    _exec.replace("%U", ""); // Unhandled TODO
    _exec.replace("%d", ""); // Deprecated
    _exec.replace("%D", ""); // Deprecated
    _exec.replace("%n", ""); // Deprecated
    _exec.replace("%N", ""); // Deprecated
    if (values["Desktop Entry"].count("Icon"))
        _exec.replace("%i", QString("--icon %1").arg(values["Desktop Entry"]["Icon"]));
    else _exec.replace("%i", "");
    _exec.replace("%c", _name);
    _exec.replace("%k", _path);
    _exec.replace("%v", ""); // Deprecated
    _exec.replace("%m", ""); // Deprecated
    _exec.replace("%%", "%");

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

    // No additional actions for terminal apps
    _term = values["Desktop Entry"]["Terminal"] == "true";
    if(_term)
        return true;

    // Root actions
    QStringList graphicalSudos({"gksu", "kdesu"});
    for (const QString &s : graphicalSudos){
        QProcess p;
        p.start("which", {s});
        p.waitForFinished(-1);
        if (p.exitCode() == 0)
            _actions.push_back(std::make_shared<DesktopAction>(this,
                                                               QString("Run %1 as root").arg(_name),
                                                               QString("%1 \"%2\"").arg(s, _exec),
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
QString Applications::Application::escapeString(const QString &unescaped) {
    QString result;
    result.reserve(unescaped.size());

    /*
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
     *
     * The escape sequences \s, \n, \t, \r, and \\ are supported for values of
     * type string and localestring, meaning ASCII space, newline, tab, carriage
     * return, and backslash, respectively.
     * Some keys can have multiple values. In such a case, the value of the key
     * is specified as a plural: for example, string(s). The multiple values
     * should be separated by a semicolon and the value of the key may be
     * optionally terminated by a semicolon. Trailing empty strings must always
     * be terminated with a semicolon. Semicolons in these values need to be
     * escaped using \;.
     */
    QString::const_iterator it = unescaped.begin();
    while (it != unescaped.end()) {
        if (*it == '\\'){
            ++it;
            if (it == unescaped.end())
                break;
            else if (*it=='s')
                result.append(' ');
            else if (*it=='n')
                result.append('\n');
            else if (*it=='t')
                result.append('\t');
            else if (*it=='r')
                result.append('\r');
            else if (*it=='\\')
                result.append('\\');
        }
        else
            result.append(*it);
        ++it;
    }
    return result;
}



/** ***************************************************************************/
QString Applications::Application::quoteString(const QString &unquoted) {
    QString result;
    result.push_back("\"");
    for (auto & qchar : unquoted){
        switch (qchar.toLatin1()) {
        case '"': case '`': case '\\': case '$':
            result.push_back('\\');
            break;
        }
        result.push_back(qchar);
    }
    result.push_back("\"");
    return result;
}



/** ***************************************************************************/
QStringList Applications::Application::execValueEscape(const QString &execValue) {

    QString part;
    QStringList result;
    QString::const_iterator it = execValue.begin();

    while(it != execValue.end()){

        // Check for a backslash (escape)
        if (*it == '\\'){
            if (++it == execValue.end()){
                qWarning() << "EOL detected. Excpected one of {\",`,\\,$, ,\\n,\\t,',<,>,~,|,&,;,*,?,#,(,)}";
                return QStringList();
            }

            switch (it->toLatin1()) {
            case 'n': part.push_back('\n');
                break;
            case 't': part.push_back('\t');
                break;
            case ' ':
            case '\'':
            case '<':
            case '>':
            case '~':
            case '|':
            case '&':
            case ';':
            case '*':
            case '?':
            case '#':
            case '(':
            case ')':
            case '"':
            case '`':
            case '\\':
            case '$': part.push_back(*it);
                break;
            default:
                qWarning() << "Invalid char following \\. Excpected one of {\",`,\\,$, ,\\n,\\t,',<,>,~,|,&,;,*,?,#,(,)}";
                return QStringList();
            }
        }

        // Check for quoted strings
        else if (*it == '"'){
            while (true){
                if (++it == execValue.end()){
                    qWarning() << "Detected EOL inside a qoute.";
                    return QStringList();
                }

                // Leave the "quotation loop" on double qoute
                else if (*it == '"')
                    break;

                // Check for a backslash (escape)
                else if (*it == '\\'){
                    if (++it == execValue.end()){
                        qWarning() << "EOL detected. Excpected one of {\",`,\\,$}";
                        return QStringList();
                    }

                    switch (it->toLatin1()) {
                    case '"':
                    case '`':
                    case '\\':
                    case '$': part.push_back(*it);
                        break;
                    default:
                        qWarning() << "Invalid char following \\. Excpected one of {\",`,\\,$}";
                        return QStringList();
                    }
                }

                // Accept everything else
                else {
                    part.push_back(*it);
                }
            }
        }

        // Check for spaces (separators)
        else if (*it == ' '){
            result.push_back(part);
            part.clear();
        }

        // Rest of input alphabet, save and continue
        else {
            part.push_back(*it);
        }

        ++it;
    }

    if (!part.isEmpty())
        result.push_back(part);

    return result;
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
