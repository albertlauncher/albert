// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"
class QueryEngine;

namespace albert
{

class ALBERT_EXPORT IndexItem
{
public:
    IndexItem(std::shared_ptr<Item> item, QString string);
//    IndexItem(IndexItem&&) = default;
//    IndexItem(const IndexItem&) = delete;
//    IndexItem& operator=(IndexItem&&) = default;
//    IndexItem& operator=(const IndexItem&) = delete;

    std::shared_ptr<Item> item; /// The item
    QString string; /// The corresponding string
};


class Index;

/// Global index query handler
/// Provides an indexed search for static item providers
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    /// Return the index items.
    /// Its okay to return the same item multiple times.
    /// This method is called when the index is built. @see updateIndex()
    virtual std::vector<IndexItem> indexItems() const = 0;

protected:
    void updateIndex();  /// Call this when your items changed
    QString synopsis() const override;
    std::vector<RankItem> rankItems(const QString &string, const bool& isValid) const final;
private:
    void setIndex(std::unique_ptr<Index>&&);
    std::unique_ptr<Index> index_;
    friend class ::QueryEngine;
};

}
