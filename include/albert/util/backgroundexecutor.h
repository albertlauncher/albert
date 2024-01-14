// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/logging.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <chrono>
#include <functional>

namespace albert
{

/// Provides a lean interface for recurring indexing tasks.
/// Takes care of the QtConcurrent boilerplate code to start,
/// abort and scheldule restarts of threads.
/// \tparam The type of results this executor produces
template<typename T> class BackgroundExecutor
{
public:
    /// The task that should be executed in background.
    /// This function is executed in a separate thread.
    /// \param abort The abort flag
    /// \return The results of the task.
    std::function<T(const bool &abort)> parallel;

    /// The results handler.
    /// When the parallel function finished, this function will be called in
    /// the main thread with the results returned by the parallel function.
    /// \param results The results parallel returned.
    std::function<void(T && results)> finish;

    /// The runtime of the last execution of parallel
    std::chrono::milliseconds runtime;

private:
    QFutureWatcher<T> future_watcher_;
    bool rerun_ = false;

    void onFinish() {
        if (rerun_){  // discard and rerun
            rerun_ = false;
            run();
        } else
            finish(std::move(future_watcher_.future().takeResult()));
    }

    T run_(const bool &abort){
        auto start = std::chrono::system_clock::now();
        T ret = parallel(abort);
        auto end = std::chrono::system_clock::now();
        runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        return ret;
    }

public:
    BackgroundExecutor() {
        QObject::connect(&future_watcher_, &QFutureWatcher<T>::finished, [this](){onFinish();});
    };

    ~BackgroundExecutor() {
        rerun_ = false;
        if (isRunning()){
            WARN << "Busy wait for BackgroundExecutor task. Abortion handled correctly?";
            auto start = std::chrono::system_clock::now();
            future_watcher_.waitForFinished();
            auto end = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
            WARN << QStringLiteral("Busy waited for %1 ms.").arg(duration.count());
        }
    };

    /// Run or schedule a rerun of the task.
    /// If a task is running this function sets the abort flag and schedules a
    /// rerun. `finish` will not be called for the cancelled run.
    void run() {
        if (future_watcher_.isRunning())
            rerun_ = true;
        else
            future_watcher_.setFuture(QtConcurrent::run(&BackgroundExecutor<T>::run_, this, rerun_));
    }

    /// Returns `true` if the asynchronous computation is currently
    /// running; otherwise returns `false`.
    bool isRunning() const { return future_watcher_.isRunning(); }
};

}
