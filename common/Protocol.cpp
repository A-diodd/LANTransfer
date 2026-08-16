#include "Protocol.h"

#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>


namespace Protocol
{


QByteArray buildMessage(
    MessageType type,
    const QJsonObject &payload)
{

    QJsonDocument document(payload);


    QByteArray body =
        document.toJson(
            QJsonDocument::Compact);


    QByteArray message;


    QDataStream stream(
        &message,
        QIODevice::WriteOnly);


    stream.setByteOrder(
        QDataStream::BigEndian);


    stream << quint32(body.size());

    stream << quint32(type);


    message.append(body);


    return message;
}



bool parseMessage(
    QByteArray &buffer,
    MessageType &type,
    QJsonObject &payload)
{

    //长度不足消息头
    if(buffer.size() < 8)
        return false;



    QDataStream stream(buffer);

    stream.setByteOrder(
        QDataStream::BigEndian);



    quint32 bodySize;

    quint32 msgType;



    stream >> bodySize;

    stream >> msgType;



    //数据还没接收完整

    if(buffer.size() < 8 + bodySize)
        return false;



    QByteArray body =
        buffer.mid(
            8,
            bodySize);



    buffer.remove(
        0,
        8 + bodySize);



    type =
        static_cast<MessageType>(
            msgType);



    QJsonParseError error;


    QJsonDocument document =
        QJsonDocument::fromJson(
            body,
            &error);



    if(error.error !=
       QJsonParseError::NoError)
    {
        return false;
    }


    payload =
        document.object();



    return true;
}


}