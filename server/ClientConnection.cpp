//server端创建新socket，尝试用用客户端连接，并行处理客户端数据。#include "ClientConnection.h"
#include "ClientConnection.h"
ClientConnection::ClientConnection(QTcpSocket *socket,QObject *parent)
	:QObject(parent),
	 m_socket(socket)
{
    m_socket->setParent(this);
	connect(m_socket,
			&QTcpSocket::readyRead,
			this,
			&ClientConnection::onReadyRead);
	
	connect(m_socket,
			&QTcpSocket::disconnected,
			this,
			&ClientConnection::onDisconnected); 
}

QTcpSocket* ClientConnection::socket() const
{
	return m_socket;
}

void ClientConnection::sendMessage(
	Protocol::MessageType type,
	const QByteArray &payload)
{
	QByteArray message =
		Protocol::buildMessage(
			type,
			payload);
	
	m_socket->write(message);
}

void ClientConnection::onReadyRead()
{
	buffer.append(
		m_socket->readAll());
		
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

void ClientConnection::onDisconnected()
{
	emit disconnected(this);
} 