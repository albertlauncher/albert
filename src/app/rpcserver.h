// Copyright (C) 2022-2024 Manuel Schneider

#pragma once
#include <QLocalServer>
#include <map>

class RPCServer
{
public:

    using RPC = std::function<QString(const QString&)>;

    RPCServer();
    ~RPCServer();

    void setPRC(std::map<QString, RPC> &&rpc);

    static QString socketPath();
    static bool trySendMessage(const QString &message);

private:

    void onNewConnection();
    QLocalServer local_server;
    std::map<QString, RPC> rpc_;

};
