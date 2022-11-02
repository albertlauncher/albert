// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "rpcserver.h"
#include <QCoreApplication>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QString>


static std::map<QString, std::function<QString(const QString&)>> actions =
{
        {"show", [](const QString& param){
            albert::show(param);
            return "Albert set visible.";
        }},
        {"hide", [](const QString& param){
            albert::hide();
            return "Albert set hidden.";
        }},
        {"toggle", [](const QString& param){
            albert::toggle();
            return "Albert visibility toggled.";
        }},
        {"settings", [](const QString& param){
            albert::showSettings();
            return "Settings opened,";
        }},
        {"restart", [](const QString& param){
            albert::restart();
            return "Triggered restart.";
        }},
        {"quit", [](const QString& param){
            albert::quit();
            return "Triggered quit.";
        }}
};


RPCServer::RPCServer()
{
    QString socket_path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/albert_socket";

    QLocalSocket socket;
    DEBG << "Checking for a running instance…";
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
                     [this](){RPCServer::onNewConnection();});
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
        DEBG << "Received message:" << message;

        auto op = message.section(' ', 0, 0, QString::SectionSkipEmpty);
        auto param = message.section(' ', 1, -1, QString::SectionSkipEmpty);

        try{
            socket->write(actions.at(op)(param).toLocal8Bit());
        } catch (const std::out_of_range &) {
            QStringList l{QString("Invalid RPC command: '%1'. Use these").arg(message)};
            for (const auto &[key, value] : actions)
                l << key;
            socket->write(l.join(QChar::LineFeed).toLocal8Bit());
            INFO << QString("Received invalid RPC command: %1").arg(message);
        }
    }
    socket->close();
    socket->deleteLater();
}
