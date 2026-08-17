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

        // 下一步：
        // 开始发送文件
        return;
    }
}

