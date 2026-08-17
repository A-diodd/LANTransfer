#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>

#include "../common/Protocol.h"

class NetworkManager;

class TransferManager : public QObject
{
    Q_OBJECT

public:
    explicit TransferManager(NetworkManager *networkManager,
                             QObject *parent = nullptr);

    void startTransfer(const QString &filePath);

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
private:
    void sendNextChunk();

private:
    NetworkManager *networkManager;
    QString currentFilePath;
    QFile sendFile;
    qint64 fileSize = 0;
    qint64 sentBytes = 0;
    bool waitingForWrite = false;
    static constexpr qint64 ChunkSize = 64 * 1024;
};

#endif // TRANSFERMANAGER_H