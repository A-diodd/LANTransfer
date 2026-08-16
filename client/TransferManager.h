#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QObject>
#include <QString>

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
    void onMessageReceived(Protocol::MessageType type,const QJsonObject &payload);

signals:
    void logMessage(const QString &message);
    void transferFailed(const QString &message);

private:
    NetworkManager *networkManager;
    QString currentFilePath;
};

#endif // TRANSFERMANAGER_H