// Copyright (c) 2023 Manuel Schneider

#include "albert/logging.h"
#include "scopedcrashindicator.h"
#include <QFile>
#include <QStandardPaths>
#include <QString>

ScopedCrashIndicator::ScopedCrashIndicator()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
    if (QFile::exists(filePath)){
        CRIT << "Application has not been terminated graciously";
    } else {
        DEBG << "Creating crash indicator file";
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            WARN << "Could not create file:" << filePath;
        file.close();
    }
}

ScopedCrashIndicator::~ScopedCrashIndicator()
{
    INFO << "Removing crash indicator file";
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");
}
