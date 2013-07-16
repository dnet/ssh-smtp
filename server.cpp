#include "server.h"
#include <iostream>

#define BUFSIZE 1024
#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_PORT 2525

using namespace std;

Server::Server(QObject* parent): QObject(parent)
{
	QSettings settings;
	connect(&server, SIGNAL(newConnection()),
			this, SLOT(acceptConnection()));

	QHostAddress addr(settings.value("daemon/addr", DEFAULT_ADDR).toString());
	quint16 port = settings.value("daemon/port", DEFAULT_PORT).toInt();
	server.listen(addr, port);
}

Server::~Server()
{
	server.close();
}

void Server::acceptConnection()
{
	client = server.nextPendingConnection();
	connect(client, SIGNAL(readyRead()),
			this, SLOT(startRead()));

	client->write("220 ssh-smtp.vsza.hu ESMTP SSH-SMTP\r\n");
}

void Server::startRead()
{
	char buffer[BUFSIZE];

	while (client->readLine(buffer, BUFSIZE)) {
		QString line(buffer);

		if (line.startsWith("HELO ")) {
			client->write("250 Hello ");
			client->write(buffer + 5);
		}
		else if (line.startsWith("MAIL FROM:")) {
			cout << buffer + 10 << endl; // TODO
			client->close();
			break;
		}
	}
}
