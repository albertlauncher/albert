// SPDX-FileCopyrightText: 2025-2026 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <memory>
#include <chrono>

namespace albert::detail
{

class ALBERT_EXPORT Acquire : public QObject
{
    Q_OBJECT
public:



signals:
    void granted();
};



class ALBERT_EXPORT RateLimiter : public QObject
{
    Q_OBJECT
public:
    RateLimiter();
    ~RateLimiter() override;

    void limit(std::chrono::milliseconds ms);
    bool isLimited() const;

    std::unique_ptr<Acquire> acquire();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace albert::detail
