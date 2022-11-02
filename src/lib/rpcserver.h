// Copyright (C) 2022 Manuel Schneider

#pragma once
#include <QLocalServer>

class RPCServer
{
public:
    RPCServer();
    ~RPCServer();
private:
    void onNewConnection();
    QLocalServer local_server;
};
