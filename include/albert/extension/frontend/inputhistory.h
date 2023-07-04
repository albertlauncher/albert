// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QStringList>

namespace albert
{
class ALBERT_EXPORT InputHistory final : public QObject  /// Input line history for frontends.
{
    Q_OBJECT
public:
    explicit InputHistory(const QString& file_path);
    ~InputHistory() override;

    Q_INVOKABLE void add(const QString& str);
    Q_INVOKABLE QString next(const QString &substring = QString{});
    Q_INVOKABLE QString prev(const QString &substring = QString{});
    Q_INVOKABLE void resetIterator();

private:
    QString file_path;
    QStringList lines_;
    int currentLine_;
};
}
