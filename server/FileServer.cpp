#include "FileServer.h"
#include<QTcpSocket>
#include "ClientConnection.h"
#include "TransferReceiver.h"

FileServer::FileServer(QObject *parent)
	: QObject(parent),
	  server(new QTcpServer(this))
{
	connect(server,
			&QTcpServer::newConnection,
			this,
            &FileServer::onNewConnection);
}

bool FileServer::start(quint16 port)
{
	return server->listen(
		QHostAddress::Any,
		port);
}

void FileServer::onNewConnection()
{
	QTcpSocket *socket =
		server->nextPendingConnection();
	
	if(!socket)
		return;
	
	auto *connection=new ClientConnection(socket,this);
    auto *receiver =new TransferReceiver(connection,connection);
	clients.append(connection);
	connect(connection,
		    &ClientConnection::messageReceived,
            receiver,
            &TransferReceiver::onMessageReceived);
	
	connect(connection,
			&ClientConnection::disconnected,
			this,
			&FileServer::onClientDisconnected);

    connect(receiver,
            &TransferReceiver::logMessage,
            this,
            [](const QString &message)
            {
                qDebug()
                  << message;
            });

	qDebug()
		<<"Client conneted:"
		<<socket->peerAddress().toString();
}


void FileServer::onClientDisconnected(
    ClientConnection *connection)
{
    qDebug()
        << "Client disconnected.";

    clients.removeAll(connection);

    connection->deleteLater();
}