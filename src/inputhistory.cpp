// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/frontend/inputhistory.h"
#include "albert/logging.h"
#include <QFile>
#include <QTextStream>
using namespace albert;

InputHistory::InputHistory(const QString& p) : file_path(p)
{
    QFile f(file_path);
    if (f.open(QIODevice::ReadOnly)){
        QTextStream ts(&f);
        while (!ts.atEnd())
            lines_ << ts.readLine();
        f.close();
    } else
        WARN << "Opening history file failed:" << file_path;
    resetIterator();
}

InputHistory::~InputHistory()
{
    QFile f(file_path);
    if (f.open(QIODevice::WriteOnly)){
        QTextStream ts(&f);
        for (const auto &line : lines_)
            ts << line << Qt::endl;
        f.close();
    } else
        WARN << "Opening history file failed:" << file_path;
}

void InputHistory::add(const QString& str)
{
    if (!str.isEmpty()){
        if (lines_.contains(str))
            lines_.removeAll(str); // Remove dups
        lines_ << str;
    }
    resetIterator();
}

QString InputHistory::next(const QString &substring)
{
    for (int l = currentLine_ - 1; 0 <= l; --l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

QString InputHistory::prev(const QString &substring)
{
    for (int l = currentLine_ + 1; l < (int)lines_.size(); ++l)
        if (lines_[l].contains(substring, Qt::CaseInsensitive))
            return lines_[currentLine_ = l];
    return QString{};
}

void InputHistory::resetIterator()
{
    currentLine_ = (int)lines_.size();
}
