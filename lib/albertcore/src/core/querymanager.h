// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QAbstractItemModel>
#include <memory>
#include <list>

namespace Core {

class ExtensionManager;
class QueryExecution;

class QueryManager final : public QObject
{
    Q_OBJECT

public:

    QueryManager(ExtensionManager* em, QObject *parent = 0);
    ~QueryManager();

    void setupSession();
    void teardownSession();
    void startQuery(const QString &searchTerm);

    bool incrementalSort();
    void setIncrementalSort(bool value);

private:

    void updateScores();

    ExtensionManager *extensionManager_;
    std::list<QueryExecution*> pastQueries_;
    bool incrementalSort_;
    std::map<QString,uint> scores_;
    std::map<QString, unsigned long long> handlerIds_;
    unsigned long long lastQueryId_;

signals:

    void resultsReady(QAbstractItemModel*);
};

}
