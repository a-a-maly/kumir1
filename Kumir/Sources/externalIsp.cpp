//****************************************************************************
//**
//** Copyright (C) 2004-2010 NIISI RAS. All rights reserved.
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
#include "externalIsp.h"
#include "kumfiledialog.h"
#include "kumsinglemodule.h"
#include "kummodules.h"
#include <QMessageBox>
#include <QTcpSocket>

externalIspDialog::externalIspDialog(QWidget *parent, Qt::WindowFlags fl) :
	QDialog(parent, fl), Ui::externalIspDialog()
{
	setupUi(this);
	parentDialog = parent;
	connect(toolButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(NetworkSW, SIGNAL(clicked()), this, SLOT(switchM()));
	connect(FileSW, SIGNAL(clicked()), this, SLOT(switchM()));
//tableUpdateReg();
	bachWaitFlag = false;
}

void externalIspDialog::connectIsp(QString url, int Port)
{
	KNPConnection *extModule = new KNPConnection(this);
	connect(extModule, SIGNAL(getFuncList()), this, SLOT(ispReady()));
	connect(extModule, SIGNAL(getFuncList()), parentDialog, SLOT(repaintTable()));

	connect(extModule, SIGNAL(Error(QString)), this, SLOT(ispError(QString)));

	if ((Port < 1) || (Port > 65534)) {
		QMessageBox::critical(this, tr("Error"), tr("The port doesn't exist"));
	}

	qDebug() << "url" << url;
	extModule->Connect(url, Port);
//emit ispStateChanged();
};
void externalIspDialog::ispReady()
{
//emit ispStateChanged();
	KNPConnection *extModule = qobject_cast<KNPConnection *>(sender());
	qDebug() << "Isp " << extModule->name << " Ready!";
	int  module_id = app()->compiler->modules()->idByName(extModule->name);
	if (module_id > 0) {
		if (!app()->isBatchMode()) {
			QMessageBox::critical(this, tr("Registration error"), tr("Module %1 has already registered").arg(extModule->name));
		}
		//extModule->close();
		return;
	};
	module_id = app()->compiler->modules()->appendNetworkModule(extModule->name, extModule);
	int st = app()->compiler->getAnalizer()->parceFuncList(module_id, extModule->funcList());
	if (st < 0) {
		if (!app()->isBatchMode()) {
			QMessageBox::critical(this, tr("Registration error"), tr("Functions list analysis error"));
		}
	};

	QStringList CurrentIsps = app()->ExtIspsList();
	QString url = app()->compiler->modules()->module(module_id)->addr;
	if ((url != "localhost") && (CurrentIsps.indexOf(QRegExp(".," + QString::number(extModule->Port) + "," + url)) == -1)) {
		app()->AppendExtIspsToList(extModule->name, extModule->Port, url);
	};
	bachWaitFlag = true;
	emit ispStateChanged();

	if (!app()->isBatchMode()) {
		QMessageBox::information(this, tr("Modules connection"), tr("Module %1 is connected").arg(extModule->name));
	}

};

void externalIspDialog::ispError(QString error)
{
	if (!app()->isBatchMode()) {
		QMessageBox::critical(this, tr("Connection error"), tr("Can't connect to module %1").arg(error));
	}
	emit ispStateChanged();
};

void externalIspDialog::tableUpdateReg()
{
// QStringList CurrentIsps=app()->ExtIspsList();
// tableWidget->clearContents();
// while(tableWidget->rowCount ()>0)tableWidget->removeRow( 0);
//
// //tableWidget->setColumnCount(3);
// //tableWidget->insertRow (tableWidget->rowCount ());
// //   tableWidget->setItem(tableWidget->rowCount()-1,0,new QTableWidgetItem("Name"));
// //   tableWidget->setItem(tableWidget->rowCount()-1,1,new QTableWidgetItem("Port"));
// for(int i=0;i<CurrentIsps.count();i++)
//  {
//  if(CurrentIsps[i].isEmpty())continue;
//  QStringList element=CurrentIsps[i].split(',');
//  if(element.count()<2)continue;
//  QTableWidgetItem *nameI= new QTableWidgetItem(element[0]);
//  QTableWidgetItem *portI= new QTableWidgetItem(element[1]);
//  QTableWidgetItem *stateI= new QTableWidgetItem(checkIsp(element[0],element[1].toInt()));
//
//         nameI->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//  portI->setFlags(Qt::ItemIsEnabled);
//         tableWidget->insertRow (tableWidget->rowCount ());
//  qDebug()<<"Insert row";
//  tableWidget->setItem(tableWidget->rowCount()-1,0,nameI);
//  tableWidget->setItem(tableWidget->rowCount()-1,1,portI);
//  tableWidget->setItem(tableWidget->rowCount()-1,2,stateI);
//  };
// qDebug()<<CurrentIsps;
};

QString externalIspDialog::checkIsp(QString name, int port)
{
	int mod_id = app()->compiler->modules()->idByName(name);
	if (mod_id > 0) {
		if (app()->compiler->modules()->module(mod_id)->port == port) {
			return QString::fromUtf8("Подключен");
		}
	};
	QTcpSocket socket;
	socket.connectToHost("localhost", port);
	if (socket.waitForConnected(700)) {
		socket.disconnectFromHost();
		return tr("Waiting");
	}
	return tr("No answer");
};

void externalIspDialog::listClicked(int row, int col)
{
	Q_UNUSED(row);
	Q_UNUSED(col);
// QTableWidgetItem *portI=tableWidget->item(row,1);
// qDebug()<<"tableWidget->item("<<row<<","<<col<<"):"<<tableWidget->item(row,col)->text()
//  <<"tableWidget->itemAt(2,col)"<<tableWidget->itemAt(2,col)->text()<<"tableWidget->itemAt(0,1)"<<tableWidget->itemAt(0,1)->text();
// port->setText(portI->text());
};
void externalIspDialog::openFile()
{
	QString defaultDir = QDir::currentPath();
	QDir lastDir = QDir(app()->settings->value("History/KumDir", defaultDir).toString());
	QString lastDirPath = lastDir.absolutePath().replace("/", QDir::separator()) + QDir::separator();
	QFileDialog *openDialog = new QFileDialog(this, tr("Open..."), lastDirPath, tr("Kumir modules (*.kum)"));
	if (openDialog->exec() == QDialog::Accepted) {
		if (! openDialog->selectedFiles().isEmpty()) {
			FileLine->setText(openDialog->selectedFiles().first());
		}
	};

};
void externalIspDialog::switchM()
{
	if (network()) {
		UrlLine->setEnabled(true);
		PortLine->setEnabled(true);
		FileLine->setEnabled(false);
		toolButton->setEnabled(false);
	} else {
		UrlLine->setEnabled(false);
		PortLine->setEnabled(false);
		FileLine->setEnabled(true);
		toolButton->setEnabled(true);
	};
};
