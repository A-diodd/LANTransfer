#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>


#include "../common/Protocol.h"

class NetworkManager;

class TransferManager : public QObject
{
    Q_OBJECT

public:

    enum class TransferState
    {
        Idle,

        Connecting,

        WaitingHelloAck,

        WaitingAccept,

        Sending,

        Finishing,

        WaitingAck,

        Paused,

        Completed,

        Canceled,

        Failed
    };

public:
    explicit TransferManager(NetworkManager *networkManager,
                             QObject *parent = nullptr);

    void startTransfer(const QString &filePath);

    void pauseTransfer();

    void resumeTransfer();

    void cancelTransfer();

private slots:
    void onConnected();
    void onMessageReceived(
        Protocol::MessageType type,
        const QByteArray &payload);
    void onBytesWritten(qint64 bytes);


signals:
    void logMessage(const QString &message);
    void transferFailed(const QString &message);
    void progressChanged(
        qint64 sent,
        qint64 total);
    void stateChanged(
        TransferState state);
private:

    void setState(
        TransferState state);

    void sendFileInfo();

    void sendNextChunk();

    void sendFileFinish();

private:
    NetworkManager *networkManager;
    QString currentFilePath;
    QFile file;
    qint64 fileSize = 0;
    qint64 sentBytes = 0;
    qint64 pendingWriteBytes = 0;


    QCryptographicHash sendHash{
        QCryptographicHash::Sha256
    };
    static constexpr qint64 ChunkSize = 64 * 1024;
    TransferState state =
        TransferState::Idle;
};

#endif // TRANSFERMANAGER_H