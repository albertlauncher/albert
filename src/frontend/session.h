// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QObject>
#include <vector>
#include <memory>
class QueryEngine;
namespace albert::detail {
class Query;
class Frontend;
}

class Session : public QObject
{
    Q_OBJECT

public:

    Session(QueryEngine &engine, albert::detail::Frontend &frontend);
    ~Session();

private:

    void runQuery(const QString &query);

    QueryEngine &engine_;
    albert::detail::Frontend &frontend_;
    std::vector<std::unique_ptr<albert::detail::Query>> queries_;

};

