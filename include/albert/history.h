// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QObject>
#include <QStringList>
#include "export.h"

namespace albert
{
class ALBERT_EXPORT History final : public QObject  /// Input line history for frontends.
{
    Q_OBJECT
public:
    History(QObject *parent = nullptr);
    Q_INVOKABLE void add(const QString& str);
    Q_INVOKABLE QString next(const QString &substring = QString{});
    Q_INVOKABLE QString prev(const QString &substring = QString{});
    Q_INVOKABLE void resetIterator();
private:
    QStringList lines_;
    int currentLine_;
};
}
