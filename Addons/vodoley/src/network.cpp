//****************************************************************************
//**
//** Copyright (C) 2004-2008 IMPB RAS. All rights reserved.
//**
//** This file is part of the KuMir.
//**
//** This file may be used under the terms of the GNU General Public
//** License version 2.0 as published by the Free Software Foundation
//** and appearing in the file LICENSE.GPL included in the packaging of
//** this file.
//**
//** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
//** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//**
//****************************************************************************/
#include "network.h"
#include "vodoley.h"
#include "pult.h"
#include <QSettings>


KNPCommand KNPParcer::ParceCommand(QString command)
{
	KNPCommand toret;
	toret.type = EXT_CMD;

	command = command.trimmed();
	if (command.length() == 0) {
		toret.type = EMPTY;
		return toret;
	};
	QStringList cmd_list = command.split(",");

	qDebug() << "Command:" << command;
	//qDebug()<<"cmd_list:"<<cmd_list;
	//qDebug()<<"cmd_list[0]:"<<cmd_list[0];
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
		toret.arguments.append(QVariant(cmd_list[1]));
		toret.arguments.append(QVariant(cmd_list[2]));
		toret.arguments.append(QVariant(cmd_list[3]));
		toret.arguments.append(QVariant(cmd_list[4]));

		return toret;
	};
	if (cmd_list[0] == "handshakeOK") {
		if (cmd_list.count() != 3) {
			return toret;
		}
		toret.type = HS_OK;
		toret.text = "handshakeOK";
		toret.arguments.append(QVariant(cmd_list[1]));
		toret.arguments.append(QVariant(cmd_list[2]));

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

		toret.type = EXIT;
		toret.text = "exit";
		return toret;
	};
	if (cmd_list[0] == "ERROR") {

		toret.type = ERROR;
		toret.text = "ERROR";
		qDebug() << "Parcer:Error command";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(QVariant(cmd_list[i]));
		}
		return toret;
	};
	if (cmd_list[0] == "loopback") {

		toret.type = LOOP_BACK;
		toret.text = "loopback";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(QVariant(cmd_list[i]));
		}
		return toret;
	};
	if (cmd_list[0] == "RETURN") {

		toret.type = RETURN;
		toret.text = "RETURN";
		for (int i = 1; i < cmd_list.count(); i++) {
			toret.arguments.append(QVariant(cmd_list[i]));
		}
		return toret;
	};
	if (command == "OK") {

		toret.type = OK_RPL;
		toret.text = "OK";
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
	if (command == "showWindow") {

		toret.type = SHOWWND;
		toret.text = "showWindow";
		return toret;
	};
	if (command == "showPult") {

		toret.type = SHOWPLT;
		toret.text = "showPult";
		return toret;
	};
	if (cmd_list[0].startsWith(QString::fromUtf8("алг "))) {

		toret.type = ALG_DESC;
		toret.text = cmd_list[0];
		return toret;
	};
	toret.text = cmd_list[0];
	for (int i = 1; i < cmd_list.count(); i++) {
		toret.arguments.append(QVariant(cmd_list[i]));
	}
	return toret;
};

KNPConnection::KNPConnection(QObject *parent)
{
	Q_UNUSED(parent);
	onLine = false;
	//tcpSocket=new QTcpSocket(this);
	connect(&tcpSocket, SIGNAL(connected()), this, SLOT(Connected()));
	connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(Disconnected()));
	connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()), Qt::BlockingQueuedConnection);
	HSOK = false;
	oldType = DUMMY;
};

void KNPConnection::Connect(QString url, quint16 port)
{
	tcpSocket.abort();
	tcpSocket.connectToHost(url, port);
	Addr = url;
	Port = port;
};
void KNPConnection::Connected() //SLOT
{
	//app()->Connections.append(&tcpSocket);
	qWarning("Connected!!!!");
	onLine = true;
};
void KNPConnection::Disconnected() //SLOT
{
	qWarning("KNPConnection Disconnected!!!!");
	onLine = false;
};
void KNPConnection::socketError(QAbstractSocket::SocketError socketError)
{
	qWarning("Error!!!");
	switch (socketError) {
	case QAbstractSocket::RemoteHostClosedError:
		break;
	case QAbstractSocket::HostNotFoundError: {
		qDebug() << "The host was not found. Please check the "
			"host name and port settings.";
		emit Error(QString::fromUtf8("Указанный адрес не наеден."));
	};
	break;
	case QAbstractSocket::ConnectionRefusedError: {
		qDebug() << "The connection was refused by the peer. "
			"Make sure the fortune server is running, "
			"and check that the host name and port "
			"settings are correct.";
		emit Error(QString::fromUtf8("Исполнитель не отвечает."));
	};
	break;
	default:
		qDebug() << "The following error occurred: " <<
			tcpSocket.errorString();
	}
};

