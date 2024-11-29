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

#include "interface.h"
#include "network.h"
#include "turtle.h"
#include "pult.h"
#include <QMessageBox>

void TurtleStarter::start()
{
	mw = new turtle();
	t_pult = new TurtlePult();
	errortext = "";

	mw->logger = t_pult->pltLogger();
	server = new KNPServer();

	server->SigCross->setTurtle(mw);

	t_pult->turtleObj = mw;
	t_pult->Connect(server);
	t_pult->libMode = true;
	mw->Tpult = t_pult;
	connect(t_pult, SIGNAL(sendText(QString)), this, SLOT(sendText2Kumir(QString)));
}

QUrl TurtleStarter::pdfUrl() const
{
	return QUrl::fromLocalFile("Help/turtle.pdf");
}

QUrl TurtleStarter::infoXmlUrl() const
{
	return QUrl();
}

QList<Alg> TurtleStarter::algList()
{

	QList<Alg> toRet;
	Alg tmp;
	tmp.kumirHeader = trUtf8("алг вперед(вещ расстояние)");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг назад(вещ расстояние)");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг вправо(вещ угол)");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг влево(вещ угол)");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг поднять хвост");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг опустить хвост");

	toRet << tmp;
	return toRet;
}

QList<QVariant> TurtleStarter::algOptResults()
{
	QList<QVariant> tmp;
	tmp << QVariant("NO OPT RESULTS");
	return tmp;
}

void TurtleStarter::runAlg(QString alg, QList<QVariant> params)
{
	errortext = "";
	if (alg == QString::fromUtf8("вперед")) {
		//qDebug()<<"Vpered";
		if (params.count() < 1) {
			errortext = "No argument.";
			sync();
			return ;
		};
		mw->step = params[0].toDouble();
		if (mw->moveT()) {
			sync();
			return ;
		} else {
			errortext = QString::fromUtf8("Край пустыни");
			sync();
			return ;
		};
	};
	if (alg == QString::fromUtf8("назад")) {

		if (params.count() < 1) {
			errortext = "No argument.\n";
			sync();
			return ;
		};
		mw->step = -params[0].toDouble();
		if (mw->moveT()) {
			sync();
			return ;
		} else {
			errortext = QString::fromUtf8("Край пустыни");
			sync();
			return ;
		};

	};
	if (alg == QString::fromUtf8("вправо")) {
		if (params.count() < 1) {
			errortext = "No argument.";
			sync();
			return;
		};
		mw->DoRotate(params[0].toDouble());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("влево")) {

		if (params.count() < 1) {
			errortext = "No argument.";
			sync();
			return ;
		};
		mw->DoRotate(-params[0].toDouble());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("поднять хвост")) {
		mw->TailUp();
		sync();
		return;
	};
	if (alg == QString::fromUtf8("опустить хвост")) {
		mw->TailDown();
		sync();
		return;
	};
	if (alg == QString::fromUtf8("|open port")) {
		int port = params[0].toInt();
		t_pult->libMode = false;
		if (port < 1024) {
			return;
		}
		openServerPort(port);
	};

}

void TurtleStarter::showField()
{
	mw->showTurtle();
	t_pult->showNormal();
	t_pult->raise();
}

void TurtleStarter::hideField()
{
	mw->hideTurtle();
}

void TurtleStarter::showPult()
{
	t_pult->showNormal();
	t_pult->raise();
}

void TurtleStarter::hidePult()
{
	t_pult->hide();
}

void TurtleStarter::reset()
{
	mw->reset();
	errortext = "";
}

QWidget *TurtleStarter::mainWidget()
{
	return mw;
}

QWidget *TurtleStarter::pultWidget()
{
	return t_pult;
}

void TurtleStarter::openServerPort(int port)
{
	t_pult->libMode = false;
	if (!server->OpenPort("localhost", port)) {
		QMessageBox::critical(NULL, QString::fromUtf8("Ошибка открытия порта"),
			QString::fromUtf8("Невозможно открыть порт %1")
			.arg(port));
	} else {
		t_pult->showMessage(QString::fromUtf8("Открыт порт %1").arg(port));
	};

}

QVariant TurtleStarter::result()
{
	return QVariant(" ");
}

void TurtleStarter::setMode(int che_mode)
{
	mode = che_mode;
	mw->Repaint();
	if (mode == 1) {
		t_pult->noLink();
	} else {
		t_pult->LinkOK();
	}

}

void TurtleStarter::connectSignalSync(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sync()), obj, method);
}

void TurtleStarter::connectSignalSendText(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sendText(QString)), obj, method);
}

void TurtleStarter::sendText2Kumir(QString text)
{
	emit sendText(text);
}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(turtleStarter, TurtleStarter)
#endif

