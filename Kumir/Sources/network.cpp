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

#include "network.h"
#include "application.h"
#include "compiler.h"
#include "run.h"
#include "mainwindow.h"
#include "programtab.h"
#include "kumsinglemodule.h"

#include <QMessageBox>
#include <QTcpSocket>

KNPCommand KNPParcer::ParceCommand(QString command)
{
	KNPCommand toret;
	toret.type = EXT_CMD;

	command = command.trimmed();
	qDebug() << "command in parcer:|" << command << "| " << command.length() << " symbols";
	if (command.length() == 0) {
		toret.type = 0;
		qDebug("EMPTY command");
		return toret;
	};
	QStringList cmd_list = command.split(",");

//qDebug()<<"Command:"<<command;
// qDebug()<<"cmd_list:"<<cmd_list;
// qDebug()<<"cmd_list[0]:"<<cmd_list[0];
	if (cmd_list.count() == 0) {
		return toret;
	}
	if (command[0] == '!') {
		toret.type = 99;
		return toret;
	};
	if ((cmd_list[0] == "handshake") || ((cmd_list[0] == "handShake"))) {
		if (cmd_list.count() != 5) {
			toret.type = ERROR;
			toret.text = "Bad agruments";
			return toret;
		};
		toret.type = HAND_SHAKE;
		toret.text = "handshake";
		toret.arguments.append(KumValueStackElem(cmd_list[1]));
		toret.arguments.append(KumValueStackElem(cmd_list[2]));
		toret.arguments.append(KumValueStackElem(cmd_list[3]));
		toret.arguments.append(KumValueStackElem(cmd_list[4]));

		return toret;
	};
	if (cmd_list[0] == "handshakeOK") {
		if (cmd_list.count() != 3) {
			return toret;
		}
		toret.type = HS_OK;
		toret.text = "handshakeOK";
		toret.arguments.append(KumValueStackElem(cmd_list[1]));
		toret.arguments.append(KumValueStackElem(cmd_list[2]));

		return toret;
	};
	if ((cmd_list[0] == "list")) {

		toret.type = LIST;
		toret.text = "list";
		return toret;
	};
	if (cmd_list[0] == "help") {

		toret.type = HELP;
		toret.text = "help";
		return toret;
	};
	if (cmd_list[0] == "exit") {

		toret.type = EXIT_CMD;
		toret.text = "exit";
		return toret;
	};
	if (cmd_list[0] == "ERROR") {
		qDebug() << "Error command in KNP parcer";
		toret.type = 100; //ERROR
		toret.text = "ERROR";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(KumValueStackElem(cmd_list[i]));
		}
		return toret;
	};
	if (command == "lockGui") {

		toret.type = LOCKGUI;
		toret.text = "lockGui";
		return toret;
	};
	if (command == "reset") {

		toret.type = RESET;
		toret.text = "reset";
		return toret;
	};
	if (command == "unlockGui") {

		toret.type = UNLOCKGUI;
		toret.text = "unlockGui";
		return toret;
	};
	if (cmd_list[0] == "loopback") {

		toret.type = LOOP_BACK;
		toret.text = "loopback";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(KumValueStackElem(cmd_list[i]));
		}
		return toret;
	};
	if (cmd_list[0] == "RETURN") {

		toret.type = RETURN;
		toret.text = "RETURN";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(KumValueStackElem(cmd_list[i]));
		}
		return toret;
	};
	if (command == "OK") {

		toret.type = OK_RPL;
		toret.text = "OK";
		return toret;
	};
	if (cmd_list[0].startsWith(QString::fromUtf8("алг "))) {

		toret.type = ALG_DESC;
		toret.text = cmd_list[0];
		return toret;
	};
	toret.text = cmd_list[0];
	for (int i = 1; i < cmd_list.count(); i++) {
		toret.arguments.append(KumValueStackElem(cmd_list[i]));
	}
	return toret;
};

