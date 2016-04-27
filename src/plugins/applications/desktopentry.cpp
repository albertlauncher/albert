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
#include <QIcon>
#include <QTextStream>
#include <QDataStream>
#include <QProcess>
#include <QDirIterator>
#include <memory>
#include <map>
#include "desktopentry.h"
#include "desktopaction.hpp"
#include "albertapp.h"
#include "xdgiconlookup.h"
using std::map;

QString Applications::DesktopEntry::terminal;
QStringList Applications::DesktopEntry::supportedGraphicalSudo = {"gksu", "kdesu"};

/** ***************************************************************************/
Applications::DesktopEntry::DesktopEntry() : usage_(0), term_(false) { }



/** ***************************************************************************/
Applications::DesktopEntry::DesktopEntry(const QString &path, short usage)
    : path_(path), usage_(usage), term_(false) { }



/** ***************************************************************************/
void Applications::DesktopEntry::activate() {
    qApp->hideWidget();

    // QTBUG-51678

    // General escape rule has been applied on parse time
    // Apply exec escape rule
    QStringList arguments = execValueEscape( (term_) ? terminal.arg(quoteString(exec_)) : exec_);
    QString program = arguments.takeFirst();

    // TODO: Apply code fiels expansion (currently done at parese time)

    // Run the application
    QProcess::startDetached(program, arguments);
    ++usage_;
}



/** ***************************************************************************/
std::vector<QString> Applications::DesktopEntry::indexKeywords() const {
    return std::vector<QString>({
                                    name_,
                                    altName_,
                                    exec_.section(" ",0,0)
                                });
}



