// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QStringList>
#include "../core_globals.h"

namespace Core {

class EXPORT_CORE History final : public QObject
{
    Q_OBJECT

public:

    History(QObject *parent = nullptr);

    Q_INVOKABLE void add(QString str);
    Q_INVOKABLE QString next(const QString &substring = QString{});
    Q_INVOKABLE QString prev(const QString &substring = QString{});
    Q_INVOKABLE void resetIterator();

private:

    QStringList lines_;
    int currentLine_;

};

}
