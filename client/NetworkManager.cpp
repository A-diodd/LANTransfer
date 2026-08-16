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

void NetworkManager::sendData(const QByteArray &data)
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
	QJsonObject payload;
	payload["device"]="LANTransfer";
	QByteArray message=
        Protocol::buildMessage(
            Protocol::MessageType::Hello,
			payload);
    sendData(message);
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

    QJsonObject payload;


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

