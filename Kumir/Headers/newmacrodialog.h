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
#ifndef NEWMACRODIALOG_H
#define NEWMACRODIALOG_H

#include <QDialog>
#include "ui_NewMacroDialog.h"

class Macro;

/** Диалог создания/редактирования макроса */
class NewMacroDialog : public QDialog, private Ui::NewMacroDialog
{
	Q_OBJECT

public:
	/**
	     * Конструктор диалога при создании нового макроса
	     * @param parent ссылка на объект-владелец
	     * @param fl флаги окна
	     */
	NewMacroDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	     * Конструктор диалога при редактировании макроса
	     * @param m ссылка на редактируемый макрос
	     * @param parent ссылка на объект-владелец
	     * @param fl флаги окна
	     */
	NewMacroDialog(Macro *m, QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	     * Деструктор
	     */
	~NewMacroDialog();
	/**
	     * Возвращает имя макроса
	     * @return имя макроса
	     */
	QString macroName();
	/**
	     * Возвращает последовательность клавиш
	     * @return последовательность клавиш
	     */
	QString macroSequence();

public slots:
	/**
	     * Закрытие диалога кнопкой OK
	     */
	void accept();

protected:
	/** Флаг "режим редактирования". Если true, то редактирование макроса, иначе - создание нового */
	bool editMode;
	/** Ссылка на редактируемый макрос */
	Macro *macro;
	QString prefix;

protected slots:
	/**
	     * Выполняет перевод произвольного символа клавиатуры в его латинский аналог (в верхрем регистре)
	     * @param s исходная клавиша
	     */
	void keyChanged(const QString &s);


};

#endif

