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
#ifndef TEXTTAB_H
#define TEXTTAB_H

#include <QWidget>
#include "tab.h"

class TextEdit;

/** Вкладка с редактором текстовых файлов */
class TextTab : public QWidget, virtual public Tab
{
	Q_OBJECT
	Q_PROPERTY(QFont font READ font WRITE setFont)
	Q_PROPERTY(QString editorText READ editorText WRITE setEditorText)
public:
	/**
	* Конструктор
	* @param parent ссылка на объект-владелец
	* @param fl флаги окна
	 */
	TextTab(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	void print(QPrinter *printer);
	/**
	 * Деструктор
	 */
	~TextTab();
	/**
	 * Возвращает шрифт редактора
	 * @return шрифт
	 */
	QFont font();
	/**
	 * Возвращает текст редактора
	 * @return текст
	 */
	QString editorText(bool withIndentation = false);
	QString marginText();
	/** Возвращает ссылку на редактор
	 * @return сслыка
	 */
	TextEdit *editor();
	/**
	 * Возвращает флаг "является редактором КУМИР-файлов"
	 * @return всегда возвращает false
	 */
	bool isProgram();
	/**
	 * Возвращает доступность операции Copy
	 * @return значение флага
	 */
	bool isCopyAvailable();
	/**
	 * Возвращает доступность операции Undo
	 * @return значение флага
	 */
	bool isUndoAvailable();
	/**
	 * Возвращает доступность операции Redo
	 * @return значение флага
	 */
	bool isRedoAvailable();
	void paste();
public slots:
	/**
	* Устанавливает шрифт редактора и полей
	* @param f шрифт
	 */
	void setFont(const QFont &);
	/**
	 * Устанавливает текст редактора
	 * @param s текст
	 */
	void setEditorText(const QString &);
	/**
	 * Разрывает связи copyAvailable, undoAvailable и redoAvailable
	 */
	void disconnectEditActions();
	/**
	 *  "Редактирование -> Вырезать"
	 */
	void editCut();
	/**
	 * "Редактирование -> Копировать"
	 */
	void editCopy();
	/**
	 *   "Редактирование -> Вставить"
	 */
	void editPaste();
	/**
	 *   "Редактирование -> Отмена"
	 */
	void editUndo();
	/**
	 *   "Редактирование -> Повтор"
	 */
	void editRedo();

protected:
	/** Ссылка на текстовый редактор */
	TextEdit *m_editor;

protected slots:
	/**
	*  Посылает сигнал copyAvailable, если связь не разорвана
	* @param v значение сигнала
	 */
	void emitCopyAvailable(bool v);
	/**
	 *  Посылает сигнал undoAvailable, если связь не разорвана
	 * @param v значение сигнала
	 */
	void emitUndoAvailable(bool v);
	/**
	 *  Посылает сигнал redoAvailable, если связь не разорвана
	 * @param v значение сигнала
	 */
	void emitRedoAvailable(bool v);


signals:
	/**
	* Доступно копирование/вырезание
	* @param v флаг доступности
	 */
	void copyAvailable(bool v);
	/**
	 * Доступна отмена
	 * @param v флаг доступности
	 */
	void undoAvailable(bool v);
	/**
	 * Доступно повторение отмененного действия
	 * @param v флаг доступности
	 */
	void redoAvailable(bool v);


};

#endif

