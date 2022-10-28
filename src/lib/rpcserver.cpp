// Copyright (c) 2022 Manuel Schneider

#include "logging.h"
#include "rpcserver.h"
#include <QLocalSocket>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>

const QString socket_path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/albert_socket";

RPCServer::RPCServer()
{
    QLocalSocket socket;
    DEBG << "Connecting to local socket…";
    socket.connectToServer(socket_path);
    if (socket.waitForConnected(100)) {
        INFO << "There is another instance of albert running.";
        qApp->exit(2);
    }

    // Remove pipes potentially leftover after crash
    QLocalServer::removeServer(socket_path);

    DEBG << "Creating local socket" << socket_path;
    if (!local_server.listen(socket_path))
        qFatal("Failed creating IPC server: %s", qPrintable(local_server.errorString()));

    QObject::connect(&local_server, &QLocalServer::newConnection,
                     this, &RPCServer::onNewConnection);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit,
                     &local_server, &QLocalServer::close);
}

RPCServer::~RPCServer()
{
    local_server.close();
}

void RPCServer::onNewConnection()
{
    QLocalSocket* socket = local_server.nextPendingConnection();
    socket->waitForReadyRead(50);
    if (socket->bytesAvailable()) {
        auto message = QString::fromLocal8Bit(socket->readAll());
        emit messageReceived(message);
        DEBG << "Received message:" << message;
    }
    socket->close();
    socket->deleteLater();
}

