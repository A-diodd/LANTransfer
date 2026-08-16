#include "FileServer.h"

#include<QTcpSocket>
#include "../common/Protocol.h"

#include <QJsonDocument>
#include <QJsonObject>

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
	QTcpSocket *socket=server->nextPendingConnection();
	
	if(!socket) return;
    qDebug()
        << "Client connected:"
        << socket->peerAddress().toString();
    
    connect(socket,
			&QTcpSocket::readyRead,
			this,
			&FileServer::onReadyRead);
}


void FileServer::onReadyRead()
{
	auto *socket=
		qobject_cast<QTcpSocket *>(sender());
	
	if(!socket)
		return;
	
	QByteArray data=
		socket->readAll();
	
	qDebug()<<"Received"
			<<data;
	
	QJsonParseError error;
	
	QJsonDocument document=
		QJsonDocument::fromJson(
			data,
			&error);
	
	if(error.error !=QJsonParseError::NoError)
	{
		qDebug() <<"Invalid JSON";
		return;
	}
	
	QJsonObject object =document.object();
	
	QString type =object["type"].toString();
	
	if(type=="hello")
	{
		QJsonObject payload;
		payload["device"]="FileServer";
		QByteArray response=
			Protocol::buildMessage(
				Protocol::MessageType::HelloAck,
				payload);    
		
		qDebug() << "Response:" << response;

        qint64 bytes = socket->write(response);

        qDebug() << "write bytes:" << bytes;
        qDebug() << "bytesToWrite:" << socket->bytesToWrite();
	}
	else if (type == "file_info")
	{
		QString fileName =
			object["file_name"].toString();

		QString fileSize =
			object["file_size"].toString();

		qDebug()
			<< "Incoming file:"
			<< fileName;

		qDebug()
			<< "File size:"
			<< fileSize;

		QJsonObject payload;

		payload["result"] = "accepted";

		QByteArray response =
			Protocol::buildMessage(
				Protocol::MessageType::FileAccept,
				payload);

		socket->write(response);
	}
	
	 
 } 