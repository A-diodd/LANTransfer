#ifndef TRANSFERRECEIVER_H
#define TRANSFERRECEIVER_H

#include <QObject>
#include <QFile>
#include <QCryptographicHash>

#include "../common/Protocol.h"

class ClientConnection;

class TransferReceiver : public QObject
{
    Q_OBJECT

public:

    enum class ReceiveState
    {
        Idle,
        WaitingFileInfo,
        Receiving,
        Finishing,
        Completed,
        Failed
    };

public:

    explicit TransferReceiver(
        ClientConnection *connection,
        QObject *parent = nullptr);


    ReceiveState state() const;


signals:

    void logMessage(
        const QString &message);

    void transferCompleted();

    void transferFailed(
        const QString &message);

    void stateChanged(
        ReceiveState state);

    void progressChanged(
        qint64 received,
        qint64 total);


public slots:

    void onMessageReceived(
        Protocol::MessageType type,
        const QByteArray &payload);


private:

    void handleHello(
        const QByteArray &payload);

    void handleFileInfo(
        const QByteArray &payload);

    void handleFileData(
        const QByteArray &payload);

    void handleFileFinish(
        const QByteArray &payload);


    void sendError(
        const QString &message);

    void setState(
        ReceiveState newState);


private:

    ClientConnection *connection;

    ReceiveState currentState =
        ReceiveState::Idle;


    QFile receiveFile;

    qint64 expectedSize = 0;

    qint64 receivedSize = 0;


    QCryptographicHash receiveHash{
        QCryptographicHash::Sha256
    };
};

#endif // TRANSFERRECEIVER_H