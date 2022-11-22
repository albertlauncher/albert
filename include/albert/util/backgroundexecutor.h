// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <functional>

namespace albert
{

/// Helper class for recurring tasks
template<typename T> class BackgroundExecutor
{
public:
    std::function<T(const bool &cancel)> parallel;
    std::function<void(T &&)> finish;
private:
    QFutureWatcher<T> future_watcher_;
    bool rerun_ = false;
    void onFinish()
    {
        if (rerun_){  // discard and rerun
            rerun_ = false;
            future_watcher_.setFuture(QtConcurrent::run(parallel, rerun_));
        } else{
            finish(std::move(future_watcher_.result()));
        }
    }

public:
    BackgroundExecutor() {
        QObject::connect(&future_watcher_, &QFutureWatcher<T>::finished, [this](){onFinish();});
    };

    void run()
    {
        if (rerun_)
            return;
        if (future_watcher_.isRunning())
            rerun_ = true;
        else
            future_watcher_.setFuture(QtConcurrent::run(parallel, rerun_));
    }
};
}