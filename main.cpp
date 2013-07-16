#include <QCoreApplication>
#include "server.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	Server server;

	QCoreApplication::setOrganizationName("dnet");
	QCoreApplication::setOrganizationDomain("vsza.hu");
	QCoreApplication::setApplicationName("ssh-smtp");

	return app.exec();
}
