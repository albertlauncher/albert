// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "inputhistory.h"
#include "logging.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <ranges>
using namespace albert::detail;
using namespace albert;
using namespace std;

class InputHistory::Private
{
public:
    QString file_path;
    QStringList history;
    qsizetype current;
    uint max = -1;  // -1: max unsigned int
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
        const auto doc = QJsonDocument::fromJson(f.readAll());

        if (doc.isArray())
        {
            const auto a = doc.array();
            d->history.reserve(a.size());
            for (const auto v : a | views::reverse)
                d->history << v.toString();
        }
        f.close();
    }

    resetIterator();
}

InputHistory::~InputHistory()
{
    if (QFile f(d->file_path); f.open(QIODevice::WriteOnly))
    {
        QJsonArray array;
        for (const auto& line : d->history | views::reverse)
            array.append(line);

        const QJsonDocument doc(array);
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
    }
    else
        WARN << "Writing history file failed:" << d->file_path;
}

void InputHistory::add(const QString& s)
{
    if (!s.isEmpty())
    {
        d->history.prepend(s);
        d->history.removeDuplicates();
        if (d->history.size() > d->max)
            d->history.resize(d->max);
    }
    resetIterator();
}

QString InputHistory::next(const QString &substring)
{
    while(true)
    {
        if (d->current == d->history.size() - 1)  // at end
            return {};

        if (const auto current_string = d->history.at(++d->current);
            current_string != substring  // skip if equals search string
            && current_string.contains(substring, Qt::CaseInsensitive))  // skip if no match
            return current_string;
    }
}

QString InputHistory::prev(const QString &substring)
{
    while(true)
    {
        if (d->current == 0)  // prev on first item: reset.
            d->current = -1;

        if (d->current == -1)  // at end
            return {};

        if (const auto current_string = d->history.at(--d->current);  // has been decremented above
            current_string != substring  // skip if equals search string
            && current_string.contains(substring, Qt::CaseInsensitive))  // skip if no match
            return current_string;
    }
}

void InputHistory::resetIterator() { d->current = -1; }

void InputHistory::clear()
{
    d->history.clear();
    resetIterator();
}

uint InputHistory::limit() const { return d->max; }

void InputHistory::setLimit(uint v)
{
    if (v != d->max)
    {
        d->max = v;

        if (d->history.size() > d->max)
            d->history.resize(d->max);
    }
}
