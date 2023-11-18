// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "report.h"
#include "rpcserver.h"
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QLocalSocket>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>
#include <iostream>

static const char* socket_file_name = "ipc_socket";

static std::map<QString, std::function<QString(const QString&)>> actions =
{
    {"commands", [](const QString&){
        QStringList rpcs;
        for (const auto &[k, v] : actions)
            rpcs << k;
        return rpcs.join('\n');
    }},
    {"show", [](const QString& param){
        albert::show(param);
        return "Albert set visible.";
    }},
    {"hide", [](const QString&){
        albert::hide();
        return "Albert set hidden.";
    }},
    {"toggle", [](const QString&){
        albert::toggle();
        return "Albert visibility toggled.";
    }},
    {"settings", [](const QString&){
        albert::showSettings();
        return "Settings opened,";
    }},
    {"restart", [](const QString&){
        albert::restart();
        return "Triggered restart.";
    }},
    {"quit", [](const QString&){
        albert::quit();
        return "Triggered quit.";
    }},
    {"report", [](const QString&){
        return report().join('\n');
    }}
};


RPCServer::RPCServer()
{
    QString socket_path = QString("%1/%2").arg(albert::cacheLocation(), socket_file_name);

    DEBG << "Checking for a running instance…";
    QLocalSocket socket;
    socket.connectToServer(socket_path);
    if (socket.waitForConnected()) {
        INFO << "There is another instance of albert running.";
        ::exit(2);
    } else {
        switch (socket.error()) {
        case QLocalSocket::ServerNotFoundError:
            // all good. no socket.
            break;
        case QLocalSocket::ConnectionRefusedError:
            // socket exists but nobody answers. probably crashed before.
            CRIT << "Albert has not been terminated properly. "
                    "Please check your logs and report an issue.";
            QLocalServer::removeServer(socket_path);
            break;
        default:
            // any other errors should bail out for now.
            WARN << socket.error();
            WARN << socket.errorString();
            ::exit(2);
            break;
        }
    }

    DEBG << "Creating local server" << socket_path;
    if (!local_server.listen(socket_path))
        qFatal("Failed creating IPC server: %s", qPrintable(local_server.errorString()));

    QObject::connect(&local_server, &QLocalServer::newConnection,
                     &local_server, [this](){RPCServer::onNewConnection();});
}

RPCServer::~RPCServer()
{
    DEBG << "Closing local RPC server.";
    local_server.close();
}

void RPCServer::onNewConnection()
{
    QLocalSocket* socket = local_server.nextPendingConnection();
    socket->waitForReadyRead(50);
    if (socket->bytesAvailable()) {
        auto message = QString::fromLocal8Bit(socket->readAll());
        DEBG << "Received message:" << message;

        static QRegularExpression re("\\S");
        message = message.mid(message.indexOf(re));  // Trim left spaces
        auto op = message.section(' ', 0, 0);
        auto param = message.section(' ', 1, -1);

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
    socket->flush();
    socket->close();
    socket->deleteLater();
}

bool RPCServer::trySendMessage(const QString &message)
{
    // Dont print logs in here

    QString socket_path = QString("%1/%2").arg(albert::cacheLocation(), socket_file_name);

    QLocalSocket socket;
    socket.connectToServer(socket_path);
    if (socket.waitForConnected(500)){
        socket.write(message.toUtf8());
        socket.flush();
        socket.waitForReadyRead(1000);
        std::cout << socket.readAll().toStdString() << std::endl;
        socket.close();
        return true;
    } else {
        std::cout << "Failed to connect to albert." << std::endl;
        return false;
    }
}
