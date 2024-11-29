//****************************************************************************
//**
//** Copyright (C) 2004-2008 IMPB RAS. All rights reserved.
//**
//** This file is part of the KuMir.
//**
//** This file may be used under the terms of the GNU General Public
//** License version 2.0 as published by the Free Software Foundation
// //** and appearing in the file LICENSE.GPL included in the packaging of
//** this file.
//**
//** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
//** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//**
//****************************************************************************/


#include "pult.h"
#include "kuznec.h"
#include "network.h"

#include <QApplication>
#include <QGraphicsSvgItem>
#include <QMessageBox>

#include <iostream>

#ifndef WIN32
#include "unistd.h"
#endif

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);


	KumKuznec * mw = new KumKuznec();
	GrasshopperPult *t_pult = new GrasshopperPult();
	bool friendMode = false;
	if (argc > 0) {
		if (QString(argv[1]).startsWith("-h")) {
			QString message = "-f <kumir port>. Start in friend mode.\n";
			std::cout << message.toUtf8().data();
			return 0;
		};
		QString initstr = QString(argv[2]);
		qDebug() << "Init:" << initstr;
		qDebug() << "Init[]:" << QString(argv[1]);
		friendMode = true;
	}

	mw->resize(450, 280);


	mw->logger = t_pult->pltLogger();
	mw->Kpult = t_pult;
	KNPServer * server = new KNPServer();
	int port = 4356;


	if (!server->OpenPort("localhost", port)) {
		QMessageBox::critical(mw, QString::fromUtf8("Ошибка открытия порта"),
			QString::fromUtf8("Невозможно открыть порт %1")
			.arg(port));
	} else {
		t_pult->showMessage(QString::fromUtf8("Открыт порт %1").arg(port));
	}

	server->SigCross->setKuznec(mw);
	qDebug() << "Friend MODE:" << friendMode;
	mw->show();

	t_pult->kuznecObj = mw;
	t_pult->Connect(server);
	t_pult->show();

	return app.exec();
}

