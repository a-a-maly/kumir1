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
#include "kuznec.h"
#include "pult.h"
#include <QMessageBox>
#include <QUrl>

void KuznecStarter::showField()
{
	mw->showHideWindow();
	mw->raise();
	showPult();
}

void KuznecStarter::hideField()
{
	mw->hide();
}

void KuznecStarter::showPult()
{
	t_pult->showNormal();
	t_pult->raise();
}

void KuznecStarter::hidePult()
{
	t_pult->hide();
}

void KuznecStarter::reset()
{
	mw->Reset();
	errortext = "";
}

QWidget * KuznecStarter::mainWidget()
{
	return (QWidget*)(mw->MV);
}

QWidget * KuznecStarter::pultWidget()
{
	return t_pult;
}

void KuznecStarter::start()
{
	mw = new KumKuznec();
	qDebug() << "KUZN START !!!!!!!!!!!1 ==================== ";

	t_pult = new GrasshopperPult();
	errortext = "";
	mw->hide();
	mw->resize(450, 280);
	mw->MV->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mw->Kpult = t_pult;
	mw->logger = t_pult->pltLogger();
	server = new KNPServer();
	server->SigCross->setKuznec(mw);

	t_pult->kuznecObj = mw;
	t_pult->Connect(server);
	t_pult->libMode = true;
	t_pult->toKumir->setEnabled(true);
	connect(t_pult, SIGNAL(sendText(QString)), this, SLOT(sendText2Kumir(QString)));
}

QUrl KuznecStarter::pdfUrl() const
{
	return QUrl::fromLocalFile("Help/kuznechik.pdf");
}

QUrl KuznecStarter::infoXmlUrl() const
{
	return QUrl();
}

void KuznecStarter::setParameter(const QString &paramName, const QVariant &paramValue)
{
	if (paramName == "Shagi") {
		QString shag_s = paramValue.toString();
		shag_s = shag_s.simplified();
		QStringList shagL = shag_s.split(" ");
		if (shagL.count() < 2) {
			qDebug() << "KuznecStarter::Bad Shag parametr";
			return;
		}

		mw->setSteps(shagL[0].toInt(), shagL[1].toInt());
		t_pult->setStepSizes(shagL[0].toInt(), shagL[1].toInt());

	}
}

QList<Alg> KuznecStarter::algList()
{

	QList<Alg> toRet;
	Alg tmp;
	tmp.kumirHeader = trUtf8("алг вперед ") + QString::number(mw->stepForward());

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг назад ") + QString::number(mw->stepBackward());

	toRet << tmp;
	tmp.kumirHeader = trUtf8("алг перекрасить");

	toRet << tmp;

	return toRet;
}

QList<QVariant> KuznecStarter::algOptResults()
{
	QList<QVariant> tmp;
	tmp << QVariant("NO OPT RESULTS");
	return tmp;
}

void KuznecStarter::runAlg(QString alg, QList<QVariant> params)
{
	errortext = "";
	if (alg.startsWith(QString::fromUtf8("вперед"))) {

		if (mw->canFwd()) {
			mw->MoveFwd();
		} else {
			errortext = QString::fromUtf8("Выход за границы");
		}
		sync();
		return ;
	}

	if (alg.startsWith(QString::fromUtf8("назад"))) {

		if (mw->canBack()) {
			mw->MoveBack();
		} else {
			errortext = QString::fromUtf8("Выход за границы");
		}
		sync();
		return ;
	}

	if (alg == QString::fromUtf8("перекрасить")) {

		mw->ColorUncolor();
		sync();
		return;
	};

	if (alg == QString::fromUtf8("|open port")) {
		int port = params[0].toInt();
		t_pult->SetAloneMode();
		if (port < 1024) {
			return;
		}
		openServerPort(port);
	};

}

void KuznecStarter::openServerPort(int port)
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

QVariant KuznecStarter::result()
{
	return QVariant(" ");
}

void KuznecStarter::setMode(int che_mode)
{
	mode = che_mode;
	if (mode == 1) {
		t_pult->noLink();
	} else {
		t_pult->LinkOK();
	}

}

void KuznecStarter::connectSignalSync(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sync()), obj, method);
}

void KuznecStarter::connectSignalSendText(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sendText(QString)), obj, method);
}

void KuznecStarter::sendText2Kumir(QString text)
{
	emit sendText(text);
}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(kuznecStarter, KuznecStarter)
#endif
