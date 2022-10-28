// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "indexqueryhandler.h"
#include <QtConcurrent>
#include <QFutureWatcher>
class ItemIndex;

struct albert::IndexQueryHandler::Private : public QObject
{
    Q_OBJECT
private:
    IndexQueryHandler *q;
    
    std::unique_ptr<ItemIndex> index_;
    
    QString separators_;
    bool case_sensitive_;
    uint n_;
    uint error_tolerance_divisor_;
    bool global_search_;
    
    QFutureWatcher<ItemIndex*> futureWatcher;
    bool rerun = false;

public:
    Private(IndexQueryHandler *q);
    ~Private();

    ItemIndex *index();

    void updateIndex();

    const QString &separators() const;
    void setSeparators(const QString &val);

    bool caseSensitive() const;
    void setCaseSensitive(bool val);

    uint fuzzy() const;
    void setFuzzy(uint val);

    bool global() const;
    void setGlobal(bool val);
};