// Copyright (c) 2023-2024 Manuel Schneider

#include "logging.h"
#include "signalhandler.h"
#include <QCoreApplication>
#include <QSocketNotifier>
#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

namespace {
enum SocketFileDescriptor { Write, Read, Count };
int socket_file_descriptors[SocketFileDescriptor::Count];
QSocketNotifier *socket_notifier = nullptr;
int handled_signals[] = { SIGTERM, SIGINT, SIGHUP, SIGPIPE };


void unixSignalHandler(int signal)
{
    if (::write(socket_file_descriptors[SocketFileDescriptor::Write],
                &signal, sizeof(signal)) != sizeof(signal))
        qFatal("Signal handler failed to write to socket!");
}

void qtSignalHandler()
{
    socket_notifier->setEnabled(false);
    int signal;
    auto bytes_read = ::read(socket_file_descriptors[SocketFileDescriptor::Read], &signal, sizeof(signal));
    //socket_notifier->setEnabled(true);
    if (bytes_read == sizeof(signal))
        INFO << QString("Received signal %1. Quit.").arg(signal);
    else
        qFatal("Signal socket received message of invalid size");
    QCoreApplication::quit();
}

}

SignalHandler::SignalHandler()
{
    if (socket_notifier)
       qFatal("Signal handler has to be unique.");

    // Create unix socket pair
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, socket_file_descriptors))
       qFatal("Couldn't create signal socketpair.");

    // Create socket notifier listening on the unix socket
    socket_notifier = new QSocketNotifier(
        socket_file_descriptors[SocketFileDescriptor::Read],
        QSocketNotifier::Read
    );

    // Handle the socket notification
    QObject::connect(socket_notifier, &QSocketNotifier::activated, qtSignalHandler);

    // Install handler on signals
    struct sigaction sigact{};
    sigact.sa_handler = unixSignalHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_RESTART | SA_RESETHAND; // https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html
    for (int sig : handled_signals)
        if (sigaction(sig, &sigact, nullptr))
            qFatal("Failed installing signal handler on signal: %d", sig);
}

SignalHandler::~SignalHandler()
{
    // Restore default signal handlers
    struct sigaction sigact{};
    sigact.sa_handler = SIG_DFL;
    for (int sig : handled_signals)
        if (sigaction(sig, &sigact, nullptr))
            qFatal("Failed restoring default signal handler on signal: %d", sig);

    socket_notifier->disconnect();
    delete socket_notifier;

    ::close(socket_file_descriptors[SocketFileDescriptor::Read]);
    ::close(socket_file_descriptors[SocketFileDescriptor::Write]);
}
