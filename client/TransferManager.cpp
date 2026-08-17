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
		emit logMessage("[INFO] Start sending file...");
		sendNextChunk();

        return;
    }
}

void TransferManager::sendNextChunk()
{
	if(!sendFile.isOpen())
		return;
	
    if(sentBytes>=fileSize)
	{
		sendFile.close();
		emit logMessage(
            "[INFO] File data sent.");
		return;
		
	}
	QByteArray chunk=sendFile.read(ChunkSize);
	
	if(chunk.isEmpty())
	{
		if(sendFile.atEnd())
		{
			sendFile.close();
			emit logMessage(
				"[INFO] File data sent.");
		}
		return;
	}
	QByteArray message=
		Protocol::buildMessage(
            Protocol::MessageType::FileData,
			chunk);
	networkManager->sendData(message);
    sentBytes+=chunk.size();
	emit progressChanged(
		sentBytes,
		fileSize);
	
	waitingForWrite =true;
}

void TransferManager::onBytesWritten(
	qint64 bytes)
{
	Q_UNUSED(bytes);
	
	if(!waitingForWrite)
		return;
	
	if(networkManager->isConnected())
	{
		waitingForWrite=false;
		sendNextChunk();
	}
}