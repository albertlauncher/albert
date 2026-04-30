// SPDX-FileCopyrightText: 2026 Manuel Schneider

#pragma once
#include <QFuture>
#include <albert/export.h>
#include <optional>

namespace albert::detail
{
/// RAII wrapper for QFuture that enforces completion on destruction.
template<typename T>
class ALBERT_EXPORT ScopedFuture
{
public:
    explicit ScopedFuture(QFuture<T> f) : future_opt(std::move(f)) {}
    ~ScopedFuture() { waitForFinished(); }

    void waitForFinished()
    {
        if (future_opt && !future_opt->isFinished())
            future_opt->waitForFinished();
    }

    ScopedFuture(const ScopedFuture &) = delete;
    ScopedFuture(ScopedFuture &&other) noexcept
        : future_opt(std::move(other.future_opt))
    { other.future_opt.reset(); };

    ScopedFuture &operator=(const ScopedFuture &) = delete;
    ScopedFuture &operator=(ScopedFuture &&other) noexcept
    {
        waitForFinished();
        future_opt = std::move(other.future_opt);
        other.future_opt.reset();
        return *this;
    }

    auto&& operator*(this auto&& self) {
        return *std::forward<decltype(self)>(self).future_opt;
    }

    auto* operator->(this auto&& self) {
        return &*std::forward<decltype(self)>(self).future_opt;
    }

    auto&& get(this auto&& self) {
        return *std::forward<decltype(self)>(self).future_opt;
    }
    void reset() { future_opt.reset(); }

private:
    std::optional<QFuture<T>> future_opt;
};

}  // namespace albert
