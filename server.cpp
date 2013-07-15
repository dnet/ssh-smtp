#include "server.h"
#include <iostream>

#define BUFSIZE 1024

using namespace std;

Server::Server(QObject* parent): QObject(parent)
{
	connect(&server, SIGNAL(newConnection()),
			this, SLOT(acceptConnection()));

	server.listen(QHostAddress::Any, 2525);
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
