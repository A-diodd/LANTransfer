#include<QCoreApplication>
#include "FileServer.h"
#include <QDebug>

int main(int argc,char *argv[])
{
	QCoreApplication app(argc,argv);
	FileServer server;
	if(!server.start(9000))
	{
		return -1;
	}
    qDebug() << "File server started on port 9000.";
	
    return app.exec();
}