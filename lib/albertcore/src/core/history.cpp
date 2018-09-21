// Copyright (C) 2014-2018 Manuel Schneider

#include <QStringList>
#include <QVariant>
#include <QSqlQuery>
#include "history.h"


Core::History::History(QObject *parent) : QObject(parent) {
    QSqlQuery query("SELECT input "
                    "FROM activation a JOIN  query q ON a.query_id = q.id "
                    "GROUP BY input  "
                    "ORDER BY max(timestamp) DESC;");
    while (query.next())
        lines_.append(query.value(0).toString());

    currentLine_ = -1; // This means historymode is not active
}


/** ***************************************************************************/
void Core::History::add(QString str) {
    if (!str.isEmpty()){
        if (lines_.contains(str))
            lines_.removeAll(str); // Remove dups
        lines_.prepend(str);
    }
}


/** ***************************************************************************/
QString Core::History::next() {
    if (currentLine_+1 < static_cast<int>(lines_.size())
            && static_cast<int>(lines_.size())!=0 ) {
        ++currentLine_;
        return lines_[currentLine_];
    } else return QString();
}


/** ***************************************************************************/
QString Core::History::prev() {
    if (0 < currentLine_) {
        --currentLine_;
        return lines_[currentLine_];
    } else return QString();
}


/** ***************************************************************************/
void Core::History::resetIterator() {
    currentLine_ = -1;
}