void KNPConnection::readData()
{
	//qDebug()<<"Bytes to read:"<<tcpSocket.bytesAvailable();
	QString line;//TODO COdirovki

	while (tcpSocket.canReadLine()) {
		line = QString::fromUtf8(tcpSocket.readLine());
		//qDebug()<<"Read from server:"<<line;
		analizeRequest(line);
	};
};

void KNPConnection::analizeRequest(QString line)
{
	KNPCommand command = Parcer.ParceCommand(line);

	if (command.type == EXIT) {
		sendCmd("exit");// getFunctionList()
		HSOK = false;
		tcpSocket.disconnectFromHost();
		return;
	}

	if (command.type == HS_OK) {
		sendCmd("list");// getFunctionList()
		name = command.arguments.first().toString();
		HSOK = true;
		oldType = HS_OK;
		return;
	}

	if (!(command.type != HS_OK) && !HSOK) {
		sendCmd(QString::fromUtf8("handshake,Водолей,1.0,1.0,eee\n"));
		return;
	}

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
			qWarning("Get func list");
		} else {
			emit getOK();
		}
		oldType = DUMMY;
	};
	if (command.type == RETURN) {
		qDebug() << line;
		if (command.arguments.count() < 1) {
			emit GetModuleError("NO RETURN DATA");
			return;
		};
		emit getReturn(command.arguments.first().toString());
		oldType = DUMMY;
	};
	if (command.type == ERROR) {
		QString text = QString::fromUtf8("Исполнитель не сообщил текста ошибки");
		if (command.arguments.count() > 0) {
			text = command.arguments.first().toString();
		}
		emit GetModuleError(text);
		if (oldType == DUMMY) {
			return;
		}
		oldType = DUMMY;
	};
	if (command.type >= 100) {
		qWarning("Interact error!");
		emit Error(QString::fromUtf8("Ошибка взаимодействия с исполнителем."));
		return;
	};
};
void KNPConnection::sendCmd(QString data)
{
	if (!onLine) {
		qWarning("KNPConnection::No connection!");
		return;
	};

	qint64 send = tcpSocket.write(data.toUtf8());

	bool flush = tcpSocket.flush();
	Q_UNUSED(send);
	Q_UNUSED(flush);
	//qDebug()<<"Send:"<<data<<" "<<send<<"bytes flush:"<<flush<<" Writable:"<<tcpSocket.isWritable() ;
};






//SERVER+++++++++++++++++++++++++++++

KNPServer::KNPServer(QObject *parent)
{
	Q_UNUSED(parent);
	onLine = false;
	SigCross = new SignalCrosser();
	//tcpSocket=new QTcpSocket(this);
	//connect(&tcpSocket,SIGNAL(connected()),this,SLOT(Connected()));
	//connect(&tcpSocket,SIGNAL(disconnected()),this,SLOT(Disconnected()));
	//connect(&Server,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
	//connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
	//HSOK=false;
};

