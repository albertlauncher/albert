// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QDirIterator>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include "indexer.h"
#include "extension.h"
#include "standardobjects.h"
#include "xdgiconlookup.h"
#include "albertapp.h"
using std::map;
using std::vector;
using std::shared_ptr;

namespace {

/******************************************************************************/
QStringList expandedFieldCodes(const QStringList & unexpandedFields,
                               const QString & icon,
                               const QString & name,
                               const QString & path) {
    /*
     * A number of special field codes have been defined which will be expanded
     * by the file manager or program launcher when encountered in the command
     * line. Field codes consist of the percentage character ("%") followed by
     * an alpha character. Literal percentage characters must be escaped as %%.
     * Deprecated field codes should be removed from the command line and
     * ignored. Field codes are expanded only once, the string that is used to
     * replace the field code should not be checked for field codes itself.
     *
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
     */
    QStringList expandedFields;

    for (const QString & field : unexpandedFields){

        if (field == "%i" && !icon.isEmpty()) {
            expandedFields.push_back("--icon");
            expandedFields.push_back(icon);
        }

        QString tmpstr;
        QString::const_iterator it = field.begin();

        while (it != field.end()) {
            if (*it == '%'){
                ++it;
                if (it == field.end())
                    break;
                else if (*it=='%')
                    tmpstr.push_back("%");
                else if (*it=='c')
                    tmpstr.push_back(name);
                else if (*it=='k')
                    tmpstr.push_back(path);
                // TODO Unhandled f F u U
            }
            else
                tmpstr.push_back(*it);
            ++it;
        }
        if (!tmpstr.isEmpty())
            expandedFields.push_back(std::move(tmpstr));
    }
    return expandedFields;
}

/******************************************************************************/
QString xdgStringEscape(const QString & unescaped) {
    /*
     * The escape sequences \s, \n, \t, \r, and \\ are supported for values of
     * type string and localestring, meaning ASCII space, newline, tab, carriage
     * return, and backslash, respectively.
     *
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
     */
    QString result;
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

/******************************************************************************/
QString getLocalizedKey(const QString &key, const map<QString,QString> &entries, const QLocale &loc) {
    map<QString,QString>::const_iterator it;
    if ( (it = entries.find(QString("%1[%2]").arg(key, loc.name()))) != entries.end()
         || (it = entries.find(QString("%1[%2]").arg(key, loc.name().left(2)))) != entries.end()
         || (it = entries.find(key)) != entries.end())
        return it->second;
    return QString();
}



/******************************************************************************/
QStringList shellLexerSplit(const QString &input) {

    QString part;
    QStringList result;
    QString::const_iterator it = input.begin();

    while(it != input.end()){

        // Check for a backslash (escape)
        if (*it == '\\'){
            if (++it == input.end()){
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
                if (++it == input.end()){
                    qWarning() << "Detected EOL inside a qoute.";
                    return QStringList();
                }

                // Leave the "quotation loop" on double qoute
                else if (*it == '"')
                    break;

                // Check for a backslash (escape)
                else if (*it == '\\'){
                    if (++it == input.end()){
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

}


/** ***************************************************************************/
void Applications::Extension::Indexer::run() {

    // Notification
    qDebug("[%s] Start indexing in background thread", extension_->id.toUtf8().constData());
    emit statusInfo("Indexing desktop entries ...");

    // Get a new index [O(n)]
    vector<SharedStdIdxItem> desktopEntries;
    QStringList xdg_current_desktop = QString(getenv("XDG_CURRENT_DESKTOP")).split(':',QString::SkipEmptyParts);
    QLocale loc;


    // Iterate over all desktop files
    for (const QString &dir : extension_->rootDirs_) {
        QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {

            // Abortion requested
            if (abort_)
                return;

            map<QString,map<QString,QString>> sectionMap;
            map<QString,map<QString,QString>>::iterator sectionIterator;

            /*
             * Get the data from the desktop file
             */

            // Read the file into a map
            {
            QFile file(fIt.next());
            if (!file.open(QIODevice::ReadOnly| QIODevice::Text)) continue;
            QTextStream stream(&file);
            QString currentGroup;
            for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine()) {
                line = line.trimmed();
                if (line.startsWith('#') || line.isEmpty())
                    continue;
                if (line.startsWith("[")){
                    currentGroup = line.mid(1,line.size()-2).trimmed();
                    continue;
                }
                sectionMap[currentGroup].emplace(line.section('=', 0,0).trimmed(),
                                                 line.section('=', 1, -1).trimmed());
            }
            file.close();
            }


            // Skip if there is no "Desktop Entry" section
            if ((sectionIterator = sectionMap.find("Desktop Entry")) == sectionMap.end())
                continue;

            map<QString,QString> const &entryMap = sectionIterator->second;
            map<QString,QString>::const_iterator entryIterator;

            // Skip, if type is not found or not application
            if ((entryIterator = entryMap.find("Type")) == entryMap.end() ||
                    entryIterator->second != "Application")
                continue;

            // Skip, if this desktop entry must not be shown
            if ((entryIterator = entryMap.find("NoDisplay")) != entryMap.end()
                    && entryIterator->second == "true")
                continue;

            // Skip if the current desktop environment is specified in "NotShowIn"
            if ((entryIterator = entryMap.find("NotShowIn")) != entryMap.end())
                for (const QString &str : entryIterator->second.split(';',QString::SkipEmptyParts))
                    if (xdg_current_desktop.contains(str))
                        continue;

            // Skip if the current desktop environment is not specified in "OnlyShowIn"
            if ((entryIterator = entryMap.find("OnlyShowIn")) != entryMap.end()) {
                bool found = false;
                for (const QString &str : entryIterator->second.split(';',QString::SkipEmptyParts))
                    if (xdg_current_desktop.contains(str)){
                        found = true;
                        break;
                    }
                if (!found)
                    continue;
            }

            bool term;
            QString name;
            QString genericName;
            QString comment;
            QString icon;
            QString exec;
            QString workingDir;
            QStringList keywords;
            QStringList actionIdentifiers;

            // Try to get the localized name, skip if empty
            name = xdgStringEscape(getLocalizedKey("Name", entryMap, loc));
            if (name.isNull())
                continue;

            // Try to get the exec key, skip if not existant
            if ((entryIterator = entryMap.find("Exec")) != entryMap.end())
                exec = xdgStringEscape(entryIterator->second);
            else
                continue;

            // Try to get the localized icon, skip if empty
            icon = xdgStringEscape(getLocalizedKey("Icon", entryMap, loc));
            if (icon.isNull())
                continue;

            // Check if this is a terminal app
            term = (entryIterator = entryMap.find("Terminal")) != entryMap.end()
                    && entryIterator->second=="true";

            // Try to get the localized genericName
            genericName = xdgStringEscape(getLocalizedKey("GenericName", entryMap, loc));

            // Try to get the localized comment
            comment = xdgStringEscape(getLocalizedKey("Comment", entryMap, loc));

            // Try to get the keywords
            keywords = xdgStringEscape(getLocalizedKey("Keywords", entryMap, loc)).split(';',QString::SkipEmptyParts);

            // Try to get the workindir
            if ((entryIterator = entryMap.find("Path")) != entryMap.end())
                workingDir = xdgStringEscape(entryIterator->second);

            // Try to get the keywords
            if ((entryIterator = entryMap.find("Actions")) != entryMap.end())
                actionIdentifiers = xdgStringEscape(entryIterator->second).split(';',QString::SkipEmptyParts);

//            // Try to get the mimetypes
//            if ((valueIterator = entryMap.find("MimeType")) != entryMap.end())
//                keywords = xdgStringEscape(valueIterator->second).split(';',QString::SkipEmptyParts);

            /*
             * Default action
             */

            vector<SharedAction> actions;

            // Unquote arguments and expand field codes
            QStringList commandline = expandedFieldCodes(shellLexerSplit(exec),
                                                         icon,
                                                         name,
                                                         fIt.filePath());

            SharedStdAction sa = std::make_shared<StandardAction>();
            sa->setText("Run");
            if (term){
                sa->setAction([commandline, workingDir](ExecutionFlags *){
                    QStringList arguments = shellLexerSplit(qApp->term());
                    arguments.append(commandline);
                    QString command = arguments.takeFirst();
                    QProcess::startDetached(command, arguments, workingDir);
                });
            } else {
                sa->setAction([commandline, workingDir](ExecutionFlags *){
                    QStringList arguments = commandline;
                    QString command = arguments.takeFirst();
                    QProcess::startDetached(command, arguments, workingDir);
                });
            }

            actions.push_back(sa);


            /*
             * Root action
             */

            if (term){
                sa = std::make_shared<StandardAction>();
                sa->setText("Run as root");
                sa->setAction([commandline, workingDir](ExecutionFlags *){
                    QStringList arguments = shellLexerSplit(qApp->term());
                    arguments.append("sudo");
                    arguments.append(commandline);
                    QString command = arguments.takeFirst();
                    QProcess::startDetached(command, arguments, workingDir);
                });
                actions.push_back(sa);
            }
//            else {
//             Root action. (FistComeFirstsServed. TODO: more sophisticated solution)
//            for (const QString &s : supportedGraphicalSudo){
//                QProcess p;
//                p.start("which", {s});
//                p.waitForFinished(-1);
//                if (p.exitCode() == 0){
//                    actions_.push_back(std::make_shared<DesktopAction>(
//                                           this, QString("Run %1 as root").arg(name_),
//                                           QString("%1 \"%2\"").arg(s, exec_)));
//                    break;
//                }
//            }
//                sa->setAction([commandline, workingDir](ExecutionFlags *){
//                    QStringList arguments = commandline;
//                    QString command = arguments.takeFirst();
//                    QProcess::startDetached(command, arguments, workingDir);
//                });
//            }


            /*
             * Desktop Actions
             */

            for (const QString &actionIdentifier: actionIdentifiers){

                sa = std::make_shared<StandardAction>();

                // Get iterator to action section
                if ((sectionIterator = sectionMap.find(QString("Desktop Action %1").arg(actionIdentifier))) == sectionMap.end())
                    continue;
                map<QString,QString> &valueMap = sectionIterator->second;

                // Get action name
                if ((entryIterator = valueMap.find("Name")) == valueMap.end())
                    continue;
                sa->setText(entryIterator->second);

                // Get action command
                if ((entryIterator = valueMap.find("Exec")) == valueMap.end())
                    continue;

                // Unquote arguments and expand field codes
                QStringList commandline = expandedFieldCodes(shellLexerSplit(entryIterator->second),
                                                             icon,
                                                             name,
                                                             fIt.filePath());

                if (term){
                    sa->setAction([commandline, workingDir](ExecutionFlags *){
                        QStringList arguments = shellLexerSplit(qApp->term());
                        arguments.append(commandline);
                        QString command = arguments.takeFirst();
                        QProcess::startDetached(command, arguments, workingDir);
                    });
                } else {
                    sa->setAction([commandline, workingDir](ExecutionFlags *){
                        QStringList arguments = commandline;
                        QString command = arguments.takeFirst();
                        QProcess::startDetached(command, arguments, workingDir);
                    });
                }
                actions.push_back(sa);
            }


            /*
             * Build the item
             */

            // Finally we got everything, build the item
            QString id = fIt.filePath().remove(QRegularExpression("^.*applications/")).replace("/","-");
            SharedStdIdxItem ssii = std::make_shared<StandardIndexItem>(id);

            // Set Name
            ssii->setText(name);

            // Set subtext/tootip
            if (comment.isEmpty())
                if (genericName.isEmpty())
                    ssii->setSubtext(exec);
                else
                    ssii->setSubtext(genericName);
            else
                ssii->setSubtext(comment);

            // Set icon
            icon = XdgIconLookup::instance()->themeIconPath(icon);
            if (icon.isEmpty())
                icon = XdgIconLookup::instance()->themeIconPath("exec");
            if (icon.isEmpty())
                icon = ":application-x-executable";
            ssii->setIconPath(icon);

            // Set keywords
            vector<IIndexable::WeightedKeyword> indexKeywords;
            indexKeywords.emplace_back(name, USHRT_MAX);
            if (!genericName.isEmpty())
                indexKeywords.emplace_back(genericName, USHRT_MAX*0.9);
            if (!extension_->indexNameOnly_) {
                for (auto & kw : keywords)
                    indexKeywords.emplace_back(kw, USHRT_MAX*0.8);
                if (!comment.isEmpty())
                    indexKeywords.emplace_back(comment, USHRT_MAX*0.5);
            }
            ssii->setIndexKeywords(std::move(indexKeywords));

            // Set actions
            ssii->setActions(std::move(actions));

            desktopEntries.push_back(std::move(ssii));
        }
    }


    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    QMutexLocker locker(&extension_->indexAccess_);

    // Abortion requested while block
    if (abort_)
        return;

    // Set the new index (use swap to shift destruction out of critical area)
    std::swap(extension_->index_, desktopEntries);

    // Rebuild the offline index
    extension_->offlineIndex_.clear();
    for (const auto &item : extension_->index_)
        extension_->offlineIndex_.add(item);

    // Finally update the watches (maybe folders changed)
    if (!extension_->watcher_.directories().isEmpty())
        extension_->watcher_.removePaths(extension_->watcher_.directories());
    for (const QString &path : extension_->rootDirs_) {
        extension_->watcher_.addPath(path);
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            extension_->watcher_.addPath(dit.next());
    }

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->id.toUtf8().constData(), static_cast<int>(extension_->index_.size()));
    emit statusInfo(QString("Indexed %1 desktop entries").arg(extension_->index_.size()));
}
