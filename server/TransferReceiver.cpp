#include "TransferReceiver.h"

#include "ClientConnection.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>


TransferReceiver::TransferReceiver(
    ClientConnection *connection,
    QObject *parent)
    :
    QObject(parent),
    connection(connection)
{
}


// =========================
// 状态
// =========================

TransferReceiver::ReceiveState
TransferReceiver::state() const
{
    return currentState;
}


void TransferReceiver::setState(
    ReceiveState newState)
{
    if(currentState == newState)
        return;

    currentState = newState;

    emit stateChanged(currentState);
}


// =========================
// 消息入口
// =========================

void TransferReceiver::onMessageReceived(
    Protocol::MessageType type,
    const QByteArray &payload)
{
    switch(type)
    {
    case Protocol::MessageType::Hello:
        handleHello(payload);
        break;


    case Protocol::MessageType::FileInfo:
        handleFileInfo(payload);
        break;


    case Protocol::MessageType::FileData:
        handleFileData(payload);
        break;


    case Protocol::MessageType::FileFinish:
        handleFileFinish(payload);
        break;


    default:
        break;
    }
}


// =========================
// Hello
// =========================

void TransferReceiver::handleHello(
    const QByteArray &payload)
{
    if(currentState != ReceiveState::Idle)
    {
        sendError(
            "Hello received in invalid state."
        );
        return;
    }
    Q_UNUSED(payload);


    QByteArray response;


    connection->sendMessage(
        Protocol::MessageType::HelloAck,
        response
    );


    emit logMessage(
        "[INFO] HelloAck sent."
    );


    setState(
        ReceiveState::WaitingFileInfo
    );
}


// =========================
// FileInfo
// =========================

void TransferReceiver::handleFileInfo(
    const QByteArray &payload)
{
    if(currentState != ReceiveState::WaitingFileInfo)
    {
        sendError(
            "FileInfo received in invalid state."
        );
        return;
    }
    QJsonParseError error;


    QJsonDocument document =
        QJsonDocument::fromJson(
            payload,
            &error
        );


    if(error.error !=
       QJsonParseError::NoError ||
       !document.isObject())
    {
        sendError(
            "Invalid FileInfo JSON."
        );

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


    if(fileName.isEmpty())
    {
        sendError(
            "Invalid file name."
        );

        return;
    }


    if(expectedSize < 0)
    {
        sendError(
            "Invalid file size."
        );

        return;
    }


    // 防止路径穿越
    QFileInfo fileInfo(fileName);

    QString safeName =
        fileInfo.fileName();


    if(safeName.isEmpty())
    {
        sendError(
            "Invalid file name."
        );

        return;
    }


    receiveFile.setFileName(
        "received_" + safeName
    );


    if(!receiveFile.open(
           QIODevice::WriteOnly))
    {
        sendError(
            "Failed to open output file."
        );

        return;
    }


    receivedSize = 0;

    receiveHash.reset();


    emit logMessage(
        "[INFO] Receive file: "
        + safeName
    );


    emit logMessage(
        "[INFO] Size: "
        + QString::number(expectedSize)
    );


    connection->sendMessage(
        Protocol::MessageType::FileAccept,
        QByteArray()
    );


    emit logMessage(
        "[INFO] FileAccept sent."
    );


    setState(
        ReceiveState::Receiving
    );
}


// =========================
// FileData
// =========================

void TransferReceiver::handleFileData(
    const QByteArray &payload)
{
    if(currentState !=
       ReceiveState::Receiving)
    {
        sendError(
            "FileData received in invalid state."
        );

        return;
    }


    if(!receiveFile.isOpen())
    {
        sendError(
            "Receive file is not open."
        );

        return;
    }


    if(receivedSize + payload.size()
       > expectedSize)
    {
        receiveFile.close();
        receiveFile.remove();


        sendError(
            "Received more data than expected."
        );

        return;
    }


    qint64 written =
        receiveFile.write(payload);


    if(written != payload.size())
    {
        receiveFile.close();
        receiveFile.remove();


        sendError(
            "Failed to write file."
        );

        return;
    }


    receiveHash.addData(payload);


    receivedSize += written;


    emit progressChanged(
        receivedSize,
        expectedSize
    );
}


// =========================
// FileFinish
// =========================

void TransferReceiver::handleFileFinish(
    const QByteArray &payload)
{
    if(currentState !=
       ReceiveState::Receiving)
    {
        sendError(
            "FileFinish received in invalid state."
        );

        return;
    }


    QJsonParseError error;


    QJsonDocument document =
        QJsonDocument::fromJson(
            payload,
            &error
        );


    if(error.error !=
       QJsonParseError::NoError ||
       !document.isObject())
    {
        sendError(
            "Invalid FileFinish."
        );

        return;
    }


    QJsonObject object =
        document.object();


    QString clientHash =
        object["sha256"].toString();


    QString serverHash =
        QString::fromLatin1(
            receiveHash.result().toHex()
        );


    emit logMessage(
        "[INFO] Client SHA256: "
        + clientHash
    );


    emit logMessage(
        "[INFO] Server SHA256: "
        + serverHash
    );


    setState(
        ReceiveState::Finishing
    );


    // 文件大小检查
    if(receivedSize != expectedSize)
    {
        receiveFile.close();
        receiveFile.remove();


        sendError(
            "File size mismatch."
        );

        return;
    }


    // SHA256检查
    if(clientHash != serverHash)
    {
        receiveFile.close();
        receiveFile.remove();


        sendError(
            "File hash mismatch."
        );

        return;
    }


    // 校验成功
    receiveFile.close();


    connection->sendMessage(
        Protocol::MessageType::Ack,
        QByteArray()
    );


    emit logMessage(
        "[INFO] File integrity check passed."
    );


    emit logMessage(
        "[INFO] ACK sent."
    );


    setState(
        ReceiveState::Completed
    );


    emit transferCompleted();
}


// =========================
// Error
// =========================

void TransferReceiver::sendError(
    const QString &message)
{
    QJsonObject object;

    object["message"] = message;


    QByteArray payload =
        QJsonDocument(object)
            .toJson(
                QJsonDocument::Compact
            );


    connection->sendMessage(
        Protocol::MessageType::Error,
        payload
    );


    emit logMessage(
        "[ERROR] "
        + message
    );


    setState(
        ReceiveState::Failed
    );


    emit transferFailed(message);
}