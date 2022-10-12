// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QMenu>
#include <QSystemTrayIcon>

namespace Core {

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:

    TrayIcon();

    void setVisible(bool = true);

signals:

    void stateChanged(bool);
};

}
