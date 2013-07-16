#include <QCoreApplication>
#include "server.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	QCoreApplication::setOrganizationName("dnet");
	QCoreApplication::setOrganizationDomain("vsza.hu");
	QCoreApplication::setApplicationName("ssh-smtp");

	Server server;

	return app.exec();
}
