// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/querycontext.h>
#include <albert/queryresults.h>
#include <albert/usagescoring.h>
#include <memory>

namespace albert
{
class QueryHandler;
class QueryExecution;

namespace detail
{

class ALBERT_EXPORT Query : public QObject
{
    Q_OBJECT
public:
    Query(const QString &trigger,
          const QString &query,
          QueryHandler &handler,
          std::vector<QueryResult> fallbacks,
          UsageScoring usage_scoring);
    ~Query();

    QString trigger() const;
    QString query() const;
    QString synopsis() const;

    QueryHandler &handler() const;

    bool isValid() const;
    bool isActive() const;
    bool canFetchMore() const;
    void fetchMore();
    void cancel();

    QueryResults &matches();
    QueryResults &fallbacks();

signals:
    void activeChanged(bool active);

private:
    uint id_;
    QString trigger_;
    QString query_;
    QString synopsis_;
    std::atomic_bool valid_;
    UsageScoring usage_scoring_;
    QueryHandler &handler_;
    QueryResults fallbacks_;
    std::unique_ptr<QueryExecution> execution_;
    bool is_active_;

    friend class albert::QueryContext;
};

}  // namespace albert::detail
}  // namespace albert
