// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include "indextreenode.h"
#include "indexfile.h"
using namespace std;



Files::IndexTreeNode::IndexTreeNode(QString name, QDateTime lastModified, shared_ptr<IndexTreeNode> parent)
    : enable_shared_from_this(), parent(parent), name(name), lastModified(lastModified) { }

Files::IndexTreeNode::IndexTreeNode(QString name, shared_ptr<IndexTreeNode> parent)
    : IndexTreeNode(name, QDateTime::fromMSecsSinceEpoch(0), parent) { }

Files::IndexTreeNode::IndexTreeNode(const IndexTreeNode &other)
    : enable_shared_from_this(),
      children(other.children),
      name(other.name),
      lastModified(other.lastModified),
      items_(other.items_) { }

Files::IndexTreeNode::~IndexTreeNode() {
    removeDownlinks();
}



void Files::IndexTreeNode::accept(Visitor &visitor) {
    visitor.visit(this);
    for ( auto & child : children )
        child->accept(visitor);
}



void Files::IndexTreeNode::removeDownlinks() {
    for ( shared_ptr<IndexTreeNode> & child : children )
        child->removeDownlinks();
    children.clear();
    items_.clear();
}



QString Files::IndexTreeNode::path() const {
    return ( parent ) ? QDir(parent->path()).filePath(name) : name;
}



void Files::IndexTreeNode::update(const bool  &abort, IndexSettings indexSettings) {

    set<QString> indexedDirs;
    QMimeDatabase mimeDatabase;
    // ignore ignorefile by default
    vector<IgnoreEntry> ignoreEntries = {IgnoreEntry(
                                         QRegularExpression(QString("%1$").arg(IGNOREFILE)),
                                         PatternType::Exclude)};
    // Start the indexing
    updateRecursion(abort, mimeDatabase, indexSettings, indexedDirs, ignoreEntries);
}


const std::vector<std::shared_ptr<Files::File> > &Files::IndexTreeNode::items() const {
    return items_;
}



void Files::IndexTreeNode::updateRecursion(const bool &abort,
                                           const QMimeDatabase &mimeDatabase,
                                           const IndexSettings &indexSettings,
                                           set<QString> &indexedDirs,
                                           const vector<IgnoreEntry> &ignoreEntries){

    if (abort) return;

    const QFileInfo fileInfo(path());

    // Skip if this dir has already been indexed (loop detection)
    if ( indexedDirs.count(fileInfo.canonicalFilePath()) )
        return;


    // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
    vector<IgnoreEntry> localIgnoreEntries = ignoreEntries;
    QFile file(QDir(fileInfo.filePath()).filePath(IGNOREFILE));
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QString pattern = QDir::cleanPath(in.readLine());

            if ( pattern.isEmpty() || pattern.startsWith("#") )
                continue;

            // Replace ** and * by their regex analogons
            pattern.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)"), "[^\\/]*");
            pattern.replace(QRegularExpression("\\*{2,}"), ".*");

            // Determine pattern type
            PatternType patternType = PatternType::Exclude;
            if ( pattern.startsWith('!') ) {
                patternType = PatternType::Include;
                pattern = pattern.mid(1, -1);
            }

            // Respect files beginning with excalmation mark
            if ( pattern.startsWith("\\!") )
                pattern = pattern.mid(1, -1);

            if ( pattern.startsWith("/") ) {
                pattern = QString("^%1$").arg(QDir(fileInfo.filePath()).filePath(pattern.mid(1, -1)));
                localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
            } else {
                pattern = QString("%1$").arg(pattern);
                localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
            }
        }
        file.close();
    }


    if ( lastModified < fileInfo.lastModified() || indexSettings.settingsChangedSinceLastUpdate() ) {

        lastModified = fileInfo.lastModified();

        items_.clear();

        // Drop nonexistant child nodes
        decltype(children)::iterator childIt = children.begin();
        while ( childIt != children.end() )
            if ( !QFile::exists((*childIt)->path()) ) {
                (*childIt)->removeDownlinks();
                childIt = children.erase(childIt);
            } else
                ++childIt;


        // Handle the directory contents
        // Prepare the iterator properties
        QDir::Filters entryListFilters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden;
        const QFileInfoList &fil = QDir(fileInfo.filePath()).entryInfoList(entryListFilters, QDir::Name);
        for ( const QFileInfo &fileInfo : fil ){

            bool isValid = true;

            // Skip check if this file should be excluded
            PatternType patternType = PatternType::Include;
            for ( const IgnoreEntry &ignoreEntry : localIgnoreEntries )
                if ( ignoreEntry.regex.match(fileInfo.filePath()).hasMatch() )
                    patternType = ignoreEntry.type;
            if ( patternType == PatternType::Exclude )
                isValid &= false;

            // Invalidate if this file is a symlink and we should skip symlinks
            if ( fileInfo.isSymLink() && !indexSettings.followSymlinks() )
                isValid &= false;

            // Invalidate if this file is hidden we should skip hidden files
            if ( fileInfo.isHidden() && !indexSettings.indexHidden() )
                isValid &= false;

            // Add directories as nodes
            if ( fileInfo.isDir() ) {

                // Get the place where the item were if would exist in log(n) time
                decltype(children)::iterator lb = lower_bound(
                            children.begin(), children.end(), fileInfo.fileName(),
                            [&](const shared_ptr<IndexTreeNode> & child, QString name) {
                    return child->name < name;
                });

                // If does not exist and is valid insert
                if ( lb == children.end() || (*lb)->name != fileInfo.fileName() ) {
                    if (isValid)
                        children.insert(lb, make_shared<IndexTreeNode>(fileInfo.fileName(), shared_from_this()));
                    // If does exist and is invalid remove node
                } else if (!isValid){
                    (*lb)->removeDownlinks();
                    children.erase(lb);
                }
            }

            // Add entries as items
            const QMimeType mimetype = mimeDatabase.mimeTypeForFile(fileInfo.filePath());
            const QString mimeName = mimetype.name();

            // If the entry is valid and a mime filter matches add it to the items
            if (isValid && any_of(indexSettings.filters().begin(), indexSettings.filters().end(),
                                  [&](const QRegExp &re){ return re.exactMatch(mimeName); }) )
                items_.push_back(make_shared<Files::IndexFile>(fileInfo.fileName(),
                                                               shared_from_this(),
                                                               mimetype));
        }
    }

    // Recursively check all childnodes too
    for ( const shared_ptr<IndexTreeNode> &child : children )
        child->updateRecursion(abort, mimeDatabase, indexSettings, indexedDirs, ignoreEntries);

}
