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

#ifndef KUMIREDIT_H
#define KUMIREDIT_H

#include "lineprop.h"
#include "textedit.h"
#include "tools.h"

class QListWidgetItem;

class SyntaxHighlighter;
class ToolTip;
class KumirMargin;

/** Текстовый редактор для КУМИР-программ */
class KumirEdit : public TextEdit
{
	Q_OBJECT;
	Q_PROPERTY(qreal indentSize READ indentSize WRITE setIndentSize);
	Q_PROPERTY(int runLine READ runLine WRITE setRunLine);
	Q_PROPERTY(bool inSettingsEditor READ isInSettingsEditor WRITE setInSettingsEditor DESIGNABLE true);
public:
	/**
	 * Конструктор
	 * @param parent сслыка на объект-владелец
	 */
	KumirEdit(QWidget *parent = 0);
	void paintWidget(QPainter *painter, const QRect &region);
	void paintCursor(QPainter *painter, const QRect &region);
	void setMargin(KumirMargin *m);
	virtual bool isKumirEditor();
	bool isJumpNextErrorAvailable();
	bool isJumpPrevErrorAvailable();
	inline void setInSettingsEditor(bool v)
	{
		m_settingsEditor = v;
	}
	inline bool isInSettingsEditor()
	{
		return m_settingsEditor;
	}
	QString toPlainText();
	bool isLineHidden(int lineNo) const;
	void setPostScriptumText(const QString &txt);

	inline SyntaxHighlighter *syntaxHighlighter()
	{
		return sh;
	}

	QList<ProgaText> normText(QPointer<QTextDocument> doc);
	QList<ProgaText> completeNormalizedText();

	/**
	 * Возвращает размер одного отступа в пикселях
	 * @return размер отступа
	 */
	qreal indentSize();
	/**
	 * Устанавливает размер одного отступа
	 * @param size размер в пикселях
	 */
	void setIndentSize(qreal size);
	/**
	 * Возвращает флаг "точка останова"
	 * @param para номер строки текста (нумерация с 0)
	 * @return true, если точка останова или false в противном случае
	 */
	bool isBreakpoint(int para);
	/**
	 * Инициализирует нагрузку новых строк текста. Вызывается при изменении количества строк
	 */
	void initLineProps(QPointer<QTextDocument> doc);
	void initLineProps(QPointer<QTextDocument> doc, const QList<int> &protectedLines, const QList<int> &hiddenLines);
	/**
	 * Инициализирует нагрузку новой, вставленной по нажатию Enter, строки и устанавливает
	 * для нее размер отступа. Используется для расстановки отступов до выполнения компиляции
	 * @param prevBlock предыдущий текстовый блок
	 * @param newBlock новый текстовый блок
	 */
	virtual void initNewBlock(QTextBlock prevBlock, QTextBlock newBlock);
	/**
	 * Возвращает номер выполняемой (подсвечиваемой) строки или -1, если ее нет
	 * @return номер строки
	 */
	int runLine();
	int errorLine();
	void setErrorLine(int lineNo);
	/**
	 * Возвращает указатель на нагрузку определенной строки
	 * @param lineNo номер строки
	 * @return указатель
	 */
	LineProp *prop(int lineNo);
	/**
	 * Устанавливает номер вкладки редактора
	 * @param tabNo номер вкладки
	 */
	void setTabNo(int tabNo);
	/** Номер вкадки, к которой относится редактор */
	int tabNo;
	/**
	 * Возвращает список списков указателей на нормализованные строки,
	 * сгруппированных по исполнителям.
	 * @return список списков указателей на ProgaText
	 */
	QList< QList< ProgaText > > normalizedText();
	void setNormalizedText(const QList<int> &modulesId);
	void setErrorsAndRanks(const QList<ProgaText> &pts);
	void setErrorsForModules(const QList<int> &ids);
	void clearErrors();
	static LineProp *linePropByLineNumber(QTextDocument *doc, int no);
public slots:

	QString getSelectedPlainText() const;
	QString getSelectedHtml() const;
	void insertPultLine(const QString &line);
	void forcedRepaint();
	void undo();
	void redo();
	void copy();
	void copyWithFormatting();
//      void gotoPrevError();
//      void gotoNextError();
	void insertLine(int lineNo, const QString &text);
	void commentLine(int lineNo);
	void uncommentLine(int lineNo, int uncommentCount);