/** ***************************************************************************/
bool Applications::DesktopEntry::parseDesktopEntry() {
    // TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
    map<QString,map<QString,QString>> sectionMap;

    // Read the file
    QFile desktopFile(path_);
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
            sectionMap[currentGroup][key]=value;
        }
        desktopFile.close();
    } else return false;

    // Skip if there is no "Desktop Entry" section
    map<QString,map<QString,QString>>::iterator sectionIterator;
    if ((sectionIterator = sectionMap.find("Desktop Entry")) == sectionMap.end())
        return false;

    /*
     * Handle "Desktop Entry" section
     */

    map<QString,QString> &valueMap = sectionIterator->second;
    map<QString,QString>::iterator valueIterator;

    // Skip, if this desktop entry must not be shown
    if ((valueIterator = valueMap.find("NoDisplay")) != valueMap.end() && valueIterator->second == "true")
        return false;

    // Skip if the current desktop environment is not specified in "OnlyShowIn"
    if ((valueIterator = valueMap.find("OnlyShowIn")) != valueMap.end())
        if (!valueIterator->second.split(';',QString::SkipEmptyParts).contains(getenv("XDG_CURRENT_DESKTOP")))
            return false;

    // Check if this is a terminal app
    if ((valueIterator = valueMap.find("Terminal")) != valueMap.end())
    term_ = valueIterator->second ==  "true";

    // Try to get the (localized name)
    QString locale = QLocale().name();
    QString shortLocale = locale.left(2);
    if ( (valueIterator = valueMap.find(QString("Name[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Name[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Name"))) != valueMap.end())
        name_ = escapeString(valueIterator->second);
    else return false;

    // Try to get the exec key (http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html)
    if ((valueIterator = valueMap.find("Exec")) != valueMap.end())
        exec_ = escapeString(valueIterator->second);
    else return false;

    exec_.replace("%f", ""); // Unhandled TODO
    exec_.replace("%F", ""); // Unhandled TODO
    exec_.replace("%u", ""); // Unhandled TODO
    exec_.replace("%U", ""); // Unhandled TODO
    exec_.replace("%d", ""); // Deprecated
    exec_.replace("%D", ""); // Deprecated
    exec_.replace("%n", ""); // Deprecated
    exec_.replace("%N", ""); // Deprecated
    if ((valueIterator = valueMap.find("Icon")) != valueMap.end())
        exec_.replace("%i", QString("--icon %1").arg(valueIterator->second));
    else exec_.replace("%i", "");
    exec_.replace("%c", name_);
    exec_.replace("%k", path_);
    exec_.replace("%v", ""); // Deprecated
    exec_.replace("%m", ""); // Deprecated
    exec_.replace("%%", "%");

    // Try to get the icon
    if ((valueIterator = valueMap.find("Icon")) != valueMap.end()){
        iconPath_ = XdgIconLookup::instance()->themeIconPath(valueIterator->second, QIcon::themeName());
    }

    // Try to get a default icon if iconUrl_ is still empty
    if (iconPath_.isEmpty()) {
        iconPath_ = XdgIconLookup::instance()->themeIconPath("exec", QIcon::themeName());
    }

    // Use bundled icon if default icon not found
    if (iconPath_.isEmpty())
        iconPath_ = ":application-x-executable";

    // Try to get any [localized] secondary information comment
    if ( (valueIterator = valueMap.find(QString("Comment[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Comment[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Comment"))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName"))) != valueMap.end())
        altName_ = escapeString(valueIterator->second);

    // Root action. (FistComeFirstsServed. TODO: more sophisticated solution)
    for (const QString &s : supportedGraphicalSudo){
        QProcess p;
        p.start("which", {s});
        p.waitForFinished(-1);
        if (p.exitCode() == 0){
            actions_.push_back(std::make_shared<DesktopAction>(
                                   this, QString("Run %1 as root").arg(name_),
                                   QString("%1 \"%2\"").arg(s, exec_)));
            break;
        }
    }

    /*
     * Handle "Desktop Action X" sections
     */

    actions_.clear();
    if ((valueIterator = valueMap.find("Actions")) != valueMap.end()){
        QStringList actionStrings = valueIterator->second.split(';',QString::SkipEmptyParts);
        QString name;
        QString exec;
        for (const QString &actionString: actionStrings){

            // Get iterator to action section
            if ((sectionIterator = sectionMap.find(QString("Desktop Action %1").arg(actionString))) == sectionMap.end())
                continue;
            map<QString,QString> &valueMap = sectionIterator->second;

            // Get action name
            if ((valueIterator = valueMap.find("Name")) == valueMap.end())
                continue;
            name = valueIterator->second;

            // Get action command
            if ((valueIterator = valueMap.find("Exec")) == valueMap.end())
                continue;
            exec = valueIterator->second;

            actions_.push_back(std::make_shared<DesktopAction>(this, name, exec));
        }
    }

    return true;
}



/** ***************************************************************************/
void Applications::DesktopEntry::serialize(QDataStream &out) {
    out << path_
        << static_cast<quint16>(usage_)
        << name_
        << altName_ << iconPath_ << exec_ << term_;
    out << static_cast<quint64>(actions_.size());
    for (const auto &action : actions_)
        out << static_cast<DesktopAction*>(action.get())->description_
            << static_cast<DesktopAction*>(action.get())->exec_
            << static_cast<DesktopAction*>(action.get())->term_;
}



/** ***************************************************************************/
void Applications::DesktopEntry::deserialize(QDataStream &in) {
    in >> path_ >> usage_ >> name_ >> altName_ >> iconPath_ >> exec_ >> term_;

    QString description;
    QString exec;
    bool term;
    quint64 amountOfActions;
    in >> amountOfActions;

    for (;amountOfActions != 0; --amountOfActions) {
        in >> description >> exec >> term;
        actions_.emplace_back(std::make_shared<DesktopAction>(this, description, exec, term));
    }
}



/** ***************************************************************************/
QString Applications::DesktopEntry::escapeString(const QString &unescaped)
{
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
QString Applications::DesktopEntry::quoteString(const QString &unquoted) {
    QString result;
    result.push_back("\"");
    for (const auto &qchar : unquoted){
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
QStringList Applications::DesktopEntry::execValueEscape(const QString &execValue) {

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



///** ***************************************************************************/
//QIcon Applications::Application::getIcon(const QString &iconName) {

//    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
//    // http://standards.freedesktop.org/desktop-entry-spec/latest/

//    /*
//     * Icon to display in file manager, menus, etc. If the name is an absolute
//     * path, the given file will be used. If the name is not an absolute path,
//     * the algorithm described in the Icon Theme Specification will be used to
//     * locate the icon. oh funny qt-bug h8 u
//     */

//    if (iconName.startsWith('/'))
//        // If it is a full path
//        return QIcon(iconName);
//    else if (QIcon::hasThemeIcon(iconName))
//        // If it is in the theme
//        return QIcon::fromTheme(iconName);
//    else {
//        QIcon result;
//        // Try hicolor
//        QString currentTheme = QIcon::themeName(); // missing fallback (qt-bug)
//        QIcon::setThemeName("hicolor");
//        if (QIcon::hasThemeIcon(iconName)) {
//            result = QIcon::fromTheme(iconName);
//            QIcon::setThemeName(currentTheme);
//        } else {
//            QIcon::setThemeName(currentTheme);
//            // if it is in the pixmaps
//            QDirIterator it("/usr/share/pixmaps", QDir::Files, QDirIterator::Subdirectories);
//            bool found = false;
//            while (it.hasNext()) {
//                it.next();
//                QFileInfo fi = it.fileInfo();
//                if (fi.isFile() && (fi.fileName() == iconName || fi.baseName() == iconName)) {
//                    result = QIcon(fi.canonicalFilePath());
//                    found = true;
//                    break;
//                }
//            }
//            if (!found) {
//                // If it is still not found use a generic one
//                qWarning() << "Unknown icon:" << iconName;
//                result = QIcon::fromTheme("exec");
//            }
//        }
//        return result;
//    }
//}
