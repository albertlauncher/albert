// Copyright (c) 2022 Manuel Schneider

#include "indexqueryhandlerprivate.h"
#include "itemindex.h"
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

albert::IndexQueryHandler::Private::Private(IndexQueryHandler *q) : q(q)
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

albert::IndexQueryHandler::Private::~Private()
{

}


ItemIndex *albert::IndexQueryHandler::Private::index() { return index_.get(); }

void albert::IndexQueryHandler::Private::updateIndex() {
    if (futureWatcher.isRunning())
        rerun = true;
    else
        futureWatcher.setFuture(QtConcurrent::run([this]()->ItemIndex*{
            return new ItemIndex(q->indexItems(), separators_, case_sensitive_, n_, error_tolerance_divisor_);
        }));
}

const QString &albert::IndexQueryHandler::Private::separators() const { return separators_; }

void albert::IndexQueryHandler::Private::setSeparators(const QString &val) {
    q->settings()->setValue(CFG_SEPARATORS, separators_ = val);
    updateIndex();
}

bool albert::IndexQueryHandler::Private::caseSensitive() const { return case_sensitive_; }

void albert::IndexQueryHandler::Private::setCaseSensitive(bool val) {
    q->settings()->setValue(CFG_SEPARATORS, case_sensitive_ = val);
    updateIndex();
}

uint albert::IndexQueryHandler::Private::fuzzy() const { return error_tolerance_divisor_; }

void albert::IndexQueryHandler::Private::setFuzzy(uint val) {
    q->settings()->setValue(CFG_ERROR_TOLERANCE_DIVISOR, error_tolerance_divisor_ = val);
    updateIndex();
}

bool albert::IndexQueryHandler::Private::global() const { return global_search_; }

void albert::IndexQueryHandler::Private::setGlobal(bool val) {
    q->settings()->setValue(CFG_ERROR_TOLERANCE_DIVISOR, global_search_ = val);
}