	/**
	 * Устанавливает ошибку для элемента нормализованной канонической строки в редакторе
	 * @param lineNo номер строки
	 * @param termNo номер канонической строки в редакторской строке
	 * @param error код ошибки
	 * @param position позиция ошибки ( в ксимволах )
	 */
	void setError(int lineNo, int termNo, int error, ErrorPosition position);
	/** Выполняет расстановку отступов */
	virtual void applyIndentation();
	virtual void applyIndentation(QTextBlock from);
	/** Выполняет переподсветку синтаксиса */
	void rehighlight();
	/**
	 * Устанавливает точку останова
	 * @param para номер строки
	 * @param value значение флага
	 */
	void setBreakpoint(int para, bool value);
	/**
	 * Устанавливает текущую подсвечиваемую строку
	 * @param line номер строки или -1, если ничего не подсвечивать
	 */
	void setRunLine(int line);
	/** Переинициализирует настроки языка */
	void updateLanguage();
	/**
	 * Возвращает доступность операции разкомментирования
	 * @return значение флага
	 */
	bool isUncommentAvailable();
	/** Переинициализирует настроки цветов и шрифтов SyntaxHighlighter'а */
	void updateSHAppearance();
	/** Устанавливает список имен SyntaxHighlighter'а */
	void setSHAlgNames(const QStringList &);
	/** Устанавливает текст в редакторе */
	void setText(const QString &txt);
	/** Комментирует выделение или текущую строку */
	void comment();
	/** Разкомментирует выделение или текущую строку */
	void uncomment();
	/** Отображает всплывающую подсказку */
	void showHelpMenu();
	/**
	 * Выполняет макрос
	 * @param m ссылка на макрос
	 */
	void playMacro(Macro *m);
	/** Проверяет появление областей прогрутку. Используется для выравнивания текста с полем */
	void checkScrollBars();

protected slots:
	void unlockEditor();
	void unprotectLines();
	void protectLines();
	void hideLine();
	void hideProlog();
	void hideEpilog();
	void unhideLine();
	void unhideProlog();
	void unhideEpilog();
	void unhideBlock();

