#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>
#include "../common/Protocol.h"

class ClientConnection;
class TransferReceiver;


class FileServer : public QObject
{
    Q_OBJECT

public:
    explicit FileServer(QObject *parent = nullptr);

    bool start(quint16 port);

private slots:
    void onNewConnection();

    void onClientDisconnected(ClientConnection *connection);


private:

    QTcpServer *server;

    QList<ClientConnection *> clients;
};

#endif // FILESERVER_H