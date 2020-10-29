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
#ifndef MACROSMANAGEMENTDIALOG_H
#define MACROSMANAGEMENTDIALOG_H

#include <QDialog>
#include "ui_MacrosManagementDialog.h"

/** Диалог управления макросами */
class MacrosManagementDialog : public QDialog, private Ui::Dialog
{
	Q_OBJECT

public:
	/**
	 * Конструктор
	 * @param parent ссылка на объект-владелец
	 * @param fl флаги окна
	 */
	MacrosManagementDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~MacrosManagementDialog();

public slots:
	/*$PUBLIC_SLOTS$*/

protected:
	/*$PROTECTED_FUNCTIONS$*/

protected slots:
	/**
	 * Выбор макроса из списка. Проверяет его редактируемость
	 * @param item ссылка на элемент списка
	 */
	void selectMacro(QListWidgetItem *item = NULL);
	/**
	 * Удаляет макрос, если это возможно
	 * @param item ссылка на элемент списка
	 */
	void deleteMacro(QListWidgetItem *item = 0);
	/**
	 * Вызывает диалог редактирования макроса, если он редактируемый
	 * @param item ссылка на элемент списка
	 */
	void editMacro(QListWidgetItem *item = 0);

};

#endif

