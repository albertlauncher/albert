// Copyright (c) 2022 Manuel Schneider

#include "albert/history.h"
#include "usagehistory.h"
#include <QStringList>
#include <QVariant>

albert::History::History(QObject *parent) : QObject(parent)
{
    lines_ = UsageHistory::inputHistory();
    currentLine_ = -1; // This means historymode is not active
}

void albert::History::add(QString str)
{
    if (!str.isEmpty()){
        if (lines_.contains(str))
            lines_.removeAll(str); // Remove dups
        lines_.prepend(str);
    }
    resetIterator();
}

QString albert::History::next(const QString &substring)
{
    int newCurrentLine = currentLine_;
    while (++newCurrentLine < static_cast<int>(lines_.size())){
        const QString &line = lines_[newCurrentLine];
        if (line.contains(substring, Qt::CaseInsensitive)){
            currentLine_ = newCurrentLine;
            return line;
        }
    }
    return QString{};
}

QString albert::History::prev(const QString &substring)
{
    int newCurrentLine = currentLine_;
    while (-1 < --newCurrentLine){
        const QString &line = lines_[newCurrentLine];
        if (line.contains(substring, Qt::CaseInsensitive)){
            currentLine_ = newCurrentLine;
            return line;
        }
    }
    return QString{};
}

void albert::History::resetIterator()
{
    currentLine_ = -1;
}
