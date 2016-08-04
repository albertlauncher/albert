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

#pragma once
#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QMutex>
#include <QString>
#include <QTimer>
#include <set>
#include <vector>
#include <utility>
#include <memory>
using std::set;
using std::vector;
using std::pair;
using std::shared_ptr;
class AbstractExtension;
class AbstractItem;
typedef shared_ptr<AbstractItem> SharedItem;


/** ***************************************************************************/
class QueryPrivate : public QAbstractListModel
{
    Q_OBJECT

public:

    QueryPrivate(const QString &query, const set<AbstractExtension*> &queryHandlers);

    void addMatch(shared_ptr<AbstractItem> item, short score = 0);
    void addMatches(vector<std::pair<SharedItem,short>>::iterator begin,
                    vector<std::pair<SharedItem,short>>::iterator end);

    const QString &searchTerm() const { return searchTerm_; }
    const QString &trigger() const { return trigger_; }
    bool isRunning() { return isRunning_; }
    bool isValid() { return isValid_; }
    void invalidate();


    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:

    void onUXTimeOut();
    void onHandlerFinished();

    void sortResults();
    void sortFallbacks();


    /** ***********************************************************************/


    const QString searchTerm_;
    const QString trigger_;
    bool isValid_;
    bool isRunning_;
    bool showFallbacks_;

    vector<QFutureWatcher<void>*> futureWatchers_;
    mutable QMutex mutex_;
    QTimer UXTimeOut_;

    vector<pair<SharedItem, short>> matches_;
    vector<SharedItem> fallbacks_;

    static vector<QString> fallbackOrder;
    static struct Initializer { Initializer(); } initializer_;

signals:

    void resultyReady(QAbstractItemModel *);
    void started();
    void finished();
};


