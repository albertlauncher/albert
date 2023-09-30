// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "scopedcrashindicator.h"
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QGuiApplication>

ScopedCrashIndicator::ScopedCrashIndicator()
{
    QString filePath = albert::cacheLocation() + "/running";
    if (QFile::exists(filePath)){
        GWARN("Albert has not been terminated properly. Please check your crash reports and report an issue.");
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
    if (!QFile::remove(albert::cacheLocation() + "/running"))
        WARN << "Removing crash indicator file failed.";
}
