// Copyright (c) 2025-2025 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "messagebox.h"
#include <QGuiApplication>
#include <QMessageBox>
using namespace albert;

bool util::question(const QString &text, QWidget *parent)
{
    return QMessageBox::question(parent ? parent
                                        : QWidget::find(App::instance()->frontend()->winId()),
                                 qApp->applicationDisplayName(),
                                 text) == QMessageBox::Yes;
}

void util::information(const QString &text, QWidget *parent)
{
    QMessageBox::information(parent ? parent
                                    : QWidget::find(App::instance()->frontend()->winId()),
                             qApp->applicationDisplayName(),
                             text);
}

void util::warning(const QString &text, QWidget *parent)
{
    QMessageBox::warning(parent ? parent
                                : QWidget::find(App::instance()->frontend()->winId()),
                         qApp->applicationDisplayName(),
                         text);
}

void util::critical(const QString &text, QWidget *parent)
{
    QMessageBox::critical(parent ? parent
                                 : QWidget::find(App::instance()->frontend()->winId()),
                          qApp->applicationDisplayName(),
                          text);
}
