#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

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

signals:
    void logMessage(const QString &message);
    void transferFailed(const QString &message);

private:
    NetworkManager *networkManager;
    QString currentFilePath;
};

#endif // TRANSFERMANAGER_H