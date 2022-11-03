// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QObject>
#include <QString>
#include <vector>
#include <memory>

namespace albert
{
class Item;

class ALBERT_EXPORT Query: public QObject
{
    Q_OBJECT

public:
    [[nodiscard]] const QString &trigger() const;  /// The trigger of this query.
    [[nodiscard]] const QString &string() const;  /// Query string _excluding_ the trigger.

    [[nodiscard]] bool isValid() const;  /// Core cancelled the query. Stop processing.
    [[nodiscard]] bool isFinished() const;    /// Asynchronous execution done.

    [[nodiscard]] const std::vector<std::shared_ptr<Item>> &results() const;

    /// Add item (perfect forward)
    template<typename T>
    void add(T&& item) {
        if (isValid()) {
            add_(std::forward<T>(item));
            emit resultsChanged();
        }
    }

    /// Add range (perfect forward, use make_move_iterator whenever possible)
    template<typename Iterator>
    void add(Iterator begin, Iterator end) {
        if (isValid()) {
            for (; begin != end; ++begin)
                add_(*begin);
            emit resultsChanged();
        }
    }

    /// Set results by std::move
    /// @note This is _not_ intended to reset the items, but for fast move. NOP if results not empty.
    void set(std::vector<std::shared_ptr<Item>> && items);

    void activateResult(uint item, uint action);

signals:
    void resultsChanged();
    void finished();

protected:
    void add_(const std::shared_ptr<Item> &item);
    void add_(std::shared_ptr<Item> &&item);

    QString trigger_;
    QString string_;
    std::vector<std::shared_ptr<Item>> results_;
    bool valid_ = true;
    bool finished_ = false;
};
}


