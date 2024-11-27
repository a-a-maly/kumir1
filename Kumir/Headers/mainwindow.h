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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include <QToolButton>
#include <QFileInfo>

class QProcess;

class KumirEdit;
class KumirMargin;
class editField;
class ProgramTab;
class Tab;
class proga;
class KumFileDialog;
class FindPanel;

class TextWindow;
class SemaphoreWidget;
class LabelEx;

/** Главное окно приложения */
class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
	Q_PROPERTY(QFont editorsFont READ editorsFont WRITE setEditorsFont)
	Q_PROPERTY(QString currentEditorText READ currentEditorText WRITE setCurrentEditorText)
public:
	/**
	     * Конструктор
	     * @param parent сслыка на объект-владелец
	     * @param fl флаги окна
	     */
	explicit MainWindow(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~MainWindow();
	/**
	     * Возвращает шрифт редакторов
	     * @return шрифт
	     */
	QFont editorsFont();
	/**
	     * Возвращает текст текущего выбранного редактора
	     * @param withIndentation флаг расстановки отступов (точка-пробел)
	     * @return текст редактора
	     */
	QString currentEditorText(bool withIndentation = false);
	QString currentMarginText();
	QString outputText();
	/**
	     * Возвращает режим функционирования ( "edit", "run", "pause", "analiz" )
	     * @return строковое представление режима
	     */
	QString formMode();
	/**
	     * Возвращает вкладку с заданным номером
	     * @param no номер вкладки
	     * @return указатель на абстрактный объект Tab*
	     */
	Tab *tab(int no);

	//QToolButton *btnNew;
	//QToolButton *btnClose;
	QToolButton *sInsert;
	LabelEx *sMessage, *sMessage1;
	QToolButton *sPos;
	FindPanel *findPanel;
	SemaphoreWidget *semaphoreWidget;
	editField *roboEditor;
	QProcess *pdfReader;
	//ReplacePanel *replacePanel;
	QString getPrgText()
	{
		return currentEditorText();
	}
	QFileInfo curFile()
	{
		return curFileInfo;
	}
	void setCurFile(QString fname)
	{
		curFileInfo.setFile(fname);
	}

public slots:
	void finishEditingRobo();
	void startTC();
	void print();
	void setInsertFirstAlgorhitmAvailable(bool);
	void insertFirstAlgorhitm();
	void paste();

	void editOutput();
	void openRecent();

	/** Показать окно настроек */
	void showSettingsDialog();
	/** Обновить язык приложения */
	void updateLanguage();
	/** Обновить шрифты и цвета всех SyntaxHighlighter'ов */
	void updateSHAppearance();
	/**
	     * Устанавливает шрифт для всех редакторов
	     * @param font шрифт
	     */
	void setEditorsFont(const QFont &font);
	void setRecordMacroMode(bool);
	/**
	     * Устанавливает текст текущего редактора
	     * @param txt текст
	     */
	void setCurrentEditorText(const QString &txt);
	/** Файл -> Сохранить */
	bool fileSave(/*int tabNo = -1, */QString fileName = ""/*, QString codec = ""*/);
	/** Файл -> Сохранить как... */
	bool fileSaveAs();
	/** Файл -> Открыть в текущей вкладке */
	void fileOpenInCurrentTab(QString fileName = ""/*, QString codec = "", bool asText = false*/);
	/** Файл -> Открыть в первой вкладке */
	void fileOpenInFirstTab(QString fileName = "");
	/** Файл -> Новая программа */
	void fileNewProgram();
	/** Файл -> Новый текст */
	TextWindow *fileNewText();
	/** Макрос -> Начать запись */
	void macroBegin();
	/** Макрос -> Остановить запись */
	void macroEnd();
	/** Выполнить макрос */
	void macroPlay();
	/** Перейти на следующую вкладку */
	//void switchNextTab();
	/** Перейти на предыдущую вкладку */
	//void switchPreviousTab();
	/**
	     * Установить имя файла из вкладки в заголовок окна
	     * @param no номер вкладки
	     */
	void setCaptionFromTab(int no);
	/**
	     * Связать действия редактирования из меню с редактором во вкладке
	     * @param no номер вкладки
	     */
	void setActionsForTab(int no);
	/**
	     * Установить режим функционирования ( "edit", "run", "pause", "analiz" )
	     * @param m текстовое представление режима
	     */
	void setFormMode(const QString &m);
	/** Вызов окна управления макросами */
	void macrosManage();
	/** Обновить список макросов в меню */
	void updateMacros();
	/**
	     *   Инфо -> Руководство КУМИР
	     */
	//      void showReference();
	/**
	     *   Инфо -> Подсказка по алгоритмам
	     */
	//      void showQuickHelp();
	/** Показать сообщение в статусбаре */
	void showMessage(QString, int time = 0);
	/** Очистить сообщение в статусбаре */
	void clearMessage();
	/** Показать количество шагов в статусбаре */
	void showSteps();
	/** Показать количество шагов в статусбаре во время выполнения */
	void showStepsAtRun();
	/** Перейти в режим редактирования */
	void changeAnalysToEdit();
	/** Найти текст */
	void findText();
	/** Заменить текст */
	void replaceText();
	/** Заменить все далее */
	void replaceAllNext();
	/** Заменить все предыдущие */
	void replaceAllPrev();

	/**
	     *  выполнение зваершилось
	     */
	void runEnds();


	/**
	     *  Пауза
	     */
	void runPause(int str, int module_id);

	/**
	     *  Начать неперерывное выполнение
	     */
	void StartRun();

	/**
	     *  Начать выполнение с выводом на поля
	     */
	void debugRun();

	/**
	     *  По шагам
	     */
	void shagRun();
	/**
	     *  По шагам, внутрь вызова
	     */

	void vnutrVizova();
	/**
	     *  Выполнить начиная с алгоритма
	     */

	void StartFromAlg(QString algName, int Client_id);
	/**
	        *  По шагам, до конца алгоритмв
	     */

	void shagOut();

	/**
	     *  Вывод текста
	     */
	void textMessage(QString text);
	void text2file(QString file);
	void textFromFile(QString file);
	/** Обновить кол-во ошибок в статусбаре */
	void refreshCountErrors(int count = -1);
	void startTesting();
	void runError(int error, int stroka, int module_id, const QString &sourceText);
	void debugMessage(QString message, int stroka, int module_id);
	void doInput(QString format);
	void InputOk(const QStringList &texts);
	void keyInputFinished(uint, bool, bool, bool, bool);
	void doKeyInput(bool);

	void uncheckVelichiny();
	void about();

	void oneStep();
	void InstrumentLoaded(int isp_id);
	void InstrumentUnLoaded(int isp_id);
	QString curMode()
	{
		return m_formMode;
	}
	void closeTab(int);
	void createFriendMenu();

signals:
	void Runsync();
	void Continue();
	void setRunError(QString);
	void FuncDebugMode(int);
	void inputOk(QStringList texts);
	//void updateVariable(QString,QString);  // Для подсвечивания строки в окне величин (имя_функции, имя_переменной)
	//void updateDebugFunction(QString); // Для обновления окна величин. Вход в функцию (имя_функции)
	void RoboSig(bool state);
	void DrawSig(bool state);
	void FileSig(bool state);
	void inputKeyOk(uint, bool, bool, bool, bool);

protected:
	void init();
	void createStatusBar();
	void initIOSaveButtons();
	bool runErrorFlag;
	bool runStopMessagePrinted;
	void addToRecent(const QString &fullPath);
	void addIspToText(const QString &name);
	void removeIspFromText(const QString &name);
	void removeModulesFromTable();
#ifdef Q_OS_WIN32
	bool winEvent(MSG *message, long *result);
#endif
	void dropEvent(QDropEvent *event);
	bool event(QEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	/**
	     * Событие закрытия окна. Используется для сохранения настроек окна
	     * @param event ссылка на событие
	     */
	void closeEvent(QCloseEvent *event);
	/** Блокировка ввода текста во время выполнения */
	void setLockTabsEdit(bool f);
	/** Режим функционирования ( "edit", "run", "pause", "analys" ) */
	QString m_formMode;
	/** Режим функционирования для интерфейса */
	enum {
		modeEdit, modeStep, modeRun, modeError, modePause, modeAnalys
	};
	QList<QAction *> listEditEn, listStepEn, listRunEn, listErrorEn, listPauseEn, listAll, listEditDi, listStepDi, listRunDi, listErrorDi, listPauseDi;
	QList<QWidget *> listEditEnW, listStepEnW, listRunEnW, listErrorEnW, listPauseEnW, listAllW, listEditDiW, listStepDiW, listRunDiW, listErrorDiW, listPauseDiW;
	void selectButtonsForModes();
	/** Можно ли закрыть текущую вкладку? */
	bool isPossibleToClose();
	/** Закрыть все окна исполнителей */
	void closeAllInstruments();
	//void fileOpenInCurrentTab(KumFileDialog*);
	QTimer *timer;
	QTimer *timerRun;
	int countSteps;
	int countErrors;
	QString getFileNameAndEncoding();
	bool showingRecentFilePath;
	void clearRedAndGreenLines();
	QLabel *sRecordMacro;
	QLabel *sMode;
	QLabel *sCount;
	void keyPressEvent(QKeyEvent *event);
	bool isTestingMode;

protected slots:
	void saveUIState();
	void askCursorPosition();
	void showCursorPosition(int row, int col);
	void changeModeInterface(int mode); // Изменить активность экшенов на интерфейсе
	void showOverwriteMode(bool);
	void changeOverwriteMode();
	void stopRunning();
	void arrangeHorizontally();
	void arrangeVertically();
	void showRecentFilePath();
	void hideRecentFilePath();
	void loadRecentList();
	//void deliverFileOpenRequest(const QString &fileName);
	void loadStartEnvironment();
	void editStartEnvironment();
	void showRobot();
	void saveEnvironment();
	void printEnvironment();
	void loadDrawing();
	void clearDrawing();
	void saveDrawing();
	void printDrawing();
	void drawZoomIn();
	void drawZoomOut();
	void drawZoomFit();
	void drawZoomRestore();
	void drawGrid();
	void drawInformation();
	void drawLayoutVertical();
	void drawLayoutHorizontal();
	void showDraw();
	void showPult();
	void showStartEnvironment();
	void showPDF();
	void showPdfCommon(const QUrl &url);
	void showPdfFromWorld();
	void howToUseMacros();
	void setExamMode();

	/**
	     *  Начать тестирование (учительский режим)
	     */

	void openHyperText();
	void insertText(QString);
	bool testLOG_true()
	{
		qDebug() << "TEST LOG true";
		return true;
	}

	bool testLOG_false()
	{
		qDebug() << "TEST LOG true";
		return true;
	}

signals:
	/**
	         * Сообщение в строку статуса
	         * @param msg текст сообщения
	         * @param time время в миллисекундах или 0, если не сбрасывать
	         */
	void sendMessage(const QString &msg, int time);

private:
	uint checkModulesTask();
	bool isChar, waitInput;
	QString inpStr;
	QTextCursor inpCurs;
	int oldBlockId, oldCursPos;
	int currentStep;
	bool stepModeRunning;
	QToolButton *btnSaveOutput;
	QToolButton *btnClearOutput;
	QMenu *friendMenu;
	QFile outTextF;
	QString outTextFile;
	QFile inTextF;
	QString inTextFile;
	QFileInfo curFileInfo;
	QString defaultDir;
};


class LabelEx : public QLabel
{
	Q_OBJECT

public:
	LabelEx(QWidget *parent = NULL) : QLabel(parent) {};
public slots:
	void setText(const QString &txt);

protected:
	void mousePressEvent(QMouseEvent *)
	{
		emit clicked();
	};

signals:
	void clicked();
};

#endif
