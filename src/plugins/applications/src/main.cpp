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


#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QList>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QTimer>
#include <QThread>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include "configwidget.h"
#include "main.h"
#include "offlineindex.h"
#include "query.h"
#include "queryhandler.h"
#include "standardaction.h"
#include "standardindexitem.h"
#include "xdgiconlookup.h"
using std::map;
using std::vector;
using std::shared_ptr;
using namespace Core;

extern QString terminalCommand;


namespace {

const char* CFG_PATHS    = "paths";
const char* CFG_FUZZY    = "fuzzy";
const bool  DEF_FUZZY    = false;
const bool  UPDATE_DELAY = 60000;

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
vector<shared_ptr<StandardIndexItem>> indexApplications(const QStringList & rootDirs) {

    // Get a new index [O(n)]
    vector<shared_ptr<StandardIndexItem>> desktopEntries;
    QStringList xdg_current_desktop = QString(getenv("XDG_CURRENT_DESKTOP")).split(':',QString::SkipEmptyParts);
    QLocale loc;


    // Iterate over all desktop files
    for ( const QString &dir : rootDirs ) {
        QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {

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

            vector<shared_ptr<Action>> actions;

            // Unquote arguments and expand field codes
            QStringList commandline = expandedFieldCodes(shellLexerSplit(exec),
                                                         icon,
                                                         name,
                                                         fIt.filePath());

            shared_ptr<StandardAction> sa = std::make_shared<StandardAction>();
            sa->setText("Run");
            if (term){
                sa->setAction([commandline, workingDir](){
                    QStringList arguments = shellLexerSplit(terminalCommand);
                    arguments.append(commandline);
                    QString command = arguments.takeFirst();
                    QProcess::startDetached(command, arguments, workingDir);
                });
            } else {
                sa->setAction([commandline, workingDir](){
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
                sa->setAction([commandline, workingDir](){
                    QStringList arguments = shellLexerSplit(terminalCommand);
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
//                sa->setAction([commandline, workingDir](){
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
                    sa->setAction([commandline, workingDir](){
                        QStringList arguments = shellLexerSplit(terminalCommand);
                        arguments.append(commandline);
                        QString command = arguments.takeFirst();
                        QProcess::startDetached(command, arguments, workingDir);
                    });
                } else {
                    sa->setAction([commandline, workingDir](){
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
            shared_ptr<StandardIndexItem> ssii = std::make_shared<StandardIndexItem>(id);

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
            vector<Indexable::WeightedKeyword> indexKeywords;
            indexKeywords.emplace_back(name, USHRT_MAX);
            if (!genericName.isEmpty())
                indexKeywords.emplace_back(genericName, USHRT_MAX*0.9);
            for (auto & kw : keywords)
                indexKeywords.emplace_back(kw, USHRT_MAX*0.8);
            if (!comment.isEmpty())
                indexKeywords.emplace_back(comment, USHRT_MAX*0.5);
            ssii->setIndexKeywords(std::move(indexKeywords));

            // Set actions
            ssii->setActions(std::move(actions));

            desktopEntries.push_back(std::move(ssii));
        }
    }
    return desktopEntries;
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Applications::ApplicationsPrivate
{
public:
    ApplicationsPrivate(Extension *q) : q(q) {}

    Extension *q;

    QPointer<ConfigWidget> widget;
    QFileSystemWatcher watcher;
    QStringList rootDirs;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    OfflineIndex offlineIndex;
    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    QTimer updateDelayTimer;

    void finishIndexing();
    void startIndexing();
};

/** ***************************************************************************/
void Applications::ApplicationsPrivate::startIndexing() {

    // Never run concurrent
    if ( futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                     std::bind(&ApplicationsPrivate::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(indexApplications, rootDirs));

    // Notification
    qDebug() << qPrintable(QString("[%1] Start indexing in background thread.").arg(q->Core::Extension::id).toUtf8().constData());
    emit q->statusInfo("Indexing desktop entries ...");

}


/** ***************************************************************************/
void Applications::ApplicationsPrivate::finishIndexing() {

    // Get the thread results
    index = futureWatcher.future().result();

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    // Finally update the watches (maybe folders changed)
    if (!watcher.directories().isEmpty())
        watcher.removePaths(watcher.directories());
    for (const QString &path : rootDirs) {
        watcher.addPath(path);
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            watcher.addPath(dit.next());
    }

    // Notification
    qDebug() << qPrintable(QString("[%1] Indexing done (%2 items).").arg(q->Core::Extension::id).arg(index.size()));
    emit q->statusInfo(QString("%1 desktop entries indexed.").arg(index.size()));
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Applications::Extension::Extension()
    : Core::Extension("org.albert.extension.applications"),
      Core::QueryHandler(Core::Extension::id),
      d(new ApplicationsPrivate(this)) {

    qunsetenv("DESKTOP_AUTOSTART_ID");

    // Load settings
    QSettings s(qApp->applicationName());
    s.beginGroup(Core::Extension::id);
    d->offlineIndex.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Load the paths or set a default
    QVariant v = s.value(CFG_PATHS);
    if (v.isValid() && v.canConvert(QMetaType::QStringList))
        d->rootDirs = v.toStringList();
    else
        restorePaths();
    s.endGroup();

    // Keep the Applications in sync with the OS
    d->updateDelayTimer.setInterval(UPDATE_DELAY);
    d->updateDelayTimer.setSingleShot(true);

    // If the filesystem changed, trigger the update delay
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged, &d->updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));

    // If the root dirs changed, trigger the update delay
    connect(this, &Extension::rootDirsChanged, &d->updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));

    // If the update delay passed, update the index
    connect(&d->updateDelayTimer, &QTimer::timeout, this, &Extension::updateIndex, Qt::QueuedConnection);

    // If the root dirs change write it to the settings
    connect(this, &Extension::rootDirsChanged, [this](const QStringList& dirs){
        QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_PATHS), dirs);
    });

    // Trigger initial update
    updateIndex();
}



/** ***************************************************************************/
Applications::Extension::~Extension() {
    delete d;
}



/** ***************************************************************************/
QWidget *Applications::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Paths
        d->widget->ui.listWidget_paths->addItems(d->rootDirs);
        connect(this, &Extension::rootDirsChanged, d->widget->ui.listWidget_paths, &QListWidget::clear);
        connect(this, &Extension::rootDirsChanged, d->widget->ui.listWidget_paths, &QListWidget::addItems);
        connect(d->widget.data(), &ConfigWidget::requestAddPath, this, &Extension::addDir);
        connect(d->widget.data(), &ConfigWidget::requestRemovePath, this, &Extension::removeDir);
        connect(d->widget->ui.pushButton_restorePaths, &QPushButton::clicked, this, &Extension::restorePaths);

        // Fuzzy
        d->widget->ui.checkBox_fuzzy->setChecked(d->offlineIndex.fuzzy());
        connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Info
        d->widget->ui.label_info->setText(QString("%1 Applications indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_info, &QLabel::setText);
    }
    return d->widget;
}



/** ***************************************************************************/
void Applications::Extension::handleQuery(Core::Query * query) {

    // Search for matches
    const vector<shared_ptr<Core::Indexable>> &indexables = d->offlineIndex.search(query->searchTerm().toLower());

    // Add results to query
    vector<pair<shared_ptr<Core::Item>,short>> results;
    for (const shared_ptr<Core::Indexable> &item : indexables)
        // TODO `Search` has to determine the relevance. Set to 0 for now
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 1);

    query->addMatches(results.begin(), results.end());
}



/** ***************************************************************************/
void Applications::Extension::addDir(const QString & dirPath) {

    QFileInfo fileInfo(dirPath);

    // Get an absolute file path
    QString absPath = fileInfo.absoluteFilePath();

    // Check existance
    if (!fileInfo.exists()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " does not exist.").exec();
        return;
    }

    // Check type
    if(!fileInfo.isDir()) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " is not a directory.").exec();
        return;
    }

    // Check if there is an identical existing path
    if (d->rootDirs.contains(absPath)) {
        QMessageBox(QMessageBox::Critical, "Error", absPath + " has already been indexed.").exec();
        return;
    }

    // Check if this dir is a subdir of an existing dir
    for (const QString &p: d->rootDirs)
        if (absPath.startsWith(p + '/')) {
            QMessageBox(QMessageBox::Critical, "Error", absPath + " is subdirectory of " + p).exec();
            return;
        }

    // Check if this dir is a superdir of an existing dir, in case delete subdir
    for (QStringList::iterator it = d->rootDirs.begin(); it != d->rootDirs.end();)
        if (it->startsWith(absPath + '/')) {
            QMessageBox(QMessageBox::Warning, "Warning",
                        (*it) + " is subdirectory of " + absPath + ". " + (*it) + " will be removed.").exec();
            it = d->rootDirs.erase(it);
        } else ++it;

    // Add the path to root dirs
    d->rootDirs << absPath;

    // Inform observers
    emit rootDirsChanged(d->rootDirs);
}



/** ***************************************************************************/
void Applications::Extension::removeDir(const QString &dirPath) {

    // Get an absolute file path
    QString absPath = QFileInfo(dirPath).absoluteFilePath();

    // Check existance
    if (!d->rootDirs.contains(absPath))
        return;

    // Remove the path
    d->rootDirs.removeAll(absPath);

    // Update the widget, if it is visible atm
    emit rootDirsChanged(d->rootDirs);
}



/** ***************************************************************************/
void Applications::Extension::restorePaths() {

    // Add standard paths
    d->rootDirs.clear();

    //  Add standard paths
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        if (QFileInfo(path).exists())
            addDir(path);
}



/** ***************************************************************************/
bool Applications::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Applications::Extension::setFuzzy(bool b) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FUZZY), b);
    d->offlineIndex.setFuzzy(b);
}



/** ***************************************************************************/
void Applications::Extension::updateIndex() {
    d->startIndexing();
}
