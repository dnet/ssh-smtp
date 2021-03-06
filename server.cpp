/*
 * server.cpp - SMTP server with SSH bridge
 *
 * Copyright (c) 2013 András Veres-Szentkirályi
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "server.h"
#include <stdio.h>
#include <QRegExp>

#define BUFSIZE 4096
#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_PORT 2525
#define DEFAULT_SSH "ssh"
#define LOCAL_SMTP "localhost:25"
#define EMAIL_RE "[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}"

Server::Server(QObject* parent): QObject(parent) {
	QSettings settings;
	connect(&server, SIGNAL(newConnection()),
			this, SLOT(acceptConnection()));

	QHostAddress addr(settings.value("daemon/addr", DEFAULT_ADDR).toString());
	quint16 port = settings.value("daemon/port", DEFAULT_PORT).toInt();
	server.listen(addr, port);
}

Server::~Server() {
	server.close();
	closeSSH();
}

void Server::acceptConnection() {
	client = server.nextPendingConnection();
	connect(client, SIGNAL(readyRead()),
			this, SLOT(startRead()));
	connect(client, SIGNAL(disconnected()),
			this, SLOT(clientDisconnected()));

	client->write("220 ssh-smtp.vsza.hu SSH-SMTP\r\n");
	ssh = NULL;
}

void Server::startRead() {
	char buffer[BUFSIZE];

	if (ssh == NULL) {
		while (client->readLine(buffer, BUFSIZE)) {
			connectBuffer.append(buffer);

			if (strncasecmp(buffer, "HELO ", 5) == 0 ||
					strncasecmp(buffer, "EHLO ", 5) == 0) {
				client->write("250 Hello ");
				client->write(buffer + 5);
			} else if (strncasecmp(buffer, "MAIL FROM:", 10) == 0) {
				QRegExp mail(EMAIL_RE, Qt::CaseInsensitive);
				QString remain(buffer + 10);
				if (mail.indexIn(remain)) {
					if (!buildTunnel(mail.cap(0))) {
						client->write("500 Unknown sender\r\n");
						client->close();
						break;
					}
				}
			} else {
				client->write("500 Unexpected command: ");
				client->write(buffer);
			}
		}
	} else {
		while (qint64 bytes = client->read(buffer, BUFSIZE)) {
			if (bytes == -1) {
				closeSSH();
				break;
			}
			ssh->write(buffer, bytes);
		}
	}
}

void Server::clientDisconnected() {
	closeSSH();
}

void Server::closeSSH() {
	if (ssh != NULL) {
		ssh->terminate();
	}
}

bool Server::buildTunnel(const QString &addr) {
	QSettings settings;
	QMutexLocker lock(&sshChannelLock);

	const QVariant target(settings.value(addr + "/host"));
	if (target.isNull()) {
		return false;
	} else {
		const QString program(settings.value("ssh/exec", DEFAULT_SSH).toString());
		QStringList arguments;

		if (settings.contains(addr + "/cmd")) {
			const QString remote(settings.value(addr + "/cmd", QVariant()).toString());
			arguments << target.toString() << remote;
		} else {
			arguments << target.toString() << "-W" << LOCAL_SMTP;
		}

		closeSSH();
		ssh = new QProcess();
		connect(ssh, SIGNAL(started()), this, SLOT(sshStarted()));
		connect(ssh, SIGNAL(finished(int, QProcess::ExitStatus)),
				this, SLOT(sshFinished(int, QProcess::ExitStatus)));
		ssh->start(program, arguments);
		return true;
	}
}

void Server::sshStarted() {
	QMutexLocker lock(&sshChannelLock);

	skippedHeloResponse = false;
	ssh->write(connectBuffer);
	connectBuffer.clear();

	connect(ssh, SIGNAL(readyReadStandardError()), this, SLOT(sshStdErrReady()));
	connect(ssh, SIGNAL(readyReadStandardOutput()), this, SLOT(sshStdOutReady()));
}

void Server::sshFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	Q_UNUSED(exitCode);
	Q_UNUSED(exitStatus);

	QMutexLocker lock(&sshChannelLock);

	if (client != NULL) client->close();
	if (ssh != NULL) {
		delete ssh;
		ssh = NULL;
	}
}

void Server::sshStdErrReady() {
	QMutexLocker lock(&sshChannelLock);
	char buffer[BUFSIZE];

	ssh->setReadChannel(QProcess::StandardError);
	while (ssh->readLine(buffer, BUFSIZE)) {
		fprintf(stderr, "[ssh stderr] %s", buffer);
	}
}

void Server::sshStdOutReady() {
	QMutexLocker lock(&sshChannelLock);
	char buffer[BUFSIZE];

	ssh->setReadChannel(QProcess::StandardOutput);
	while (ssh->readLine(buffer, BUFSIZE)) {
		if (!skippedHeloResponse) {
			if (strncmp(buffer, "250 ", 4) == 0) {
				skippedHeloResponse = true;
			}
		} else {
			client->write(buffer);
		}
	}
}
