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
#ifndef KNPConnect_H
#define KNPConnect_H
#include <QtCore>
#include <QtNetwork>

#define VER 0.2

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
#define EXIT             98
#define DUMMY            99
#define ERROR            100
#define PARCE_ERROR      101

        class QTextEdit;
class turtle;
//class KumValueStackElem;
class KNPEvent : public QEvent
{
public:
    KNPEvent(int type, const QString& s) : QEvent(static_cast<Type>(type)), s(s) {};
    const QString& str() { return s; };
private: 
    QString s;
};
struct KNPCommand
{
    QString text;
    uint type;
    QList<QVariant> arguments;
};

class KNPParcer
{
public:
    KNPParcer(){};
    KNPCommand ParceCommand(QString command);

};


class SignalCrosser : public QObject
{
    Q_OBJECT
public:
    inline SignalCrosser( QObject * parent=0) {
        Q_UNUSED(parent);
    }

    void RunStart();
    void RunEnds();
    QString DoCommand(KNPCommand command, int client_id,QTcpSocket *Client);
    inline void setTurtle(turtle* trtl){turtleObj=trtl;}
signals:

    void do_move(int d);
    void do_rotate(int d);
    void do_tailUp();
    void do_tailDown();

    void Sync();
public slots:
    void OK();
    void LostIsp(int module_id);
private:
    turtle* turtleObj;

};

class KNPConnection : public QObject
{
    Q_OBJECT
public:
    KNPConnection( QObject * parent=0);
    void Connect(QString addr,quint16 port);

    QString name,Addr;
    uint Port;
    inline QStringList funcList() const {return alg_desc;}
signals:
    void Ready();
    void Error(QString text);//Ошибки подключения-взаимодействия
    void GetModuleError(QString text);//Ошибки выполнения
    void getFuncList();
    void getOK();
    void getReturn(QString value);
public slots:
    void sendCmd(QString text); //Послать сообщение
    //void runCommand(int funct_id,QString text); //Вызвать фунцию
private slots:
    void readData(); //Чтение из сокета
    void socketError(QAbstractSocket::SocketError socketError); //Обработка ошибок сокета
    void Connected();
    void Disconnected();
private:
    void analizeRequest(QString line);
    
    QTcpSocket tcpSocket;
    KNPParcer Parcer;
    bool onLine;
    bool HSOK;
    QStringList alg_desc;

    int oldType;
};

class KumClient
{
public:
	inline KumClient(QTcpSocket* Socket)
	{
		HSOK=false;
		tcpSocket=Socket;
	}

	QTcpSocket* tcpSocket;
	bool HSOK;

};
class KNPServer : public QObject
{
	Q_OBJECT
public:
	KNPServer( QObject * parent=0);
	bool OpenPort(QString addr,quint16 port);
	inline int ClientId(QTcpSocket* tcpSocket)
	{

		for(int i=0;i<ClientList.count();i++)
			if(ClientList[i].tcpSocket==tcpSocket)return i;
		return -1;
	}
	SignalCrosser* SigCross;
	inline bool hsok() const {return HSOK;}
signals:
	void Ready();
	void Error(QString text);
	void newClient(QString text);
	void lockGui();
	void unlockGui();
	void clientDisconnect();
	void reset();
public slots:
	void sendCmdAllClients(QString text); //Послать сообщение

private slots:
	//void readData(); //Чтение из сокета
	void socketError(QAbstractSocket::SocketError socketError); //Обработка ошибок сокета
	void ClientConnected();
	void ClientDisconnected();
	void reciveMessage();
	void deleteConnection() ;
	void servReplay(QString text,int client_id);
private:
    QStringList ExtIspsList();
    void AppendExtIspsToList(QString name,uint port);
    void sendMessage(QTcpSocket * tcpSocket, QString newMessage);

    void sendList(QTcpSocket * tcpSocket);
    inline bool hs_ok(QTcpSocket * tcpSocket) {
        int id=ClientId(tcpSocket);
        bool result = false;
        if (id>-1)
            result =  ClientList[id].HSOK;
        return result;
    }
    inline void set_hs_ok(QTcpSocket * tcpSocket) {
        int id=ClientId(tcpSocket);
        if(id>-1)ClientList[id].HSOK=true;
    }
    QTcpServer Server;
    KNPParcer Parcer;
    bool onLine;
    bool HSOK;
    QList<KumClient> ClientList;
    QTcpSocket *lastKumir;
};



#endif
