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

class detail::Acquire::Private
{
public:
    Acquire *q;
    atomic_bool is_granted_ = false;

    void grant()
    {
        is_granted_ = true;
        emit q->granted();
    }

};

Acquire::Acquire() : d(make_unique<detail::Acquire::Private>(this, false)) {}

Acquire::~Acquire() {}

bool Acquire::isGranted() { return d->is_granted_; }

bool Acquire::await(std::function<bool()> stop_requested)
{
    while (true)
    {
        if (stop_requested())
            return false;
        else if (d->is_granted_)
            return true;
        else
            QThread::msleep(10);
    }
}

// -------------------------------------------------------------------------------------------------


class detail::RateLimiterPrivate
{
public:
    RateLimiter *q;
    milliseconds delay;
    time_point<system_clock> last_grant;
    queue<QPointer<Acquire>> wait_queue;
    mutex mtx;

    inline uint remaining()
    {
        auto ms = duration_cast<milliseconds>(last_grant + delay - system_clock::now()).count();
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
                last_grant = system_clock::now();
                acquire->d->grant();
                if (!wait_queue.empty())
                    QTimer::singleShot(delay, q, [this] { grantNext(); });
                return;
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

RateLimiter::RateLimiter(uint ms):
    d(make_unique<RateLimiterPrivate>(this, milliseconds(ms)))
{}

RateLimiter::~RateLimiter() {}

void RateLimiter::setDelay(uint delay)
{
    lock_guard lock(d->mtx);
    d->delay = milliseconds(delay);
}

uint RateLimiter::delay() const
{
    lock_guard lock(d->mtx);
    return d->delay.count();
}

unique_ptr<Acquire> RateLimiter::acquire()
{
    auto acquire = make_unique<Acquire>();
    d->queueAcquire(acquire.get());
    return acquire;
}
