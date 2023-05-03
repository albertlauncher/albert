// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../extension.h"
#include "item.h"
#include <QStringList>
#include <functional>
#include <memory>
#include <vector>
class QueryEngine;
class GlobalQueryHandlerPrivate;
class IndexQueryHandlerPrivate;

namespace albert
{

///////////////////////////////////////////////////////////////////////////////////////////////////


/// Fallback providing extension
class ALBERT_EXPORT FallbackHandler : virtual public Extension
{
public:
    /// Fallbacks provided by this handler
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////


/// Exclusive/Triggered only query handler.
/// Use this if you dont want your results to be rearranged
/// or if the query takes too much time to be in the global search.
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
{
public:
    class TriggerQuery
    {
    public:
        virtual ~TriggerQuery();
        virtual const QString &trigger() const = 0;  ///< The trigger of this query
        virtual const QString &string() const = 0;  ///< The query string
        virtual bool isValid() const = 0;  ///< True if query has not been cancelled
        virtual void add(const std::shared_ptr<Item> &item) = 0;  ///< Copy add item
        virtual void add(std::shared_ptr<Item> &&item) = 0;  ///< Move add item
        virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;  ///< Copy add items
        virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  ///< Move add items
    };

    virtual QString synopsis() const;  ///< The synopsis, displayed on empty query. Default empty.
    virtual QString defaultTrigger() const;  ///< The default (not user defined) trigger. Default Extension::id().
    virtual bool allowTriggerRemap() const;  ///< Enable user remapping of the trigger. Default false.
    virtual void handleTriggerQuery(TriggerQuery &) const = 0;  ///< Called on triggered query.
};


///////////////////////////////////////////////////////////////////////////////////////////////////


class ALBERT_EXPORT RankItem
{
public:
    using Score = uint16_t;
    static const constexpr Score MAX_SCORE = std::numeric_limits<Score>::max();
    RankItem(std::shared_ptr<Item> item, Score score);
    std::shared_ptr<Item> item;  ///< The matched item
    Score score;  ///< The match score. @note MAX_SCORE represents a full match.
};


/// Global search query handler.
/// Use this if you want your results appear in the global, untriggered search.
/// @note Inherits TriggeredQueryHandler, therefore this extension also handles triggers.
/// @note Do _not_ use this for long running tasks! @see TriggeredQueryHandler
class ALBERT_EXPORT GlobalQueryHandler : virtual public Extension
{
public:
    GlobalQueryHandler();
    ~GlobalQueryHandler() override;

    class GlobalQuery
    {
    public:
        virtual ~GlobalQuery();
        virtual const QString &string() const = 0;  ///< The query string
        virtual bool isValid() const = 0;  ///< True if query has not been cancelled
    };

    /// The query handling function. Subclasses should return matched items with appropriate match scores.
    /// The match score should make sense and often (if not always) the fraction of the string match makes sense.
    /// @note has to be thread safe!
    virtual std::vector<RankItem> handleGlobalQuery(const GlobalQuery&) const = 0;

protected:
    std::unique_ptr<GlobalQueryHandlerPrivate> d;
    friend class ::QueryEngine;
};


///////////////////////////////////////////////////////////////////////////////////////////////////


/// Convenience GlobalQueryHandler providing generic TriggerQueryHandler
class ALBERT_EXPORT QueryHandler : public GlobalQueryHandler, public TriggerQueryHandler
{
protected:
    /// Provides triggered query handling
    /// @implements QueryHandler::handleQuery
    /// @note Override this if handlers should behave differently when triggered
    void handleTriggerQuery(TriggerQuery &) const override;

};


///////////////////////////////////////////////////////////////////////////////////////////////////

class ALBERT_EXPORT IndexItem
{
public:
    IndexItem(std::shared_ptr<Item> item, QString string);
    std::shared_ptr<Item> item; ///< The item
    QString string; ///< The corresponding string
};


/// Global search index query handler. This is a GlobalQueryHandler which does indexing and
/// matching for you. You just have to provide your items with lookup strings.
class ALBERT_EXPORT IndexQueryHandler : public QueryHandler
{
public:
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    /// Update the index. Called when the index needs to be updated (or probably by yourself if
    /// your items changed), i.e. whenever the user made changes to the index config or initially
    /// on creation. Therefore…
    /// @note You dont have to call this in you constructor. It will be called after construction.
    /// @see void IndexQueryHandler::setIndexItems(std::vector<IndexItem>&&)
    virtual void updateIndexItems() = 0;

protected:
    /// Set the items of the index. You should probably call this in updateIndexItems().
    void setIndexItems(std::vector<IndexItem>&&);

    /// @implements GlobalQueryHandler::handleQuery
    /// Uses the index to find items
    std::vector<RankItem> handleGlobalQuery(const GlobalQuery &) const override final;

private:
    std::unique_ptr<IndexQueryHandlerPrivate> d;
    friend class ::QueryEngine;
};

}
