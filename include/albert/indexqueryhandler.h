// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/globalqueryhandler.h>
#include <albert/indexitem.h>
#include <memory>
#include <vector>

namespace albert
{

/// Index query handler class.
/// A GlobalQueryHandler providing implicit indexing and matching.
/// You just have to provide your items with lookup strings.
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    IndexQueryHandler();

    /// Returns "True"
    bool supportsFuzzyMatching() const override;

    /// Set the fuzzy mode of the internal index.
    /// Triggers a rebuild by calling updateIndexItems.
    void setFuzzyMatching(bool) override;

    /// Uses the index to override GlobalQueryHandler::handleGlobalQuery
    std::vector<RankItem> handleGlobalQuery(const Query*) override;

    /// Update the index.
    /// Called when the index needs to be updated, i.e. for initialization
    /// and on user changes to the index config (fuzzy, etc…) and probably by
    /// the client itself if the items changed. This function should call
    /// setIndexItems(std::vector<IndexItem>&&) to update the index.
    /// @note Don't call this method in the constructor. It will be called on plugin
    /// initialization.
    virtual void updateIndexItems() = 0;

    /// Set the items of the index.
    /// Call this in updateIndexItems().
    /// @threadsafe
    void setIndexItems(std::vector<IndexItem>&&);

protected:

    ~IndexQueryHandler() override;

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
