#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>

class Server: public QObject {
	Q_OBJECT
	public:
		Server(QObject * parent = 0);
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
