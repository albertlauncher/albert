// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/frontend/inputhistory.h"
#include "albert/logging.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
using namespace albert;

InputHistory::InputHistory() : file_path(QDir(albert::dataLocation()).filePath("albert.history"))
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
    }
}

void InputHistory::add(const QString& str)
{
    if (auto trimmed = str.trimmed(); !trimmed.isEmpty()){
        if (lines_.contains(trimmed))
            lines_.removeAll(trimmed); // Remove dups
        lines_ << trimmed;
    }
    resetIterator();
}

QString InputHistory::next(const QString &substring)
{
    for (int l = currentLine_ - 1; 0 <= l; --l)
        // Simple hack to avoid the seemingly-noop-on-first-history-iteration on disabled clear-on-hide
        if (lines_[l].contains(substring, Qt::CaseInsensitive) && substring != lines_[l])
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
