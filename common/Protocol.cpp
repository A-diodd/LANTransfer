#include "Protocol.h"

#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>

// 
namespace Protocol
{

QByteArray buildMessage(
    MessageType type,
    const QByteArray &payload)
{
    QByteArray message;

    QDataStream stream(
        &message,
        QIODevice::WriteOnly);

    stream.setByteOrder(
        QDataStream::BigEndian);

    stream << quint32(payload.size());
    stream << quint32(type);

    message.append(payload);

    return message;
}



bool parseMessage(
    QByteArray &buffer,
    MessageType &type,
    QByteArray &payload)
{
    // 连消息头都没收到完整
    if (buffer.size() < 8)
        return false;

    QDataStream stream(buffer);

    stream.setByteOrder(
        QDataStream::BigEndian);

    quint32 bodySize;
    quint32 messageType;

    stream >> bodySize;
    stream >> messageType;

    // 整个消息还没接收完整
    if (buffer.size() < 8 + bodySize)
        return false;

    payload = buffer.mid(8, bodySize);

    buffer.remove(0, 8 + bodySize);

    type = static_cast<MessageType>(messageType);

    return true;
}

}