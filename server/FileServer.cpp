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

QByteArray payload;


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
    const QByteArray &payload)
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
    const QByteArray &payload)
{


    Q_UNUSED(payload);



    QByteArray response;




    socket->write(

        Protocol::buildMessage(
            Protocol::MessageType::HelloAck,
            response)

    );


}

void FileServer::handleFileInfo(
    QTcpSocket *socket,
    const QByteArray &payload)
{
    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            payload,
            &error);

    if (error.error !=
        QJsonParseError::NoError)
    {
        qDebug()
            << "Invalid FileInfo JSON";

        return;
    }

    QJsonObject object =
        document.object();

    QString name =
        object["file_name"].toString();

    qint64 size =
        object["file_size"]
            .toString()
            .toLongLong();

    qDebug()
        << "Receive file:"
        << name;

    qDebug()
        << "Size:"
        << size;

    QJsonObject response;

    response["result"] =
        "accepted";

    QJsonDocument responseDocument(
        response);

    QByteArray json =
        responseDocument.toJson(
            QJsonDocument::Compact);

    socket->write(
        Protocol::buildMessage(
            Protocol::MessageType::FileAccept,
            json));
}