KNPConnection::KNPConnection(QObject *parent)
	: QObject(parent)
{

	onLine = false;
	tcpSocket = new QTcpSocket();
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(Connected()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(Disconnected()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
	HSOK = false;
	oldType = DUMMY;
};

KNPConnection::KNPConnection(QTcpSocket *friendConn)
{
	tcpSocket = friendConn;
	onLine = true;
	HSOK = true;
	oldType = DUMMY;
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(Connected()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(Disconnected()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
};

void KNPConnection::Connect(QString url, quint16 port)
{
	tcpSocket->abort();
	qDebug() << "TRY TO CONNECT TO " << url << ":" << port;
	tcpSocket->connectToHost(url, port);
	Addr = url;
	Port = port;

};
void KNPConnection::Connected() //SLOT
{
	app()->Connections.append(tcpSocket);
	qDebug("Connected!!!!");
	onLine = true;
	emit Ready();
};
void KNPConnection::Disconnected() //SLOT
{

	Compiler *comp = app()->compiler;
	int mod_id = comp->modules()->idByKNPConnection(this);
	app()->SigCross->LostIsp(mod_id);
	qDebug("KNPConnection Disconnected!!!!");
	onLine = false;
};
void KNPConnection::socketError(QAbstractSocket::SocketError socketError)
{
	qDebug("Error!!!");
	switch (socketError) {
	case QAbstractSocket::RemoteHostClosedError:
		break;
	case QAbstractSocket::HostNotFoundError: {
		qDebug() << "The host was not found. Please check the "
			"host name and port settings.";
		emit Error(tr("The host not found"));
	};
	break;
	case QAbstractSocket::ConnectionRefusedError: {
		qDebug() << "The connection was refused by the peer. "
			"Make sure the server is running, "
			"and check that the host name and port "
			"settings are correct.";
		emit Error(tr("The module doesn't answer"));
	};
	break;
	default:
		qDebug() << "The following error occurred: " <<
			tcpSocket->errorString();
	}
};

void KNPConnection::readData()
{
	qDebug() << "Bytes to read:" << tcpSocket->bytesAvailable();
	QString line;//TODO COdirovki

	while (tcpSocket->canReadLine()) {
		line = QString::fromUtf8(tcpSocket->readLine());
		qDebug() << "Read from server:" << line;
		analizeRequest(line);
	};
};

void KNPConnection::analizeRequest(QString line)
{

	KNPCommand command = Parcer.ParceCommand(line);
	KNPCommand debug;
	debug.text = tr("Message");

	debug.arguments.append(KumValueStackElem("|" + command.text + "/" + QString::number(command.type) + "|"));
	qDebug() << "Command type in KNPConnection:" << command.type;
	// app()->SigCross->DoCommand(debug,0,&tcpSocket);
	if (command.type == EXIT_CMD) {
		sendCmd("exit");// getFunctionList()
		HSOK = false;
		tcpSocket->disconnectFromHost();
		return;
	};
	if (command.type == HS_OK) {
		sendCmd("list");// getFunctionList()
		name = command.arguments.first().asString();
		HSOK = true;
		oldType = HS_OK;
		return;
	};
	if (!command.type != HS_OK && !HSOK) {

		sendCmd(QString::fromUtf8("handshake,Кумир,0.2,1.0,eee\n"));
		return;
	};

	if (command.type == ALG_DESC) {
		if (alg_desc.indexOf(line) == -1) {
			alg_desc.append(line.left(line.count() - 1));
		}
		qDebug() << "Alg_desc:" << alg_desc;
		oldType = ALG_DESC;
	};

	if (command.type == OK_RPL) {
		if (oldType == ALG_DESC) {
			emit getFuncList();
			qDebug("Get func list");
		} else {
			emit getOK();

			//qDebug()<<"getOk!";
		};
		oldType = DUMMY;
	};
	if (command.type == RETURN) {
		qDebug() << line;
		if (command.arguments.count() < 1) {
			emit GetModuleError("NO RETURN DATA");
			return;
		};
		emit getReturn(command.arguments.first().asString());
		oldType = DUMMY;
	};
	if (command.type == ERROR) {
		qDebug() << "Get Module Error";
		QString text = tr("Module reported no error message");
		if (command.arguments.count() > 0) {
			text = command.arguments.first().asString();
			qDebug() << "Before set error text";
			app()->setIspErrorText(text);
		}
		qDebug() << "Before emit";

		emit GetModuleError(text);
		qDebug() << "EMIT Get Module Error";
		if (oldType == DUMMY) {
			return;
		}
		oldType = DUMMY;
	};
	if (command.type == EXT_CMD) {

		qDebug("DO_EXT_CMD!!!");

		sendCmd(app()->SigCross->DoCommand(command, 0, tcpSocket) + "\n");
	};
	if (command.type >= 100) {
		qDebug("Interact error!");
		emit Error(tr("Error in connection with module"));
		return;
	};

};
void KNPConnection::emitGetOk()
{
	emit getOK();
};
void KNPConnection::sendCmd(QString data)
{
	if (!onLine) {
		qDebug("KNPConnection::No connection!");
		return;
	};

	qint64 send = tcpSocket->write(data.toUtf8());

	bool flush = tcpSocket->flush();
	qDebug() << "Send:" << data << " " << send << "bytes flush:" << flush << " Writable:" << tcpSocket->isWritable() ;
};


void KNPConnection::switchMode(int mode)
{

	qDebug() << "Sw mode" << mode;
	if (mode == RUN_MODE) {
		sendCmd("lockGui\n");
	} else {
		sendCmd("unlockGui\n");
		qDebug() << "Send unlock";
	}


};
void KNPConnection::showIsp()
{

	sendCmd("showWindow\n");
};
void KNPConnection::showPult()
{
	sendCmd("showPult\n");
};

//SERVER+++++++++++++++++++++++++++++

KNPServer::KNPServer(QObject *parent)
	: QObject(parent)
{

	onLine = false;
	waitForFriends_flag = false;
//tcpSocket=new QTcpSocket(this);
//connect(&tcpSocket,SIGNAL(connected()),this,SLOT(Connected()));
//connect(&tcpSocket,SIGNAL(disconnected()),this,SLOT(Disconnected()));
//connect(&Server,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
//connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
//HSOK=false;
};

void KNPServer::OpenPort(QString addr, quint16 port)
{
	Q_UNUSED(addr);
	if (!Server.listen(QHostAddress::Any, port)) {
		qDebug() << "KNPServer error:" << Server.errorString();
		QMessageBox::critical(app()->mainWindow, tr("Error in port opening"), tr("Unable to open port %1").arg(port));
		return;
	}

	QStringList currentIsps = app()->ExtIspsList();
	if (currentIsps.indexOf("Kumir," + QString::number(port)) < 0) {
		app()->AppendExtIspsToList("Kumir", port);
	};


	connect(&Server, SIGNAL(newConnection()), this, SLOT(ClientConnected()));
};
void KNPServer::ClientConnected() //SLOT
{
	qDebug("Client Connected!!!!");

	QTcpSocket *clientConnection = Server.nextPendingConnection();
	ClientList.append(KumClient(clientConnection));
	app()->Connections.append(clientConnection);
	connect(clientConnection, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(reciveMessage()));

	if (waitForFriends_flag) {
		sendMessage(clientConnection, QString::fromUtf8("handshake,Кумир,0.2,1.0,eee\n"));
		qDebug() << "Send HS";
	} else {
		sendMessage(clientConnection, "!Kumir 1.6 KNPServer\n");
	}
	onLine = true;
};
void KNPServer::ClientDisconnected() //SLOT
{
	qDebug("KNPConnection Disconnected!!!!");
	onLine = false;
};
void KNPServer::socketError(QAbstractSocket::SocketError)
{
	qDebug("KNPServer::Error!!!");
};

void KNPServer::reciveMessage()
{
	QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(sender());

	qDebug("KNPServer::Recive Message.");
	QString newMessage;
	newMessage = QString::fromUtf8(tcpSocket->readLine());

	KNPCommand command = Parcer.ParceCommand(newMessage);
	if ((command.type == HS_OK) && waitForFriends_flag) {
		KNPConnection *friendConn = new KNPConnection(tcpSocket);
		app()->compiler->modules()->setFriendSocket(command.arguments.first().asString(), friendConn);
		app()->mainWindow->createFriendMenu();
		return;
	};
	int id = app()->compiler->modules()->idByTcpSokcet(tcpSocket);
	if ((id > 0) && (app()->compiler->modules()->module(id)->isFriend())) {
		app()->compiler->modules()->module(id)->Connection()->analizeRequest(newMessage);
		return;
	};
	if (command.type >= 100) {
		qDebug("KNPServer::Parce Error!");
		emit Error(tr("Message parce error"));
		sendMessage(tcpSocket, "ERROR,Can't parce command.");
		return;
	};
	if (command.type == EXIT_CMD) {
		tcpSocket->disconnectFromHost();
		return;
	};
	if (command.type == 0) {
		set_hs_ok(tcpSocket);
		sendMessage(tcpSocket, "handshakeOK,Kumir,1.6\n");

	};
	if (command.type == HELP) {
		if (command.arguments.count() == 0) {
			sendMessage(tcpSocket, "Kumir 1.7\nhandshake,<CLIENT NAME>,<CLIENT VER>,<PROTOCOL VER>,<KEY>\nlist\n");
		} else {
			sendMessage(tcpSocket, helpCommand(command.arguments[0].asString()));
		}

	};
	if (command.type == LOOP_BACK) {
		QString replay = "";
		for (int i = 0; i < command.arguments.count(); i++) {
			replay += command.arguments[i].asString();
		};
		sendMessage(tcpSocket, replay);

	};
	if (command.type == LIST) {
		if (!hs_ok(tcpSocket)) {
			qDebug("No handshake!");
			sendMessage(tcpSocket, "ERROR,NO handShake\n");
			return;
		};
		sendList(tcpSocket);
	};
	if (command.type == LOCKGUI) {

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == OK_RPL) {
		int id = app()->compiler->modules()->idByTcpSokcet(tcpSocket);
		if (id > 0) {
			app()->compiler->modules()->module(id)->Connection()->emitGetOk();
		}
		return;

	};
	if (command.type == RESET) {

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == UNLOCKGUI) {

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == EXT_CMD) {
		if (!hs_ok(tcpSocket)) {
			qDebug("No handshake!");
			sendMessage(tcpSocket, "ERROR,NO handShake\n");
			return;
		};
		qDebug("DO_EXT_CMD!!!");//TODO Docomand
		//app()->SigCross->DoCommand(command);
		int id = ClientId(tcpSocket);
		if (id > -1) {
			ClientList[id].lastCommand = command;
		};

		sendMessage(tcpSocket, app()->SigCross->DoCommand(command, ClientId(tcpSocket), tcpSocket) + "\n");
	};

}
void KNPServer::sendMessage(QTcpSocket *tcpSocket, QString newMessage)
{
	tcpSocket->write(newMessage.toUtf8());
};

void KNPServer::sendList(QTcpSocket *tcpSocket)
{
	tcpSocket->write(QString::fromUtf8("алг Выполнить\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Большой Шаг\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Stop\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Вызвать(лит имя,лит аргумент)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Сообщение(лит текст)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Текст программы(лит текст)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Выполнить алгоритм(лит текст)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Ошибка(лит текст)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг лит Значение(лит имя)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг цел Малый Шаг\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг лит Текущий Текст Программы\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг лит Поле Разбора\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг лит Поле Вывода\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг Вставить Текст(лит текст)\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("OK\n").toUtf8());
};

void KNPServer::deleteConnection()
{
	QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(sender());
	tcpSocket->deleteLater();
	qDebug("KNPServer::Client disconected.");
};
void KNPServer::servReplay(QString text, int client_id)
{
	if (ClientList.count() <= client_id) {
		qDebug("KNPServer::No client.");
		return;
	};
	KumClient *Client = &ClientList[client_id];
	Client->tcpSocket->write(text.toUtf8());


};


QString KNPServer::helpCommand(QString command)
{
	if (command == QString::fromUtf8("Большой Шаг")) {
		return QString::fromUtf8("Выполнение исполнителем кекущей команды без остановок в вызываемых алгоритмах \nOK\n");
	};
	if (command == QString::fromUtf8("Stop")) {

		return QString::fromUtf8("Прекратить выполнение программы исполнителя\nOK");
	};
	if (command == QString::fromUtf8("Выполнить")) {
		return QString::fromUtf8("Выполнение программы исполнителя\nOK");
	};
	if (command == QString::fromUtf8("Сообщение")) {
		return QString::fromUtf8("Вывести сообшение\nOK");
	};
	return "ERROR,unknown command - kumir";
};

//Signal Crosser
void SignalCrosser::RunStart()
{
	connect(this, SIGNAL(do_Call(QString, QString)), app()->run, SLOT(callFunct(QString, QString)), Qt::BlockingQueuedConnection);
// for(int i=2;i<=Modules->lastModuleId();i++)
//  {
//      if(Modules->module(i)->isNetwork()){
//  connect(Modules->module(i)->Connection(),SIGNAL(getOK()),this,SLOT(OK()));
//      qDebug("get OK connected");};
//  };

};
void SignalCrosser::RunEnds()
{
	connect(this, SIGNAL(do_Call(QString, QString)), app()->run, SLOT(callFunct(QString, QString)), Qt::BlockingQueuedConnection);
	for (int i = 2; i <= Modules->lastModuleId(); i++) {
//  if(Modules->module(i)->isNetwork())
//      {
//      disconnect(Modules->module(i)->Connection(),SIGNAL(getOK()),(QObject*)Run,SLOT(SyncMessage()));
//      qDebug("get OK disconnected");
//      };
	};
};
QString SignalCrosser::DoCommand(KNPCommand KNPcommand, int client_id, QTcpSocket *Client)
{
	QString command = KNPcommand.text;
//qDebug()<<"Command:"<<command.toUtf8()<<"|"<<command;
	if (command == QString::fromUtf8("Большой Шаг")) {
		qDebug("B SHAG");
		emit do_b_shag();
		return "OK";
	};
	if (command == QString::fromUtf8("Stop")) {
		qDebug("Stop");
		emit do_stop();
		return "OK";
	};
	if (command == QString::fromUtf8("Выполнить")) {
		qDebug("Start");

		//emit do_start_run();
		KNPEvent startEvent(1000, "StartRun");
		QApplication::sendEvent(app()->mainWindow, &startEvent);
		return "OK";
	};
	if (command == QString::fromUtf8("Текущий Текст Программы")) {
		return "RETURN," + app()->mainWindow->tab(0)->editorText(false) + "\n";
	};
	if (command == QString::fromUtf8("Поле Разбора")) {
		return app()->mainWindow->tab(0)->marginText() + "\nOK";
	};
	if (command == QString::fromUtf8("Поле Вывода")) {
		//QString text=app()->mainWindow->textEdit3->curText();

		return "\nOK";
	};
	if (command == QString::fromUtf8("Малый Шаг")) {
		qDebug("SmallStep");

		//emit do_start_run();
		if (app()->mainWindow->curMode() != "run") {
			KNPEvent startEvent(1000, "vnutrVizova");
			QApplication::sendEvent(app()->mainWindow, &startEvent);
			return "";
		}
		return "ERROR,Kumir program running!";
	};
	if (command == QString::fromUtf8("Сообщение")) {
		qDebug("Message");
		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}
		emit do_message(KNPcommand.arguments[0].asString());
		qDebug("Message OK");
		return "OK";
	};
	if (command == QString::fromUtf8("Вызвать")) {

		int connect_id = app()->Connections.indexOf(Client);
		if (connect_id < 0) {
			return "ERROR,No argument.";
		}
		if (KNPcommand.arguments.count() < 2) {
			return "ERROR,No argument.";
		}
		emit do_Call(KNPcommand.arguments[0].asString(), KNPcommand.arguments[0].asString(), connect_id);

		return "\n";//RUN -zapishet OK esli nujno
	};
	if (command == QString::fromUtf8("Текст программы")) {
		qDebug("Set Text");
		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}
		if (!app()->run->isRunning()) {
			setEditorText(KNPcommand.arguments[0].asString());
		} else {
			return "ERROR,program running!";
		}
		emit do_set_text(KNPcommand.arguments[0].asString());
		return "OK";
	};
	if (command == QString::fromUtf8("Вставить Текст")) {
		qDebug("Insert Text");
		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}
		if (!app()->run->isRunning()) {
			insertText(KNPcommand.arguments[0].asString());
		} else {
			return "ERROR,program running!";
		}
		return "OK";
	};
	if (command == QString::fromUtf8("Выполнить алгоритм")) {
		qDebug("Start alg");
		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}

		emit do_start_alg(KNPcommand.arguments[0].asString(), client_id);
		return "OK";
	};
	if (command == QString::fromUtf8("Ошибка")) {

		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}
		emit do_message(KNPcommand.arguments[0].asString());
		return "ERROR," + KNPcommand.arguments[0].asString();
	};
	if (command == QString::fromUtf8("Значение")) {

		if (KNPcommand.arguments.count() < 1) {
			return "ERROR,No argument.";
		}
		PeremPrt perem_p;
		QString name = KNPcommand.arguments[0].asString();
		perem_p.mod_id = app()->compiler->modules()->idByName("main");
		perem_p.perem_id = app()->compiler->modules()->module(perem_p.mod_id)->Values()->inTable(name, "global"); //TODO algname
		if (perem_p.perem_id < 0) {
			return "ERROR,No such value";
		}
		if (!app()->compiler->modules()->isUsed(perem_p)) {
			return "ERROR,Value not used";
		}

		return "RETURN," + app()->compiler->modules()->getStringValue(perem_p);
	};
	return "ERROR,unknown command";
};
void SignalCrosser::OK()
{
	emit Sync();
};
void SignalCrosser::LostIsp(int module_id)
{
	emit do_stop();
	emit do_message(tr("\n Message: Module disconnected: ") + Modules->module(module_id)->name);
	Modules->removeModule(module_id);
};

void SignalCrosser::setEditorText(QString Text)
{
	QString text = Text;
	app()->mainWindow->tab(0)->setEditorText("");
	text.replace(QString("\\n"), QString("\n"));

	app()->mainWindow->tab(0)->setEditorText(text);
};

void SignalCrosser::insertText(QString Text)
{
	QString text = Text;
	text.replace(QString("\\n"), QString("\n"));
	qDebug() << "INSERT TEXT" << text;
	int line_no = app()->mainWindow->tab(0)->editor()->textCursor().blockNumber();
	Q_UNUSED(line_no);
	app()->mainWindow->tab(0)->editor()->doInsert(text);



};

