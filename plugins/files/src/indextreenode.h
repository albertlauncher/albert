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

#pragma once
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <memory>
#include <vector>
#include <set>
#include "file.h"

namespace Files {

class Visitor;

class IndexSettings
{
public:
    const std::vector<QRegExp> &filters() const { return mimefilters_;}
    void setFilters(std::vector<QRegExp> value) { settingsChanged_= true; mimefilters_= value; }
    void setFilters(QStringList value) {
        settingsChanged_= true;
        mimefilters_.clear();
        for ( const QString &re : value )
            mimefilters_.emplace_back(re, Qt::CaseInsensitive, QRegExp::Wildcard);
    }

    bool indexHidden() const { return indexHidden_;}
    void setIndexHidden(bool value) { settingsChanged_= true; indexHidden_= value; }

    bool followSymlinks() const { return followSymlinks_;}
    void setFollowSymlinks(bool value) { settingsChanged_= true; followSymlinks_= value; }

    bool settingsChangedSinceLastUpdate() const { return settingsChanged_;}
    void setSettingsChangedSinceLastUpdate(bool value) { settingsChanged_= value; }

private:

    std::vector<QRegExp> mimefilters_;
    bool indexHidden_;
    bool followSymlinks_;
    bool settingsChanged_;

};

enum class PatternType {
    Include,
    Exclude
};

struct IgnoreEntry {
    IgnoreEntry(QRegularExpression regex, PatternType type) : regex(regex), type(type) {}
    QRegularExpression regex;
    PatternType type;
};

class IndexTreeNode final : public std::enable_shared_from_this<IndexTreeNode>
{
public:

    IndexTreeNode(const IndexTreeNode & other);
    IndexTreeNode(QString name, QDateTime lastModified, std::shared_ptr<IndexTreeNode> parent = std::shared_ptr<IndexTreeNode>());
    IndexTreeNode(QString name, std::shared_ptr<IndexTreeNode> parent = std::shared_ptr<IndexTreeNode>());
    ~IndexTreeNode();

    void accept(Visitor &visitor);

    void removeDownlinks();

    QString path() const;

    void update(const bool &abort, IndexSettings indexSettings);

    std::vector<std::shared_ptr<Files::File>> items;

private:

    void updateRecursion(const bool &abort,
                         const QMimeDatabase &mimeDatabase,
                         const IndexSettings &indexSettings,
                         std::set<QString> &indexedDirs,
                         const std::vector<IgnoreEntry> &ignoreEntries);

    std::shared_ptr<IndexTreeNode> parent;
    std::vector<std::shared_ptr<IndexTreeNode>> children;
    QString name;
    QDateTime lastModified;

    static constexpr const char* IGNOREFILE = ".albertignore";

};

/** ***************************************************************************/
class Visitor {
public:
    virtual ~Visitor() { }
    virtual void visit(IndexTreeNode *) = 0;
};

///** ***************************************************************************/
//class ConsoleLoggerVisitor : public Visitor {
//public:
//    void visit(IndexTreeNode *node) override {
//        qDebug() << node->path();
//    }
//};

}
