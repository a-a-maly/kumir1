//
// C++ Interface: httpdaemon
//
// Description:
//
//
// Author: suboch <suboch@suboch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QTcpServer>

class HttpDaemon : public QTcpServer
{
	Q_OBJECT
public:
	HttpDaemon(quint16 port, QObject *parent = 0);
	void incomingConnection(int socket);
	void pause();
	void resume();
	bool isEnabled();
	static QString defaultPath();
	static const quint16 defaultPort;

private slots:
	void readClient();
	void discardClient();

private:
	bool disabled;

signals:
	void setProgramText(const QString &);
	void setProgramFile(const QString &);
	void runProgram();
	void doSTEP();
	void doSmallStep();
	void stop();
	void runAlg(const QString &, int);
	void setRobotEnvironment(const QString &);
};

#endif
