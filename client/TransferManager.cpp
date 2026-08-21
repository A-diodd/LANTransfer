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
	if(state!=TransferState::Idle&&
	   state!=TransferState::Completed&&
	   state!=TransferState::Failed&&
	   state!=TransferState::Canceled)
	{
		return;
	}
	
	QFileInfo info(filePath);
	
	if(!info.exists()|| !info.isFile())
	{
		emit transferFailed(
			"File does not exits.");
		
		setState(TransferState::Failed);
		return;
	}
	
    currentFilePath = filePath;

    file.setFileName(
        currentFilePath);

    if(!file.open(
        QIODevice::ReadOnly))
    {

        emit transferFailed(
            "Cannot open file");

        setState(
            TransferState::Failed);

        return;
    }

    fileSize =
        file.size();

    sentBytes = 0;

    sendHash.reset();

    setState(
        TransferState::Connecting);
        
    networkManager->connectToServer(
        "127.0.0.1",
        9000);

}

void TransferManager::onConnected()
{
    setState(
        TransferState::WaitingHelloAck);


    QByteArray payload;


    QByteArray message =
        Protocol::buildMessage(
            Protocol::MessageType::Hello,
            payload);


    networkManager->sendData(message);


    emit logMessage(
        "[INFO] Hello sent.");
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
        
        sendFileInfo();
    
        return;
    }

    if (type ==
        Protocol::MessageType::FileAccept)
    {
        emit logMessage(
            "[INFO] Server accepted file.");
        
		fileSize=file.size();
		sentBytes=0;
		file.seek(0);
		pendingWriteBytes =0;

		sendHash.reset(); 
        setState(
            TransferState::Sending);
		emit logMessage("[INFO] Start sending file...");
		sendNextChunk();

        return;
    }
    if(type ==Protocol::MessageType::Ack)
    {
    	if(file.isOpen())
    		file.close();

        setState(
            TransferState::Completed
        );
    	emit progressChanged(
			fileSize,
			fileSize);
		
		emit logMessage(
			"[INFO] File transfer completed.");
		
		return;
	}
	
	if(type ==Protocol::MessageType::Error)
	{
		if(file.isOpen())
			file.close();
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
        setState(
        TransferState::Failed
    );
	}
}

void TransferManager::sendFileInfo()
{
	QFileInfo info(
		currentFilePath);
	
	QJsonObject payload;
	payload["file_name"]=info.fileName();
	payload["file_size"]=QString::number(info.size());
	QJsonDocument document(payload);
	QByteArray json=document.toJson(QJsonDocument::Compact);
	QByteArray message=
		Protocol::buildMessage(
			Protocol::MessageType::FileInfo,
			json);
	networkManager->sendData(message);
	emit logMessage("[INFO] FileInfo sent.");
    setState(
    TransferState::WaitingAccept
);
}

void TransferManager::sendNextChunk()
{
    if (!file.isOpen())
        return;

    if (pendingWriteBytes > 0)
        return;

    // 所有文件数据已经读取完
    if (sentBytes >= fileSize)
    {
        sendFileFinish();
        return;
    }

    QByteArray chunk =
        file.read(ChunkSize);

    if (chunk.isEmpty())
    {
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

    emit progressChanged(
        sentBytes,
        fileSize);
}

void TransferManager::onBytesWritten(
    qint64 bytes)
{
    pendingWriteBytes -= bytes;

    if (pendingWriteBytes > 0)
        return;

    pendingWriteBytes = 0;

    if(state ==
       TransferState::WaitingAck)
    {
        emit logMessage(
            "[INFO] Waiting server ACK."
        );

        return;
    }

    if(state ==
       TransferState::Sending)
    {
        sendNextChunk();
    }
}

void TransferManager::setState(
	TransferState newState)
{
	if(state==newState)
		return;
	
	state=newState;
	emit stateChanged(state);
	qDebug()
		<<"Transfer state changed:"
		<<static_cast<int>(state);
}

void TransferManager::sendFileFinish()
{
    setState(
        TransferState::Finishing
    );

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

    networkManager->sendData(
        message
    );

    pendingWriteBytes =
        message.size();

    setState(
        TransferState::WaitingAck
    );

    emit logMessage(
        "[INFO] FileFinish sent."
    );
}