#include <QLocalSocket>
#include <QStandardPaths>
const QString socket_path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/albert_socket";

#include <iostream>
int main(int argc, char **argv)
{
    // Prepare the message
    QStringList args;
    for (int i = 1; i < argc; ++i)
        args << QString(argv[i]);
    QString message = args.join(" ");

    if (message.isEmpty())
        std::cout << "Nothing to send." << std::endl;
    else {
        QLocalSocket socket;
        socket.connectToServer(socket_path);
        if (socket.waitForConnected(500)){
            socket.write(message.toUtf8());
            socket.flush();
            socket.close();
        } else {
            std::cout << "Failed to connect to albert." << std::endl;
            ::exit(EXIT_FAILURE);
        }
    }
}
