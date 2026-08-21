#include "NetworkManager.h"
#include "../common/Protocol.h"
NetworkManager::NetworkManager(QObject *parent)
	:QObject(parent),
	 socket(new QTcpSocket(this))
{
	connect(socket,
			&QTcpSocket::connected,
			this,
			&NetworkManager::onConnected);
			
	connect(socket,
			&QTcpSocket::disconnected,
			this,
			&NetworkManager::onDisconnected);
			
	connect(socket,
			&QTcpSocket::readyRead,
			this,
			&NetworkManager::onReadyRead);
	
	connect(socket,
			&QTcpSocket::errorOccurred,
			this,
			&NetworkManager::onErrorOccurred);
	
	connect(socket,
			&QTcpSocket::bytesWritten,
			this,
			&NetworkManager::onBytesWritten);
}



	

void NetworkManager::connectToServer(
    const QString &ip,
    quint16 port)
{
    socket->connectToHost(ip, port);
}

void NetworkManager::disconnectFromServer()
{
    socket->disconnectFromHost();
}

void NetworkManager::sendData( const QByteArray &data)
{
    if (!isConnected())
        return;

    socket->write(data);
}

bool NetworkManager::isConnected() const
{
    return socket->state() ==
           QAbstractSocket::ConnectedState;
}

void NetworkManager::onConnected()
{
    emit connected();
}

void NetworkManager::onDisconnected()
{
    emit disconnected();
}

void NetworkManager::onReadyRead()
{

    buffer.append(
        socket->readAll());


    Protocol::MessageType type;

    QByteArray payload;


    while(
        Protocol::parseMessage(
            buffer,
            type,
            payload))
    {


        emit messageReceived(
            type,
            payload);

    }

}

void NetworkManager::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
	Q_UNUSED(socketError);
	
	emit errorOccurred(socket->errorString());
}

void NetworkManager::onBytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}
