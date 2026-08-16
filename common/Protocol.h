#ifndef PROTOCOL_H
#define PROTOCOL_h

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
	
	QByteArray buildMessage(MessageType type,const QJsonObject &payload);

	bool parseMessage( QByteArray &buffer,MessageType &type,QJsonObject &payload);
}
#endif // PROTOCOL_H