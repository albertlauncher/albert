// Copyright (c) 2022 Manuel Schneider

#include "albert/history.h"
#include "usagehistory.h"
#include <QStringList>
#include <QVariant>

albert::History::History(QObject *parent) : QObject(parent)
{
    lines_ = UsageHistory::inputHistory();
    for (auto l : lines_)
        qDebug() << l;
    resetIterator();
}

/// Temporarily add a line to the history. @note Will vanish on next reset
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
    for (int l = currentLine_ + 1; l < (int)lines_.size(); ++l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

QString albert::History::prev(const QString &substring)
{
    for (int l = currentLine_ - 1; 0 <= l; --l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

void albert::History::resetIterator()
{
    currentLine_ = -1;
}
