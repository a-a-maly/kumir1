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


/* ExternalModulesDialog
Подключение сетевых исполнителей
*/
#ifndef KUM_EXTISP_H
#define KUM_EXTISP_H
#include "ui_externalIsp.h"
#include "kum_instrument.h"
#include "text_analiz.h"

#include <QDialog>



class externalIspDialog : public QDialog, Ui::externalIspDialog
{
	Q_OBJECT

public:
	externalIspDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~externalIspDialog() {};

	bool network()
	{
		return NetworkSW->isChecked();
	}

	QString file()
	{
		return FileLine->text();
	}

	bool bachWaitFlag;

signals:
	void ispStateChanged();

public slots:
	void connectIsp(QString Url, int port);

	void connectIsp()
	{
		connectIsp(UrlLine->text(), PortLine->text().toInt());
	}

	void appendIspToList()
	{
		int port = PortLine->text().toInt();
		if ((port < 1025) || (port > 65535)) {
			return;
		}
		QStringList currentIsps = app()->ExtIspsList();
		if (currentIsps.indexOf(UrlLine->text() + "," + QString::number(port)) < 0) {
			app()->AppendExtIspsToList(UrlLine->text(), port);
		}

	}

	void ispReady();
	void ispError(QString error);
	void tableUpdateReg();
	void listClicked(int row, int col);
	void openFile();
	void switchM();

private:
	QString checkIsp(QString name, int port);
	QWidget  *parentDialog;

};
#endif
