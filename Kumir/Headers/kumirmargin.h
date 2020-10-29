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
#ifndef KUMIRMARGIN_H
#define KUMIRMARGIN_H

class KumirEdit;
class ProgramTab;

#include <QTextEdit>


/** Поле редактора для вывода текстов ошибок и значений при выполнении */
class KumirMargin : public QTextEdit
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссылка на объект-владелец
	 */
	KumirMargin(QWidget *parent = 0);
	void setEditor(KumirEdit *editor);
	void setTab(ProgramTab *t);
public slots:
	void setText(const QString &text);
	void setDebugText(const QString &text, int lineNo, int columnNo);
	void clearError(int lineNo);
	void removeLines(int from, int count);
	void insertEmptyLines(int from, int count);
protected:
	/**
	* Перехват события изменения размера. Вызывает проверку областей прокрутки.
	* @param event ссылка на событие
	 */
	void resizeEvent(QResizeEvent *event);
	/**
	 * Перехват события прокрутки. Посылает сигнал для синхронизации с редактором.
	 * @param dx прокрутка по горизонтали
	 * @param dy прокрутка по вертикали
	 */
	void scrollContentsBy(int dx, int dy);
	void keyPressEvent(QKeyEvent *event);
	int scrollBarHeight;

protected slots:
	/** Проверяет появление областей прогрутку. Используется для выравнивания текста с полем */
	void checkScrollBars();
signals:
	/**
	 * Произошла прокрутка текста
	 * @param dx величина по горизонтали
	 * @param dy величина по вертикали
	 */
	void contentsMoving(int dx, int dy);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	KumirEdit *editor;
	ProgramTab *tab;
	QVector<QStringList> debugTable;
};

#endif
