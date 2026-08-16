#include "Protocol.h"
#include <QJsonDocument>
namespace Protocol
{
    QString messageTypeToString(MessageType type)
	{
		switch(type)
		{
		case MessageType::Hello:
	        return "hello";
	
	    case MessageType::HelloAck:
	        return "hello_ack";
	
	    case MessageType::FileInfo:
	        return "file_info";
	
	    case MessageType::FileAccept:
	        return "file_accept";
	
	    case MessageType::FileData:
	        return "file_data";
	
	    case MessageType::FileFinish:
	        return "file_finish";
	
	    case MessageType::Pause:
	        return "pause";
	
	    case MessageType::Resume:
	        return "resume";
	
	    case MessageType::Cancel:
	        return "cancel";
	
	    case MessageType::Ack:
	        return "ack";
	
	    case MessageType::Error:
	        return "error";
			
		}
		return"unknow";

	} 
		MessageType messageTypeFromString(const QString &type)
	{
	    if (type == "hello")
	        return MessageType::Hello;
	
	    if (type == "hello_ack")
	        return MessageType::HelloAck;
	
	    if (type == "file_info")
	        return MessageType::FileInfo;
	
	    if (type == "file_accept")
	        return MessageType::FileAccept;
	
	    if (type == "file_data")
	        return MessageType::FileData;
	
	    if (type == "file_finish")
	        return MessageType::FileFinish;
	
	    if (type == "pause")
	        return MessageType::Pause;
	
	    if (type == "resume")
	        return MessageType::Resume;
	
	    if (type == "cancel")
	        return MessageType::Cancel;
	
	    if (type == "ack")
	        return MessageType::Ack;
	
	    return MessageType::Error;
	}
	QByteArray buildMessage(
	    MessageType type,
	    const QJsonObject &payload)
	{
	    QJsonObject object = payload;
	
	    object["type"] =
	        messageTypeToString(type);
	
	    QJsonDocument document(object);
	
	    return document.toJson(
	        QJsonDocument::Compact);
	}
	
	bool parseMessage(const QByteArray &data, MessageType &type, QJsonObject &payload)
	{
		QJsonParseError error;
		
		QJsonDocument document =
			QJsonDocument::fromJson(
				data,
				&error);
				
        if(error.error!=QJsonParseError::NoError)
		{
			return false;
		}
		
		if(!document.isObject())
			return false;
			
		QJsonObject object=document.object();
		
		if(!object.contains("type"))
			return false;
			
		type=messageTypeFromString(object["type"].toString());
		
		payload = object;
		
		return true; 
	}


}

