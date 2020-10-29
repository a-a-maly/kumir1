/****************************************************************************
**
** Copyright (C) 2004-2008 NIISI RAS. All rights reserved.
**
** This file is part of the KuMir.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef KNPConnect_H
#define KNPConnect_H
#include "int_proga.h"
#include "kum_tables.h"
#include <QObject>
#include <QAbstractSocket>
#include <QTcpServer>
class QTcpSocket;

#define HAND_SHAKE       0
#define EXT_CMD          1
#define OK_RPL           2
#define RETURN           3
#define HS_OK            4
#define LIST             5
#define HELP             6

#define ALG_DESC         10
#define LOOP_BACK        11

#define LOCKGUI          21
#define UNLOCKGUI        22
#define RESET            23

#define EXIT_CMD         98
#define DUMMY            99
#define ERROR            100
#define PARCE_ERROR      101

class KumValueStackElem;
class KumModules;
class KNPEvent : public QEvent
{
public:
	KNPEvent(int type, const QString &s) : QEvent(static_cast<Type>(type)), s(s) {};
	const QString &str()
	{
		return s;
	};
private:
	QString s;
};
struct KNPCommand {
	QString text;
	uint type;
	QList<KumValueStackElem> arguments;
};

class KNPParcer
{
public:
	KNPParcer() {};
	KNPCommand ParceCommand(QString command);

};


class KNPConnection : public QObject
{
	Q_OBJECT
public:
	KNPConnection(QObject *parent = 0);
	KNPConnection(QTcpSocket *friendConn);


	void Connect(QString addr, quint16 port);

	QString name, Addr;
	uint Port;
	QStringList funcList()
	{
		return alg_desc;
	};
	QTcpSocket *Socket()
	{
		return tcpSocket;
	};
	void emitGetOk();
	void analizeRequest(QString line);
signals:
	void Ready();
	void Error(QString text);//Ошибки подключения-взаимодействия
	void GetModuleError(QString text);//Ошибки выполнения
	void getFuncList();
	void getOK();
	void getReturn(QString value);

public slots:
	void sendCmd(QString text); //Послать сообщение
	void showIsp();
	void showPult();
	//void runCommand(int funct_id,QString text); //Вызвать фунцию
private slots:
	void readData(); //Чтение из сокета
	void socketError(QAbstractSocket::SocketError socketError); //Обработка ошибок сокета
	void Connected();
	void Disconnected();
	void switchMode(int mode);

private:


	QTcpSocket *tcpSocket;
	KNPParcer Parcer;
	bool onLine;
	bool HSOK;
	QStringList alg_desc;

	int oldType;
};

class KumClient
{
public:
	KumClient(QTcpSocket *Socket)
	{
		HSOK = false;
		tcpSocket = Socket;
		lastCommand.type = DUMMY;
	};

	QTcpSocket *tcpSocket;
	bool HSOK;
	KNPCommand lastCommand;
};



class KNPServer : public QObject
{
	Q_OBJECT
public:
	KNPServer(QObject *parent = 0);
	void OpenPort(QString addr, quint16 port);
	int ClientId(QTcpSocket *tcpSocket)
	{

		for (int i = 0; i < ClientList.count(); i++)
			if (ClientList[i].tcpSocket == tcpSocket) {
				return i;
			}
		return -1;
	};
	void sendMessage(int clientId, QString newMessage)
	{
		if (clientId < 0 || (clientId > ClientList.count() - 1)) {
			qDebug("KNPServer::No such client!");
			return;
		};
		sendMessage(ClientList[clientId].tcpSocket, newMessage);
	};
	void sendClientReplay(QString lastClientCommand, QString toSend)
	{
		for (int i = 0; i < ClientList.count(); i++) {
			if (ClientList[i].lastCommand.text == lastClientCommand) {
				sendMessage(i, toSend);
			};
		};
	};
	void waitForFriends()
	{
		waitForFriends_flag = true;
	};
	bool isListening()
	{
		return Server.isListening();
	};
signals:
	void Ready();
	void Error(QString text);
	void Friend(QString name);
public slots:
	//void sendCmd(QString text); //Послать сообщение
private slots:
	//void readData(); //Чтение из сокета
	void socketError(QAbstractSocket::SocketError socketError); //Обработка ошибок сокета
	void ClientConnected();
	void ClientDisconnected();
	void reciveMessage();
	void deleteConnection();
	void servReplay(QString text, int client_id);
private:
	void sendMessage(QTcpSocket *tcpSocket, QString newMessage);
	QString helpCommand(QString command);
	void sendList(QTcpSocket *tcpSocket);
	inline bool hs_ok(QTcpSocket *tcpSocket)
	{
		int id = ClientId(tcpSocket);
		bool result = false;
		if (id > -1) {
			result = ClientList[id].HSOK;
		}
		return result;
	}
	inline void set_hs_ok(QTcpSocket *tcpSocket)
	{
		int id = ClientId(tcpSocket);
		if (id > -1) {
			ClientList[id].HSOK = true;
		}
	}
	QTcpServer Server;
	KNPParcer Parcer;
	bool onLine;
	bool HSOK;
	bool waitForFriends_flag;
	QList<KumClient> ClientList;

};

class SignalCrosser : public QObject
{
	Q_OBJECT
public:
	SignalCrosser(QObject *parent = 0)
	{
		Q_UNUSED(parent);
	};
	void setModulesNRun(KumModules *modules, Kumir_Run *run)
	{
		Modules = modules;
		Run = run;
	};
	void RunStart();
	void RunEnds();
	QString DoCommand(KNPCommand command, int client_id, QTcpSocket *Client);
	void insertText(QString text);
signals:
	void do_b_shag();
	void do_Call(QString name, QString arg, int client_id);
	void do_stop();
	void do_start_run();
	void do_start_alg(QString alg, int client_id);

	void do_message(QString message);
	void do_set_text(QString text);
	void Sync();
public slots:
	void OK();
	void LostIsp(int module_id);
private:
	void setEditorText(QString text);

	KumModules *Modules;
	Kumir_Run *Run;
};

#endif
