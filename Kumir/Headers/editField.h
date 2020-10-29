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
#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QMainWindow>
#include "ui_editField.h"
#include "kum_instrument.h"

#define MAX_COLUMNS 16
#define MAX_ROWS 10

class editField : public QMainWindow, public Ui::editField
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссыка на объект-владелец
	 * @param fl флаги окна
	 */
	editField(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~editField();
public:
	RoboField *field;
signals:
	void editFinished();
public slots:
	void PrintPDF();
	void Help();
	void setSizes();
	void markModeChange();
	void New();
	void loadFromFile();
	void Save();
	void SaveAs();
	void SaveAsStart();
	void Close();
	void openRecent();
	void btnOK1Click();
	void getLastList()
	{
		lastFiles.clear();
		// app()->settings->setValue("Robot/LastFiles",lastFiles.join(";"));
		QStringList lFiles = app()->settings->value("Robot/LastFiles").toString().split(";");
		for (int i = 0; i < lFiles.count(); i++)
			if (!lFiles[i].isEmpty()) {
				lastFiles.append(lFiles[i]);
			}
		while (lastFiles.count() > 7) {
			lastFiles.removeLast();
		}
	};
	void fieldChange();
	void setWindowTitleText(QString fullFileName)
	{
		CurrentFileName = fullFileName;
		QFileInfo fi(CurrentFileName);
		QString name = fi.fileName();
		setWindowTitle(QString::fromUtf8("Обстановка - ") + name);
		this->setWindowModified(false);
	};
	void prepare()
	{
		setSizes();
	};
	void append2LastFiles(QString fullFileName)
	{
		QString tmp = fullFileName.replace('\\', '/');
		if (lastFiles.indexOf(fullFileName) != -1) {
			return;
		}
		if (lastFiles.indexOf(tmp) != -1) {
			return;
		}
		lastFiles.prepend(fullFileName);
	};
	void MoveUp(QString fullFileName)
	{
		QString tmp = fullFileName.replace('\\', '/');
		int index = lastFiles.indexOf(fullFileName);
		if (index == -1) {
			index = lastFiles.indexOf(tmp);
		}
		if (index == -1) {
			return;
		}
		lastFiles.move(index, 0);

	};

public:
	void createRescentMenu();
	QString curDir, CurrentFileName;
	QWidget *NewWindow;
	QPushButton *btnOK1;
	QPushButton *btnCancel1;  //Кнопки диалога новая обстановка
	QSpinBox *eXSizeEdit;
	QSpinBox *eYSizeEdit;
protected:
	virtual void closeEvent(QCloseEvent *event);
private:
	QStringList lastFiles;
	QMenu *rescentMenu;
	PrintDialog *pDialog;
	void prepareNewWindow();
	int menuH;
	bool fOpenErr;


};
#endif
