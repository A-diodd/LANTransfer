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
			&NetworkManager::dataReceived,
			this,
			&TransferManager::onDataReceived);
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
	
	payload["file_name"] =fileInfo.fileName();
	
	payload["file_size"] =QString::number(fileInfo.size());
	
	QByteArray message=
		Protocol::buildMessage(
			Protocol::MessageType::FileInfo,
			payload);
	
	networkManager->sendData(message);
	
	emit logMessage("[INFO] FileInfo sent.");
}

void TransferManager::onDataReceived(
	const QByteArray &data)
{
	Protocol::MessageType type;
	QJsonObject payload;
	
    if(!Protocol::parseMessage(
			data,
			type,
			payload))
	{
		emit transferFailed(
			"Invalid server response.");

        return ;
	}
	if(type==Protocol::MessageType::FileAccept)
	{
		emit logMessage(
			"[INFO] Server accepted file.");
	 } 
}
