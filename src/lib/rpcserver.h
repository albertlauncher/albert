// Copyright (C) 2022 Manuel Schneider

#pragma once
#include <QLocalServer>

class RPCServer : public QObject
{
    Q_OBJECT
public:
    RPCServer();
    ~RPCServer();
private:
    void onNewConnection();
    QLocalServer local_server;
signals:
    void messageReceived(QString message);
};
