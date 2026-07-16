// Copyright (c) 2025-2026 Manuel Schneider

#include "messagebox.h"
#include <QGuiApplication>
#include <QMessageBox>
using namespace albert;

bool albert::question(const QString &text, QWidget *parent)
{
    return QMessageBox::question(parent, qApp->applicationDisplayName(), text) == QMessageBox::Yes;
}

void albert::information(const QString &text, QWidget *parent)
{
    QMessageBox::information(parent, qApp->applicationDisplayName(), text);
}

void albert::warning(const QString &text, QWidget *parent)
{
    QMessageBox::warning(parent, qApp->applicationDisplayName(), text);
}

void albert::critical(const QString &text, QWidget *parent)
{
    QMessageBox::critical(parent, qApp->applicationDisplayName(), text);
}
