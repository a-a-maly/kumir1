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
#ifndef MODULESREGISTRATION_H
#define MODULESREGISTRATION_H

#include "ui_ModulesRegistration.h"
#include "ui_CheckModules.h"
#include "kum_tables.h"
#include "compiler.h"
#include "externalIsp.h"

#include <QDialog>

#define LOCAL_WAIT 700
#define NET_WAIT 2000


class QCheckBox;

struct IspNames {
	QString name;
	QString error;
};
struct IspData {
	QString file_name;
	int file_id;
	int table_id;
};

/** Окно включения исполнителей */
class CheckModules : public QDialog, public Ui::CheckModules
{
	Q_OBJECT
public:
	/**
	* Конструктор
	* @param parent ссыка на объект-владелец
	* @param fl флаги окна
	 */
	CheckModules(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~CheckModules();

	void addStrings(const QList<IspNames> &list);

protected:
	int count;
	QCheckBox **cbs;
	void closeEvent(QCloseEvent *event);
	void showEvent(QShowEvent *event);
	QList<int> answer;
	void showMessage(int n, const QString &message);

protected slots:
	void sendFlags();
	void sendNone();

signals:
	void okPressed(QList<int>);

};

/** Окно регистрации исполнителей */
class ModulesRegistration : public QDialog, private Ui::ModulesRegistration
{
	Q_OBJECT
public:
	/**
	* Конструктор
	* @param parent ссыка на объект-владелец
	* @param fl флаги окна
	 */
	ModulesRegistration(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~ModulesRegistration();
	int regModule(QString file, int id);
	QString checkIsp(QString name, quint16 port, QString url);
	void tryToConnect(uint port)
	{
		dialog->connectIsp("localhost", port);
	};
	bool bachReady()
	{
		return dialog->bachWaitFlag;
	};
protected:
	CheckModules *checkModules;
	QList<QList<ProgaText> > fileText;
	QList<IspNames> getIspNames(QList<QList<ProgaText> > *ispText);
	QList<IspNames> getIspNames(const QList< QList<proga> > &modules_data);
	void closeEvent(QCloseEvent *event);
	void showEvent(QShowEvent *event);
	QString globFileName;
	QStringList moduleNames;

protected slots:
	void openFileWithModules(QString fileName);
	void removeModule();
	void connectExternalModule();//Добавить исполнитель в таблицу
	void connectNetwork();//Подключить сетевой исполнитель

	void selectModules(QList<int>);
	void repaintExtModulesTable();
	void removeIspData(int tableId);
	void repaintTable();
	void showForm();
	void cellSelect(int row, int col);
private:
	QList<IspNames> ispNames;
	QList<IspData>  ispData;
	QList< QList<proga> > loadedModules;
	externalIspDialog *dialog;

};

#endif
