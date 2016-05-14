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
#include <QDebug>
#include <QVariant>
#include <QMutex>
#include <QTimer>
#include <QMutexLocker>
#include <QAbstractListModel>
#include <utility>
#include <algorithm>
#include <vector>
#include <memory>
using std::shared_ptr;
using std::unique_ptr;
#include "abstractobjects.hpp"
#include "roles.hpp"
#include "albertapp.h"

struct Match {
    ItemSPtr item;
    short score;
};

/** ***************************************************************************/
class QueryPrivate final : public QAbstractListModel
{
    Q_OBJECT
    friend class ExtensionManager;

public:
    /** ***********************************************************************/
    QueryPrivate(const QString &term)
        : searchTerm_(term), dynamicSort_(false) {
        // Use this when muoltithreadeing
//        QTimer::singleShot(50, this, &QueryPrivate::makeDynamic);
        }



    /** ***********************************************************************/
    void addMatch(shared_ptr<AlbertItem> item, short score) {
        QMutexLocker locker(&mutex_);
        if (dynamicSort_) {
            //beginInsertRows(...);
            throw "Not implemented yet.";
        } else {
            matches_.push_back({item, score});
        }
    }



//    /** ***********************************************************************/
//    void removeMatches(IExtension *ext) {
//        QMutexLocker locker(&mutex_);
//        if (dynamicSort_) {
//            //beginremoveRows(...);
//            throw "Not implemented yet.";
//        } else {
//            std::remove_if(matches_.begin(), matches_.end(),
//                           [&](const Match &m){ return m.item.data()->extension()==ext; });
//        }
//    }



    /** ***********************************************************************/
    void reset() {
        QMutexLocker locker(&mutex_);
        if (dynamicSort_) {
            beginResetModel();
            throw "Not implemented yet.";
            endResetModel();
        } else {
            isValid_ = false;
            matches_.clear();
        }
    }



    /** ***********************************************************************/
    void setValid(bool b = true) {
        isValid_ = b;
    }



    /** ***********************************************************************/
    bool isValid() {
        return isValid_;
    }



    /** ***********************************************************************/
    const QString &searchTerm() const {
        return searchTerm_;
    }



    /** ***********************************************************************/
    void makeDynamic() {
        QMutexLocker locker(&mutex_);
        beginResetModel();
        std::stable_sort(matches_.begin(), matches_.end(),
                         [](const Match &lhs, const Match &rhs) {
                            return lhs.score > rhs.score;
                         });
        endResetModel();
        dynamicSort_ = true;
    }




    /** ***********************************************************************/
    int rowCount(const QModelIndex &) const override {
        return static_cast<int>(matches_.size());
    }


    /** ***********************************************************************/
    QVariant data(const QModelIndex & index, int role) const override {
         if (index.isValid()) {
            ItemSPtr item = matches_[index.row()].item;
            switch (role) {
            case Roles::Text:
                return item->text();
            case Roles::SubText:
                return item->subtext();
            case Roles::IconPath:
                return item->iconPath();
            case Roles::Actions: {
                QStringList actionTexts;
                for (ActionSPtr &action : item->actions())
                    actionTexts.append(action->text());
                return actionTexts;
            }
            default:
                return QVariant();
            }
        }
        return QVariant();
    }



    /** ***********************************************************************/
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (index.isValid()) {
            ItemSPtr item = matches_[index.row()].item;
            switch (role) {
            case Roles::Activate: {
                int actionValue = value.toInt();
                executeAction(item, actionValue);
                return true;
            }
            default:
                return false;
            }
        }
        return false;
    }



    /** ***********************************************************************/
    void executeAction(shared_ptr<AlbertItem> item, int actionValue) const {
        Action::ExecutionFlags flags;
        if (0 <= actionValue && actionValue < static_cast<int>(item->actions().size()))
            item->actions()[actionValue]->activate(&flags);
        else
            item->activate(&flags);

        if (flags.hideWidget)
            qApp->hideWidget();

        if (flags.clearInput)
            qApp->clearInput();
    }

private:
    QString searchTerm_;
    std::vector<Match> matches_;
    bool isValid_;
    QMutex mutex_;
    bool dynamicSort_;
};


