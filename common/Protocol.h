#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QJsonObject>

namespace Protocol
{
	enum class MessageType
	{
		Hello =1,
		HelloAck,
		FileInfo,
		FileAccept,
		FileData,
		FileFinish,
		Pause,
		Resume,
		Cancel,
		Ack,
		Error
	};
	QString messageTypeToString(MessageType type);
	
	MessageType messageTypeFromString(const QString &type);
	
	QByteArray buildMessage(
        MessageType type,
        const QByteArray &payload);

	bool parseMessage(
		QByteArray &buffer,    //buffer可能包括或者不包括整个文件，他就是一个二进制数据。
		MessageType &type,     //文件的类型
		QByteArray &payload);  //payload就是整个文件的内容
}
#endif // PROTOCOL_H