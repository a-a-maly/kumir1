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
#include <QMessageBox>
#include <QUrl>

void VodoleyStarter::start()
{
	mw = new Vodoley();
	t_pult = new VodoleyPult();
	errortext = "";

	mw->resize(500, 500);
	//mw->scene->addLine(-600,0,100,0);
	//mw->scene->addLine(0,-200,0,100);
	mw->logger = t_pult->pltLogger();
	server = new KNPServer();


	server->SigCross->setVodoley(mw);
	//  mw->show();
//	scene->addLine(0,0,200,0);

	t_pult->VodoleyObj = mw;
	mw->pult = t_pult;
	t_pult->Connect(server);
	t_pult->libMode = true;
	t_pult->toKumir->setEnabled(true);
	connect(t_pult, SIGNAL(sendText(QString)), this, SLOT(sendText2Kumir(QString)));
	// t_pult->show();
}

QUrl VodoleyStarter::pdfUrl() const
{
	return QUrl::fromLocalFile("Help/vodoley.pdf");
}

QUrl VodoleyStarter::infoXmlUrl() const
{
	return QUrl();
}

QList<Alg> VodoleyStarter::algList()
{

	QList<Alg> toRet;
	Alg tmp;

	tmp.kumirHeader = trUtf8("алг перелей из A в B");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перелей из A в C");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перелей из B в A");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перелей из B в C");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перелей из C в A");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перелей из C в B");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг наполни A");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг наполни B");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг наполни C");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг вылей A");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг вылей B");

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг вылей C");

	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг цел в сосуде A");
	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг цел в сосуде B");
	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг цел в сосуде C");
	toRet << tmp;


	tmp.kumirHeader = trUtf8("алг цел размер A");
	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг цел размер B");
	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг цел размер C");
	toRet << tmp;

	tmp.kumirHeader = trUtf8("алг лог @решено");

	toRet << tmp;

	return toRet;
}

QList<QVariant> VodoleyStarter::algOptResults()
{
	QList<QVariant> tmp;
	tmp << QVariant("NO OPT RESULTS");
	return tmp;
}

void VodoleyStarter::runAlg(QString alg, QList<QVariant> params)
{
	errortext = "";

	if (alg == QString::fromUtf8("перелей из A в B")) {
		mw->MoveFromTo(0, 1);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("перелей из A в C")) {
		mw->MoveFromTo(0, 2);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("перелей из B в A")) {
		mw->MoveFromTo(1, 0);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("перелей из B в C")) {
		mw->MoveFromTo(1, 2);
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("перелей из C в A")) {
		mw->MoveFromTo(2, 0);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("перелей из C в B")) {
		mw->MoveFromTo(2, 1);
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("вылей A")) {
		mw->MoveFromTo(0, 3);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("вылей B")) {
		mw->MoveFromTo(1, 3);
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("вылей C")) {
		mw->MoveFromTo(2, 3);
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("наполни A")) {
		mw->FillA();
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("наполни B")) {
		mw->FillB();
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("наполни C")) {
		mw->FillC();
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("@решено")) {
		result_p = QVariant(mw->isReady());
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("в сосуде A")) {
		result_p = QVariant(mw->CurA());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("в сосуде B")) {
		result_p = QVariant(mw->CurB());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("в сосуде C")) {
		result_p = QVariant(mw->CurC());
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("размер A")) {
		result_p = QVariant(mw->Asize());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("размер B")) {
		result_p = QVariant(mw->Bsize());
		sync();
		return ;
	};
	if (alg == QString::fromUtf8("размер C")) {
		result_p = QVariant(mw->Csize());
		sync();
		return ;
	};

	if (alg == QString::fromUtf8("|open port")) {
		t_pult->libMode = false;
		t_pult->toKumir->setEnabled(false);
		int port = params[0].toInt();
		if (port < 1024) {
			return;
		}
		openServerPort(port);
		return;
	};

	errortext = "ISP Vodoley unknown command";
	sync();
}

void VodoleyStarter::openServerPort(int port)
{
	t_pult->libMode = false;
	if (!server->OpenPort("localhost", port)) {
		QMessageBox::critical(NULL, QString::fromUtf8("Ошибка открытия порта"),
			QString::fromUtf8("Невозможно открыть порт %1")
			.arg(port));
	} else {
		t_pult->showMessage(QString::fromUtf8("Открыт порт %1").arg(port));
	}

}

QVariant VodoleyStarter::result()
{
	return result_p;
}

void VodoleyStarter::setMode(int che_mode)
{
	mode = che_mode;
	if (mode == 1) {
		t_pult->noLink();
	} else {
		t_pult->LinkOK();
	}

}

void VodoleyStarter::connectSignalSync(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sync()), obj, method);
}

void VodoleyStarter::connectSignalSendText(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sendText(QString)), obj, method);
}

void VodoleyStarter::sendText2Kumir(QString text)
{
	emit sendText(text);
}

void VodoleyStarter::setParameter(const QString &paramName, const QVariant &paramValue)
{
	if (paramName == "environment") {
		mw->loadFile(paramValue.toString());
		qDebug() << "Load env" << paramValue.toString();
	}
	return;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(VodoleyStarter, VodoleyStarter)
#endif
