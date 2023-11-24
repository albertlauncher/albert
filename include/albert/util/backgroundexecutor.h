// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/logging.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <chrono>
#include <functional>

namespace albert
{

/// Helper class for recurring tasks
template<typename T> class BackgroundExecutor
{
public:
    /// The task that should be executed in background
    std::function<T(const bool &abort)> parallel;

    /// The function that handles the results when the task is done
    std::function<void(T &&)> finish;
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

    /// Run or schedule a rerun of the task
    void run() {
        if (future_watcher_.isRunning())
            rerun_ = true;
        else
            future_watcher_.setFuture(QtConcurrent::run(parallel, rerun_));
    }

    /// Indicator if the task is still running
    bool isRunning() const { return future_watcher_.isRunning(); }
};

}
