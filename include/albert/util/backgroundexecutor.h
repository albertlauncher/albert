// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/logging.h"
#include "albert/util/timeprinter.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
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
            TimePrinter tp("Busy waited for %1µs.");
            WARN << "Busy wait for BackgroundExecutor task. Abortion handled correctly?";
            future_watcher_.waitForFinished();
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
