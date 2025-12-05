// Copyright (c) 2025-2025 Manuel Schneider

#include "application.h"
#include "frontend.h"
#include "messagebox.h"
#include <QGuiApplication>
#include <QMessageBox>
using namespace albert;

inline static QWidget *mainWindow() { return QWidget::find(Application::instance().frontend()->winId()); }

bool albert::question(const QString &text, QWidget *parent)
{
    return QMessageBox::question(parent ? parent : mainWindow(),
                                 qApp->applicationDisplayName(),
                                 text)
           == QMessageBox::Yes;
}

void albert::information(const QString &text, QWidget *parent)
{
    QMessageBox::information(parent ? parent : mainWindow(),
                             qApp->applicationDisplayName(),
                             text);
}

void albert::warning(const QString &text, QWidget *parent)
{
    QMessageBox::warning(parent ? parent : mainWindow(),
                         qApp->applicationDisplayName(),
                         text);
}

void albert::critical(const QString &text, QWidget *parent)
{
    QMessageBox::critical(parent ? parent : mainWindow(),
                          qApp->applicationDisplayName(),
                          text);
}
