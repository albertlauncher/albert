// Copyright (c) 2022 Manuel Schneider
#pragma once
#include <QStandardPaths>
#include <QFile>

struct ScopedCrashIndicator
{
    ScopedCrashIndicator()
    {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
        if (QFile::exists(filePath)){
            WARN << "Application has not been terminated graciously";
        } else {
            DEBG << "Creating crash indicator file";
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly))
                WARN << "Could not create file:" << filePath;
            file.close();
        }
    }

    ~ScopedCrashIndicator()
    {
        INFO << "Removing crash indicator file";
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");
    }
};