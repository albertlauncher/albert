// Copyright (c) 2025-2025 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "messagebox.h"
#include <QGuiApplication>
#include <QMessageBox>
using namespace albert;

bool util::question(const QString &text)
{
    return QMessageBox::question(QWidget::find(App::instance()->frontend()->winId()),
                                 qApp->applicationDisplayName(),
                                 text) == QMessageBox::Yes;
}

void util::information(const QString &text)
{
    QMessageBox::information(QWidget::find(App::instance()->frontend()->winId()),
                             qApp->applicationDisplayName(),
                             text);
}

void util::warning(const QString &text)
{
    QMessageBox::warning(QWidget::find(App::instance()->frontend()->winId()),
                         qApp->applicationDisplayName(),
                         text);
}

void util::critical(const QString &text)
{
    QMessageBox::critical(QWidget::find(App::instance()->frontend()->winId()),
                          qApp->applicationDisplayName(),
                          text);
}
