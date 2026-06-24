// SPDX-FileCopyrightText: 2025-2026 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <memory>
#include <chrono>
#include <coroutine>

namespace albert::detail
{

class ALBERT_EXPORT Acquire : public QObject
{
    Q_OBJECT
public:

    struct Awaiter {
        Acquire* acquire;
        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> handle);
        void await_resume() const noexcept;
    };

    inline auto operator co_await() { return Awaiter{this}; }

signals:
    void granted();
};

template<typename T>
    requires std::same_as<typename T::element_type, Acquire>
inline auto operator co_await(T&& acquire) {
    return Acquire::Awaiter{acquire.get()};
}


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
