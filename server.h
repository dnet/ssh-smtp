/*
 * server.h - SMTP server with SSH bridge
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
#ifndef SERVER_H_
#define SERVER_H_

#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>

class Server: public QObject {
	Q_OBJECT
	public:
		explicit Server(QObject * parent = 0);
		~Server();
		public slots:
			void acceptConnection();
			void clientDisconnected();
			void sshStarted();
			void sshFinished(int exitCode, QProcess::ExitStatus exitStatus);
			void sshStdErrReady();
			void sshStdOutReady();
			void startRead();
	private:
		QTcpServer server;
		QTcpSocket* client;
		QByteArray connectBuffer;
		QProcess* ssh;
		QMutex sshChannelLock;
		bool skippedHeloResponse;
		bool buildTunnel(const QString &addr);
		void closeSSH();
};

#endif  // SERVER_H_
