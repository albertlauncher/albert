// Copyright (c) 2025-2025 Manuel Schneider

#include "ratelimiter.h"
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <queue>
using namespace albert::detail;
using namespace albert;
using namespace std::chrono;
using namespace std;


class detail::RateLimiter::Private
{
public:
    RateLimiter *q;
    time_point<system_clock> until;
    queue<QPointer<Acquire>> wait_queue;
    mutex mtx;

    inline uint remaining()
    {
        auto ms = duration_cast<milliseconds>(until - system_clock::now()).count();
        return static_cast<uint>(max<decltype(ms)>(0, ms));
    }

    inline void grantNext()
    {
        lock_guard lock(mtx);
        while (!wait_queue.empty())
        {
            auto *acquire = wait_queue.front().get();
            wait_queue.pop();

            if (acquire)
            {
                emit acquire->granted();  // grant
                if(q->isLimited()) // if grant has been effectively used cooldown, else next
                {
                    QTimer::singleShot(remaining(), q, [this] { grantNext(); });
                    return;
                }
            }
        }
    }

    inline void queueAcquire(Acquire *acquire)
    {
        lock_guard lock(mtx);
        wait_queue.push(acquire);
        if (wait_queue.size() == 1) // trigger
            QTimer::singleShot(remaining(), q, [this] { grantNext(); });
    }
};

RateLimiter::RateLimiter():
    d(make_unique<RateLimiter::Private>(this))
{}

RateLimiter::~RateLimiter() {}

void RateLimiter::limit(milliseconds delay) { d->until = system_clock::now() + delay; }

bool RateLimiter::isLimited() const { return d->remaining() > 0; }

unique_ptr<Acquire> RateLimiter::acquire()
{
    auto acquire = make_unique<Acquire>();
    d->queueAcquire(acquire.get());
    return acquire;
}

bool Acquire::Awaiter::await_ready() const noexcept { return false; }

void Acquire::Awaiter::await_suspend(std::coroutine_handle<> handle)
{
    QObject::connect(acquire, &Acquire::granted,
                     acquire, [handle]() mutable { handle.resume(); },
                     Qt::QueuedConnection);
}

void Acquire::Awaiter::await_resume() const noexcept {}
