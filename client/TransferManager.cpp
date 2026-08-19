#include "TransferManager.h"

#include"NetworkManager.h"
#include"../common/Protocol.h"

#include<QFileInfo>
#include<QJsonObject>
#include<QDebug>

TransferManager::TransferManager(
	NetworkManager *networkManager,
	QObject *parent)
	: QObject(parent),
	  networkManager(networkManager)
{
	connect(networkManager,
			&NetworkManager::connected,
			this,
			&TransferManager::onConnected);
			
	connect(networkManager,
			&NetworkManager::messageReceived,
			this,
			&TransferManager::onMessageReceived);
	connect(networkManager,
            &NetworkManager::bytesWritten,
            this,
            &TransferManager::onBytesWritten);

		
}

void TransferManager::startTransfer(
	const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	
	if(!fileInfo.exists()|| !fileInfo.isFile())
	{
		emit transferFailed(
			"File does not exits.");
			
		return;
	}
	
	currentFilePath =filePath;
    fileSize = fileInfo.size();
    sentBytes = 0;
	emit logMessage(
		"[INFO] Preparing file:"
		+ fileInfo.fileName());
	
	networkManager->connectToServer(
		"127.0.0.1",
		 9000);
}

void TransferManager::onConnected()
{
    QFileInfo fileInfo(currentFilePath);

    QJsonObject payload;

    payload["file_name"] =
        fileInfo.fileName();

    payload["file_size"] =
        QString::number(fileInfo.size());

    QJsonDocument document(payload);

    QByteArray json =
        document.toJson(
            QJsonDocument::Compact);

    QByteArray message =
        Protocol::buildMessage(
            Protocol::MessageType::FileInfo,
            json);

    networkManager->sendData(message);

    emit logMessage(
        "[INFO] FileInfo sent.");
}

//服务端收到回应的时候开始处理。。。
void TransferManager::onMessageReceived(
    Protocol::MessageType type,
    const QByteArray &payload)
{
    if (type ==
        Protocol::MessageType::HelloAck)
    {
        emit logMessage(
            "[INFO] HelloAck received.");

        return;
    }

    if (type ==
        Protocol::MessageType::FileAccept)
    {
        emit logMessage(
            "[INFO] Server accepted file.");
        
		sendFile.setFileName(currentFilePath);
		
		if(!sendFile.open(QIODevice::ReadOnly))
		{
			emit transferFailed(
				"Failed to open file.");
			return;	
		}   
		fileSize=sendFile.size();
		sentBytes=0;
		
		waitingForWrite =false;
		pendingWriteBytes =0;
		finishSent= false;
		sendHash.reset(); 
		emit logMessage("[INFO] Start sending file...");
		sendNextChunk();

        return;
    }
    if(type ==Protocol::MessageType::Ack)
    {
    	if(sendFile.isOpen())
    		sendFile.close();
    	
    	emit progressChanged(
			fileSize,
			fileSize);
		
		emit logMessage(
			"[INFO] File transfer completed.");
		
		return;
	}
	
	if(type ==Protocol::MessageType::Error)
	{
		if(sendFile.isOpen())
			sendFile.close();
		QJsonParseError error;
		QJsonDocument document=
			QJsonDocument::fromJson(
				payload,
				&error);
		QString message="Server reported transfer error";
		
        if (error.error ==
            QJsonParseError::NoError &&
            document.isObject())
        {
            QJsonObject object =
                document.object();

            message =
                object["message"]
                    .toString(message);
        }

        emit transferFailed(message);
	}
}

void TransferManager::sendNextChunk()
{
    if (!sendFile.isOpen())
        return;

    if (waitingForWrite)
        return;

    // 所有文件数据已经读取完
    if (sentBytes >= fileSize)
    {
        if (finishSent)
            return;

        QByteArray hash =
            sendHash.result().toHex();

        QJsonObject payload;

        payload["sha256"] =
            QString::fromLatin1(hash);

        payload["file_size"] =
            QString::number(fileSize);

        QJsonDocument document(payload);

        QByteArray json =
            document.toJson(
                QJsonDocument::Compact);

        QByteArray message =
            Protocol::buildMessage(
                Protocol::MessageType::FileFinish,
                json);

        networkManager->sendData(message);

        pendingWriteBytes =
            message.size();

        waitingForWrite = true;
        finishSent = true;

        emit logMessage(
            "[INFO] FileFinish sent.");

        return;
    }


    QByteArray chunk =
        sendFile.read(ChunkSize);

    if (chunk.isEmpty())
    {
        if (sendFile.atEnd())
        {
            sentBytes = fileSize;

            sendNextChunk();
        }

        return;
    }


    // 增量计算 SHA-256
    sendHash.addData(chunk);


    QByteArray message =
        Protocol::buildMessage(
            Protocol::MessageType::FileData,
            chunk);


    networkManager->sendData(message);


    sentBytes += chunk.size();


    pendingWriteBytes =
        message.size();

    waitingForWrite = true;


    emit progressChanged(
        sentBytes,
        fileSize);
}

void TransferManager::onBytesWritten(
    qint64 bytes)
{
    if (!waitingForWrite)
        return;

    pendingWriteBytes -= bytes;

    if (pendingWriteBytes > 0)
        return;

    waitingForWrite = false;
    pendingWriteBytes = 0;

    if (finishSent)
    {
        if (sendFile.isOpen())
            sendFile.close();

        emit logMessage(
            "[INFO] FileFinish sent completely.");

        return;
    }

    if (networkManager->isConnected())
    {
        sendNextChunk();
    }
}