bool KNPServer::OpenPort(QString addr, quint16 port)
{
	Q_UNUSED(addr);
	if (!Server.listen(QHostAddress::Any, port)) {
		qDebug() << "KNPServer error:" << Server.errorString();
		return false;
	}

	QStringList currentIsps = ExtIspsList();
	if (currentIsps.indexOf(QString::fromUtf8("Водолей,") + QString::number(port)) < 0) {
		AppendExtIspsToList(QString::fromUtf8("Водолей"), port);
	};


	connect(&Server, SIGNAL(newConnection()), this, SLOT(ClientConnected()));
	return true;
};
QStringList KNPServer::ExtIspsList()
{
	QSettings kumSett("NIISI RAS", "Kumir");
	QString data = kumSett.value("Isps", "").toString();
	return data.split(';');
};
void KNPServer::AppendExtIspsToList(QString name, uint port)
{
	QSettings kumSett("NIISI RAS", "Kumir");
	QString data = kumSett.value("Isps", "").toString();
	data = data + ";" + name + "," + QString::number(port);
	kumSett.setValue("Isps", data);
};
void KNPServer::initConnection(uint port)
{
	QTcpSocket *clientConnection = new QTcpSocket();
	clientConnection->connectToHost("localhost", port);
	connect(clientConnection, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(reciveMessage()), Qt::DirectConnection);
};
void KNPServer::ClientConnected() //SLOT
{
	qWarning("Client Connected!!!!");

	QTcpSocket *clientConnection = Server.nextPendingConnection();
	ClientList.append(KumClient(clientConnection));
	//ClientList.last().tcpSocket->open(QIODevice::ReadWrite);
	connect(clientConnection, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(reciveMessage()), Qt::DirectConnection);
	sendMessage(clientConnection, "!Vodoley 1.6 KNPServer\n");
	onLine = true;
};
void KNPServer::ClientDisconnected() //SLOT
{
	qWarning("KNPConnection Disconnected!!!!");
	onLine = false;
};
void KNPServer::socketError(QAbstractSocket::SocketError)
{
	qWarning("KNPServer::Error!!!");
};

