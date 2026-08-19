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


// 客户端有数据发送过来触发的槽函数
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


    case Protocol::MessageType::FileData:
    {
        handleFileData(socket,payload);
        break;
    }
    case Protocol::MessageType::FileFinish:
    {
        handleFileFinish(socket, payload);
        break;
    }



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

    QString fileName =
        object["file_name"].toString();

    expectedSize =
        object["file_size"]
            .toString()
            .toLongLong();

    receivedSize = 0;
    receiveHash.reset();

    QFileInfo fileInfo(fileName);

    QString safeName =
        fileInfo.fileName();

    receiveFile.setFileName(
        "received_" + safeName);

    if (!receiveFile.open(
            QIODevice::WriteOnly))
    {
        qDebug()
            << "Failed to open output file:"
            << receiveFile.errorString();

        return;
    }

    qDebug()
        << "Receive file:"
        << safeName;

    qDebug()
        << "Size:"
        << expectedSize;

    QJsonObject response;

    response["result"] = "accepted";

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
    {
        qDebug()
            << "Receive file is not open.";

        return;
    }


    if (receivedSize + payload.size()
        > expectedSize)
    {
        qDebug()
            << "Received more data than expected.";

        receiveFile.close();
        receiveFile.remove();

        return;
    }


    qint64 written =
        receiveFile.write(payload);


    if (written != payload.size())
    {
        qDebug()
            << "Failed to write file.";

        receiveFile.close();
        receiveFile.remove();

        return;
    }


    receiveHash.addData(payload);

    receivedSize += written;


    qDebug()
        << "Received:"
        << receivedSize
        << "/"
        << expectedSize;
}

void FileServer::handleFileFinish(
    QTcpSocket *socket,
    const QByteArray &payload)
{
    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            payload,
            &error);

    if (error.error !=
        QJsonParseError::NoError ||
        !document.isObject())
    {
        qDebug()
            << "Invalid FileFinish.";

        return;
    }


    QJsonObject object =
        document.object();


    QString clientHash =
        object["sha256"].toString();


    QString serverHash =
        QString::fromLatin1(
            receiveHash.result().toHex());


    qDebug()
        << "Client SHA256:"
        << clientHash;

    qDebug()
        << "Server SHA256:"
        << serverHash;


    // 1. 文件大小检查
    if (receivedSize != expectedSize)
    {
        qDebug()
            << "File size mismatch.";

        receiveFile.close();
        receiveFile.remove();

        QJsonObject response;

        response["message"] =
            "File size mismatch.";

        QJsonDocument responseDocument(
            response);

        QByteArray json =
            responseDocument.toJson(
                QJsonDocument::Compact);

        socket->write(
            Protocol::buildMessage(
                Protocol::MessageType::Error,
                json));

        return;
    }


    // 2. SHA-256 检查
    if (clientHash != serverHash)
    {
        qDebug()
            << "File hash mismatch.";

        receiveFile.close();
        receiveFile.remove();

        QJsonObject response;

        response["message"] =
            "File hash mismatch.";

        QJsonDocument responseDocument(
            response);

        QByteArray json =
            responseDocument.toJson(
                QJsonDocument::Compact);

        socket->write(
            Protocol::buildMessage(
                Protocol::MessageType::Error,
                json));

        return;
    }


    // 3. 校验成功
    receiveFile.close();


    qDebug()
        << "File integrity check passed.";

    QJsonObject response;

    response["result"] =
        "success";

    QJsonDocument responseDocument(
        response);

    QByteArray json =
        responseDocument.toJson(
            QJsonDocument::Compact);


    socket->write(
        Protocol::buildMessage(
            Protocol::MessageType::Ack,
            json));
}