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

#ifndef PROGRAMTAB_H
#define PROGRAMTAB_H

#include <QWidget>
#include "ui_ProgramTab.h"
#include "tab.h"

/** Вкладка с окном редактора КУМИР-программ и полем вывода */
class ProgramTab : public QWidget, public Tab, private Ui::ProgramTab
{
	friend class KumirEdit;
	Q_OBJECT
	Q_PROPERTY(QFont font READ font WRITE setFont)
	Q_PROPERTY(QString editorText READ editorText WRITE setEditorText)
public:

	/**
	 * Конструктор
	 * @param parent ссылка на объект-владелец
	 * @param fl флаги окна
	 */
	ProgramTab(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~ProgramTab();
	/**
	 * Возвращает шрифт редактора
	 * @return шрифт
	 */
	virtual QFont font();

	void print(QPrinter *printer);
	/**
	* Возвращает текст редактора
	  * @return текст
	  */
	virtual QString editorText(bool withIndentation = false);
	virtual QString marginText();
	/**
	 * Возвращает флаг "является редактором КУМИР-файлов"
	 * @return всегда возвращает true
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
	/**
	 * Возвращает доступность операции разкомментирования
	 * @return значение флага
	 */
	bool isUncommentAvailable();
	/** Устанавливает номер вкладки. Используется для идентификации при компиляции */
	void setTabNo(int tabNo);
	/** Возвращает ссылку на редактор
	 * @return ссылка
	 */
	KumirEdit *editor();
	/** Возвращает ссылка на поле
	 * @return ссылка
	 */
	KumirMargin *margin();



//                void setFileName(QString name)
//                      {
//                       fileName=name;
//                      };
//                QString FileName()
//                      {
//                       return fileName;
//                      };

	void paste();
	//void fixSplitterAfterResize();
	//QList<int> getSplitterSizes();
	//void setSplitterSizes(const QList<int>&);
	//void fixSplitterLeft();
	//void fixSplitterRight();
	//int horizStretchAtW0();

public slots:
	/**
	 * Устанавливает шрифт редактора и полей
	 * @param f шрифт
	 */
	void setFont(const QFont &f);
	/**
	 * Обновляет язык
	 */
	void updateLanguage();
	/**
	 * Обновляет шрифты и цвета SyntaxHighlighter'а
	 */
	void updateSHAppearance();
	/**
	 * Устанавливает текст редактора
	 * @param s текст
	 */
	void setEditorText(const QString &s);
	/**
	 * Сохраняет позицию разделителя между редактором и полем
	 */
	void saveSplitterState();
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
	/**
	 *   "Редактирование -> Закомментировать"
	 */
	void editComment();
	/**
	 *   "Редактирование -> Разкомментировать"
	 */
	void editUncomment();
	/**
	 * Расстановка отступов редактора
	 */
	void applyIndentation();

	void synchonizeScroll(int dx, int dy);
	/**
	 * Устанавливает текст на поля
	 * и выравнивает его в соответствии с текстом программы
	 * @param txt текст
	 */
	void setMarginText(const QString &txt);
	void syncronizeLineHeights();
	void saveCursorPosition();
	void restoreCursorPosition();

protected:
	bool scrollLock;
	int textCursorPosition;

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
	/**
	 *  Посылает сигнал uncommentAvailable, если связь не разорвана
	 * @param v значение сигнала
	 */
	void emitUncommentAvailable(bool v);

	void syncCursorPosition(int no);


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
	/**
	 ;* Доступно разкомментирование
	 * @param v флаг доступности
	 */
	void uncommentAvailable(bool v);
private:
//                QString fileName; // Она же уже в tab.h объявлена??
};

#endif

