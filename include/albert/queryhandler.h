// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include "query.h"
#include <QString>

namespace albert
{

struct ALBERT_EXPORT QueryHandler : virtual public Extension
{
    virtual QString synopsis() const { return {}; }  /// The synopsis, displayed on empty query
    virtual QString default_trigger() const { return id(); }  /// The default (not user defined) trigger
    virtual bool allow_trigger_remap() const { return true; }  /// Enable user remapping of the trigger
    virtual void handleQuery(albert::Query &query) const = 0;  /// The query handling function
};


struct Match
{
    SharedItem item;
    using Score = uint8_t;
    Score score;
};


struct ALBERT_EXPORT BatchQueryHandler : public QueryHandler
{
    virtual std::vector<Match> batchHandleQuery(const albert::Query &query) const = 0;
    void handleQuery(albert::Query &query) const override final;
};


struct ALBERT_EXPORT IndexQueryHandler : virtual public BatchQueryHandler
{
    virtual std::map<albert::SharedItem,std::map<QString,uint8_t>> indexItems() const = 0;  // Item>(String>Score)
    void updateIndex();
    // SortQueryHandler
    QString synopsis() const override;
    std::vector<Match> batchHandleQuery(const albert::Query &query) const override;
protected:
    IndexQueryHandler();
    ~IndexQueryHandler();
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}


