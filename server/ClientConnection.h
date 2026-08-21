#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H
#include <QObject>
#include <QTcpSocket>

#include "../common/Protocol.h"

class ClientConnection: public QObject
{
	Q_OBJECT
	
public:
	explicit ClientConnection(
		QTcpSocket *socket,
		QObject *parent=nullptr);
	
	QTcpSocket* socket() const;
	
	void sendMessage(
		Protocol::MessageType type,
		const QByteArray &payload);
		
signals:
	void messageReceived(
		Protocol::MessageType type,
		const QByteArray &payload);
		
	void disconnected(
		ClientConnection *connection);

private slots:
	void onReadyRead();
	
	void onDisconnected();
	
private:
	QTcpSocket *m_socket;
	
	QByteArray buffer;
};
#endif