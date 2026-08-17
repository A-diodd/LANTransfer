#include "FileServer.h"

#include<QTcpSocket>
#include "../common/Protocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

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

    expectedSize =
        object["file_size"]
            .toString()
            .toLongLong();
    
    receivedSize=0;
    
    QFileInfo fileInfo(name);
    
    QString safeName =
        fileInfo.fileName();

    receiveFile.setFileName(
        "received_" + safeName);

    if (!receiveFile.open(
            QIODevice::WriteOnly))
    {
        qDebug()
            << "Failed to open output file.";

        return;
    }

    qDebug()
        << "Receive file:"
        << safeName;

    qDebug()
        << "Size:"
        << expectedSize;

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

void FileServer::handleFileData(
    QTcpSocket *socket,
    const QByteArray &payload)
{
    Q_UNUSED(socket);

    if (!receiveFile.isOpen())
        return;

    qint64 written =
        receiveFile.write(payload);

    if (written != payload.size())
    {
        qDebug()
            << "Failed to write file.";

        return;
    }

    receivedSize += written;

    qDebug()
        << "Received:"
        << receivedSize
        << "/"
        << expectedSize;

    if (receivedSize >= expectedSize)
    {
        receiveFile.close();

        qDebug()
            << "File received completely.";
    }
}