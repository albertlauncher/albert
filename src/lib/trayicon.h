// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QMenu>
#include <QSystemTrayIcon>

class TrayIcon : public QSystemTrayIcon
{
public:
    TrayIcon();
    void setVisible(bool = true);
    QMenu menu;
};

