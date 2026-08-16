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

auto *socket =
qobject_cast<QTcpSocket*>(sender());


buffer.append(socket->readAll());


Protocol::MessageType type;

QJsonObject payload;


while(Protocol::parseMessage(
      buffer,
      type,
      payload))
{

    handleMessage(
        socket,
        type,
        payload);

}

}


void FileServer::handleMessage(
    QTcpSocket *socket,
    Protocol::MessageType type,
    const QJsonObject &payload)
{

    switch(type)
    {

    case Protocol::MessageType::Hello:
    {
        handleHello(socket,payload);
        break;
    }


    case Protocol::MessageType::FileInfo:
    {
        handleFileInfo(socket,payload);
        break;
    }


  /*  case Protocol::MessageType::FileData:
    {
        handleFileData(socket,payload);
        break;
    }*/


    default:
        break;

    }

}

void FileServer::handleHello(
    QTcpSocket *socket,
    const QJsonObject &payload)
{


    Q_UNUSED(payload);



    QJsonObject response;


    response["device"]
        ="FileServer";



    socket->write(

        Protocol::buildMessage(
            Protocol::MessageType::HelloAck,
            response)

    );


}

void FileServer::handleFileInfo(
    QTcpSocket *socket,
    const QJsonObject &payload)
{


    QString name =
        payload["file_name"]
        .toString();



    QString size =
        payload["file_size"]
        .toString();



    qDebug()
        <<"Receive file:"
        <<name;



    qDebug()
        <<"Size:"
        <<size;



    QJsonObject response;


    response["result"]
        ="accepted";



    socket->write(

        Protocol::buildMessage(
            Protocol::MessageType::FileAccept,
            response)

    );


}