// Copyright (c) 2022-2024 Manuel Schneider

#include "albert.h"
#include "inputhistory.h"
#include "logging.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>
using namespace albert;
using namespace std;
using namespace util;

class InputHistory::Private
{
public:
    QString file_path;
    QStringList history;
    qsizetype current;
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
            d->history << ts.readLine();
        f.close();
    }

    resetIterator();
}

InputHistory::~InputHistory()
{
    if (QFile f(d->file_path); f.open(QIODevice::WriteOnly))
    {
        QTextStream ts(&f);
        for (const auto &line : d->history)
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
        if (d->history.contains(s))
            d->history.removeAll(s); // Remove dups
        d->history << s;
    }
    resetIterator();
}

QString InputHistory::next(const QString &substring)
{
    // already at end or no history
    if (d->history.isEmpty() || d->current < 0)
        return {};

    while(0 <= --d->current)
    {
        const auto &c = d->history[d->current];
        if (substring != c  // avoid effective noop on disabled clear-on-hide
            && c.contains(substring, Qt::CaseInsensitive))
            return c;
    }

    return {};
}

QString InputHistory::prev(const QString &substring)
{
    // already at end or no history
    if (d->history.isEmpty() || d->current >= d->history.size())
        return {};

    while(++d->current < d->history.size())
    {
        const auto &c = d->history[d->current];
        if (c.contains(substring, Qt::CaseInsensitive))
            return c;
    }

    return {};
}

void InputHistory::resetIterator()
{
    d->current = (int)d->history.size();
}

void InputHistory::clear()
{
    d->history.clear();
    resetIterator();
}
