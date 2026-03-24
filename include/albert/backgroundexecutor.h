// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <atomic>
#include <functional>

namespace albert
{

///
/// Convenience class for recurring indexing tasks.
///
/// Takes care of the QtConcurrent boilerplate code to start, abort and schedule restarts of threads.
///
/// \ingroup util_query
///
template<typename T>
class BackgroundExecutor
{
    std::unique_ptr<QFutureWatcher<T>> future_watcher_;
    bool rerun_ = false;
    std::atomic_bool stop_ = false;

public:

    ///
    /// The task to be executed in a thread.
    ///
    /// Return the results of type `T`.  Abort if _abort_ is `true`.
    ///
    std::function<T(const bool &abort)> parallel;

    ///
    /// The finish callback.
    ///
    /// When the \ref parallel function finished, this function will be called in the main thread.
    /// Use \ref BackgroundExecutor::takeResult to get the _results_ returned from \ref parallel.
    ///
    std::function<void()> finish;


    /// Constructs the background executor.
    BackgroundExecutor() = default;

    ///
    /// Destructs the background executor.
    ///
    /// Silently blocks execution until a running task is finished.
    /// See \ref isRunning() and \ref waitForFinished().
    ///
    ~BackgroundExecutor()
    {
        stop_ = true;
        rerun_ = false;
        if (future_watcher_ && !future_watcher_->isFinished())
            future_watcher_->waitForFinished();
    }

    ///
    /// Run or schedule a rerun of the task.
    ///
    /// If a task is running this function sets the abort flag and schedules a rerun.
    /// \ref finish will not be called for the cancelled run.
    ///
    void run()
    {
        if (isRunning())
        {
            stop_ = true;
            rerun_ = true;
        }
        else
        {
            stop_ = false;
            rerun_ = false;

            future_watcher_ = std::make_unique<QFutureWatcher<T>>();

            QObject::connect(future_watcher_.get(), &QFutureWatcher<T>::finished,
                             future_watcher_.get(), [this]
            {
                if (rerun_)
                {
                    future_watcher_.reset();
                    run();  // discard results and rerun
                }
                else
                {
                    try {
                        finish();  // may throw
                    } catch (...) {}
                    future_watcher_.reset();
                }
            });

            future_watcher_->setFuture(QtConcurrent::run([this]{ return parallel(stop_); }));

        }
    }

    /// Stops the current execution.
    inline void stop() { stop_ = true; }

    /// Returns `true` if the asynchronous computation is currently running; otherwise returns `false`.
    inline bool isRunning() const { return future_watcher_.get(); }

    /// Blocks until the current task finished.
    inline void waitForFinished()
    {
        if (future_watcher_)
            future_watcher_->waitForFinished();
    }

    ///
    /// Takes the result from the future.
    ///
    /// Must be called from \ref finish only. Rethrows any exception thrown in \ref parallel.
    ///
    inline T takeResult() { return future_watcher_->future().takeResult(); }

};

}
