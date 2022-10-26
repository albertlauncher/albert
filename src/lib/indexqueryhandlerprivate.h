// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "itemindex.h"
#include "indexqueryhandler.h"
#include <QtConcurrent>
#include <QFutureWatcher>
using namespace std;
const char* CFG_GLOBAL_SEARCH = "global";
const bool DEF_GLOBAL_SEARCH = true;
const char* CFG_ERROR_TOLERANCE_DIVISOR = "error_tolerance_divisor";
const uint DEF_ERROR_TOLERANCE_DIVISOR = 3;
const char* CFG_CASE_SENSITIVE = "case_sensitive";
const bool  DEF_CASE_SENSITIVE = false;
const char* CFG_SEPARATORS = "separators";
const char* DEF_SEPARATORS = R"R([!?<>"'=+*.:,;\\\/ _\-]+)R";
const uint GRAM_SIZE = 2;

struct albert::IndexQueryHandler::Private : public QObject
{
    Q_OBJECT
private:
    IndexQueryHandler *q;
    
    unique_ptr<ItemIndex> index_;
    
    QString separators_;
    bool case_sensitive_;
    uint n_;
    uint error_tolerance_divisor_;
    bool global_search_;
    
    QFutureWatcher<ItemIndex*> futureWatcher;
    bool rerun = false;

public:
    Private(IndexQueryHandler *q) : q(q)
    {
        auto s = q->settings();

        global_search_ = s->value(CFG_GLOBAL_SEARCH, DEF_GLOBAL_SEARCH).toBool();
        error_tolerance_divisor_ = s->value(CFG_ERROR_TOLERANCE_DIVISOR, DEF_ERROR_TOLERANCE_DIVISOR).toBool();
        case_sensitive_ = s->value(CFG_CASE_SENSITIVE, DEF_CASE_SENSITIVE).toBool();
        separators_ = s->value(CFG_SEPARATORS, DEF_SEPARATORS).toString();
        n_ = GRAM_SIZE;

        index_.reset(new ItemIndex);

        // Fetch results in main thread
        QObject::connect(&futureWatcher, &decltype(futureWatcher)::finished, this, [this](){
            index_.reset(futureWatcher.future().result());
            if (rerun){
                rerun = false;
                updateIndex();
            }
        });

    }

    ItemIndex *index() { return index_.get(); }

    void updateIndex()
    {
        if (futureWatcher.isRunning())
            rerun = true;
        else
            futureWatcher.setFuture(QtConcurrent::run([this]()->ItemIndex*{
                return new ItemIndex(q->items(), separators_, case_sensitive_, n_, error_tolerance_divisor_);
            }));
    }

    const QString &separators() const { return separators_; }
    void setSeparators(const QString &val)
    {
        q->settings()->setValue(CFG_SEPARATORS, separators_ = val);
        updateIndex();
    }

    bool caseSensitive() const { return case_sensitive_; }
    void setCaseSensitive(bool val)
    {
        q->settings()->setValue(CFG_SEPARATORS, case_sensitive_ = val);
        updateIndex();
    }

    uint fuzzy() const { return error_tolerance_divisor_; }
    void setFuzzy(uint val)
    {
        q->settings()->setValue(CFG_ERROR_TOLERANCE_DIVISOR, error_tolerance_divisor_ = val);
        updateIndex();
    }

    bool global() const { return global_search_; }
    void setGlobal(bool val)
    {
        q->settings()->setValue(CFG_ERROR_TOLERANCE_DIVISOR, global_search_ = val);
    }


};