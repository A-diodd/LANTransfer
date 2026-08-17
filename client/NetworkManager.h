#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include<QObject>
#include<QTcpSocket>

#include "../common/Protocol.h"

class NetworkManager: public QObject
{
	Q_OBJECT
	
	public:
		explicit NetworkManager(QObject *parent=nullptr); 
		
		void connectToServer(const QString &ip,quint16 port);
		
		void disconnectFromServer();
		
        void sendData( const QByteArray &data);

		bool isConnected() const;
	
	signals:
		void connected();
		void disconnected();
		void errorOccurred(const QString &message);
		void messageReceived(
			Protocol::MessageType type,
			const QByteArray &payload);
	

        //槽函数
	private slots:
		void onConnected();
		void onDisconnected();
		void onReadyRead();
        void onErrorOccurred(QAbstractSocket::SocketError socketError);
		
	private:
		QTcpSocket *socket;
		QByteArray buffer;
} ;
#endif