void KNPServer::reciveMessage()
{
	QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(sender());
	lastKumir = tcpSocket;
	//qWarning("KNPServer::Recive Message.");
	QString newMessage;
	newMessage = QString::fromUtf8(tcpSocket->readLine());
	qDebug() << "Message:" << newMessage;
	KNPCommand command = Parcer.ParceCommand(newMessage);
	if (command.type >= 100) {
		qWarning("KNPServer::Parce Error!");
		emit Error(QString::fromUtf8("Ошибка разбора сообщения."));
		sendMessage(tcpSocket, "ERROR,Cant parce command.");
		return;
	};
	if (command.type == EXIT) {
		tcpSocket->disconnectFromHost();
		return;
	};
	if (command.type == 0) {
		set_hs_ok(tcpSocket);
		if (command.arguments[2].toDouble() < VER) {
			sendMessage(tcpSocket, QString::fromUtf8("badver"));
		}
		sendMessage(tcpSocket, QString::fromUtf8("handshakeOK,Водолей,") + QString::number(VER) + "\n");
		emit newClient(command.arguments[0].toString());
	};
	if (command.type == HELP) {

		sendMessage(tcpSocket, QString::fromUtf8("Водолей 0.1\nhandshake,<CLIENT NAME>,<CLIENT VER>,<PROTOCOL VER>,<KEY>\nlist\n"));

	};
	if (command.type == LOOP_BACK) {
		QString replay = "";
		for (int i = 0; i < command.arguments.count(); i++) {
			replay += command.arguments[i].toString();
		};
		sendMessage(tcpSocket, replay);

	};
	if (command.type == LOCKGUI) {
		emit lockGui();

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == RESET) {
		emit reset();

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == EXIT) {
		QCoreApplication::exit(0);
	};
	if (command.type == UNLOCKGUI) {
		emit unlockGui();
		qDebug() << "Unlock";

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == SHOWWND) {
		SigCross->showWindow();
		qDebug() << "Unlock";

		sendMessage(tcpSocket, "OK\n");
	};

	if (command.type == SHOWPLT) {
		SigCross->showPult();
		qDebug() << "Unlock";

		sendMessage(tcpSocket, "OK\n");
	};
	if (command.type == LIST) {
		if (!hs_ok(tcpSocket)) {
			qWarning("No handshake!");
			sendMessage(tcpSocket, "ERROR,NO handShake\n");
			return;
		};
		sendList(tcpSocket);
	};
	if (command.type == EXT_CMD) {
		if (!hs_ok(tcpSocket)) {
			qWarning("No handshake!");
			sendMessage(tcpSocket, "ERROR,NO handShake\n");
			return;
		};
		//qWarning("DO_EXT_CMD!!!");
		//app()->SigCross->DoCommand(command);
		sendMessage(tcpSocket, SigCross->DoCommand(command, ClientId(tcpSocket), tcpSocket) + "\n");
	};

}
void KNPServer::sendMessage(QTcpSocket *tcpSocket, QString newMessage)
{
	qDebug() << "Wr 2 kumir |" << newMessage << "|";
	tcpSocket->write(newMessage.toUtf8());

};
void KNPServer::sendCmdAllClients(QString newMessage)
{
	qDebug() << "Open for write:" << lastKumir->isWritable();
	sendMessage(lastKumir, newMessage);
};
void KNPServer::sendList(QTcpSocket *tcpSocket)
{

	tcpSocket->write(QString::fromUtf8("алг перелей из A в B\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг перелей из A в C\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг перелей из B в A\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг перелей из B в C\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг перелей из C в A\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг перелей из C в B\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг наполни A\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг наполни B\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг наполни C\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг вылей A\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг вылей B\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("алг вылей C\n").toUtf8());
	tcpSocket->write(QString::fromUtf8("OK\n").toUtf8());
};

void KNPServer::deleteConnection()
{
	QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(sender());
	tcpSocket->deleteLater();
	qWarning("KNPServer::Client disconected.");
	emit clientDisconnect();
};
void KNPServer::servReplay(QString text, int client_id)
{
	if (ClientList.count() <= client_id) {
		qWarning("KNPServer::No client.");
		return;
	};
	KumClient *Client = &ClientList[client_id];
	Client->tcpSocket->write(text.toUtf8());


};


//Signal Crosser
void SignalCrosser::RunStart()
{


};
void SignalCrosser::RunEnds()
{

};
QString SignalCrosser::DoCommand(KNPCommand KNPcommand, int client_id, QTcpSocket *Client)
{
	Q_UNUSED(client_id);
	Q_UNUSED(Client);
	QString command = KNPcommand.text;
	qDebug() << "Command in Sig Cross:" << "|" << command;
	if (!VodoleyObj) {
		return "ERROR,internal module error: VodoleyObj is NULL! ";
	}
	if (command == QString::fromUtf8("перелей из A в B")) {
		VodoleyObj->MoveFromTo(0, 1);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("перелей из A в C")) {
		VodoleyObj->MoveFromTo(0, 2);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("перелей из B в A")) {
		VodoleyObj->MoveFromTo(1, 0);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("перелей из B в C")) {
		VodoleyObj->MoveFromTo(1, 2);
		return QString::fromUtf8("OK\n");
	};

	if (command == QString::fromUtf8("перелей из C в A")) {
		VodoleyObj->MoveFromTo(2, 0);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("перелей из C в B")) {
		VodoleyObj->MoveFromTo(2, 1);
		return QString::fromUtf8("OK\n");
	};

	if (command == QString::fromUtf8("вылей A")) {
		VodoleyObj->MoveFromTo(0, 3);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("вылей B")) {
		VodoleyObj->MoveFromTo(1, 3);
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("вылей C")) {
		VodoleyObj->MoveFromTo(2, 3);
		return QString::fromUtf8("OK\n");
	};



	if (command == QString::fromUtf8("наполни A")) {
		VodoleyObj->FillA();
		return QString::fromUtf8("OK\n");
	};

	if (command == QString::fromUtf8("наполни B")) {
		VodoleyObj->FillB();
		return QString::fromUtf8("OK\n");
	};
	if (command == QString::fromUtf8("наполни C")) {
		VodoleyObj->FillC();
		return QString::fromUtf8("OK\n");
	};

	return "ERROR,unknown command -" + KNPcommand.text + "\n";
};
void SignalCrosser::OK()
{
	emit Sync();
};

void SignalCrosser::LostIsp(int module_id)
{
	Q_UNUSED(module_id);
	qDebug() << "Lost isp";
};
void SignalCrosser::showPult()
{

	VodoleyObj->pult->show();
	VodoleyObj->pult->raise();
	VodoleyObj->pult->showNormal();
};
void SignalCrosser::showWindow()
{
	VodoleyObj->show();
	VodoleyObj->raise();
	VodoleyObj->showNormal();
};
