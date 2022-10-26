// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QObject>
#include <QString>
#include <vector>
#include <memory>

namespace albert
{
class ALBERT_EXPORT Query: public QObject
{
    Q_OBJECT

public:
    const QString &trigger() const;  /// The trigger of this query.
    const QString &string() const;  /// Query string _excluding_ the trigger.

    bool isValid() const;  /// Core cancelled the query. Stop processing.
    bool isFinished() const;    /// Asynchronous execution done.

    const SharedItemVector &results() const;
    const SharedItemVector &fallbacks() const;

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
                // Must not use operator->() !!! dereferencing a pointer returns an lvalue
                add_(*begin);
            emit resultsChanged();
        }
    }

    /// Set results by std::move
    void set(std::vector<SharedItem> && items);

    void activateResult(uint item, uint action);
    void activateFallback(uint item, uint action);

signals:
    void resultsChanged();
    void finished();

protected:
    void add_(const SharedItem &item);
    void add_(SharedItem &&item);

    QString trigger_;
    QString string_;
    std::vector<albert::SharedItem> results_;
    std::vector<albert::SharedItem> fallbacks_;
    bool valid_ = true;
    bool finished_ = false;
};
}


