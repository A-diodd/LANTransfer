#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QTcpServer>
#include "../common/Protocol.h"
#include <QJsonObject>

class FileServer : public QObject
{
    Q_OBJECT

public:
    explicit FileServer(QObject *parent = nullptr);

    bool start(quint16 port);

private slots:
    void onNewConnection();
    void onReadyRead();

private:


    void handleMessage(
        QTcpSocket *socket,
        Protocol::MessageType type,
        const QByteArray &payload);
    
    void handleHello(
        QTcpSocket *socket,
        const QByteArray &payload);

    void handleFileInfo(
        QTcpSocket *socket,
        const QByteArray &payload);

private:

    QTcpServer *server;

    QByteArray buffer;
};

#endif // FILESERVER_H