// Copyright (C) 2022-2025 Manuel Schneider

#pragma once
#include <functional>
#include <memory>
namespace albert { class ExtensionRegistry; }
class QByteArray;

class RPCServer
{
public:

    RPCServer();
    ~RPCServer();

    void setMessageHandler(std::function<QByteArray(const QByteArray&)> handler);

    static QByteArray sendMessage(const QByteArray &bytes, bool await_response = true);

private:

    class Private;
    std::unique_ptr<Private> d;

};
