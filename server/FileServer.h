#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QTcpServer>

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
    QTcpServer *server;
};

#endif // FILESERVER_H