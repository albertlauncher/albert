// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QObject>
#include <vector>
#include <memory>
class QueryEngine;
class QueryExecution;
namespace albert::detail {
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
    std::vector<std::unique_ptr<QueryExecution>> queries_;

};

