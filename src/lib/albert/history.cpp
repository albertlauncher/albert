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

void albert::History::add(const QString& str)
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
    int new_current_line = currentLine_;
    while (++new_current_line < static_cast<int>(lines_.size())){
        const QString &line = lines_[new_current_line];
        if (line.contains(substring, Qt::CaseInsensitive)){
            currentLine_ = new_current_line;
            return line;
        }
    }
    return QString{};
}

QString albert::History::prev(const QString &substring)
{
    int new_current_line = currentLine_;
    while (-1 < --new_current_line){
        const QString &line = lines_[new_current_line];
        if (line.contains(substring, Qt::CaseInsensitive)){
            currentLine_ = new_current_line;
            return line;
        }
    }
    return QString{};
}

void albert::History::resetIterator()
{
    currentLine_ = -1;
}
