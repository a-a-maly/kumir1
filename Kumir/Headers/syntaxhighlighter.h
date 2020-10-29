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
#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QMutex>


class KumirEdit;
class LineProp;



/** Класс подсветки синтаксиса */
class SyntaxHighlighter : public QSyntaxHighlighter
{
	struct Block {
		inline Block(int a, int b, char c) { pos=a; len=b; val=c; }
		int pos;
		int len;
		char val;
	};

	Q_OBJECT
	public:
		/**
		 * Конструктор
		 * @param te ссылка на редактор КУМИР-программ
		 */
		SyntaxHighlighter(KumirEdit *te);
		/**
		 * Добавить новое имя функции
		 * @param name имя функции
		 */
		void addName(QString name);
		bool isOverloadLatinFont() const;
		bool isAlwaysBold() const;
		/**
		 * Очистить список имен функций
		 */
		void clearNames();
		/**
		 * Инициализировать регулярное выражение по списку имен функций
		 */
		void flushNames();
		/**
		 * Обработка текстового блока
		 * @param text текст
		 */
		void highlightBlock ( const QString &text );
		QString highlightBlockAsHtml( const QString &text, LineProp *lp );
		/**
		 * Возвращает значение флага модификации таблицы имен
		 * @return значение флага
		 */
		bool isNameTableModifed() { return nameTableModifed; }
		/**
		 * Устанавливает значение флага модификации таблицы имен
		 * @param val значение флага
		 */
		void setNameTableModifed(bool val) { nameTableModifed = val; }
		/** Регулярное выражение для поиска имен функций */
		QRegExp rxNames;
		/** Блокировка на время изменения списка функций */
		void lock();
		/** Разблокировка после изменения списка функций */
		void unlock();
		QString keywordCSS() const;
		QString typeCSS() const;
		QString valueCSS() const;
	public slots:
		/**
		 *  Инициализация начертаний шрифтов и цветов
		 */
		void updateAppearance();
	protected:
		QByteArray generateFormatString(const QString &text, LineProp *lp);
		QList<Block> splitFormatString(const QByteArray &in);
		/**
		 * Поиск литеральной строки в тексте (между двойными или одинарными кавычками)
		 * @param IniString исходная строка
		 * @param startPos начальная позиция для поиска
		 * @param endPos конечная позиция для поиска
		 * @param LitBeg начало литеральной строки, если она найдена, или -1 в ином случае
		 * @param LitEnd конец лителальной строки, если она найдена, или -1 в ином случае
		 * @return 1, если найдена литеральная строка или 0, если не найдена
		 */
		int FindLit(QString IniString,int startPos,int endPos,int *LitBeg,int *LitEnd);
	private:
		/** Мьютекс, защищающий список имен функций */
		QMutex namesLock;
		/** Ссылка на редактор КУМИР-программ */
		KumirEdit *ce;
		/** Регулярное выражение для поиска числовых констант */
		QRegExp rxInteger;
		QRegExp rxIntegerHex;
		QRegExp rxDouble;
		QRegExp rxDoubleExp;
		/** Регулярное выражение для поиска комментариев */
		QRegExp rxComment;
		/** Регулярное выражение для поиска строк документации */
		QRegExp rxDoc;
		/** Формат простого текста */
		QTextCharFormat Plain;
		/** Формат гиперссылки в литеральной константе*/
		QTextCharFormat Hyper;
		/** Формат гиперссылки в комментарии */
		QTextCharFormat Hyper1;
		/** Формат ключевого слова */
		QTextCharFormat Keyword;
		/** Формат имени типа */
		QTextCharFormat Type;
		/** Формат имени алгоритма */
		QTextCharFormat Name;
		/** Формат имени исполнителя */
		QTextCharFormat Isp;
		/** Формат числового значения */
		QTextCharFormat Value;
		/** Формат числового значения с ошибкой */
		QTextCharFormat String;
		/** Формат комментария */
		QTextCharFormat Comment;
		/** Формат строки документации */
		QTextCharFormat Doc;
		/** Регулярное выражения для поиска кавычек */
		QRegExp quotes;
		/** Текущий список имен алгоритмов */
		QStringList names;
		/** Предыдущий список имен алгоритмов */
		QStringList prevNames;
		/** Флаг "список имен алгоритмов изменился" */
		bool nameTableModifed;

};

#endif
