// Copyright (c) 2022-2024 Manuel Schneider

#include "inputhistory.h"
#include "logging.h"
#include "util.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>
using namespace albert;
using namespace std;

class InputHistory::Private
{
public:
    QString file_path;
    QStringList lines_;
    int currentLine_;
};


InputHistory::InputHistory(const QString &path):
    d(make_unique<Private>())
{
    if (path.isEmpty())
        d->file_path = QDir(albert::dataLocation()).filePath("albert.history");
    else
        d->file_path = path;

    if (QFile f(d->file_path); f.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&f);
        while (!ts.atEnd())
            d->lines_ << ts.readLine();
        f.close();
    }

    resetIterator();
}

InputHistory::~InputHistory()
{
    if (QFile f(d->file_path); f.open(QIODevice::WriteOnly))
    {
        QTextStream ts(&f);
        for (const auto &line : d->lines_)
            ts << line << Qt::endl;
        f.close();
    }
    else
        WARN << "Writing history file failed:" << d->file_path;
}

void InputHistory::add(const QString& s)
{
    if (!s.isEmpty())
    {
        if (d->lines_.contains(s))
            d->lines_.removeAll(s); // Remove dups
        d->lines_ << s;
    }
    resetIterator();
}

QString InputHistory::next(const QString &substring)
{
    for (int l = d->currentLine_ - 1; 0 <= l; --l)
        // Simple hack to avoid the seemingly-noop-on-first-history-iteration on disabled clear-on-hide
        if (d->lines_[l].contains(substring, Qt::CaseInsensitive) && substring != d->lines_[l])
            return d->lines_[d->currentLine_ = l];
    return QString{};
}

QString InputHistory::prev(const QString &substring)
{
    for (int l = d->currentLine_ + 1; l < (int)d->lines_.size(); ++l)
        if (d->lines_[l].contains(substring, Qt::CaseInsensitive))
            return d->lines_[d->currentLine_ = l];
    return QString{};
}

void InputHistory::resetIterator()
{
    d->currentLine_ = (int)d->lines_.size();
}
