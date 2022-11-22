// Copyright (c) 2022 Manuel Schneider

#include "albert/util/history.h"
#include "usagehistory.h"
using namespace albert;

History::History(QObject *parent) : QObject(parent)
{
    lines_ = UsageHistory::inputHistory();
    lines_.removeAll("");
    for (const auto& l : lines_)
    resetIterator();
}

/// Temporarily add a line to the history. @note Will vanish on next reset
void History::add(const QString& str)
{
    if (!str.isEmpty()){
        if (lines_.contains(str))
            lines_.removeAll(str); // Remove dups
        lines_.prepend(str);
    }
    resetIterator();
}

QString History::next(const QString &substring)
{
    for (int l = currentLine_ + 1; l < (int)lines_.size(); ++l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

QString History::prev(const QString &substring)
{
    for (int l = currentLine_ - 1; 0 <= l; --l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

void History::resetIterator()
{
    currentLine_ = -1;
}
