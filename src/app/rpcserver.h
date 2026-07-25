// Copyright (C) 2022-2025 Manuel Schneider

#pragma once
#include <QString>
#include <expected>
#include <functional>
#include <memory>
class QByteArray;

class RPCServer
{
public:

    RPCServer(const QString socket_path);
    ~RPCServer();

    void setMessageHandler(std::function<QByteArray(const QByteArray&)> handler);

    static std::expected<QByteArray, QString> sendMessage(const QString socket_path,
                                                          const QByteArray &bytes);

private:

    class Private;
    std::unique_ptr<Private> d;

};
