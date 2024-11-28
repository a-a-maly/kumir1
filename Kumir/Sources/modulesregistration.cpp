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

#include "modulesregistration.h"
#include "kumfiledialog.h"
#include "application.h"
#include "tools.h"
#include "pushdownautomata.h"
#include "text_analiz.h"
#include "messagesprovider.h"
#include "config.h"
#include "kummodules.h"
#include "kumsinglemodule.h"

#include <QShowEvent>
#include <QCloseEvent>
#include <QTcpSocket>
#include <QCheckBox>
#include <QSettings>

ModulesRegistration::ModulesRegistration(QWidget *parent, Qt::WindowFlags fl)  : QDialog(parent, fl), Ui::ModulesRegistration()
{
	setupUi(this);
	checkModules = NULL;
	connect(buttonRegNew, SIGNAL(clicked()), this, SLOT(connectExternalModule()));
	repaintExtModulesTable();
	connect(buttonDelete, SIGNAL(clicked()), this, SLOT(removeModule()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(connectNetwork()));
	connect(tableExtModules, SIGNAL(cellClicked(int, int)), this, SLOT(cellSelect(int, int)));
	dialog = new externalIspDialog(this);
	connect(dialog, SIGNAL(ispStateChanged()), this, SLOT(repaintTable()));
	setWindowModality(Qt::ApplicationModal);

}

ModulesRegistration::~ModulesRegistration()
{
	if (checkModules) {
		delete checkModules;
	}
}

void ModulesRegistration::closeEvent(QCloseEvent *event)
{
	app()->settings->setValue("ModulesRegistration/Geometry", saveGeometry());
	app()->settings->setValue("ModulesRegistration/ExtModTable-w0", tableExtModules->columnWidth(0));
	app()->settings->setValue("ModulesRegistration/ExtModTable-w1", tableExtModules->columnWidth(1));
	//app()->settings->setValue("ModulesRegistration/ExtNetTable-w0",tableNetModules->columnWidth(0));
	//app()->settings->setValue("ModulesRegistration/ExtNetTable-w1",tableNetModules->columnWidth(1));
	//app()->settings->setValue("ModulesRegistration/ExtNetTable-w2",tableNetModules->columnWidth(2));
	qDebug() << "Count in closing = " << ispData.count();
	app()->settings->setValue("ModulesRegistration/IspData/Count", ispData.count());
	for (int i = 0; i < ispData.count(); ++i) {
		app()->settings->setValue("ModulesRegistration/IspData/Filename-" + QString::number(i), ispData[i].file_name);
		app()->settings->setValue("ModulesRegistration/IspData/ID-" + QString::number(i), ispData[i].file_id);
	}
	app()->doCompilation();
	event->accept();
}

void ModulesRegistration::showEvent(QShowEvent *event)
{
	restoreGeometry(app()->settings->value("ModulesRegistration/Geometry").toByteArray());
	tableExtModules->setColumnWidth(0, app()->settings->value("ModulesRegistration/ExtModTable-w0", 150).toInt());
	tableExtModules->setColumnWidth(1, app()->settings->value("ModulesRegistration/ExtModTable-w1", 150).toInt());
	//tableNetModules->setColumnWidth(0,app()->settings->value("ModulesRegistration/ExtNetTable-w0",70).toInt());
	//tableNetModules->setColumnWidth(1,app()->settings->value("ModulesRegistration/ExtNetTable-w1",70).toInt());
	//tableNetModules->setColumnWidth(2,app()->settings->value("ModulesRegistration/ExtNetTable-w2",70).toInt());
	event->accept();
}

#ifdef USE_OLD_TEST_OPER_SKOBKA
void ModulesRegistration::openFileWithModules()
{
	QString defaultDir = QDir::currentPath();
	QDir lastDir = QDir(app()->settings->value("History/KumDir", defaultDir).toString());
	QString lastDirPath = lastDir.absolutePath().replace("/", QDir::separator()) + QDir::separator();
	QFileDialog *openDialog = new KumFileDialog(this, tr("Open..."), lastDirPath, tr("Kumir modules (*.kum)"));
	QString fileName;
	if (openDialog->exec() == QDialog::Accepted) {
		if (! openDialog->selectedFiles().isEmpty()) {
			fileName = openDialog->selectedFiles().first();
			app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
			qDebug() << fileName;
			moduleNames.clear();
			QFile modulesFile(fileName);
			globFileName = fileName;


			KumTools::instance()->splitUserFile(modulesFile, &fileText);
			ispNames = getIspNames(&fileText);
			if (checkModules == NULL) {
				checkModules = new CheckModules(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

				connect(checkModules, SIGNAL(okPressed(QList<int>)), this, SLOT(selectModules(QList<int>)));
			}
			checkModules->addStrings(ispNames);
			checkModules->show();
		}
	}
	delete openDialog;

}
#else
void ModulesRegistration::openFileWithModules(QString fileName)
{
//  QString defaultDir = QDir::currentPath();
//  QDir lastDir = QDir(app()->settings->value("History/KumDir",defaultDir).toString());
//  QString lastDirPath = lastDir.absolutePath().replace("/",QDir::separator())+QDir::separator();
//  KumFileDialog *openDialog = new KumFileDialog(this, tr("Open..."), lastDirPath, tr("Kumir modules (*.kum)"), false);
//  QString fileName;


	app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
	qDebug() << fileName;
	moduleNames.clear();
	QFile modulesFile(fileName);
	globFileName = fileName;

	QStringList readData;

	QList<ProgaText> normalizedText;

	if (modulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream ts(&modulesFile);
		ts.setCodec("UTF-16LE");
		readData = ts.readAll().split("\n");
		modulesFile.close();
	}

	foreach (QString line, readData) {
		QPoint errorPosition;
		KumTools::instance()->splitUserLine(line, errorPosition, &normalizedText);
	}



	for (int i = 0; i < normalizedText.count(); i++) {
		normalizedText[i].Base_type = KumTools::instance()->getStringType(normalizedText[i].Text);
	}

	app()->compiler->getPdAutomata()->init(normalizedText, QFileInfo(fileName).absoluteFilePath(), NULL);
	if (app()->compiler->getPdAutomata()->process() == 1) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(" (при компиляции файла: ") + fileName + " )";
		qDebug() << QString::fromUtf8(">> Не хватает правил описания ошибочных ситуаций!");
		qDebug() << "===========================================================";
	} else {
		if (!app()->compiler->getPdAutomata()->postProcess()) {
			qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
			qDebug() << QString::fromUtf8(" (при компиляции файла: ") + fileName + " )";
			qDebug() << QString::fromUtf8(">> Ошибка в одном из скриптов!");
			qDebug() << "===========================================================";
		}
	}
//			int debug = app()->compiler->getPdAutomata()->modules().count();

//          ispNames=getIspNames(&fileText);
	ispNames = getIspNames(app()->compiler->getPdAutomata()->modules());
	loadedModules = app()->compiler->getPdAutomata()->modules();
	if (checkModules == NULL) {
		checkModules = new CheckModules(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

		connect(checkModules, SIGNAL(okPressed(QList<int>)), this, SLOT(selectModules(QList<int>)));
	}
	checkModules->addStrings(ispNames);
	checkModules->show();


}
#endif
void ModulesRegistration::connectExternalModule()
{
	externalIspDialog *dialog = new externalIspDialog(this);
	if (dialog->exec()) {
		qDebug() << "OK";
		if (!dialog->network()) {
			openFileWithModules(dialog->file());
			repaintExtModulesTable();
			return;
		} else {

			dialog->connectIsp();
			repaintExtModulesTable();
			return;
		};

	};

};



void ModulesRegistration::removeIspData(int tableId)
{
	for (int i = 0; i < ispData.count(); i++) {
		if (ispData[i].table_id == tableId) {
			ispData.removeAt(i);
			return;
		};
	};



};
void ModulesRegistration::removeModule()
{
	pushButton->setEnabled(false);
	qDebug() << "pushButt disable";
	QList<int> toDelete;
	QList<QTableWidgetSelectionRange> ranges = tableExtModules->selectedRanges();
	for (int i = 0; i < ranges.count(); i++) {
		for (int j = ranges[i].topRow(); j < ranges[i].bottomRow() + 1; j++) {
			toDelete.append(j);
		}
	};
	for (int i = 0; i < toDelete.count(); i++) {
		QTableWidgetItem *nameItem = tableExtModules->item(toDelete[i], 0);
		QString toDeleteName = nameItem->text();
		int id = app()->compiler->modules()->idByName(toDeleteName);
		app()->compiler->modules()->removeModule(id);

		removeIspData(id);
		if (nameItem->type() == 1) {
			QStringList CurrentIsps = app()->ExtIspsList();
			QString addr = tableExtModules->item(toDelete[i], 1)->text();
			QStringList addrL = addr.split(':');
			if (addrL.count() < 2) {
				qDebug() << "ModulesRegistration::removeModule:bad list";
				return;
			}
			int id = CurrentIsps.indexOf(QRegExp(toDeleteName + "," + addrL[1] + "," + addrL[0]));
			if (addrL[0] == "localhost") {
				id = CurrentIsps.indexOf(QRegExp(toDeleteName + "," + addrL[1]));
			}
			app()->RemoveAtExtIspsList(id);
			qDebug() << "ToDel id:" << id;
		}
	};
	repaintExtModulesTable();

};

//repaintExtModulesTable();}
QList<IspNames> ModulesRegistration::getIspNames(const QList< QList<proga> > &modules_data)
{
	QList<IspNames> ispNames;
	foreach (const QList<proga> &module_data, modules_data) {
		K_ASSERT(module_data.count() > 1);
		const proga pv = module_data[0];
		IspNames ispName;
		if (pv.str_type != Kumir::ModuleBegin) {
			ispName.name = pv.line;
			ispName.error = tr("Error in file");
			ispNames << ispName;
		} else {
			int err_start = 0;
			int err_len = 0;
			QString name;
			int error = 0;
			error = app()->compiler->getAnalizer()->parce_isp(pv.line, NULL, NULL, -1, &err_start, &err_len, pv.real_line.fileName, &name);
			if (error > 0) {
				ispName.name = name.trimmed().isEmpty() ? pv.line : name;
				ispName.error = app()->messagesProvider->errorMessage(error);
				ispNames << ispName;
			} else {
				int id = app()->compiler->modules()->idByName(name);
				if (id != -1) {
					ispName.name = name;
					ispName.error = tr("Module has already registered");
					ispNames << ispName;
				} else {
					ispName.name = name;
					ispName.error = "";
					ispNames << ispName;
				}
			}
		}
	}
	return ispNames;
}


QList<IspNames> ModulesRegistration::getIspNames(QList<QList<ProgaText> > *ispText)
{
	QList<IspNames> ispNames;

	for (int i = 0; i < ispText->count(); i++) {
		IspNames t_ispNames;
		ProgaText p_text = ispText->at(i).at(0);
		QString instr = p_text.text();
		t_ispNames.error = "";
		if (instr[0] != QChar(KS_ISP)) {
			t_ispNames.name = instr;
			t_ispNames.error = tr("Error in file");
			ispNames.append(t_ispNames);
			continue;
		};
		instr.remove(0, 1);
		instr = instr.trimmed();

		if (instr.isEmpty()) {
			t_ispNames.name = instr;
			t_ispNames.error = tr("No name");
			ispNames.append(t_ispNames);
			continue;

		};
		int err_start = 0, err_len = 0;
		int err = KumTools::instance()->test_name(instr, err_start, err_len);
		if (err > 0) {
			t_ispNames.name = instr;
			t_ispNames.error = tr("Bad name");
			ispNames.append(t_ispNames);

			continue;
		};
		QString ispName = QString::fromUtf8(instr.toUtf8().data());
		int id = app()->compiler->modules()->idByName(ispName);
		if (id == -1) {
			t_ispNames.name = ispName;
			ispNames.append(t_ispNames);
		} else {
			t_ispNames.name = ispName;
			t_ispNames.error = trUtf8("Module has already registered");
			ispNames.append(t_ispNames);
		};


	};

	return ispNames;
};

CheckModules::CheckModules(QWidget *parent, Qt::WindowFlags fl)    : QDialog(parent, fl), Ui::CheckModules()
{
	setupUi(this);
	count = 0;
	cbs = NULL;
	QStringList headers;
	//headers.append( tr("Modules") );
	//table->setHorizontalHeaderLabels(headers);
	table->horizontalHeader()->setVisible(true);
	table->verticalHeader()->setVisible(false);
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(sendFlags()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(sendNone()));
}

CheckModules::~CheckModules()
{
}

void CheckModules::sendFlags()
{
	answer.clear();
	for (int i = 0; i < count; ++i)
		if (cbs[i]->checkState() == Qt::Checked) {
			answer << i;
		}
	close();
}

void CheckModules::sendNone()
{
	answer.clear();
	close();
}

void CheckModules::closeEvent(QCloseEvent *event)
{
	emit okPressed(answer);
	app()->settings->setValue("CheckModules/Geometry", saveGeometry());
	app()->settings->setValue("CheckModules/w0", table->columnWidth(0));
	app()->settings->setValue("CheckModules/w1", table->columnWidth(1));
	event->accept();
}

void CheckModules::showEvent(QShowEvent *event)
{
	restoreGeometry(app()->settings->value("CheckModules/Geometry").toByteArray());
	table->setColumnWidth(0, app()->settings->value("CheckModules/w0", 80).toInt());
	table->setColumnWidth(1, app()->settings->value("CheckModules/w1", 80).toInt());
	event->accept();
}

#ifdef USE_OLD_TEST_OPER_SKOBKA
void ModulesRegistration::selectModules(QList<int> list)
{
	qDebug() << "selectModules";
	qDebug() << list;
	IspData t_ispData;
	t_ispData.file_name = globFileName;
	qDebug() << fileText.count();
	for (int i = 0; i < list.count(); i++) {
		qDebug() << ispNames[list.at(i)].name;
		app()->compiler->modules()->appendModuleFromTab(ispNames[list.at(i)].name, NULL, fileText[list.at(i)]);
		t_ispData.file_id = list.at(i);
		ispData.append(t_ispData);
		app()->compiler->modules()->lastModule()->setFile(globFileName);
		ispData.last().table_id = app()->compiler->modules()->lastModuleId();
		app()->compiler->compileModule(app()->compiler->modules()->lastModuleId());
	};
	repaintExtModulesTable();
}
#else
void ModulesRegistration::selectModules(QList<int> list)
{
	qDebug() << "selectModules";
	qDebug() << list;
	IspData t_ispData;
	t_ispData.file_name = globFileName;
	qDebug() << fileText.count();
	for (int i = 0; i < list.count(); i++) {
		qDebug() << ispNames[list.at(i)].name;
		app()->compiler->modules()->appendExternalFileModule(ispNames[list.at(i)].name, loadedModules[list.at(i)]);
		t_ispData.file_id = list.at(i);
		ispData.append(t_ispData);
		app()->compiler->modules()->lastModule()->setFile(globFileName);
		ispData.last().table_id = app()->compiler->modules()->lastModuleId();
		app()->compiler->compileModule(app()->compiler->modules()->lastModuleId());
	};
	repaintExtModulesTable();
}
#endif

#ifdef USE_OLD_TEST_OPER_SKOBKA
int ModulesRegistration::regModule(QString file, uint id)
{
	qDebug() << "ModulesRegistration::regModule( " << file << ", " << id << " )";
	QFile modulesFile(file);

	KumTools::instance()->splitUserFile(modulesFile, &fileText);
	QList<IspNames> l_ispNames = getIspNames(&fileText);
	qDebug() << l_ispNames[id].name;
	if (id >= l_ispNames.count()) {
		qDebug("ModulesRegistration::regModule:ispId>count!!!!");
		return -1;
	};

	app()->compiler->modules()->appendModuleFromTab(l_ispNames[id].name, NULL, fileText[id]);
	IspData t_ispData;
	t_ispData.file_id = id;
	t_ispData.file_name = file;
	ispData.append(t_ispData);
	app()->compiler->modules()->lastModule()->setFile(file);
	ispData.last().table_id = app()->compiler->modules()->lastModuleId();
	app()->compiler->compileModule(app()->compiler->modules()->lastModuleId());

	repaintExtModulesTable();
	return app()->compiler->modules()->lastModuleId();
};
#else

int ModulesRegistration::regModule(QString file, int id)
{
	qDebug() << "ModulesRegistration::regModule( " << file << ", " << id << " )";
	return 0;
	if (!QFile::exists(file)) {
		return -1;
	}
	QFile modulesFile(file);
	//return app()->compiler->modules()->lastModuleId();
	QStringList lines;
	QList<ProgaText> normalizedText;

	if (modulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream ts(&modulesFile);
		ts.setCodec("UTF-16LE");
		lines = ts.readAll().split("\n");
		modulesFile.close();
	}

	foreach (QString line, lines) {
		QPoint errorPosition;
		KumTools::instance()->splitUserLine(line, errorPosition, &normalizedText);
	}



	for (int i = 0; i < normalizedText.count(); i++) {
		normalizedText[i].Base_type = KumTools::instance()->getStringType(normalizedText[i].Text);
	}

	app()->compiler->getPdAutomata()->init(normalizedText, QFileInfo(file).absoluteFilePath(), NULL);
	if (app()->compiler->getPdAutomata()->process() == 1) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(" (при компиляции файла: ") + file + " )";
		qDebug() << QString::fromUtf8(">> Не хватает правил описания ошибочных ситуаций!");
		qDebug() << "===========================================================";
		return -1;
	} else {
		if (!app()->compiler->getPdAutomata()->postProcess()) {
			qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
			qDebug() << QString::fromUtf8(" (при компиляции файла: ") + file + " )";
			qDebug() << QString::fromUtf8(">> Ошибка в одном из скриптов!");
			qDebug() << "===========================================================";
			return -1;
		}
	}
//	int debug = app()->compiler->getPdAutomata()->modules().count();
	loadedModules = app()->compiler->getPdAutomata()->modules();

	QList<IspNames> l_ispNames = getIspNames(app()->compiler->getPdAutomata()->modules());
	qDebug() << l_ispNames[id].name;
	if (id >= l_ispNames.count()) {
		qDebug("ModulesRegistration::regModule:ispId>count!!!!");
		return -1;
	};

	app()->compiler->modules()->appendExternalFileModule(l_ispNames[id].name, loadedModules[id]);

	IspData t_ispData;
	t_ispData.file_id = id;
	t_ispData.file_name = file;
	ispData.append(t_ispData);
	app()->compiler->modules()->lastModule()->setFile(file);
	ispData.last().table_id = app()->compiler->modules()->lastModuleId();
	app()->compiler->compileModule(app()->compiler->modules()->lastModuleId());

	repaintExtModulesTable();
	return app()->compiler->modules()->lastModuleId();
};
#endif

void CheckModules::addStrings(const QList<IspNames> &list)
{
	table->clearContents();
	while (table->rowCount() > 0) {
		table->removeRow(0);
	}
	count = list.count();
	cbs = new QCheckBox*[count];
	for (int i = 0; i < count; ++i) {
		cbs[i] = new QCheckBox(list[i].name);
		table->insertRow(table->rowCount());
		table->setCellWidget(table->rowCount() - 1, 0, cbs[i]);
		qDebug() << list[i].error;
		if (list[i].error != "") {
			cbs[i]->setEnabled(false);
			cbs[i]->setCheckState(Qt::Unchecked);
		};
		QTableWidgetItem *errorIt = new QTableWidgetItem(list[i].error);
		errorIt->setFlags(Qt::ItemIsEnabled);
		table->setItem(table->rowCount() - 1, 1, errorIt);
	}
}

void ModulesRegistration::repaintExtModulesTable()
{
	if (app()->isBatchMode()) {
		return;
	}
	//tableExtModules->clearContents();
	while (tableExtModules->rowCount() > 0) {
		tableExtModules->removeRow(0);
	}
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); i++) {
		//continue;
		if ((app()->compiler->modules()->module(i)->isKumIsp()
				&& (app()->compiler->modules()->module(i)->file() != ""))) {
			QTableWidgetItem *nameIt = new QTableWidgetItem(app()->compiler->modules()->module(i)->name, 0);
			nameIt->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			QTableWidgetItem *fileIt = new QTableWidgetItem(app()->compiler->modules()->module(i)->file());
			fileIt->setFlags(Qt::ItemIsEnabled);
			tableExtModules->insertRow(tableExtModules->rowCount());
			QTableWidgetItem *stateIt = new QTableWidgetItem(tr("Connected"));

			tableExtModules->setItem(tableExtModules->rowCount() - 1, 0, nameIt);
			tableExtModules->setItem(tableExtModules->rowCount() - 1, 2, fileIt);
			tableExtModules->setItem(tableExtModules->rowCount() - 1, 3, stateIt);
		};
	};
	QStringList CurrentIsps = app()->ExtIspsList();
	for (int i = 0; i < CurrentIsps.count(); i++) {
		QString element = CurrentIsps[i];
		QStringList elementData = element.split(',');
		if (elementData.count() < 2) {
			continue;
		}
		//QTableWidgetItem *nameIt = new QTableWidgetItem(elementData[0],1);
		//nameIt->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		//QTableWidgetItem *portIt = new QTableWidgetItem("localhost:"+elementData[1]);
		//portIt->setFlags(Qt::ItemIsEnabled);
//                QTableWidgetItem *portIt = new QTableWidgetItem("localhost:"+elementData[1],1);
		QString state = "!";
		//state=" "+;
		qDebug() << state;
		QString url = "localhost";
		if (elementData.count() > 2) {
			url = elementData[2];
		}
		tableExtModules->insertRow(tableExtModules->rowCount());
		tableExtModules->setItem(tableExtModules->rowCount() - 1, 0, new
			QTableWidgetItem(elementData[0], 1));
		tableExtModules->setItem(tableExtModules->rowCount() - 1, 1, new
			QTableWidgetItem(url + ":" + elementData[1], 1));
		tableExtModules->setItem(tableExtModules->rowCount() - 1, 3,
			new QTableWidgetItem(checkIsp(elementData[0], elementData[1].toInt(), url), 1));
		qDebug() << "Rows:" << tableExtModules->rowCount() << " Columns" << tableExtModules->columnCount();

	};
};

