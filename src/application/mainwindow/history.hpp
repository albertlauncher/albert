// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <QObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QStringList>
#include <QDataStream>

class History final : public QObject
{
    Q_OBJECT
public:
    explicit History(QObject *parent = 0) : QObject(parent) {
        currentLine_ = -1; // This means historymode is not active
    }

    Q_INVOKABLE void add(QString str) {
        if (!str.isEmpty()){
            if (lines_.contains(str))
                lines_.removeAll(str); // Remove dups
            lines_.prepend(str);
        }
    }

    Q_INVOKABLE QString next() {
        // Update the history at the beginnig
        if (currentLine_ == -1)
            updateHistory();

        if (currentLine_+1 < static_cast<int>(lines_.size())
                && static_cast<int>(lines_.size())!=0 ) {
            ++currentLine_;
            return lines_[currentLine_];
        } else return QString();
    }

    Q_INVOKABLE QString prev() {
        if (0 < currentLine_) {
            --currentLine_;
            return lines_[currentLine_];
        } else return QString();
    }

public slots:

    void resetIterator() {currentLine_ = -1;}

private:

    void updateHistory() {
        lines_.clear();
        QSqlQuery query;
        query.exec("SELECT input FROM usages GROUP BY input ORDER BY max(timestamp) DESC");
        while (query.next())
            lines_.append(query.value(0).toString());
    }

    QStringList lines_;
    int currentLine_;

};

