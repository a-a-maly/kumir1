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
#ifndef TAB_H
#define TAB_H

#include <QFont>
class QPrinter;

class TextEdit;

/** Интерфейс абстрактной вкладки */
class Tab
{
	public:
		/**
		 * Шрифт редактора
		 * @return шрифт
		 */
		virtual QFont font() = 0;
		virtual void print(QPrinter *printer) = 0;
		/**
		 * Текст редактора
		 * @return текст
		 */
		virtual QString editorText(bool withIndentation = false) = 0;
		virtual QString marginText() = 0;
		/**
		 * Деструктор
		 */
		virtual ~Tab() {}
		/**
		 * Установка шрифта редатора [и поля]
		 * @param f шрифт
		 */
		virtual void setFont(const QFont &f) = 0;
		/**
		 * Установка текста редактора
		 * @param s текст
		 */
		virtual void setEditorText(const QString &s) = 0;
		/**
		 * Возвращает true, если ProgramTab или false, если TextTab
		 * @return флаг програмной вкладки
		 */
		virtual bool isProgram() = 0;
		/**
		 * Возвращает флаг "является исполнителем"
		 * @return значение флага
		 */
		bool isModule() { return module; }
		/**
		 * Устанавливает флаг "является исполнителем"
		 * @param v значение флага
		 */
		void setModule(bool v) { module = v; };
		/**
		 * Обновление языка
		 */
		virtual void updateLanguage() {};
		/**
		 * Обновление шрифтов и цветов SyntaxHighlighter'а
		 */
		virtual void updateSHAppearance() {};
		/**
		 * Сохранение позиции разделителя между редактором и полем
		 */
		virtual void saveSplitterState() {};
		/**
		 * Отсоединение действий редактирования
		 */
		virtual void disconnectEditActions() = 0;
		/**
		 * Возвращает доступность операции Copy
		 * @return значение флага
		 */
		virtual bool isCopyAvailable() = 0;
		/**
		 * Возвращает доступность операции Undo
		 * @return значение флага
		 */
		virtual bool isUndoAvailable() = 0;
		/**
		 * Возвращает доступность операции Redo
		 * @return значение флага
		 */
		virtual bool isRedoAvailable() = 0;
		/**
		 * Устанавливает имя файла для вкладки
		 * @param name имя файла
		 */
		void setFileName(const QString & name);
		/**
		 * Возвращает имя файла во вкладке
		 * @return имя файла
		 */
		QString getFileName();
		/**
		 * Возвращает указатель на редактор
		 * @return 
		 */
		virtual TextEdit *editor() = 0;
		/**
		 * Устанавливает флаг изменения для вкладки
		 * @param name флаг изменения
		 */
		void setModified(bool flag);
		/**
		 * Возвращает флаг изменения во вкладке
		 * @return флаг изменения
		 */
		bool isModified();
		virtual void paste() = 0;
	protected:
		/** Имя файла */
		QString m_fileName;
		/** Флаг "является исполнителем" */
		bool module;
};

#endif
