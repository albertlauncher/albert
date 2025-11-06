// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <memory>

namespace albert::detail
{
class RateLimiterPrivate;

class ALBERT_EXPORT Acquire : public QObject
{
    Q_OBJECT
public:
    Acquire();
    ~Acquire() override;

    bool isGranted();

    bool await(std::function<bool()> stop_requested);

signals:
    void granted();

private:
    class Private;
    std::unique_ptr<Private> d;

    friend class RateLimiterPrivate;
};

class ALBERT_EXPORT RateLimiter : public QObject
{
    Q_OBJECT
public:
    RateLimiter(uint delay);
    ~RateLimiter() override;

    void setDelay(uint delay);
    uint delay() const;

    std::unique_ptr<Acquire> acquire();

private:
    std::unique_ptr<RateLimiterPrivate> d;
};

}  // namespace albert::detail
