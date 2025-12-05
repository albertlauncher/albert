// Copyright (c) 2022-2025 Manuel Schneider

#include "albert/app.h"
#include "logging.h"
#include "rpcserver.h"
#include <QDir>
#include <QFile>  // QtPrivate::fromFilesystemPath
#include <QLocalServer>
#include <QLocalSocket>
using namespace albert;
using namespace std;

static QString socketPath()
{ return QtPrivate::fromFilesystemPath(App::cacheLocation() / "ipc_socket"); }

class RPCServer::Private
{
public:
    QLocalServer local_server;
    function<QByteArray(const QByteArray&)> handler;

    void onConnection()
    {
        QLocalSocket* socket = local_server.nextPendingConnection();
        socket->waitForReadyRead(50);
        if (socket->bytesAvailable())
        {
            if (handler)
                socket->write(handler(socket->readAll()));

        }
        socket->flush();
        socket->close();
        socket->deleteLater();
    }
};

RPCServer::RPCServer() : d(make_unique<Private>())
{
    auto socket_path = socketPath();

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
    if (!d->local_server.listen(socket_path))
        qFatal("Failed creating IPC server: %s", qPrintable(d->local_server.errorString()));

    QObject::connect(&d->local_server, &QLocalServer::newConnection,
                     &d->local_server, [this]{ d->onConnection(); });
}

RPCServer::~RPCServer()
{
    DEBG << "Closing local RPC server.";
    d->local_server.close();
}

void RPCServer::setMessageHandler(function<QByteArray(const QByteArray &)> h){ d->handler = h; }

QByteArray RPCServer::sendMessage(const QByteArray &bytes, bool await_response)
{
    QLocalSocket socket;
    socket.connectToServer(socketPath());
    if (socket.waitForConnected(500))
    {
        socket.write(bytes);
        socket.flush();

        if(!await_response)
            return {};

        if (socket.waitForReadyRead(1000))
            return socket.readAll();
        else if (auto e = socket.error(); e == QLocalSocket::PeerClosedError)
            return {};
        else
            throw runtime_error(socket.errorString().toStdString());
    }
    else
        throw runtime_error("Failed to connect to albert.");
}