void CheckModules::showMessage(int n, const QString &message)
{
	if (n < table->rowCount()) {
		table->setItem(n, 1, new QTableWidgetItem(message));
		((QCheckBox *)table->cellWidget(n, 0))->setCheckState(Qt::Unchecked);
		table->cellWidget(n, 0)->setEnabled(false);
	}
}


QString ModulesRegistration::checkIsp(QString name, quint16 port, QString url)
{
	int mod_id = app()->compiler->modules()->idByName(name);
	if (mod_id > 0) {
		if (app()->compiler->modules()->module(mod_id)->port == port) {
			return QString::fromUtf8("Подключен");
		}
	};
	mod_id = app()->compiler->modules()->idByPort(port);
	if (mod_id > 0) {
		return QString::fromUtf8("Подключен");
	}

	QTcpSocket socket;
	uint wait = LOCAL_WAIT;
	if (url != "localhost") {
		wait = NET_WAIT;
	}
	socket.connectToHost(url, port);//TODO Vneshnie
	if (socket.waitForConnected(wait)) {
		socket.disconnectFromHost();
		return QString::fromUtf8("Ожидает");
	};
	return QString::fromUtf8("Нет ответа");
};

void ModulesRegistration::repaintTable()
{
	if (!app()->isBatchMode()) {
		repaintExtModulesTable();
	}
	qDebug() << "Repaint";
};
void ModulesRegistration::showForm()
{
	repaintExtModulesTable();
	show();
};
void ModulesRegistration::connectNetwork()
{
	int row = tableExtModules->currentRow();
	QTableWidgetItem *first = tableExtModules->item(row, 0);
	if (first->type() == 0) {
		return;
	}
	qDebug() << "Row:" << row << " Columns:" << tableExtModules->columnCount();
	if (tableExtModules->columnCount() < 2) {
		return;
	}
	QTableWidgetItem *url_port = tableExtModules->item(row, 1);
	QString addr = url_port->text();
	QStringList addrSplit = addr.split(':');
	if (addrSplit.count() < 2) {
		return;
	}
	dialog->connectIsp(addrSplit[0], addrSplit[1].toInt());

};
void ModulesRegistration::cellSelect(int row, int col)
{
	Q_UNUSED(col);
	QTableWidgetItem *first = tableExtModules->item(row, 0);
	if (first->type() == 1) {
		pushButton->setEnabled(true);
	} else {
		pushButton->setEnabled(false);
	}

};
