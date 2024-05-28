// Copyright (c) 2024 Manuel Schneider

#include "messagehandler.h"
#include <QMessageBox>
#include <QString>
#include <QTime>

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // Todo use std::format as soon as apple gets it off the ground
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s \x1b[34m[debg:%s]\x1b[0m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtInfoMsg:
        fprintf(stdout, "%s \x1b[32m[info:%s]\x1b[0m %s\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());

        break;
    case QtWarningMsg:
        fprintf(stdout, "%s \x1b[33m[warn:%s]\x1b[0m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        fprintf(stdout, "%s \x1b[31m[crit:%s] %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s \x1b[41;30;4m[fatal:%s]\x1b[0;1m %s  --  [%s]\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData(),
                context.function);
        QMessageBox::critical(nullptr, "Fatal error", message);
        exit(1);
    }
    fflush(stdout);
}
