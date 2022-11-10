// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"

namespace albert
{
class Index;

/// Provides an indexed item search
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    QString synopsis() const override;  /// Overwrite default to '<filter>'
    std::vector<RankItem> rankItems(const Query &query) const final;  /// Queries and returns index items
    void setIndex(std::unique_ptr<Index>&&);  /// Call this when your items changed

protected:
    virtual std::vector<IndexItem> indexItems() const = 0;  // Return your items. Needs to be thread safe.
    void updateIndex();  /// Call this when your items changed

private:
    std::unique_ptr<Index> index_;
};

}
