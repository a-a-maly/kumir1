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
#ifndef MACRO_H
#define MACRO_H

#include <QObject>
#include <QKeyEvent>
class QAction;
class QDomDocument;
class QDomNode;

/** Макрос */
class Macro : public QObject
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent сслыка на объект-владелец
	     */
	Macro(QObject *parent = 0);

	/**
	     * Устанавливает имя макроса
	     * @param name имя макроса
	     */
	void setName(QString name);
	/**
	     *  Возвращает имя макроса
	     * @return имя макроса
	     */
	QString name();
	/**
	     * Устанавливает сочетание клавиш для вызова макроса
	     * @param ks сочетание клавиш
	     */
	void setKeySequence(const QString &ks);
	/**
	     * Возвращает сочетание клавиш для вызова макроса
	     * @return сочетание клавиш
	     */
	QString keySequence();
	/**
	     * Добавляет событие к макросу
	     * @param event сслыка на событие
	     */
	void addCommand(QKeyEvent *event);
	/**
	     * Возвращает список событий макроса
	     * @return список событий
	     */
	QList<QKeyEvent> commands();
	/**
	     * Устанавливает объект меню приложения для вызова макроса
	     * @param action ссылка на QAction
	     */
	void setAction(QAction *action);
	/**
	     * Возвращает объект меню приложения для вызова макроса
	     * @return ссылка на QAction
	     */
	QAction *action();
	/**
	     * Сохраняет макрос в XML-структуре
	     * @param parent узел родительского XML-дерева
	     */
	void save(QDomDocument &parent);
	/**
	     * Восстанавливает макрос из XML-структуры
	     * @param tree узел XML-структуры
	     * @return true, если успешно или false в противном случае
	     */
	bool load(const QDomNode &tree);
	/**
	     * Возращает флаг "редактируемый"
	     * @return значение флага
	     */
	bool isEditable();
	/**
	     * Устанавливает флаг "редактируемый"
	     * @param v значение флага
	     */
	void setEditable(bool v);
	/**
	     * Устанавливает имя файла, в котором сохранен макрос
	     * @param name имя файла
	     */
	void setFileName(const QString &name);
	/**
	     * Возвращает имя файла, в котором сохранен макрос
	     * @return имя файла
	     */
	QString fileName();
	/** Деструктор */
	~Macro();
protected:
	/** Имя макроса */
	QString m_name;
	/** Последовательность клавиш */
	QString m_keySequence;
	QString m_nativeKeySequence;
	/** Список событий */
	QList<QKeyEvent> events;
	/** Ссылка на QAction */
	QAction *m_action;
	/** Флаг редактируемости */
	bool editable;
	/** Имя связанного файла */
	QString m_fileName;
};

#endif