	void updateDummyCursor();
	void toggleRobotAllowToInsertText();
	// +++++ Действия над текстом, которые заносятся в undo стек
	void doComment(int from, int to);
	void doUncomment(int from, int to);
	void doInsertFirst(const QString &txt);
	void doInsertLine(int no, const QString &txt);
	void doChangeLineText(int no, const QString &txt);
	void doFastInsertText(const QString &txt, int removeCount);
	void doEnter();
	// -----
	/** Изменяет флаг "точка останова" для текущей строки */
	void toggleBreakpoint();
	/** Проверяет необходимость компиляции по правилу "изменен текст + переход на новую строку" */
	void checkTextChanges();
	/** Проверяет доступность дейтвия "разкомментировать" для текущего выделения или позиции курсора */
	void checkUncommentAvailability();
//      void checkJumpErrorsAvailability();
	/** Подствечивает парную скобку */
	void highlightBrackets();
	/** Проверяет изменение текста в редакторе. Используется для фильтрации стандартного
	 * сигнала textChanged, который реагирует не только на изменение текста, но и
	 * на изменение форматирования */
	void verifyTextChanged();
	/** Проверяет переход курсора на другую строку */
	void checkLineChanged();
	/** Выбор текста из всплывающей подсказки */
	void insertTxtFromPopup(QListWidgetItem *, const QString &reason);
	/** Выполняет нормализацию строк. */
	void normalizeLines(QPointer<QTextDocument> doc);
	void normalizeLines(QPointer<QTextDocument> doc, QList<QTextBlock> blocks);
	void checkForCompilationRequired();
	virtual void keepEmptyLine();

protected:
	bool m_blockKeyboardFlag;
	int m_blockKeyboardCounter;
	void trim(int blockNumber);
	KumirMargin *margin;
	ToolTip *toolTipWindow;
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
	 * Обработка события нажатия Enter
	 * @param event ссылка на событие
	 */
	void keyPressEnter(QKeyEvent *event);
	/**
	 * Перехват события перерисовки. Используется для рисования
	 * поля слева (номера строк + точки останова), областей нередактируемых
	 * и скрытых строк а также выделения текущей выполняемой строки.
	 * @param event ссылка на событие
	 */
	void paintEvent(QPaintEvent *event);
	/**
	 * Перехват события изменения размера. Вызывает проверку областей прокрутки.
	 * @param event ссылка на событие
	 */
	void resizeEvent(QResizeEvent *event);
	/**
	 * Перехват события создания контексного меню
	 * @param event ссылка на событие
	 */
	void contextMenuEvent(QContextMenuEvent *event);
	void contextMenuEvent(const QPoint &position, const QPoint &globalPosition, bool shiftPressed);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	bool canInsertFromMimeData(const QMimeData *source) const;
	/**
	 * Перехват события прокрутки. Посылает сигнал для синхронизации с полями.
	 * @param dx прокрутка по горизонтали
	 * @param dy прокрутка по вертикали
	 */
	void scrollContentsBy(int dx, int dy);
	/** Размер одного отступа */
	qreal m_indentSize;
	/** Ссылка на SyntaxHighlighter */
	SyntaxHighlighter *sh;
	/** Номер текущей выполняемой строки */
	int m_runLine;
	/** Номер текущей ошибочной строки */
	int m_errorLine;
	/** Номер предыдущей строки, где находился курсор.
	* Используется для определения необходимости компиляции */
	int prevCursorBlock;
	/** Предыдущее количество строк.
	 * Используется для определения необходимости компиляции */
	int prevLinesCount;
	/** Флаг блокировки редактора на время расстановки отступов */
	bool lockTextChangedFlag;
	/** Номер строки, в которой вызван запрос на установление точки
	* останова из контексного меню */
	int contextMenuBreakLine;
	/** Мьютек, защищающий флаг lockTextChangedFlag */
	QMutex indentationLocker;
	/** Флаг изменения текста.
	 * Используется для определения необходимости компиляции */
	bool textChanged;
	/** Флаг перехода на другую строку.
	 * Используется для определения необходимости компиляции */
	bool lineChanged;
	/** Предыдущий текст редактора.
	 * Используется для определения необходимости компиляции */
	QString previousText;

	QPoint hyperStart, hyperEnd;
	bool underlineHyper;

	QList<QLine> lines;
	QList<QRect> rects;

	int lastCompileTime;
	bool shiftPressed;
	int normLinesTotalCount;
	bool needCompilation;
	bool messageEmited;
	bool scrollRepaintLock;
	bool allowRobotToInsertLines;
	bool m_settingsEditor;

	QPointer<QTextDocument> hiddenProlog;
	QPointer<QTextDocument> hiddenEpilog;
	QPointer<QTextDocument> postScriptum;

	void createTeacherActions(QMenu *menu);
	void createHideActions(QMenu *menu, bool hasSelection);
	void createUnhideActions(QMenu *menu, bool hasSelection);


signals:
	/**
	 * Отправка сообщения в строку статуса
	 * @param msg текст сообщения
	 * @param time время отображения (в миллисекундах) или 0, если постоянно
	 */
	void sendMessage(const QString &msg, int time);
	/** Очистка сообщения из строки статуса */
	void clearMessage();
	/**
	 * Запрос на компиляцию
	 * @param tabNo номер вкладки
	 */
	void requestCompilation(int tabNo);
	/**
	 * Доступна операция "закомментировать"
	 * @param value флаг доступности
	 */
	void commentAvailable(bool value);
	/**
	 * Доступна операция "разкомментировать"
	 * @param value флаг доступности
	 */
	void uncommentAvailable(bool value);
	/**
	 * Произошла прокрутка текста
	 * @param dx величина по горизонтали
	 * @param dy величина по вертикали
	 */
	void contentsMoving(int dx, int dy);
	/**
	 * Double-click на имени открываемого файла
	 * @param fileName имя файла
	*/
	void fileOpenRequest(const QString &fileName);
	void fileOpenHereRequest(const QString &fileName);
	void ensureMarginCursorVisible(int lineNo);
	void jumpNextErrorAvailable(bool);
	void jumpPrevErrorAvailable(bool);
	void clearError(int lineNo);
};

#endif
