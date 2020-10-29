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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QUndoStack>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextBlock>
#include <QPointer>
#include <QMutex>
class QTextEdit;
class QTimer;


class Macro;
struct KumUndoStack;
struct KumRedoStack;

class DummyCursor : public QObject
{
	Q_OBJECT
public:
	DummyCursor(QTextEdit *parent);
	enum { On, Off } state;
	inline bool isVisible()
	{
		return visible;
	}
	void show(uint lineNo, uint indent);
	void hide();
	void moveLeft();
	void moveRight();
	uint line;
	uint indent;
protected:
	QTimer *timer;
	QTextEdit *parent;
	bool visible;
protected slots:
	void blinkCursor();
signals:
	void needRepaint();
};


/** Редактор текстовых файлов */
class TextEdit : public QTextEdit
{
	Q_OBJECT
public:
	/**
	* Конструктор
	* @param parent сслыка на объект-владелец
	 */
	TextEdit(QWidget *parent = 0);
	virtual void paintWidget(QPainter *painter);
	/**
	 * Переопределен в дочернем классе KumirEdit. Здесь не используется
	 */
	virtual void initLineProps(QPointer<QTextDocument> doc);
	void setOverwriteMode(bool);
	/** Возвращает true, если редактор КУМИР-программ */
	virtual bool isKumirEditor();

	/** Т.к. delete сначала выполняет redo, а только потом добавляется в стек undo, нужен этот флаг */
	bool forceEmitCompilation;
	/**
	 * Возвращает доступность отмены действия
	 * @return флаг доступности
	 */
	bool isUndoAvailable();
	/**
	 * Возвращает доступность повторения отмененного действия
	 * @return флаг доступности
	 */
	bool isRedoAvailable();
	/**
	 * Переопределен в дочернем классе KumirEdit. Здесь не используется
	 * @param prevBlock
	 * @param newBlock
	 */
	virtual void initNewBlock(QTextBlock prevBlock, QTextBlock newBlock);
	/** Блокировка редактора другим потоком */
	void lock();
	/** Разблокировка редактора из другого потока */
	void unlock();
	/** Мьютекс, защищающий редактор */
	QMutex editMutex;
	/**
	 * Начало записи макроса
	 * @param m ссылка на макрос
	 */
	virtual void recordMacro(Macro *m);
	/**
	 * Выполняет макрос
	 * @param m ссылка на макрос
	 */
	virtual void playMacro(Macro *m);
	/** Останавливает запись макроса */
	virtual void stopMacro();
	/** Устанавливает флаг изменения */
	void setModified(bool f);
	/** Возвращает изменен ли файл */
	bool isModified();
	/** Очищает стек Undo-Redo */
	void clearUndoStack();
public slots:
	void deleteCurrentLine();
	void clearIndents();
	// +++++ Действия над текстом, которые заносятся в undo стек
	virtual void doCharect(const QString &txt);
	virtual void doEnter();
	virtual void doDelete();
	virtual void doCut();
	virtual void doBackspace();
	virtual void doInsertFirst(const QString &txt);
	virtual void doInsert(const QString &txt);
	virtual void doComment(uint from, uint to);
	virtual void doUncomment(uint from, uint to);
	virtual void doSetText(const QString &txt);
	virtual void doInsertLine(uint no, const QString &txt);
	virtual void doChangeLineText(uint no, const QString &txt);
	virtual void doFastInsertText(const QString &txt, uint removeCount);
	// -----
	virtual void copyWithFormatting();
	virtual void checkForCompilationRequired();
	/** Выполняет расстановку отступов */
	virtual void applyIndentation();
	void setFont(const QFont &);

	/**
	 * Устанавливает текст редактора
	 * @param txt текст
	 */
	virtual void setText(const QString &txt);
	/** Операция "отмена" */
	void undo();
	/** Операция "повтор" */
	void redo();
	/** Операция "вырезать" */
	void cut();
	/**
	 * Устанавливает новый текст и заносит это действие в Undo-стек
	 * @param txt новый текст
	 */
	void changeText(const QString &txt);
protected:
	virtual void save(uint fromLine = 0, uint cmd = 0, QChar charect = QChar('\0'), bool saveToRedoStack = false);
	virtual void revert(bool fromRedo = false);
	void strictSelection();
	KumUndoStack *newUndoStack;
	KumRedoStack *newRedoStack;
	/**
	 * Вставляет текст из буфера (например paste или перетаскивание мышью)
	 * @param source ссылка на источник текста
	 */
	void insertFromMimeData(const QMimeData *source);
	/**
	 * Перехват события нажатия клавиши
	 * @param event ссылка на событие
	 */
	void keyPressEvent(QKeyEvent *event);
	/**
	 * Обработка события нажатия символьной клавиши
	 * @param event ссылка на событие
	 */
	void keyPressCharect(QKeyEvent *event);
	/**
	 * Обработка события нажатия Delete
	 * @param event ссылка на событие
	 */
	void keyPressDelete(QKeyEvent *event);
	/**
	 * Обработка события нажатия BackSpace
	 * @param event ссылка на событие
	 */
	void keyPressBackspace(QKeyEvent *event);
	/**
	 * Обработка события нажатия Enter
	 * @param event ссылка на событие
	 */
	virtual void keyPressEnter(QKeyEvent *event);
	/**
	 * Перехват события перерисовки. Используется для рисования
	 * поля слева (номера строк)
	 * @param event ссылка на событие
	 */
	virtual void paintEvent(QPaintEvent *event);
	virtual void keyPressUp(QKeyEvent *event);
	virtual void keyPressDown(QKeyEvent *event);
	uint getIndentCount(QTextBlock block);
	/** Инициализирует раскладки символов клавиатуры */
	void initKeymaps();
	/** Отображения символов между латинской раскладкой и кириллицей */
	QMap<QString, QString> rusLatMap;


	/** Стек undo/redo */
	QUndoStack undoStack;
	/** Текущий записываемый макрос */
	Macro *macro;
	int i_lineNumberingMarginWidth;
	/** Размер стека Undo-Redo */
	int prevIndexUndoStack;
	/** Удалить текущую строку */

	virtual void connectNotify(const char *signal);
	virtual void disconnectNotify(const char *signal);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	bool lockKeepEmptyLine;
	DummyCursor *dummyCursor ;
	bool teacherMode;
protected slots:
	void emitCursorPosition();
	/** Изменение размера стека Undo-Redo */
	void checkUndoStackModified(int c);
	void checkUndoStackModified();
	virtual void normalizeLines(QPointer<QTextDocument> doc);
	virtual void normalizeLines(QPointer<QTextDocument> doc, QList<QTextBlock>);
	virtual void keepEmptyLine();

signals:
	/**
	* Отправка сообщения в строку статуса
	* @param msg текст сообщения
	* @param time время отображения (в миллисекундах) или 0, если постоянно
	 */
	void sendMessage(const QString &msg, int time);
	/** Очистка сообщения из строки статуса */
	void clearMessage();
	/** Нажатие Option + KeyLeft */
	void altLeftPressed();
	/** Нажатие Option + KeyRight */
	void altRightPressed();
	void cursorPositionChanged(int row, int col);
	void overwriteModeChanged(bool);
	void linesInserted(int from, int count);
	void linesRemoved(int from, int count);
	void yellowIndicator();
};

#endif
