// Copyright (c) 2025-2025 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "widgetsutil.h"
#include <QApplication>
#include <QGuiApplication>
#include <QObject>
#include <QStyle>
#include <QWidget>
using namespace albert::util;
using namespace albert;
using namespace std;

QMessageBox::StandardButton util::question(const QString &text,
                                           QMessageBox::StandardButtons buttons,
                                           QMessageBox::StandardButton defaultButton)
{
    return QMessageBox::information(QWidget::find(App::instance()->frontend()->winId()),
                                    qApp->applicationDisplayName(),
                                    text,
                                    buttons,
                                    defaultButton);
}

QMessageBox::StandardButton util::information(const QString &text,
                                              QMessageBox::StandardButtons buttons,
                                              QMessageBox::StandardButton defaultButton)
{
    return QMessageBox::information(QWidget::find(App::instance()->frontend()->winId()),
                                    qApp->applicationDisplayName(),
                                    text,
                                    buttons,
                                    defaultButton);
}

QMessageBox::StandardButton util::warning(const QString &text,
                                          QMessageBox::StandardButtons buttons,
                                          QMessageBox::StandardButton defaultButton)
{
    return QMessageBox::information(QWidget::find(App::instance()->frontend()->winId()),
                                    qApp->applicationDisplayName(),
                                    text,
                                    buttons,
                                    defaultButton);
}

QMessageBox::StandardButton util::critical(const QString &text,
                                           QMessageBox::StandardButtons buttons,
                                           QMessageBox::StandardButton defaultButton)
{
    return QMessageBox::information(QWidget::find(App::instance()->frontend()->winId()),
                                    qApp->applicationDisplayName(),
                                    text,
                                    buttons,
                                    defaultButton);
}
