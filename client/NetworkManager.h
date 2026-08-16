#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include<QObject>
#include<QTcpSocket>


class NetworkManager: public QObject
{
	Q_OBJECT
	
	public:
		explicit NetworkManager(QObject *parent=nullptr); 
		
		void connectToServer(const QString &ip,quint16 port);
		
		void disconnectFromServer();
		
		void sendData(const QByteArray &data);
		
		bool isConnected() const;
	
	signals:
		void connected();
		void disconnected();
		void dataReceived(const QByteArray &data);
		void errorOccurred(const QString &message);
	

        //槽函数
	private slots:
		void onConnected();
		void onDisconnected();
		void onReadyRead();
        void onErrorOccurred(QAbstractSocket::SocketError socketError);
		
	private:
		QTcpSocket *socket;
} ;
#endif