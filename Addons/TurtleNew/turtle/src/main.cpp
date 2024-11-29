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


#include "turtle.h"
#include "pult.h"
#include "network.h"

#include <QMessageBox>
#include <QApplication>
#include <QGraphicsSvgItem>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);

	turtle *mw = new turtle();
	TurtlePult *t_pult = new TurtlePult();


	mw->resize(500, 500);
	//mw->scene->addLine(-600,0,100,0);
	//mw->scene->addLine(0,-200,0,100);
	mw->logger = t_pult->pltLogger();
	KNPServer* server = new KNPServer();
	int port = 4354;
	if (!server->OpenPort("localhost", port)) {
		QMessageBox::critical(mw, QString::fromUtf8("Ошибка открытия порта"),
			QString::fromUtf8("Невозможно открыть порт %1")
			.arg(port));
	} else {
		t_pult->showMessage(QString::fromUtf8("Открыт порт %1").arg(port));
	};
	server->SigCross->setTurtle(mw);
	mw->show();
//	scene->addLine(0,0,200,0);

	t_pult->turtleObj = mw;
	t_pult->Connect(server);
	t_pult->show();
	mw->Tpult = t_pult;
	//  t_pult->Connect();
	return app.exec();
}

