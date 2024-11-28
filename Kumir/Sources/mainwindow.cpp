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

#include "mainwindow.h"
#include "editField.h"
#include "settingsdialog.h"
#include "application.h"
#include "kumiredit.h"
#include "kumirmargin.h"
#include "kumfiledialog.h"
#include "modulesregistration.h"
#include "programtab.h"
#include "texttab.h"
#include "macro.h"
#include "newmacrodialog.h"
#include "macrosmanagementdialog.h"
#include "assistant.h"
#include "gotolinedialog.h"

#include "network.h"
#include "messagesprovider.h"
#include "compiler.h"
#include "run.h"
#include "httpdaemon.h"
#include "debugdialog.h"
#include "kumtabwidget.h"
#include "findpanel.h"
#include "aboutdialog.h"
#include "textwindow.h"
#include "semaphorewidget.h"
#include "kumsinglemodule.h"
#include "kumrobot.h"
#include "kumdraw.h"

#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTimer>
#include <QProcess>
#include <QMimeData>
#include <QDesktopWidget>
#include <QDesktopServices>

void disableTearOff(QWidget *node);
bool mustDisableTearOff();
void fixQtBug11693(QWidget *node);

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags fl) :
	QMainWindow(parent, fl),
	Ui::MainWindow()
{
	runErrorFlag = false;
#if QT_VERSION >= 0x050000
	defaultDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
	defaultDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
	setupUi(this);
	init();
}

void MainWindow::init()
{
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveUIState()));
#ifdef Q_OS_MAC
	actionDebug->setShortcut(QKeySequence("F5"));
	actionStart->setShortcut(QKeySequence("Shift+F5"));
	actionStep->setShortcut(QKeySequence("F6"));
	actionStepInto->setShortcut(QKeySequence("F7"));
	actionStepOut->setShortcut(QKeySequence("Shift+F7"));
	actionMacroEnd->setShortcut(QKeySequence("Meta+Esc"));
#endif
	setAcceptDrops(true);
	showingRecentFilePath = false;
	loadRecentList();
	pdfReader = NULL;
	setWindowTitle(tr("New program") + " - " + tr("Kumir"));
//	restoreGeometry(app()->settings->value("MainWindow/Geometry","").toByteArray());
	QRect r = app()->settings->value("MainWindow/Rect", QRect(0, 0, 800, 600)).toRect();
	resize(r.size());
	move(r.topLeft());
	QList<QVariant> sizesV = app()->settings->value("MainWindow/SplitterSizes", QList<QVariant>() << QVariant(600 - 100 - 24) << QVariant(100)).toList();
	QList<int> sizes;
	for (int i = 0; i < sizesV.count(); i++) {
		sizes << sizesV[i].toInt();
	}
	splitter_2->setSizes(sizes);
//	QByteArray splitter2_settings = app()->settings->value("MainWindow/Splitter2","").toByteArray();
//	if ( !splitter2_settings.isEmpty() ) {
//		splitter_2->restoreState(splitter2_settings);
//	}
//	else // если Кумир запускается первый раз
//	{
//		splitter_2->setSizes(QList<int>() << 336 << 100);
//	}


//	fileToolBar->setVisible(app()->settings->value("MainWindow/ToolBar/FileToolBarVisible",true).toBool());
//	editToolBar->setVisible(app()->settings->value("MainWindow/ToolBar/EditToolBarVisible",true).toBool());
//	runToolBar->setVisible(app()->settings->value("MainWindow/ToolBar/RunToolBarVisible",true).toBool());
//	modulesToolBar->setVisible(app()->settings->value("MainWindow/ToolBar/ModulesToolBarVisible",true).toBool());

	connect(actionInsert_first_algorhitm, SIGNAL(triggered()), this, SLOT(insertFirstAlgorhitm())) ;

	QString defaultFont;
#ifdef Q_OS_X11
	defaultFont = "DejaVu Sans Mono";
#endif
#ifdef Q_OS_WIN32
	if (QSysInfo::WindowsVersion >= 0x0080) {
		defaultFont = "Consolas";
	} else {
		defaultFont = "Courier";
	}
#endif

	QString fntF = app()->settings->value("Appearance/Font", defaultFont).toString();
	int fntS = app()->settings->value("Appearance/FontSize", 12).toInt();
	bool boldFont = app()->settings->value("Appearance/FontBold", false).toBool();
	if (!fntF.isEmpty() && fntS > 0) {
		QFont edFont(fntF, fntS);
		edFont.setBold(boldFont);
		textEdit3->setFont(edFont);
	}
	m_formMode = "edit";
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timerRun = new QTimer(this);
	timerRun->setSingleShot(false);
	countSteps = 0;
	stepModeRunning = false;

	roboEditor = new editField();

	tabWidget->removeTab(0);
	tabWidget->setUsesScrollButtons(false);
	tabWidget->hideTabBar();

	if (app()->isExamMode()) {
		setExamMode();    //Режим ЕГЭ
	}

	createStatusBar();
	initIOSaveButtons();

	selectButtonsForModes();
	outTextFile = "";
	connect(actionPaste, SIGNAL(triggered()), this, SLOT(paste()));

	connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
	connect(textEdit3, SIGNAL(sendMessage(const QString &, int)), this, SLOT(showMessage(QString, int)));
	connect(app()->run, SIGNAL(sendMessage(const QString &, int)), this, SLOT(showMessage(QString, int)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCaptionFromTab(int)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setActionsForTab(int)));

	connect(actionSettings, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(fileSave()));
	connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	connect(actionOpenInCurrentTab, SIGNAL(triggered()), this, SLOT(fileOpenInCurrentTab()));

	connect(actionNew, SIGNAL(triggered()), this, SLOT(fileNewProgram()));
	connect(actionNewText, SIGNAL(triggered()), this, SLOT(fileNewText()));

	connect(actionMacroBegin, SIGNAL(triggered()), this, SLOT(macroBegin()));
	connect(actionMacroEnd, SIGNAL(triggered()), this, SLOT(macroEnd()));
	connect(actionEditMacros, SIGNAL(triggered()), this, SLOT(macrosManage()));

	connect(actionGoToLine, SIGNAL(triggered()), this, SLOT(askCursorPosition()));



	connect(actionStart, SIGNAL(triggered()), this, SLOT(StartRun()));
	connect(actionStartTesting, SIGNAL(triggered()), this, SLOT(startTesting()));
	connect(actionStep, SIGNAL(triggered()), this, SLOT(shagRun()));
	connect(actionDebug, SIGNAL(triggered()), this, SLOT(debugRun()));
	connect(actionStepInto, SIGNAL(triggered()), this, SLOT(vnutrVizova()));
	connect(actionStepOut, SIGNAL(triggered()), this, SLOT(shagOut()));
	connect(actionStepOut, SIGNAL(triggered()), app()->run, SLOT(DoConcaAlg()));

	connect(app()->run, SIGNAL(textMessage(QString)), this, SLOT(textMessage(QString)));
	connect(app()->run, SIGNAL(finished()), this, SLOT(runEnds()));

	connect(this, SIGNAL(Runsync()), app()->run, SLOT(SyncMessage()));
	connect(this, SIGNAL(Continue()), app()->run, SLOT(Continue()));
	connect(this, SIGNAL(FuncDebugMode(int)), app()->run, SLOT(setFuncDebugMode(int)));
	connect(actionStop, SIGNAL(triggered()), this, SLOT(stopRunning()));

	connect(app()->run, SIGNAL(runError(int, int, int, QString)), this, SLOT(runError(int, int, int, QString)));
	connect(app()->run, SIGNAL(debugMessage(QString, int, int)), this, SLOT(debugMessage(QString, int, int)), Qt::BlockingQueuedConnection);
	connect(app()->run, SIGNAL(waitForInput(QString)), this, SLOT(doInput(QString)));
	connect(app()->run, SIGNAL(waitForKey(bool)), textEdit3, SLOT(scanKey(bool)));
	connect(app()->run, SIGNAL(setInFile(QString)), this, SLOT(textFromFile(QString)));
	connect(app()->run, SIGNAL(setOutFile(QString)), this, SLOT(text2file(QString)));
	connect(this, SIGNAL(setRunError(QString)), app()->run, SLOT(ispError(QString)));

	connect(textEdit3, SIGNAL(inputFinished(QStringList)), this, SLOT(InputOk(const QStringList &)));
	connect(textEdit3, SIGNAL(scanKeyFinished(uint, bool, bool, bool, bool)), this, SLOT(keyInputFinished(uint, bool, bool, bool, bool)));
	connect(app()->run, SIGNAL(pause(int, int)), this, SLOT(runPause(int, int)));



	textEdit3->setUndoRedoEnabled(false);
	connect(this, SIGNAL(inputOk(QStringList)), app()->run, SLOT(inputReady(QStringList)));
	connect(this, SIGNAL(inputKeyOk(uint, bool, bool, bool, bool)), app()->run, SLOT(inputKeyReady(uint, bool, bool, bool, bool)));
	isChar = false;
	waitInput = false;
	inpStr = "";


	connect(app()->compiler, SIGNAL(changeCountErrors(int)), this, SLOT(refreshCountErrors(int)));
	connect(app()->compiler, SIGNAL(kumInstrumentLoaded(int)), this, SLOT(InstrumentLoaded(int)));
	connect(app()->compiler, SIGNAL(kumInstrumentUnLoaded(int)), this, SLOT(InstrumentUnLoaded(int)));

	connect(this, SIGNAL(sendMessage(QString, int)), this, SLOT(showMessage(QString, int)));
	connect(timer, SIGNAL(timeout()), this, SLOT(clearMessage()));
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	connect(tabWidget, SIGNAL(closeTab1(int)), this, SLOT(closeTab(int)));

	connect(textEdit3, SIGNAL(copyAvailable(bool)), textEdit3->saveSelected, SLOT(setEnabled(bool)));
	textEdit3->saveAll->setEnabled(true);
	textEdit3->saveLast->setEnabled(true);
	textEdit3->saveSelected->setEnabled(false);

	connect(app()->run, SIGNAL(Step()), this, SLOT(showSteps()));
	connect(timerRun, SIGNAL(timeout()), this, SLOT(showStepsAtRun()));

	connect(actionFind, SIGNAL(triggered()), findPanel, SLOT(show()));
	connect(findPanel, SIGNAL(findTextPressed()), this, SLOT(findText()));
	connect(findPanel, SIGNAL(replacePressed()), this, SLOT(replaceText()));
	connect(findPanel, SIGNAL(replaceNextAllPressed()), this, SLOT(replaceAllNext()));
	connect(findPanel, SIGNAL(replacePrevAllPressed()), this, SLOT(replaceAllPrev()));
	connect(findPanel, SIGNAL(keyPressed()), this, SLOT(clearMessage()));

	connect(actionHorizontally, SIGNAL(triggered()), this, SLOT(arrangeHorizontally()));
	connect(actionVertically, SIGNAL(triggered()), this, SLOT(arrangeVertically()));

	connect(menuOpenRecent, SIGNAL(aboutToShow()), this, SLOT(loadRecentList()));
	connect(menuOpenRecent, SIGNAL(aboutToHide()), this, SLOT(hideRecentFilePath()));

	connect(actionLoadEnvironment, SIGNAL(triggered()), this, SLOT(loadStartEnvironment()));
	connect(actionEditStartEnvironment, SIGNAL(triggered()), this, SLOT(editStartEnvironment()));
	connect(actionShowRobot, SIGNAL(triggered()), this, SLOT(showRobot()));
	connect(actionControlPanel, SIGNAL(triggered()), this, SLOT(showPult()));
	connect(actionSaveEnvironment, SIGNAL(triggered()), this, SLOT(saveEnvironment()));
	connect(actionPrintEnvironment, SIGNAL(triggered()), this, SLOT(printEnvironment()));

	connect(actionLoadDrawing, SIGNAL(triggered()), this, SLOT(loadDrawing()));
	connect(actionSave_drawing, SIGNAL(triggered()), this, SLOT(saveDrawing()));
	connect(actionClear_drawing, SIGNAL(triggered()), this, SLOT(clearDrawing()));
	connect(actionPrint_drawing, SIGNAL(triggered()), this, SLOT(printDrawing()));
	connect(actionDrawZoomIn, SIGNAL(triggered()), this, SLOT(drawZoomIn()));
	connect(actionDrawZoomOut, SIGNAL(triggered()), this, SLOT(drawZoomOut()));
	connect(actionDrawFitAll, SIGNAL(triggered()), this, SLOT(drawZoomFit()));
	connect(actionDrawZoomIn, SIGNAL(triggered()), this, SLOT(drawZoomRestore()));
	connect(actionDrawGrid, SIGNAL(triggered()), this, SLOT(drawGrid()));
	connect(actionDrawInformation, SIGNAL(triggered()), this, SLOT(drawInformation()));
	connect(actionDrawLayoutHorizontal, SIGNAL(triggered()), this, SLOT(drawLayoutHorizontal()));
	connect(actionDrawLayoutVertical, SIGNAL(triggered()), this, SLOT(drawLayoutVertical()));

	connect(actionShowDraw, SIGNAL(triggered()), this, SLOT(showDraw()));
	connect(actionShowStartEnvironment, SIGNAL(triggered()), this, SLOT(showStartEnvironment()));

	connect(actionPDF, SIGNAL(triggered()), this, SLOT(showPDF()));

	connect(roboEditor, SIGNAL(editFinished()), this, SLOT(finishEditingRobo()));
	connect(actionHowToUseMacros, SIGNAL(triggered()), this, SLOT(howToUseMacros()));

	connect(actionHT, SIGNAL(triggered()), this, SLOT(openHyperText()));

	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->Modules.module(i)->Name() == "Robot") {
			connect(actionRobot, SIGNAL(triggered()), app()->compiler->Modules.module(i)->instrument(), SLOT(showHideWindow()));
		}
		if (app()->compiler->Modules.module(i)->Name() == "Draw") {
			connect(actionDraw, SIGNAL(triggered()), app()->compiler->Modules.module(i)->instrument(), SLOT(showHideWindow()));
		}
	}

	if (app()->isTaskControlLoaded()) {
		actionTC->setEnabled(true);
		connect(actionTC, SIGNAL(triggered()), this, SLOT(startTC()));
	}
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(0);
	emit changeModeInterface(modeEdit);

	isTestingMode = false;
	friendMenu = menubar->addMenu(tr("Worlds"));
	friendMenu->menuAction()->setVisible(false);
	if (!app()->isExamMode()) {
		friendMenu->menuAction()->setVisible(true);
	}
	friendMenu->setTearOffEnabled(true);

	if (mustDisableTearOff()) {
		disableTearOff(menubar);
	}

#if QT_VERSION < 0x040800
	fixQtBug11693(menubar);
#endif

	messageArea->setVisible(QCoreApplication::arguments().contains("-t") || QCoreApplication::arguments().contains("--teacher"));

}

bool mustDisableTearOff()
{
#ifdef Q_OS_LINUX
#if QT_VERSION < 0x040603
	return !QFile::exists("/etc/SuSE-release");
#else
	return false;
#endif
#else
	return false;
#endif
}



void disableTearOff(QWidget *node)
{
	QMenu *menu = qobject_cast<QMenu *>(node);
	if (menu) {
		menu->setTearOffEnabled(false);
	}
	for (int i = 0; i < node->children().size(); i++) {
		QWidget *child = qobject_cast<QWidget *>(node->children().at(i));
		if (child) {
			disableTearOff(child);
		}
	}
}

void fixQtBug11693(QWidget *node)
{
	QMenu *menu = qobject_cast<QMenu *>(node);
	if (menu) {
		QString menuTitle = menu->title();
		menuTitle.remove("&");
		menu->setWindowTitle(menuTitle);
	}
	for (int i = 0; i < node->children().size(); i++) {
		QWidget *child = qobject_cast<QWidget *>(node->children().at(i));
		if (child) {
			fixQtBug11693(child);
		}
	}
}

void MainWindow::createStatusBar()
{
	QFrame *outputBtnsFrame = new QFrame(this);
	btnSaveOutput = new QToolButton(outputBtnsFrame);
	outputBtnsFrame->setLayout(new QHBoxLayout(outputBtnsFrame));
	btnSaveOutput->setObjectName(QString::fromUtf8("btnSaveOutput"));
	btnSaveOutput->setIcon(QIcon(QString::fromUtf8(":/icons/document-save.png")));
	btnSaveOutput->setPopupMode(QToolButton::InstantPopup);
	btnSaveOutput->setAutoRaise(true);
	btnSaveOutput->setArrowType(Qt::NoArrow);
	btnSaveOutput->setIconSize(QSize(16, 16));
	outputBtnsFrame->layout()->addWidget(btnSaveOutput);
	btnClearOutput = new QToolButton(outputBtnsFrame);
	btnClearOutput->setAutoRaise(true);
	btnClearOutput->setObjectName(QString::fromUtf8("btnClearOutput"));
	btnClearOutput->setIcon(QIcon(QString::fromUtf8(":/icons/output-close.png")));
	btnSaveOutput->setToolTip(trUtf8("MainWindow", "Save output"));
	btnSaveOutput->setText(trUtf8("MainWindow", "Save output"));
	btnClearOutput->setToolTip(trUtf8("MainWindow", "Clear output"));
	btnClearOutput->setText(trUtf8("MainWindow", "Clear output"));
	btnClearOutput->setIconSize(QSize(16, 16));
	outputBtnsFrame->layout()->addWidget(btnClearOutput);
	outputBtnsFrame->layout()->setContentsMargins(8, 0, 8, 0);
	statusbar->addWidget(outputBtnsFrame);
	connect(btnClearOutput, SIGNAL(clicked()), textEdit3, SLOT(clear()));
	QWidget *modeContainer = new QWidget(this);
	QHBoxLayout *l = new QHBoxLayout(modeContainer);
	l->setContentsMargins(0, 0, 0, 0);
	sRecordMacro = new QLabel(modeContainer);
	sRecordMacro->setFixedSize(16, 16);
	sMode = new QLabel(modeContainer);
	l->addWidget(sRecordMacro);
	l->addWidget(sMode);
	semaphoreWidget = new SemaphoreWidget(modeContainer);
	l->addWidget(semaphoreWidget);
	QFont bigFont = sMode->font();
	bigFont.setPointSize(bigFont.pointSize() + 2);
	sMode->setFont(bigFont);
	sMode->setText(tr("Edit mode"));
	sMode->setMinimumWidth(110);
	sMode->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	sCount = new QLabel(this);
	sCount->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	sCount->setText(tr("No Errors"));
	sCount->setMinimumWidth(90);
	sCount->setFont(bigFont);

	sInsert = new QToolButton(this);
	sInsert->setText(tr("INS"));
	sInsert->setAutoRaise(true);
	sInsert->setFont(bigFont);
	connect(sInsert, SIGNAL(clicked()), this, SLOT(changeOverwriteMode()));
	sInsert->setMinimumWidth(50);
	//  sInsert->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	sPos = new QToolButton(this);
	sPos->setText(tr("Line 0, Col: 0"));
	sPos->setAutoRaise(true);
	sPos->setFont(bigFont);
	connect(sPos, SIGNAL(clicked()), this, SLOT(askCursorPosition()));
	sMessage = new LabelEx(this);
	sMessage->setMinimumWidth(100);
	sMessage->setFont(bigFont);
	sMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	connect(sMessage, SIGNAL(clicked()), this, SLOT(clearMessage()));
	sMessage1 = new LabelEx(this);
	sMessage1->setFont(bigFont);
	sMessage1->setVisible(false);
	statusbar->addWidget(modeContainer);
	statusbar->addWidget(sCount);
	statusbar->addPermanentWidget(sMessage, 1);
	statusbar->addPermanentWidget(sPos);
	statusbar->addPermanentWidget(sInsert);
	statusbar->setFont(bigFont);

	findPanel = new FindPanel(tr("Find"));
	addToolBar(findPanel);
	findPanel->move(0, toolBar->y());
	findPanel->setMovable(false);
	findPanel->setVisible(false);
	removeToolBar(toolBar);
	//replacePanel = new ReplacePanel(tr("Replace"), this);
	//replacePanel->setVisible(false);
	//addToolBar(Qt::BottomToolBarArea, replacePanel);
}

void MainWindow::initIOSaveButtons()
{
	QMenu *saveOutputMenu = new QMenu(tr("Save output"), btnSaveOutput);
	saveOutputMenu->setTearOffEnabled(true);
	saveOutputMenu->setIcon(btnSaveOutput->icon());
	QString saveAllText = tr("Save all output");
	QString saveLastText = tr("Save only last output");
	QString saveSelectedText = tr("Save selected text");
	textEdit3->saveAll = saveOutputMenu->addAction(saveAllText, textEdit3, SLOT(saveAllOutput()));
	textEdit3->saveLast = saveOutputMenu->addAction(saveLastText, textEdit3, SLOT(saveLastOutput()));
	textEdit3->saveSelected = saveOutputMenu->addAction(saveSelectedText, textEdit3, SLOT(saveSelectedOutput()));
	btnSaveOutput->setMenu(saveOutputMenu);

	menuTools->insertMenu(actionSettings, saveOutputMenu);
	QAction *qa = new QAction(btnClearOutput->icon(), btnClearOutput->text(), NULL);
	menuTools->insertAction(actionSettings, qa);
	connect(qa, SIGNAL(triggered()), textEdit3, SLOT(clear()));
	listAll.append(qa);
	listRunDi.append(qa);
	listStepDi.append(qa);
	listPauseDi.append(qa);
}
void MainWindow::setExamMode()
{
	actionModules->setVisible(false);
	actionModulesRegistration->setVisible(false);
	actionShowDraw->setVisible(false);
	actionShowStartEnvironment->setVisible(false);
	actionShowRobot->setVisible(false);
	actionEditStartEnvironment->setVisible(false);
	actionControlPanel->setVisible(false);
	actionCatchPultCommands->setVisible(false);
	menuR_obot->menuAction()->setVisible(false);
	menu_Draw->menuAction()->setVisible(false);
	menuModulesManuals->menuAction()->setVisible(false);


}

void MainWindow::selectButtonsForModes()
{
	listAll.append(actionNew);
	listAll.append(actionNewText);
	listAll.append(actionOpenInCurrentTab);
	listAll.append(actionOpenInNewTab);
	listAll.append(actionClose);
	listAll.append(actionUndo);
	listAll.append(actionRedo);
	listAll.append(actionDelete_a_line_under_cursor);
	listAll.append(actionCut);
	listAll.append(actionCopy);
	listAll.append(actionPaste);
	listAll.append(actionComment);
	listAll.append(actionUncomment);
	listAll.append(actionFind);
	listAll.append(actionReplace);
	listAll.append(actionMacroBegin);
	listAll.append(actionEditMacros);
	listAll.append(actionStart);
	listAll.append(actionDebug);
	listAll.append(actionStep);
	listAll.append(actionStepInto);
	listAll.append(actionStepOut);
	listAll.append(actionStop);
	//  listAll.append(actionConvertToC);
	listAll.append(actionSettings);
	listAll << actionSave << actionSaveAs << actionSaveAll << actionPrint << actionHelpTip;
	listAll << actionModulesRegistration;

	listAll << actionLoadEnvironment << actionEditStartEnvironment << actionShowRobot << actionControlPanel << actionShowStartEnvironment << actionSaveEnvironment << actionPrintEnvironment << actionLoadDrawing << actionShowDraw;

	listEditDi.append(actionCut);
	listEditDi.append(actionCopy);
	listEditDi.append(actionStepOut);
	listEditDi.append(actionStop);

	listRunDi.append(actionDelete_a_line_under_cursor);
	listRunDi.append(actionNew);
	listRunDi.append(actionNewText);
	listRunDi.append(actionOpenInCurrentTab);
	listRunDi.append(actionOpenInNewTab);
	listRunDi.append(actionClose);
	listRunDi.append(actionUndo);
	listRunDi.append(actionRedo);
	listRunDi.append(actionCut);
	listRunDi.append(actionCopy);
	listRunDi.append(actionPaste);
	listRunDi.append(actionComment);
	listRunDi.append(actionUncomment);
	listRunDi.append(actionFind);
	listRunDi.append(actionReplace);
	listRunDi.append(actionMacroBegin);
	listRunDi.append(actionEditMacros);
	listRunDi.append(actionStart);
	listRunDi.append(actionDebug);
	listRunDi.append(actionStep);
	listRunDi.append(actionStepInto);
	listRunDi.append(actionStepOut);
	//  listRunDi.append(actionConvertToC);
	listRunDi.append(actionSettings);
	listRunDi << actionSave << actionSaveAs << actionSaveAll << actionPrint << actionHelpTip;
	listRunDi << actionModulesRegistration;
	listRunDi << actionLoadEnvironment << actionEditStartEnvironment << actionShowStartEnvironment << actionSaveEnvironment << actionPrintEnvironment << actionLoadDrawing;

	listStepDi.append(actionNew);

	listStepDi.append(actionDelete_a_line_under_cursor);
	listStepDi.append(actionNewText);
	listStepDi.append(actionOpenInCurrentTab);
	listStepDi.append(actionOpenInNewTab);
	listStepDi.append(actionClose);
	listStepDi.append(actionUndo);
	listStepDi.append(actionRedo);
	listStepDi.append(actionCut);
	listStepDi.append(actionCopy);
	listStepDi.append(actionPaste);
	listStepDi.append(actionComment);
	listStepDi.append(actionUncomment);
	listStepDi.append(actionFind);
	listStepDi.append(actionReplace);
	listStepDi.append(actionMacroBegin);
	listStepDi.append(actionEditMacros);
	listStepDi.append(actionStep);
	listStepDi.append(actionStepInto);
	listStepDi.append(actionStepOut);
	listStepDi.append(actionStop);
	//  listStepDi.append(actionConvertToC);
	listStepDi.append(actionSettings);
	listStepDi << actionSave << actionSaveAs << actionSaveAll << actionPrint << actionHelpTip;
	listStepDi << actionModulesRegistration;
	listStepDi << actionLoadEnvironment << actionEditStartEnvironment << actionShowStartEnvironment << actionSaveEnvironment << actionPrintEnvironment << actionLoadDrawing;

	listErrorDi.append(actionStep);
	listErrorDi.append(actionStepInto);
	listErrorDi.append(actionStepOut);

	listPauseDi.append(actionNew);
	listPauseDi.append(actionNewText);
	listPauseDi.append(actionOpenInCurrentTab);
	listPauseDi.append(actionOpenInNewTab);
	listPauseDi.append(actionClose);
	listPauseDi.append(actionUndo);
	listPauseDi.append(actionRedo);
	listPauseDi.append(actionCut);
	listPauseDi.append(actionCopy);
	listPauseDi.append(actionPaste);
	listPauseDi.append(actionComment);
	listPauseDi.append(actionUncomment);
	listPauseDi.append(actionReplace);
	listPauseDi.append(actionMacroBegin);
	listPauseDi.append(actionEditMacros);
	listPauseDi.append(actionDelete_a_line_under_cursor);
	//  listPauseDi.append(actionConvertToC);
	//  listPauseDi.append(actionSettings);
	listPauseDi << actionHelpTip << actionModulesRegistration;
	listPauseDi << actionLoadEnvironment << actionEditStartEnvironment << actionSaveEnvironment << actionShowStartEnvironment << actionPrintEnvironment << actionLoadDrawing;

	listAllW.append(menuOpenRecent);
	//  listAllW.append(btnEditOutput);
	listAllW.append(btnSaveOutput);
	listAllW.append(btnClearOutput);
	//  listAllW << menuNew;

	//listEditDi.append(actionCut);

	listRunDiW.append(menuOpenRecent);
	//  listRunDiW.append(btnEditOutput);
	listRunDiW.append(btnSaveOutput);
	listRunDiW.append(btnClearOutput);
	//  listRunDiW << menuNew;

	listStepDiW.append(menuOpenRecent);
	//  listStepDiW.append(btnEditOutput);
	listStepDiW.append(btnSaveOutput);
	listStepDiW.append(btnClearOutput);
	//  listStepDiW << menuNew;

	//listErrorDi.append(actionStep);

	listPauseDiW.append(menuOpenRecent);
	//  listPauseDiW.append(btnEditOutput);
	listPauseDiW.append(btnSaveOutput);
	listPauseDiW.append(btnClearOutput);
	//  listPauseDiW << menuNew;

	listEditEn = listAll;
	foreach (QAction *qa, listEditDi) {
		listEditEn.removeAll(qa);
	}
	listRunEn = listAll;
	foreach (QAction *qa, listRunDi) {
		listRunEn.removeAll(qa);
	}
	listStepEn = listAll;
	foreach (QAction *qa, listStepDi) {
		listStepEn.removeAll(qa);
	}
	listPauseEn = listAll;
	foreach (QAction *qa, listPauseDi) {
		listPauseEn.removeAll(qa);
	}
	listErrorEn = listAll;
	foreach (QAction *qa, listErrorDi) {
		listErrorEn.removeAll(qa);
	}

	listEditEnW = listAllW;
	foreach (QWidget *qa, listEditDiW) {
		listEditEnW.removeAll(qa);
	}
	listRunEnW = listAllW;
	foreach (QWidget *qa, listRunDiW) {
		listRunEnW.removeAll(qa);
	}
	listStepEnW = listAllW;
	foreach (QWidget *qa, listStepDiW) {
		listStepEnW.removeAll(qa);
	}
	listPauseEnW = listAllW;
	foreach (QWidget *qa, listPauseDiW) {
		listPauseEnW.removeAll(qa);
	}
	listErrorEnW = listAllW;
	foreach (QWidget *qa, listErrorDiW) {
		listErrorEnW.removeAll(qa);
	}

}

void MainWindow::about()
{
	qDebug() << "true || false && TEST:" << (true || 1 * 2 > 5);
	//qDebug()<<"false || true && TEST:"<<( || true || false );

	// app()->addEventToHistory("MainWindow::about()");
	AboutDialog aboutDialog(this, Qt::Dialog | Qt::WindowSystemMenuHint);
	/*  if (app()->kumirStyle() == QChar('c'))
	{
	    aboutDialog.labelMode->setText(tr("Classic style"));
	}
	else if (app()->kumirStyle() == QChar('t'))
	{
	    aboutDialog.labelMode->setText(tr("Text style"));
	    actionNewModule->setVisible(false);
	}
	else if (app()->kumirStyle() == QChar('f'))
	{
	    aboutDialog.labelMode->setText(tr("Full style"));
	}*/
	//  if (app()->isZMode())
	//  {
	//      aboutDialog.label->setText(aboutDialog.label->text() + '\n' + app()->getZPath());
	//  }
	aboutDialog.exec();
}

void MainWindow::uncheckVelichiny()
{
	// app()->addEventToHistory("MainWindow::uncheckVelichiny()");
	actionVariables->setChecked(false);
}

void MainWindow::showSettingsDialog()
{
	// app()->addEventToHistory("MainWindow::showSettingsDialog()");
	app()->settingsDialog->exec();
}

MainWindow::~MainWindow()
{}

QFont MainWindow::editorsFont()
{
	Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(0));
	return tab->font();
}

void MainWindow::setEditorsFont(const QFont &font)
{
	// app()->addEventToHistory("MainWindow::setEditorsFont()");
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		tab->setFont(font);
	}
	textEdit3->setFont(font);
}

void MainWindow::updateLanguage()
{
	// app()->addEventToHistory("MainWindow::updateLanguage()");
	retranslateUi(this);
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		tab->updateLanguage();
	}
}

void MainWindow::updateSHAppearance()
{
	// app()->addEventToHistory("MainWindow::updateSHAppearance()");
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		tab->updateSHAppearance();
	}
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->UpdateColors();
	roboEditor->field->UpdateColors();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	// app()->addEventToHistory("MainWindow::closeEvent()");
	app()->variablesWindow->close();
	app()->assistantWindow->close();
	for (int i = 0; i < tabWidget->count(); ++i) {
		if (dynamic_cast<Tab *>(tabWidget->widget(i))->isModified()) {
			tabWidget->setCurrentIndex(i);
			QMessageBox::StandardButton ans;
			ans = QMessageBox::question(this, tr("Kumir"), tr("This file not saved. Save it before closing?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
			if (ans == QMessageBox::Cancel) {
				event->ignore();
				return;
			} else if (ans == QMessageBox::Yes) {
				if (!fileSave()) {
					return;
				}
			}
		}
	}

	foreach (QWidget *w, app()->secondaryWindows) {
		w->close();
	}
	//  if (app()->secondaryWindows.count()!=0)
	//  {
	//      event->ignore();
	//      return;
	//  }


//	app()->settings->setValue("MainWindow/Splitter2",splitter_2->saveState());
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		tab->saveSplitterState();
	}

	closeAllInstruments();
	event->accept();
	app()->emergencyCleanState();
	roboEditor->close();
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Robot" || app()->compiler->modules()->module(i)->Name() == "Draw") {


			int x, y;
			app()->compiler->modules()->module(i)->instrument()->getWindowPos(&x, &y);
			app()->settings->setValue(app()->compiler->modules()->module(i)->Name() + "/Xpos", x);
			app()->settings->setValue(app()->compiler->modules()->module(i)->Name() + "/Ypos", y);
			qDebug("Isp pos saved");


			app()->compiler->modules()->module(i)->instrument()->close();

		}
		if (app()->compiler->modules()->module(i)->isFriend()) {
			app()->compiler->modules()->module(i)->quitModule();
		}
	}
	qApp->quit();
}

void MainWindow::saveUIState()
{
//	app()->settings->setValue("MainWindow/ToolBar/FileToolBarVisible", fileToolBar->isVisible());
//	app()->settings->setValue("MainWindow/ToolBar/EditToolBarVisible", editToolBar->isVisible());
//	app()->settings->setValue("MainWindow/ToolBar/RunToolBarVisible", runToolBar->isVisible());
//	app()->settings->setValue("MainWindow/ToolBar/ModulesToolBarVisible", modulesToolBar->isVisible());
//	app()->settings->setValue("MainWindow/Geometry",saveGeometry());
	QPoint p = pos();
	QSize sz = size();
	app()->settings->setValue("MainWindow/Rect", QRect(p, sz));
	QList<QVariant> sizesV;
	for (int i = 0; i < splitter_2->sizes().count(); i++) {
		sizesV << QVariant(splitter_2->sizes()[i]);
	}
	app()->settings->setValue("MainWindow/SplitterSizes", sizesV);
	app()->settings->sync();
}

QString MainWindow::currentEditorText(bool withIndentation)
{
	// app()->addEventToHistory("MainWindow::currentEditorText("+QString(withIndentation?"true":"false")+")");
	Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
	return tab->editorText(withIndentation);
}

QString MainWindow::currentMarginText()
{
	// app()->addEventToHistory("MainWindow::currentMarginText()");
	Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
	return tab->marginText();
}

void MainWindow::setCurrentEditorText(const QString &txt)
{
	// app()->addEventToHistory("MainWindow::setCurrentEditorText()");
	Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
	return tab->setEditorText(txt);
}

bool MainWindow::fileSave(QString fileName)
{
	// Если имя файла пустое, нужно взять его с закладки (Save)
	if (fileName.isEmpty()) {
		fileName = (dynamic_cast<Tab *>(tabWidget->currentWidget()))->getFileName();
	}

	if (fileName.isEmpty()) {
		// Если сохраняем первый раз,то Save превращается в Save As
		return fileSaveAs();
	} else {
		QString codec = QString::fromUtf8("UTF-16LE");
		QFile f(fileName);
		curFileInfo.setFile(fileName);
		QTextStream ts(&f);
		if (f.open(QIODevice::WriteOnly)) {
			//          qDebug(codec.toUtf8());
			ts.setCodec(codec.toUtf8());
			ts << currentEditorText();
			tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(f.fileName()).fileName());
			setWindowTitle(QFileInfo(f.fileName()).fileName() + " - " + tr("Kumir"));
			(dynamic_cast<Tab *>(tabWidget->currentWidget()))->setFileName(fileName);
			tab(tabWidget->currentIndex())->setModified(false);
			emit sendMessage(tr("File saved succesfully") + " (" + codec + ")", 0);
			f.close();
			tab(tabWidget->currentIndex())->setFileName(fileName);
			actionSave->setToolTip(tr("Save as") + " " + fileName);
			addToRecent(QFileInfo(f).absoluteFilePath());
			return true;
		} else {
			QMessageBox::critical(this,
				tr("Error saving file"),
				tr("There is an error occured while writing file.\nPossible reason is out of space on drive or you do not have write access to the selected directory."));
			emit sendMessage(tr("File not saved"), 0);
			return false;
		}
	}
}

QString suggestFileName(QString programText)
{
	QString result;
	QMap<QString, QString> translit;
	translit[QString::fromUtf8(" ")] = QString::fromUtf8("_");
	translit[QString::fromUtf8("а")] = QString::fromUtf8("a");
	translit[QString::fromUtf8("б")] = QString::fromUtf8("b");
	translit[QString::fromUtf8("в")] = QString::fromUtf8("v");
	translit[QString::fromUtf8("г")] = QString::fromUtf8("g");
	translit[QString::fromUtf8("д")] = QString::fromUtf8("d");
	translit[QString::fromUtf8("е")] = QString::fromUtf8("e");
	translit[QString::fromUtf8("ё")] = QString::fromUtf8("jo");
	translit[QString::fromUtf8("ж")] = QString::fromUtf8("zh");
	translit[QString::fromUtf8("з")] = QString::fromUtf8("z");
	translit[QString::fromUtf8("и")] = QString::fromUtf8("i");
	translit[QString::fromUtf8("й")] = QString::fromUtf8("j");
	translit[QString::fromUtf8("к")] = QString::fromUtf8("k");
	translit[QString::fromUtf8("л")] = QString::fromUtf8("l");
	translit[QString::fromUtf8("м")] = QString::fromUtf8("m");
	translit[QString::fromUtf8("н")] = QString::fromUtf8("n");
	translit[QString::fromUtf8("о")] = QString::fromUtf8("o");
	translit[QString::fromUtf8("п")] = QString::fromUtf8("p");
	translit[QString::fromUtf8("р")] = QString::fromUtf8("r");
	translit[QString::fromUtf8("с")] = QString::fromUtf8("s");
	translit[QString::fromUtf8("т")] = QString::fromUtf8("t");
	translit[QString::fromUtf8("у")] = QString::fromUtf8("u");
	translit[QString::fromUtf8("ф")] = QString::fromUtf8("f");
	translit[QString::fromUtf8("х")] = QString::fromUtf8("kh");
	translit[QString::fromUtf8("ц")] = QString::fromUtf8("ts");
	translit[QString::fromUtf8("ч")] = QString::fromUtf8("ch");
	translit[QString::fromUtf8("ш")] = QString::fromUtf8("sh");
	translit[QString::fromUtf8("щ")] = QString::fromUtf8("sch");
	translit[QString::fromUtf8("ъ")] = QString::fromUtf8("");
	translit[QString::fromUtf8("ы")] = QString::fromUtf8("y");
	translit[QString::fromUtf8("ь")] = QString::fromUtf8("");
	translit[QString::fromUtf8("э")] = QString::fromUtf8("e");
	translit[QString::fromUtf8("ю")] = QString::fromUtf8("ju");
	translit[QString::fromUtf8("я")] = QString::fromUtf8("ja");
	translit[QString::fromUtf8("α")] = QString::fromUtf8("alpha");
	translit[QString::fromUtf8("β")] = QString::fromUtf8("beta");
	translit[QString::fromUtf8("γ")] = QString::fromUtf8("gamma");
	translit[QString::fromUtf8("δ")] = QString::fromUtf8("delta");
	translit[QString::fromUtf8("ε")] = QString::fromUtf8("epsilon");
	translit[QString::fromUtf8("σ")] = QString::fromUtf8("sigma");
	translit[QString::fromUtf8("υ")] = QString::fromUtf8("sigma");
	translit[QString::fromUtf8("ι")] = QString::fromUtf8("iota");
	translit[QString::fromUtf8("θ")] = QString::fromUtf8("theta");
	translit[QString::fromUtf8("π")] = QString::fromUtf8("pi");
	translit[QString::fromUtf8("φ")] = QString::fromUtf8("phi");
	translit[QString::fromUtf8("η")] = QString::fromUtf8("eta");
	translit[QString::fromUtf8("ξ")] = QString::fromUtf8("ksi");
	translit[QString::fromUtf8("κ")] = QString::fromUtf8("kappa");
	translit[QString::fromUtf8("λ")] = QString::fromUtf8("lambda");
	translit[QString::fromUtf8("ζ")] = QString::fromUtf8("dzetta");
	translit[QString::fromUtf8("χ")] = QString::fromUtf8("khi");
	translit[QString::fromUtf8("ψ")] = QString::fromUtf8("psi");
	translit[QString::fromUtf8("ω")] = QString::fromUtf8("omega");
	translit[QString::fromUtf8("μ")] = QString::fromUtf8("mu");
	translit[QString::fromUtf8("ν")] = QString::fromUtf8("nu");

	translit[QString::fromUtf8("|")] = QString::fromUtf8("");
	translit[QString::fromUtf8(":")] = QString::fromUtf8("");
	translit[QString::fromUtf8(";")] = QString::fromUtf8("");
	translit[QString::fromUtf8("/")] = QString::fromUtf8("");
	translit[QString::fromUtf8("\\")] = QString::fromUtf8("");
	translit[QString::fromUtf8(".")] = QString::fromUtf8("");
	translit[QString::fromUtf8(",")] = QString::fromUtf8("");
	translit[QString::fromUtf8("#")] = QString::fromUtf8("");
	translit[QString::fromUtf8("$")] = QString::fromUtf8("");
	translit[QString::fromUtf8("%")] = QString::fromUtf8("");
	translit[QString::fromUtf8("^")] = QString::fromUtf8("");
	translit[QString::fromUtf8("!")] = QString::fromUtf8("");
	translit[QString::fromUtf8(")")] = QString::fromUtf8("");
	translit[QString::fromUtf8("(")] = QString::fromUtf8("");
	translit[QString::fromUtf8("[")] = QString::fromUtf8("");
	translit[QString::fromUtf8("]")] = QString::fromUtf8("");

	QStringList lines = programText.split("\n");

	for (int i = 0; i < lines.count(); i++) {
		QString line = lines[i];
		if (line.startsWith(QString::fromUtf8("исп "))) {
			QRegExp rxAlgName(QString::fromUtf8(
					"исп\\s+(.*)"));
			if (rxAlgName.exactMatch(line)) {
				QString algName = rxAlgName.cap(1).toLower().trimmed();
				for (int i = 0; i < translit.keys().count(); i++) {
					QString letter_before = translit.keys()[i];
					QString letter_after = translit[letter_before];
					algName.replace(letter_before, letter_after);
				}
				result = algName;
				break;
			}
		}
		if (line.startsWith(QString::fromUtf8("алг "))) {
			QRegExp rxAlgName(QString::fromUtf8(
					"алг\\s+(цел|вещ|лог|лит|сим)?\\s*(таб)?\\s*(.*)\\(?(.*)"));
			if (rxAlgName.exactMatch(line)) {
				QString algName = rxAlgName.cap(3).toLower().trimmed();
				for (int i = 0; i < translit.keys().count(); i++) {
					QString letter_before = translit.keys()[i];
					QString letter_after = translit[letter_before];
					algName.replace(letter_before, letter_after);
				}
				result = algName;
				break;
			}
		}
	}
	return result;
}

bool MainWindow::fileSaveAs()
{
	// app()->addEventToHistory("MainWindow::fileSaveAs()");
	qDebug() << splitter_2->sizes() << tabWidget->height() + textEdit3->height();

	QDir lastDir = QDir(app()->settings->value("History/KumDir", defaultDir).toString());
	if (!lastDir.exists()) {
		lastDir = QDir::current();
	}

	QString typ(tr("Kumir programs (*.kum)"));
	bool encodingVisible = false;
	QString enc("UTF-16LE");
	QString ext(".kum");

	QString fileName = (dynamic_cast<Tab *>(tabWidget->currentWidget()))->getFileName();
	if (fileName.isEmpty()) {
		fileName = suggestFileName(currentEditorText());
	}
	if (fileName.isEmpty()) {
		fileName = lastDir.absolutePath();
	} else {
		if (!fileName.endsWith(ext)) {
			fileName += ext;
		}
		fileName = lastDir.absoluteFilePath(fileName);
	}

	bool result = false;
	QString newFileName;
	if (app()->settings->value("UI/LegacyFileDialog", false).toBool()) {
		KumFileDialog *saveDialog;
		saveDialog = new KumFileDialog(this, tr("Save..."), fileName, typ, encodingVisible);
		saveDialog->setEncoding(enc);
		saveDialog->setAcceptMode(QFileDialog::AcceptSave);
		if (saveDialog->exec() == QDialog::Accepted && !saveDialog->selectedFiles().isEmpty()) {
			enc = saveDialog -> encoding();
			newFileName = saveDialog->selectedFiles().first();
		}
		saveDialog->deleteLater();
	} else {
		newFileName = QFileDialog::getSaveFileName(this, tr("Save..."), fileName, typ);
	}
	if (!newFileName.isEmpty()) {
		app()->settings->setValue("History/KumDir", QFileInfo(newFileName).dir().absolutePath());
		if (!newFileName.endsWith(ext)) {
			newFileName += ext;
		}
		result = fileSave(newFileName);
	}
	if (!app()->isTeacherMode()) {
		app()->mainWindow->actionSave->setEnabled(true);
	}
	return result;
}

void MainWindow::fileOpenInFirstTab(QString fileName) // Для batch-режима
{
	//app()->addEventToHistory("MainWindow::fileOpenInFirstTab("+fileName+")");
	if (fileName.isEmpty()) {
		return;
	}
	if (fileName.endsWith(".kum")) {
		QFile f(fileName);
		if (f.open(QIODevice::ReadOnly)) {
			QTextStream ts(&f);
			ts.setCodec("UTF-16LE");
			QString data = ts.readAll();
			f.close();

			QFileInfo fi(fileName);
			ProgramTab *tab;
			if (tabWidget->count() > 0) {
				tab = dynamic_cast<ProgramTab *>(tabWidget->widget(0));
				tab->setTabNo(0);
				//tab->setModule(false);
			} else {
				tab = new ProgramTab(tabWidget);
				tabWidget->addTab(tab, fi.fileName());
				tab->setTabNo(tabWidget->count());
			}
			//tabWidget->setCurrentWidget(tab);
			tabWidget->setTabIcon(tabWidget->count() - 1, QIcon(":/icons/file-import.png"));
			tab->setFileName(fi.fileName());
			tabWidget->setTabText(tabWidget->currentIndex(), fi.fileName());
			semaphoreWidget->fastRepaint = false;
			tab->setEditorText(data);
			semaphoreWidget->fastRepaint = true;
			tabWidget->setCurrentWidget(tab);
			if (tabWidget->count() > 1) {
				actionClose->setEnabled(true);
				//btnClose->setEnabled(true);
			}
			tab->editor()->setFocus();
			app()->doCompilation();
			if (!app()->isTeacherMode()) {
				app()->mainWindow->actionSave->setEnabled(true);
			}
		}
	}

}

void MainWindow::fileOpenInCurrentTab(QString fileName)
{
	if (!isPossibleToClose()) {
		return;
	}

	if (fileName.isEmpty()) {
		fileName = getFileNameAndEncoding();
	}

	if (fileName.isEmpty()) {
		return;
	}

	QFile f(fileName);

	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this,
			tr("Error opening file"),
			tr("There is an error occured while reading file.\nCheck your access rights to this file."));
		emit sendMessage(tr("Error opening file"), 0);
		return;
	}

	const QByteArray binData = f.readAll();
	f.close();

	bool UTF8_BOM = false;
	if (binData.size() >= 3) {
		quint8 byte0 = binData[0];
		quint8 byte1 = binData[1];
		quint8 byte2 = binData[2];
		UTF8_BOM = byte0 == 0xEF && byte1 == 0xBB && byte2 == 0xBF;
	}


	QTextStream ts(binData);
	ts.setAutoDetectUnicode(true);
	ts.setCodec("UTF-16LE");
	QString data = ts.readAll();
	QString msgVersion = tr(" (format Kumir %1)").arg("1.x");

	if (UTF8_BOM) {
		// File created in Kumir 2.x
		// Check for compability flag
		QStringList minVersion;
		QStringList myVersion = qApp->applicationVersion().split(".");
		const QStringList lines = data.split("\n");
		data = "";
		foreach (const QString &line, lines) {
			if (line.startsWith("| @minVersion ")) {
				minVersion = line.mid(13).trimmed().split(".");
			} else {
				data += line + "\n";
			}
		}
		if (!minVersion.isEmpty()) {
			int reqMaj = 0;
			int reqMin = 0;
			int reqRel = 0;
			int myMaj = myVersion[0].toInt();
			int myMin = myVersion[1].toInt();
			int myRel = myVersion[2].toInt();
			if (minVersion.size() > 0) {
				reqMaj = minVersion[0].toInt();
			}
			if (minVersion.size() > 1) {
				reqMin = minVersion[1].toInt();
			}
			if (minVersion.size() > 2) {
				reqRel = minVersion[2].toInt();
			}
			if (myMaj < reqMaj || myMin < reqMin || myRel < reqRel) {
				QMessageBox::critical(this,
					tr("Error opening file"),
					tr("This file was created by recent version of Kumir and requires version at least %1").arg(minVersion.join(".")));
				emit sendMessage(tr("Error opening file"), 0);
				return;
			}
			msgVersion = tr(" (format Kumir %1)").arg(minVersion.join("."));
		}
	}

	addToRecent(QFileInfo(f).absoluteFilePath());
	app()->settings->setValue("History/KumDir", QFileInfo(fileName).absolutePath());
	if (fileName.endsWith(".kum")/* && !asText*/) {

		// для совместимости со старыми версиями КуМира убиваем отступы в начале
		QStringList lines = data.split("\n", QString::KeepEmptyParts);
		for (int i = 0; i < lines.count(); i++) {
			QString line = lines[i];
			while (lines[i].startsWith(".") || lines[i].startsWith(" ")) {
				lines[i].remove(0, 1);
			}
		}
		data = lines.join("\n");
		ProgramTab *tab = dynamic_cast<ProgramTab *>(tabWidget->currentWidget());
		QFileInfo fi(fileName);
		curFileInfo = fi;
		tabWidget->setTabText(tabWidget->currentIndex(), fi.fileName());
		setWindowTitle(fi.fileName() + " - " + tr("Kumir"));
		setFormMode("edit");
		removeModulesFromTable();
		tab->setFileName(fileName);
		actionSave->setToolTip(tr("Save as") + " " + fileName);
		semaphoreWidget->fastRepaint = false;
		tab->setEditorText(data);
		QString margin;
		for (int i = 0; i < data.count("\n"); i++) {
			margin += "\n";
		}
		tab->setMarginText(margin);
		tab->syncronizeLineHeights();
		semaphoreWidget->fastRepaint = true;
		if (tabWidget->count() > 1) {
			actionClose->setEnabled(true);
			//btnClose->setEnabled(true);
		}
		app()->doCompilation();
		tab->editor()->setFocus();
		emit sendMessage(tr("Program loaded") + msgVersion, 0);
	}
	tabWidget->setTabToolTip(tabWidget->currentIndex(), fileName);
	dynamic_cast<Tab *>(tabWidget->currentWidget())->editor()->clearUndoStack();
	dynamic_cast<Tab *>(tabWidget->currentWidget())->setModified(false);
	if (!app()->isTeacherMode()) {
		app()->mainWindow->actionSave->setEnabled(true);
	}
	if (!app()->isTeacherMode() && app()->isTaskControlLoaded()) {
		app()->TaskControl()->setCSmode(-1);
	}
}

QString MainWindow::getFileNameAndEncoding()
{
	QString fileName;
	bool encodingVisible = false;
	QString typ = tr("Kumir programs (*.kum)");
	QString enc("UTF-16LE");
	QDir lastDir = QDir(app()->settings->value("History/KumDir", defaultDir).toString());
	QString lastDirPath = lastDir.absolutePath().replace("/", QDir::separator()) + QDir::separator();
	if (app()->settings->value("UI/LegacyFileDialog", false).toBool()) {
		KumFileDialog *openDialog;
		openDialog = new KumFileDialog(this, tr("Open...").remove("&"), lastDirPath, typ, encodingVisible);
		if (openDialog->exec() == QDialog::Accepted) {
			if (! openDialog->selectedFiles().isEmpty()) {
				fileName = openDialog->selectedFiles().first();
			}
		}
		openDialog->deleteLater();
	} else {
		fileName = QFileDialog::getOpenFileName(this, tr("Open...").remove("&"), lastDirPath, typ);
	}
	if (!fileName.isEmpty()) {
		app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
	}
	return fileName;
}


bool MainWindow::isPossibleToClose()
{
	// Проверка на измененность
	if (dynamic_cast<Tab *>(tabWidget->currentWidget())->isModified()) {
		QMessageBox::StandardButton ans;
		ans = QMessageBox::question(this, tr("Kumir"), tr("Current file not saved. Save it?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
		if (ans == QMessageBox::Cancel) {
			return false;
		}
		if (ans == QMessageBox::Yes) {
			return fileSave();
		}
	}

	return true;
}

void MainWindow::fileNewProgram()
{
	// app()->addEventToHistory("MainWindow::fileNewProgram()");
	if (tabWidget->count() == 0) {
		QString ico = ":/icons/document-new.png";
		ProgramTab *tab = new ProgramTab(tabWidget);
		tab->setTabNo(tabWidget->count());
		tabWidget->addTab(tab, tr("New program"));
		tabWidget->setCurrentWidget(tab);
		tabWidget->setTabIcon(tabWidget->count() - 1, QIcon(ico));
		tab->editor()->setFocus();
		connect(tab->editor(), SIGNAL(sendMessage(QString, int)), this, SLOT(showMessage(QString, int)));
		//connect( tab->editor(), SIGNAL(fileOpenRequest(const QString&)), this, SLOT(deliverFileOpenRequest(const QString&)) );
		connect(tab->editor(), SIGNAL(fileOpenHereRequest(const QString &)), this, SLOT(fileOpenInCurrentTab(const QString &)));
		connect(tab->editor(), SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
	} else {
		tabWidget->setCurrentIndex(0);
		if (isPossibleToClose()) {
			closeAllInstruments();
			tabWidget->setTabText(0, tr("New program"));
			setWindowTitle(tr("New program") + " - " + tr("Kumir"));
			actionSave->setToolTip(tr("Save"));
			dynamic_cast<ProgramTab *>(tabWidget->currentWidget())->setFileName("");
			KumirEdit *editor = dynamic_cast<ProgramTab *>(tabWidget->currentWidget())->editor();
			disconnect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
			removeModulesFromTable();
			QString startText;
			if (qApp->arguments().contains("-e")) {
				startText = QString::fromUtf8("алг \nнач\n\nкон\n");
			} else {
				startText = QString::fromUtf8("| использовать Робот\nалг \nнач\n\nкон\n");
			}
			startText = app()->settings->value("Start/Text", startText).toString();


			editor->changeText(startText);

			//          editor->changeText(QString::fromUtf8("|использовать Робот\n\nалг \nнач\n\nкон\n"));

			//          editor->moveCursor(QTextCursor::Down);
			//          editor->moveCursor(QTextCursor::Down);
			//          editor->moveCursor(QTextCursor::EndOfLine);
			dynamic_cast<Tab *>(tabWidget->widget(0))->editor()->clearUndoStack();
			dynamic_cast<Tab *>(tabWidget->widget(0))->setModified(false);
			dynamic_cast<Tab *>(tabWidget->widget(0))->setFileName("");
			connect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
			curFileInfo = QFileInfo("");
		}
	}
	if (!app()->isTeacherMode()) {
		app()->mainWindow->actionSave->setEnabled(true);
	}
	if (!app()->isTeacherMode() && (app()->isTaskControlLoaded())) {
		app()->TaskControl()->setCSmode(-1);
	}

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
#if 0
	if (event->mimeData()->hasUrls()) {
		QString path;
		foreach (QUrl url, event->mimeData()->urls()) {
			QString p = url.toLocalFile();
			if (p.endsWith(".kum")) {
				path = p;
				break;
			}
		}
	}
#endif
	event->accept();

}

void MainWindow::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasUrls()) {
		QString path;
		foreach (QUrl url, event->mimeData()->urls()) {
			QString p = url.toLocalFile();
			if (p.endsWith(".kum")) {
				path = p;
				break;
			}
		}
		if (!path.isEmpty()) {
			fileOpenInCurrentTab(path);
			event->accept();
		} else {
			event->ignore();
		}
	} else {
		event->ignore();
	}
}

TextWindow *MainWindow::fileNewText()
{
//	// app()->addEventToHistory("MainWindow::fileNewText()");
//	TextWindow *tab = new TextWindow(NULL, Qt::Window);
//	tab->setTextEncoding(QTextCodec::codecForLocale()->name());
//	tab->activateWindow();
//	tab->show();
//	return tab;
	QString textEditor = app()->settings->value(
			"System/TextEditor",
			SettingsDialog::defaultTextEditor()).toString();
#ifdef Q_OS_WIN32
	if (textEditor.contains(" ") && !textEditor.contains("\"")) {
		textEditor = "\"" + textEditor + "\"";
	}
#endif
	textEditor.replace("$KUMIR_DIR", QApplication::applicationDirPath());
	QProcess::startDetached(textEditor, QStringList(), defaultDir);
	return NULL;
}

/*void MainWindow::switchPreviousTab()
{
    // app()->addEventToHistory("MainWindow::switchPreviousTab()");
    if ( tabWidget->count() == 1 )
        return;
    if ( tabWidget->currentIndex() == 0 )
    {
        tabWidget->setCurrentIndex(tabWidget->count()-1);
    }
    else
    {
        tabWidget->setCurrentIndex(tabWidget->currentIndex()-1);
    }
}

void MainWindow::switchNextTab()
{
    // app()->addEventToHistory("MainWindow::switchNextTab()");
    if ( tabWidget->count() == 1 )
        return;
    if ( tabWidget->currentIndex() == (tabWidget->count()-1) )
    {
        tabWidget->setCurrentIndex(0);
    }
    else
    {
        tabWidget->setCurrentIndex(tabWidget->currentIndex()+1);
    }
}*/

void MainWindow::setCaptionFromTab(int tabNo)
{
	// app()->addEventToHistory("MainWindow::setCaptionFromTab("+QString::number(tabNo)+")");
	Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(tabNo));
	//  QString newCaption = tr("Kumir")+" - "+tab->getFileName()+" ";
	//  if ( tab->isProgram() && tab->isModule() )
	//  {
	//      newCaption += tr("(Module)");
	//  }
	//  else if ( tab->isProgram() )
	//  {
	//      newCaption += tr("(Program)");
	//  }
	//  else
	//  {
	//      newCaption += tr("(Text)");
	//  }
	//  setWindowTitle(newCaption);
	tab->editor()->setFocus();
}

void MainWindow::setActionsForTab(int tabNo)
{
	// Устанавливаем доступность действий для текущей вкладки
	// app()->addEventToHistory("MainWindow::setActionsForTab("+QString::number(tabNo)+")");
	Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(tabNo));
	if (tab->isProgram()) {
		actionComment->setEnabled(true);
		actionUncomment->setEnabled(true);
		if (!tab->isModule() || (m_formMode == "pause") || (m_formMode == "run")) {
			actionStart->setEnabled(true);
			actionDebug->setEnabled(true);
			actionStep->setEnabled(true);
			actionStepInto->setEnabled(true);
			//actionStop->setEnabled(true);
		} else {
			actionStart->setEnabled(false);
			actionDebug->setEnabled(false);
			actionStep->setEnabled(false);
			actionStepInto->setEnabled(false);
			actionStop->setEnabled(false);
		}
	} else {
		actionComment->setEnabled(false);
		actionUncomment->setEnabled(false);
		if ((m_formMode == "pause") || (m_formMode == "run")) {
			actionStart->setEnabled(true);
			actionDebug->setEnabled(true);
			actionStep->setEnabled(true);
			actionStepInto->setEnabled(true);
			actionStop->setEnabled(true);
		} else {
			actionStart->setEnabled(false);
			actionDebug->setEnabled(false);
			actionStep->setEnabled(false);
			actionStepInto->setEnabled(false);
			actionStop->setEnabled(false);
		}
	}
	// Убираем старые связи
	actionCut->disconnect();
	actionCopy->disconnect();
	//  actionPaste->disconnect();
	actionUndo->disconnect();
	actionRedo->disconnect();
	actionComment->disconnect();
	actionUncomment->disconnect();
	//actionPrevError->disconnect();
	//actionNextError->disconnect();
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(tabNo));
		tab->disconnectEditActions();
	}
	// Устанавливаем новые связи
	if (tab->isProgram()) {
		ProgramTab *pTab = dynamic_cast<ProgramTab *>(tab);
		connect(actionCut, SIGNAL(triggered()), pTab, SLOT(editCut()));
		connect(actionCopy, SIGNAL(triggered()), pTab, SLOT(editCopy()));
		actionDelete_a_line_under_cursor->disconnect();
		connect(actionDelete_a_line_under_cursor, SIGNAL(triggered()), pTab->editor(), SLOT(deleteCurrentLine()));
		//      connect ( actionPaste, SIGNAL(triggered()), pTab, SLOT(editPaste()) );
		connect(actionUndo, SIGNAL(triggered()), pTab, SLOT(editUndo()));
		connect(actionRedo, SIGNAL(triggered()), pTab, SLOT(editRedo()));
		connect(pTab, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));
		connect(actionComment, SIGNAL(triggered()), pTab, SLOT(editComment()));
		connect(actionUncomment, SIGNAL(triggered()), pTab, SLOT(editUncomment()));
		connect(pTab, SIGNAL(uncommentAvailable(bool)), actionUncomment, SLOT(setEnabled(bool)));
		//connect ( pTab->editor(), SIGNAL(jumpNextErrorAvailable(bool)), actionNextError, SLOT(setEnabled(bool)) );
		//connect ( pTab->editor(), SIGNAL(jumpPrevErrorAvailable(bool)), actionPrevError, SLOT(setEnabled(bool)) );
		connect(pTab->editor(), SIGNAL(cursorPositionChanged(int, int)), this, SLOT(showCursorPosition(int, int)));
		//connect ( actionNextError, SIGNAL(triggered()), pTab->editor(), SLOT(gotoNextError()) );
		//connect ( actionPrevError, SIGNAL(triggered()), pTab->editor(), SLOT(gotoPrevError()) );
	} else {
		TextTab *pTab = dynamic_cast<TextTab *>(tab);
		connect(actionCut, SIGNAL(triggered()), pTab, SLOT(editCut()));
		connect(actionCopy, SIGNAL(triggered()), pTab, SLOT(editCopy()));
		//      connect ( actionPaste, SIGNAL(triggered()), pTab, SLOT(editPaste()) );
		connect(actionUndo, SIGNAL(triggered()), pTab, SLOT(editUndo()));
		connect(actionRedo, SIGNAL(triggered()), pTab, SLOT(editRedo()));
		connect(pTab, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
		connect(pTab, SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));
		connect(pTab->editor(), SIGNAL(cursorPositionChanged(int, int)), this, SLOT(showCursorPosition(int, int)));
	}
	// Делаем первую проверку на доступность Cut, Copy, Undo и Redo
	actionCut->setEnabled(tab->isCopyAvailable());
	actionCopy->setEnabled(tab->isCopyAvailable());
	actionUndo->setEnabled(tab->isUndoAvailable());
	actionRedo->setEnabled(tab->isRedoAvailable());

	connect(tab->editor(), SIGNAL(overwriteModeChanged(bool)), this, SLOT(showOverwriteMode(bool)));
	connect(tab->editor(), SIGNAL(yellowIndicator()), semaphoreWidget, SLOT(setModeYellow()));
	if (tab->editor()->overwriteMode()) {
		sInsert->setText(tr("OVR"));
	} else {
		sInsert->setText(tr("INS"));
	}

	if (tab->isProgram()) {
		ProgramTab *pTab = dynamic_cast<ProgramTab *>(tab);
		actionComment->setEnabled(true);
		actionUncomment->setEnabled(pTab->isUncommentAvailable());
		actionRefactoring->setEnabled(true);
		//      actionNextError->setEnabled(pTab->editor()->isJumpNextErrorAvailable());
		//      actionPrevError->setEnabled(pTab->editor()->isJumpPrevErrorAvailable());
	} else {
		actionComment->setEnabled(false);
		actionUncomment->setEnabled(false);
		actionRefactoring->setEnabled(false);
		//actionNextError->setEnabled(false);
		//actionPrevError->setEnabled(false);
	}
}

QString MainWindow::formMode()
{
	// app()->addEventToHistory("MainWindow::formMode()");
	return m_formMode;
}

void MainWindow::setLockTabsEdit(bool f)
{
	// app()->addEventToHistory("MainWindow::setLockTabsEdit("+QString(f?"true":"false")+")");
	//  dynamic_cast<ProgramTab*>(tabWidget->widget(0))->editor()->setReadOnly(f);
	for (int i = 0; i < tabWidget->count(); ++i)
		if (dynamic_cast<Tab *>(tabWidget->widget(i))->isProgram()) {
			dynamic_cast<ProgramTab *>(tabWidget->widget(i))->editor()->setReadOnly(f);
		}
}

void MainWindow::oneStep() //TODO MODULES
{
	//  app()->addEventToHistory("MainWindow::oneStep()");
	//  if (stepModeRunning) {
	//      proga pr = app()->compiler->compiledLine(currentStep);
	//      RealLine rl = pr.real_line;
	//      if (rl.tab!=NULL) {
	//          tabWidget->setCurrentWidget(rl.tab);
	//          ProgramTab *pt = dynamic_cast<ProgramTab*>(rl.tab);
	//          QTextCursor tc = pt->editor()->textCursor();
	//          int blockNo = tc.blockNumber();
	//          if (blockNo<rl.lineNo) {
	//              for (int i=blockNo; i<rl.lineNo; i++) {
	//                  tc.movePosition(QTextCursor::NextBlock);
	//              }
	//          }
	//          if (blockNo>rl.lineNo) {
	//              for (int i=blockNo; i>rl.lineNo; i++) {
	//                  tc.movePosition(QTextCursor::PreviousBlock);
	//              }
	//          }
	//          pt->editor()->ensureCursorVisible();
	//      }
	//  }
	//  currentStep++;
}

void MainWindow::setFormMode(const QString &mode)
{
	// Режим функционирования ( "edit", "run", "pause", "analiz" )
	// app()->addEventToHistory("MainWindow::setFormMode("+mode+")");
	Macro *macro;
	QAction *qa;
	QWidget *qw;
	if (mode == "run") {
		currentStep = 0;


		if (m_formMode != "pause") {
			app()->compiler->Modules.lockGUI();
		}
		setLockTabsEdit(true);
		sMode->setText(tr("Run mode"));
		semaphoreWidget->hide();
		foreach (qa, listRunEn)
			if (!qa->isEnabled()) {
				qa->setEnabled(true);
			}
		foreach (qa, listRunDi)
			if (qa->isEnabled()) {
				qa->setEnabled(false);
			}
		foreach (qw, listRunEnW)
			if (!qw->isEnabled()) {
				qw->setEnabled(true);
			}
		foreach (qw, listRunDiW)
			if (qw->isEnabled()) {
				qw->setEnabled(false);
			}
		foreach (macro, app()->macros) {
			QAction *action = macro->action();
			if (action) {
				action->setEnabled(false);
			}
		}
		for (int i = 0; i < tabWidget->count(); i++) {
			Tab *t = dynamic_cast<Tab *>(tabWidget->widget(i));
			if (t->isProgram()) {
				ProgramTab *pt = dynamic_cast<ProgramTab *>(t);
				Q_CHECK_PTR(pt);
				pt->saveCursorPosition();
			}
		}
		if (dynamic_cast<ProgramTab *>(tabWidget->widget(0))->editor() != NULL) {
			clearRedAndGreenLines();
		}
		timerRun->start(1000);
		sPos->setEnabled(false);
	} else if (mode == "edit") {

		app()->compiler->Modules.unLockGUI();
		setLockTabsEdit(false);
		sMode->setText(tr("Edit mode"));
		semaphoreWidget->show();
		foreach (qa, listEditEn)
			if (!qa->isEnabled()) {
				qa->setEnabled(true);
			}
		foreach (qa, listEditDi)
			if (qa->isEnabled()) {
				qa->setEnabled(false);
			}
		foreach (qw, listEditEnW)
			if (!qw->isEnabled()) {
				qw->setEnabled(true);
			}
		foreach (qw, listEditDiW)
			if (qw->isEnabled()) {
				qw->setEnabled(false);
			}
		foreach (macro, app()->macros)
			if (macro != NULL && macro->action() != NULL) {
				macro->action()->setEnabled(true);
			}
		for (int i = 0; i < tabWidget->count(); i++) {
			Tab *t = dynamic_cast<Tab *>(tabWidget->widget(i));
			if (t->isProgram()) {
				actionUndo->setEnabled(t->isUndoAvailable());
				actionRedo->setEnabled(t->isRedoAvailable());
				ProgramTab *pt = dynamic_cast<ProgramTab *>(t);
				pt->restoreCursorPosition();
			}
		}
		timerRun->stop();
		sPos->setEnabled(true);
	} else if (mode == "analys") {

		app()->compiler->Modules.unLockGUI();
		setLockTabsEdit(false);
		sMode->setText(tr("Analys mode"));
		semaphoreWidget->hide();
		foreach (qa, listEditEn)
			if (!qa->isEnabled()) {
				qa->setEnabled(true);
			}
		foreach (qa, listEditDi)
			if (qa->isEnabled()) {
				qa->setEnabled(false);
			}
		foreach (qw, listEditEnW)
			if (!qw->isEnabled()) {
				qw->setEnabled(true);
			}
		foreach (qw, listEditDiW)
			if (qw->isEnabled()) {
				qw->setEnabled(false);
			}
		foreach (macro, app()->macros) {
			QAction *action = macro->action();
			if (action) {
				action->setEnabled(true);
			}
		}
		for (int i = 0; i < tabWidget->count(); i++) {
			Tab *t = dynamic_cast<Tab *>(tabWidget->widget(i));
			if (t->isProgram()) {
				actionUndo->setEnabled(t->isUndoAvailable());
				actionRedo->setEnabled(t->isRedoAvailable());
				ProgramTab *pt = dynamic_cast<ProgramTab *>(t);
				pt->restoreCursorPosition();
			}
		}
		timerRun->stop();
		KumirEdit *editor = dynamic_cast<ProgramTab *>(tabWidget->widget(0))->editor();
		if (editor != NULL) {
			clearRedAndGreenLines();
		}
		sPos->setEnabled(true);
	} else if (mode == "pause") {

		setLockTabsEdit(true);
		sMode->setText(tr("Pause mode"));
		semaphoreWidget->hide();
		foreach (qa, listPauseEn)
			if (!qa->isEnabled()) {
				qa->setEnabled(true);
			}
		foreach (qa, listPauseDi)
			if (qa->isEnabled()) {
				qa->setEnabled(false);
			}
		foreach (qw, listPauseEnW)
			if (!qw->isEnabled()) {
				qw->setEnabled(true);
			}
		foreach (qw, listPauseDiW)
			if (qw->isEnabled()) {
				qw->setEnabled(false);
			}
		foreach (macro, app()->macros) {
			macro->action()->setEnabled(false);
		}
		sPos->setEnabled(false);
	}
	m_formMode = mode;
}

void MainWindow::changeModeInterface(int mode)
{
	// Я сначала написал эту функцию,а потом заметил, что существует уже написанная
	// void MainWindow::setFormMode(const QString & mode). Перевожу туда.
	// app()->addEventToHistory("MainWindow::changeModeInterface("+QString::number(mode)+")");
	switch (mode) {
	case modeRun:
		setFormMode("run");
		break;
	case modeEdit:
		setFormMode("edit");
		break;
	case modeStep:
		setFormMode("run");
		break;
	case modeError:
		setFormMode("edit");
		break;
	case modePause:
		setFormMode("pause");
		break;
	case modeAnalys:
		setFormMode("analys");
	}
}

void MainWindow::clearRedAndGreenLines()
{
	KumirEdit *editor = dynamic_cast<ProgramTab *>(tabWidget->widget(0))->editor();
	editor->setRunLine(-1);
	editor->viewport()->update();
	//  if (editor->runLine() != -1) // "Ручное" перерисовавание
	//  {
	//      disconnect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
	//      int pos = editor->textCursor().position();
	//      for( ; editor->textCursor().blockNumber() > editor->runLine(); editor->moveCursor(QTextCursor::Up) );
	//      for( ; editor->textCursor().blockNumber() < editor->runLine(); editor->moveCursor(QTextCursor::Down) );
	//      editor->moveCursor(QTextCursor::EndOfLine);
	//      editor->textCursor().insertText("*");
	//      editor->textCursor().deletePreviousChar();
	//      QTextCursor qtc(editor->textCursor());
	//      qtc.setPosition(pos);
	//      editor->setTextCursor(qtc);
	//      connect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
	//  }
	//  if (editor->errorLine() != -1) // "Ручное" перерисовавание
	//  {
	//      disconnect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
	//      int pos = editor->textCursor().position();
	//      for( ; editor->textCursor().blockNumber() > editor->errorLine(); editor->moveCursor(QTextCursor::Up) );
	//      for( ; editor->textCursor().blockNumber() < editor->errorLine(); editor->moveCursor(QTextCursor::Down) );
	//      editor->moveCursor(QTextCursor::EndOfLine);
	//      editor->textCursor().insertText("*");
	//      editor->textCursor().deletePreviousChar();
	//      QTextCursor qtc(editor->textCursor());
	//      qtc.setPosition(pos);
	//      editor->setTextCursor(qtc);
	//      connect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
	//  }
	//  editor->setRunLine(-1);
}

Tab *MainWindow::tab(int i)
{
	if (i >= tabWidget->count()) {
		return NULL;
	} else {
		return dynamic_cast<Tab *>(tabWidget->widget(i));
	}
}

void MainWindow::macroBegin()
{
	// app()->addEventToHistory("MainWindow::macroBegin()");
	Macro *m = new Macro(this);
	app()->macros << m;
	Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
	tab->editor()->recordMacro(m);
	actionMacroBegin->setEnabled(false);
	actionMacroEnd->setEnabled(true);
//	sendMessage(tr("Macro is being recorded..."), 0);
	setRecordMacroMode(true);
}

void MainWindow::macroEnd()
{
	// app()->addEventToHistory("MainWindow::macroEnd()");
	clearMessage();
	Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
	tab->editor()->stopMacro();
	Macro *m = app()->macros.last();
	NewMacroDialog dialog(this, Qt::Dialog | Qt::WindowSystemMenuHint);
	actionMacroBegin->setEnabled(true);
	actionMacroEnd->setEnabled(false);
	setRecordMacroMode(false);
	if (dialog.exec() == QDialog::Accepted) {
		m->setName(dialog.macroName());
		m->setKeySequence(dialog.macroSequence());
		//m->setKeySequence(QKeySequence(Qt::Key_Escape, Qt::Key_Space));
		// TODO make check of macro's uniquess
		QAction *mAction = menu_Macro->addAction(m->name(), this, SLOT(macroPlay()), m->keySequence());
		mAction->setIcon(QIcon(":/icons/media-playback-start.png"));
		m->setAction(mAction);
	} else {
		app()->macros.pop_back();
		return;
	}
	QString curDir = app()->getMacroDir();
	uint macroNumber = 0;
	while (QFile::exists(curDir + "/macro" + QString::number(macroNumber) + ".mcr")) {
		macroNumber++;
	}
	QFile f(curDir + "/macro" + QString::number(macroNumber) + ".mcr");
	if (f.open(QIODevice::WriteOnly)) {
		QTextStream ts(&f);
		ts.setCodec("UTF-8");
		QDomDocument document("KumirMacro-1.5");
		app()->macros.last()->save(document);
		document.save(ts, 1);
		f.close();
		m->setFileName("macro" + QString::number(macroNumber) + ".mcr");
	}
}

void MainWindow::macroPlay()
{
	// app()->addEventToHistory("MainWindow::macroPlay()");
	QAction *from = dynamic_cast<QAction *>(sender());
	foreach (Macro *m, app()->macros + app()->standardMacros) {
		if (m->name() == from->text()) {
			Tab *tab = dynamic_cast<Tab *>(tabWidget->currentWidget());
			tab->editor()->playMacro(m);
		}
	}
	qDebug() << "play macro: " << from->text();
}

void MainWindow::macrosManage()
{
	// app()->addEventToHistory("MainWindow::macrosManage()");
	MacrosManagementDialog dialog(this, Qt::Dialog | Qt::WindowSystemMenuHint);
	dialog.exec();
	updateMacros();
}

void MainWindow::updateMacros()
{
	// app()->addEventToHistory("MainWindow::updateMacros()");
	if (menuMacro->isEmpty()) {
		foreach (Macro *m, app()->standardMacros) {
			QAction *mAction = menuMacro->addAction(m->name(), this, SLOT(macroPlay()), m->keySequence());
			m->setAction(mAction);
		}
	}
	for (; menu_Macro->actions().count() > 5 ;) {
		delete menu_Macro->actions().takeLast();
	}
	foreach (Macro *m, app()->macros) {
		QAction *mAction = menu_Macro->addAction(QIcon(":/icons/media-playback-start.png"), m->name(), this, SLOT(macroPlay()), m->keySequence());
		m->setAction(mAction);
	}
}

// void MainWindow::showReference()
// {
//  app()->addEventToHistory("MainWindow::showReference()");
//  app()->assistant->show();
// }

// void MainWindow::showQuickHelp()
// {
//  app()->addEventToHistory("MainWindow::showQuickHelp()");
//  TextEdit *editor = tab(tabWidget->currentIndex())->editor();
//  if (editor->textCursor().selectedText().isEmpty())
//  {
//      editor->moveCursor(QTextCursor::StartOfWord);
//      editor->moveCursor(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
//  }
//  app()->assistant->showTip(editor->textCursor().selectedText());
// }

void MainWindow::askCursorPosition()
{
	// app()->addEventToHistory("MainWindow::askCursorPosition()");
	GoToLineDialog *dialog = new GoToLineDialog(this);
	dialog->horizontalSlider->setMinimum(1);
	int current = tab(tabWidget->currentIndex())->editor()->textCursor().blockNumber() + 1;
	int max = tab(tabWidget->currentIndex())->editor()->document()->blockCount();
	dialog->horizontalSlider->setMaximum(max);
	dialog->spinBox->setMinimum(1);
	dialog->spinBox->setMaximum(max);
	dialog->horizontalSlider->setValue(current);
	if (dialog->exec() == QDialog::Accepted) {
		int goTo = dialog->horizontalSlider->value();
		QTextCursor cursor = tab(tabWidget->currentIndex())->editor()->textCursor();
		cursor.setPosition(0);
		while ((cursor.blockNumber() + 1) != goTo) {
			cursor.movePosition(QTextCursor::NextBlock);
		}
		tab(tabWidget->currentIndex())->editor()->setTextCursor(cursor);
	}
}




QString MainWindow::outputText()
{
	// app()->addEventToHistory("MainWindow::outputText()");
	return textEdit3->toPlainText();
}

void MainWindow::runEnds()
{
	// app()->addEventToHistory("MainWindow::runEnds()");
	outTextFile = "";
	inTextFile = "";
	if (outTextF.isOpen()) {
		outTextF.close();
	}
	if (inTextF.isOpen()) {
		inTextF.close();
	}

	if (app()->variablesWindow) {
		app()->debugDialog->refreshVariables();
	}

	ProgramTab *tab = (ProgramTab *)this->tab(0);
	tab->editor()->setUpdatesEnabled(true);
	tab->editor()->viewport()->repaint();
	// textEdit3->append("Runing finished");
	app()->finishBatch();
	stepModeRunning = false;
	showStepsAtRun();

	QString s(isTestingMode ? tr("Testing finished") : tr("Program finished"));
	isTestingMode = false;

	if (sMessage->text() != tr("Run error") && sMessage->text() != tr("Running is interrupted")) {
		showMessage(s, 0);
	}
	if (!runStopMessagePrinted) {
		textEdit3->stop(s, "gray");
		runStopMessagePrinted = true;
	}
	qDebug() << "Run ERR FLAG" << runErrorFlag;

	app()->compiler->modules()->module(2)->instrument()->SwitchMode(3);//Robot Analize
	emit changeModeInterface(modeAnalys);
	if (app()->isScriptTesting()) {
		KumSingleModule *module = app()->compiler->mainModule();
		int id = module->Values()->inTable(QString::fromUtf8("@тестирование"), "global");
		if (id < 0) {
			qDebug() << "No Debug perem";
		}
		int  mark = module->Values()->asInt(id);
		if (runErrorFlag) {
			mark = 1;
		}
		app()->TaskControl()->checkFinished(mark);
		if (mark > 7) {
			app()->continueCheck();
		} else {
			app()->endCheck();
		}


	};
	runErrorFlag = false;
	//clearMessage();
}

void MainWindow::runPause(int str, int module_id)
{
	// app()->addEventToHistory("MainWindow::runPause("+QString::number(str)+")");

	app()->debugDialog->refreshVariables();

	// Убрать зеленую строку со всех закладок
	for (int i = 0; i < tabWidget->count(); ++i)
		if (dynamic_cast<Tab *>(tabWidget->widget(i))->isProgram()) {
			dynamic_cast<ProgramTab *>(tabWidget->widget(i))->editor()->setRunLine(-1);
		}

	ProgramTab *messageTab = app()->compiler->modules()->module(module_id)->Proga_Value_Line(str).real_line.tab;
	messageTab->editor()->setRunLine(app()->compiler->modules()->module(module_id)->Proga_Value_Line(str).real_line.lineNo);
	tabWidget->setCurrentWidget(dynamic_cast<QWidget *>(messageTab));

	QTextCursor::MoveOperation dir =
		(messageTab->editor()->textCursor().blockNumber() < app()->compiler->compiledLine(str, module_id).real_line.lineNo) ?
		QTextCursor::NextBlock : QTextCursor::PreviousBlock;
	for (; messageTab->editor()->textCursor().blockNumber() != app()->compiler->compiledLine(str, module_id).real_line.lineNo;
		messageTab->editor()->moveCursor(dir));
	messageTab->editor()->ensureCursorVisible();

	messageTab->editor()->setUpdatesEnabled(true);
	messageTab->editor()->viewport()->repaint();
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->SwitchMode(4);
	app()->kumServer->sendClientReplay(QString::fromUtf8("Малый Шаг"), "RETURN," + QString::number(app()->compiler->compiledLine(str, module_id).real_line.lineNo) + '\n');
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(4);//Robot Analize
	emit changeModeInterface(modePause);
}

void MainWindow::runError(int error, int stroka, int module_id, const QString &sourceText)
{
	// app()->addEventToHistory("MainWindow::runError("+QString::number(error)+","+QString::number(stroka)+")");
	if (outTextF.isOpen()) {
		outTextF.close();
	}
	if (inTextF.isOpen()) {
		inTextF.close();
	}
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(3);//Robot Analize
	if (error == -1) {
		qDebug() << "Error -1!";
		emit changeModeInterface(modeError);
	} else {
		qDebug() << "Error !-1 ";
		if (isTestingMode) {
			app()->TaskControl()->checkFinished(1);
		};
		QString messageIO(isTestingMode ? tr("TESTING ERROR") : tr("RUNTIME ERROR"));
		int lineNo = app()->compiler->compiledLine(stroka, module_id).real_line.lineNo;
		bool isHidden = dynamic_cast<ProgramTab *>(tab(0))->editor()->isLineHidden(lineNo);
		if (!isHidden) {
			QTextCursor cursor = textEdit3->textCursor();
			if (!cursor.atEnd()) {
				textEdit3->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
			}
			textEdit3->ensureCursorVisible();
			ProgramTab *messageTab = app()->compiler->compiledLine(stroka, module_id).real_line.tab;

			if (!runStopMessagePrinted) {
				textEdit3->stop(messageIO + ": " + app()->messagesProvider->errorMessage(error), "red");
				runStopMessagePrinted = true;
			}
			runErrorFlag = true;
			messageTab->editor()->setErrorLine(app()->compiler->compiledLine(stroka, module_id).real_line.lineNo);

			QTextCursor::MoveOperation dir =
				(messageTab->editor()->textCursor().blockNumber() < app()->compiler->compiledLine(stroka, module_id).real_line.lineNo) ?
				QTextCursor::NextBlock : QTextCursor::PreviousBlock;
			for (; messageTab->editor()->textCursor().blockNumber() != app()->compiler->compiledLine(stroka, module_id).real_line.lineNo; messageTab->editor()->moveCursor(dir));
			messageTab->editor()->ensureCursorVisible();

			messageTab->editor()->setUpdatesEnabled(true);
			messageTab->editor()->viewport()->repaint();

			app()->compiler->modules()->module(2)->instrument()->SwitchMode(3);//Robot Analize
		} else {
			QTextCursor cursor = textEdit3->textCursor();
			if (!cursor.atEnd()) {
				textEdit3->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
			}
			textEdit3->ensureCursorVisible();
			if (!runStopMessagePrinted) {
				textEdit3->stop(messageIO + ": " + app()->messagesProvider->errorMessage(error) + "\n\t" + sourceText, "red");
				runStopMessagePrinted = true;
			}

		}

		app()->finishBatch();
		stepModeRunning = false;
		showStepsAtRun();
		showMessage(tr("Run error"), 0);
		emit changeModeInterface(modeAnalys);
		isTestingMode = false;
	}
}

void MainWindow::debugMessage(QString message, int stroka, int module_id)
{
	//// app()->addEventToHistory("MainWindow::debugMessage("+message+","+QString::number(stroka)+")");
	//TODO debugMessages
	ProgramTab *messageTab = app()->compiler->compiledLine(stroka, module_id).real_line.tab;
	if (messageTab == NULL) {
		return;
	}
	KumirMargin *field = messageTab->margin();
	//QList<proga> *pv = app()->compiler->ProgaValue();
	int p = 0;
	int l = app()->compiler->compiledLine(stroka, module_id).real_line.lineNo;
	stroka--;
	while (stroka > 0) {
		RealLine rl = app()->compiler->compiledLine(stroka, module_id).real_line;
		if (rl.tab == NULL || rl.lineNo != l) {
			break;
		}
		p++;
		stroka--;
	}
	field->setDebugText(message, l, p);
	emit Runsync();

	sCount->setText(tr("Steps count: ") + QString::number(countSteps));
	if (app()->variablesWindow) {
		app()->debugDialog->refreshVariables();
	}
};

void MainWindow::closeAllInstruments()
{
	// app()->addEventToHistory("MainWindow::closeAllInstruments()");
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Robot" || app()->compiler->modules()->module(i)->Name() == "Draw") {
			if (app()->compiler->modules()->module(i)->isEnabled()) {
				app()->compiler->modules()->module(i)->instrument()->showHideWindow(false);
			}
		}
	}
}

void MainWindow::textMessage(QString text)
{

	if (outTextFile != "") {
		outTextF.write(text.toUtf8());
		return;
	};

	textEdit3->output(text);
}

void MainWindow::startTesting()
{
	// app()->addEventToHistory("MainWindow::startTesting()");
	app()->compiler->start();
	bool isTestingExist = false;
	for (int i = 0; i < app()->compiler->Modules.lastModuleId() + 1; ++i)
		if (app()->compiler->Modules.module(i)->Name() == "@main")
			for (int j = 0; j < app()->compiler->Modules.module(i)->Functions()->size(); ++j) {
				qDebug() << app()->compiler->Modules.module(i)->Functions()->nameById(j);
				if (app()->compiler->Modules.module(i)->Functions()->nameById(j) == QString::fromUtf8("@тестирование")) {
					isTestingExist = true;
					break;
				}
			}
	if (isTestingExist) {
		app()->doCompilation();
		isTestingMode = true;
		StartFromAlg(QString::fromUtf8("@тестирование"), -1);
	} else {
		app()->TaskControl()->checkFinished(0);

		QMessageBox::warning(this, tr("Testing"), tr("There is no testing algorithm"));

		app()->endCheck();


	}
}

void MainWindow::StartRun()
{
	// app()->addEventToHistory("MainWindow::StartRun()");
	if (!app()->run->isRunning()) {
		outTextFile = "";
		inTextFile = "";
		if (outTextF.isOpen()) {
			outTextF.close();
		}
		if (inTextF.isOpen()) {
			inTextF.close();
		}

		app()->emergencySaveState();
		textEdit3->reset(tabWidget->tabText(0));
		app()->doCompilation();
		//app()->compiler->M->resetInstruments();//TODO Modules
		RunSetup neprerivnoSetup(app()->compiler->modules(), textEdit3, textEdit3, this);
		neprerivnoSetup.setMode(neprerivno);
		neprerivnoSetup.setFuncMode(neprerivno);
		neprerivnoSetup.setStart(app()->compiler->StartPos());
		KumModules *modules = app()->compiler->modules();
		int main_id = modules->idByName("@main");
		app()->run->init(neprerivnoSetup, main_id);
		// textEdit3->append("Runing Started");
		countSteps = 0;

		app()->compiler->modules()->showGraphicsInstr();
		runStopMessagePrinted = false;
		app()->startProgram();
		stepModeRunning = false;
		app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
		emit changeModeInterface(modeRun);
		return;
	} else {
		app()->run->setMode(neprerivno);
		emit FuncDebugMode(2);//2 - neprerivno
		emit Continue();
	};
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		if (tab->isProgram()) {
			KumirEdit *editor = dynamic_cast<KumirEdit *>(tab->editor());
			if (editor->runLine() != -1) {
				int rl = editor->runLine();
				editor->setRunLine(-1);
				disconnect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
				int pos = editor->textCursor().position();
				for (; editor->textCursor().blockNumber() > rl; editor->moveCursor(QTextCursor::Up));
				for (; editor->textCursor().blockNumber() < rl; editor->moveCursor(QTextCursor::Down));
				editor->moveCursor(QTextCursor::EndOfLine);
				editor->textCursor().insertText("*");
				editor->textCursor().deletePreviousChar();
				QTextCursor qtc(editor->textCursor());
				qtc.setPosition(pos);
				editor->setTextCursor(qtc);
				connect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
			}
		}
	}
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
	emit changeModeInterface(modeRun);

}

void MainWindow::debugRun()
{
	// app()->addEventToHistory("MainWindow::debugRun()");
	if (!app()->run->isRunning()) {
		outTextFile = "";
		inTextFile = "";
		if (outTextF.isOpen()) {
			outTextF.close();
		}
		if (inTextF.isOpen()) {
			inTextF.close();
		}
		app()->emergencySaveState();
		textEdit3->reset(tabWidget->tabText(0));
		app()->doCompilation();
		//app()->compiler->ispTable->resetInstruments();TODO Modules
		KumModules *mods = app()->compiler->modules();
		RunSetup DebugSetup(mods, textEdit3, textEdit3, this);
		DebugSetup.setMode(debug);
		DebugSetup.setStart(app()->compiler->StartPos());
		int mod_id = mods->idByName("@main");
		app()->run->init(DebugSetup, mod_id);
		app()->compiler->modules()->showGraphicsInstr();
		// textEdit3->append("Runing Started");
		emit FuncDebugMode(4);
		countSteps = 0;
		runStopMessagePrinted = false;
		app()->startProgram();
		app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
		emit changeModeInterface(modeRun);
		return;
	} else {
		app()->run->setMode(debug);
		emit FuncDebugMode(4);//4 - debug
		emit Continue();
	};
	for (int i = 0; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		if (tab->isProgram()) {
			KumirEdit *editor = dynamic_cast<KumirEdit *>(tab->editor());
			if (editor->runLine() != -1) {
				int rl = editor->runLine();
				editor->setRunLine(-1);
				disconnect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
				int pos = editor->textCursor().position();
				for (; editor->textCursor().blockNumber() > rl; editor->moveCursor(QTextCursor::Up));
				for (; editor->textCursor().blockNumber() < rl; editor->moveCursor(QTextCursor::Down));
				editor->moveCursor(QTextCursor::EndOfLine);
				editor->textCursor().insertText("*");
				editor->textCursor().deletePreviousChar();
				QTextCursor qtc(editor->textCursor());
				qtc.setPosition(pos);
				editor->setTextCursor(qtc);
				connect(editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()));
			}
		}
	}
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
	emit changeModeInterface(modeRun);
}

void MainWindow::vnutrVizova()
{
	// app()->addEventToHistory("MainWindow::vnutrVizova()");
	/*if(state)
	emit FuncDebugMode(1);//Po shagam
	else
	emit FuncDebugMode(2);//neprerivno*/
	if (!app()->run->isRunning()) {
		shagRun();
		emit FuncDebugMode(1);
		return;
	};
	emit FuncDebugMode(1);//Po shagam
	emit Continue();
};

void MainWindow::StartFromAlg(QString alg_name, int client_id)
{
	// app()->addEventToHistory("MainWindow::StartFromAlg()");

	if (app()->run->isRunning()) {
		if (client_id > -1) {
			app()->kumServer->sendMessage(client_id, "ERROR,Program running");
		}
		return;
	};
	outTextFile = "";
	inTextFile = "";
	if (outTextF.isOpen()) {
		outTextF.close();
	}
	if (inTextF.isOpen()) {
		inTextF.close();
	}
	app()->emergencySaveState();
	textEdit3->reset(tabWidget->tabText(0), alg_name == QString::fromUtf8("@тестирование"));
	//app()->doCompilation(0);
	int mod_id, f_id, start_pos;
	app()->compiler->modules()->FuncByName(alg_name, &mod_id, &f_id);
	if (mod_id < 1 || f_id < 0) {
		return;
	}
	start_pos = app()->compiler->modules()->module(mod_id)->Functions()->posById(f_id);
	if (start_pos < 0) {
		return;
	}
	RunSetup neprerivnoSetup(app()->compiler->modules(), textEdit3, textEdit3, this);
	neprerivnoSetup.setMode(neprerivno);
	neprerivnoSetup.setFuncMode(neprerivno);
	neprerivnoSetup.setStart(start_pos);
	if (isTestingMode) {
		neprerivnoSetup.setInitMain(true);
	}

	app()->run->init(neprerivnoSetup, app()->compiler->modules()->idByName("@main"));
	app()->run->setSkipHiddenAlgorhitm(true);
	// textEdit3->append("Runing Started");
	countSteps = 0;

	app()->compiler->modules()->showGraphicsInstr();
	runStopMessagePrinted = false;
	app()->startProgram();
	stepModeRunning = false;
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
	emit changeModeInterface(modeRun);
};



void MainWindow::shagOut()
{
	// app()->addEventToHistory("MainWindow::shagOut()");
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
	emit changeModeInterface(modeRun);
}

void MainWindow::shagRun()
{
	// app()->addEventToHistory("MainWindow::shagRun()");
	stepModeRunning = true;
	if (!app()->run->isRunning()) {
		outTextFile = "";
		inTextFile = "";
		if (outTextF.isOpen()) {
			outTextF.close();
		}
		if (inTextF.isOpen()) {
			inTextF.close();
		}
		app()->emergencySaveState();
		textEdit3->reset(tabWidget->tabText(0));
		app()->doCompilation();
		//app()->compiler->ispTable->resetInstruments();//TODO Modules
		RunSetup neprerivnoSetup(app()->compiler->modules(), textEdit3, textEdit3, this);
		neprerivnoSetup.setMode(po_shagam);
		neprerivnoSetup.setFuncMode(neprerivno);
		neprerivnoSetup.setStart(app()->compiler->StartPos());
		int module_id = app()->compiler->modules()->idByName("@main");
		app()->run->init(neprerivnoSetup, app()->compiler->modules()->idByName("@main"));
		// textEdit3->append("Runing Started");
		if (actionStepInto->isChecked()) {
			emit FuncDebugMode(1);
		}
		countSteps = 0;
		app()->compiler->modules()->showGraphicsInstr();
		runStopMessagePrinted = false;
		app()->startProgram();
		QString lastClientCommand = QString::fromUtf8("Малый Шаг");
		int lineNo = 0;
		int startModuleNo = module_id;
		int startInstructionNo = app()->compiler->StartPos();
		if (!app()->compiler->isEmpty(startModuleNo)) {
			lineNo = app()->compiler->compiledLine(startInstructionNo, startModuleNo).real_line.lineNo;
		}
		QString reply = "RETURN," + QString::number(lineNo) + '\n';
		app()->kumServer->sendClientReplay(lastClientCommand, reply);
	} else {
		emit FuncDebugMode(2);
		emit Continue();
	};
	showStepsAtRun();
	app()->compiler->modules()->module(2)->instrument()->SwitchMode(1);//Robot Analize
	emit changeModeInterface(modeStep);
}

void MainWindow::doInput(QString format)
{
	if (inTextFile != "") {
		this->activateWindow();
		textEdit3->activateWindow();
		if (!inTextF.atEnd()) {
			QString inp = QString::fromUtf8(inTextF.readLine());
			QString error;
			QStringList result;
			qDebug() << "inp:" << inp << " format" << format;
			KumTools::instance()->parceInputString(format, inp, result, error);

			if (error != "") {
				textEdit3->append(trUtf8("Ошибка ввода из файла:") + inTextFile);
				emit setRunError(error);
				return;
			}

			emit InputOk(result);
		} else {
			textEdit3->append(trUtf8("Ошибка ввода из файла:") + inTextFile);
			emit setRunError(trUtf8("Файл закончился."));
			qDebug() << "ERROR EMIT";
		};
		return;
	};
	// app()->addEventToHistory("MainWindow::doInput("+format+")");
//  isChar=isCharct;
//  waitInput=true;
	//
//  inpCurs=textEdit3->textCursor();
//  if(!inpCurs.atEnd())textEdit3->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor);
//  inpCurs=textEdit3->textCursor();
//  oldBlockId=inpCurs.blockNumber();
//  oldCursPos=inpCurs.position () ;
//  connect ( textEdit3, SIGNAL(cursorPositionChanged()), this, SLOT(InputOk()));
//  this->activateWindow();
	textEdit3->setFocus();
	textEdit3->input(format);

};

void MainWindow::doKeyInput(bool ignoreModifiers)
{
//  this->activateWindow();
	raise();
	textEdit3->setFocus();
	textEdit3->scanKey(ignoreModifiers);
}

void MainWindow::keyInputFinished(uint code, bool shift, bool control, bool alt, bool meta)
{
	emit inputKeyOk(code, shift, control, alt, meta);
}


void MainWindow::InstrumentLoaded(int isp_id)
{
	// app()->addEventToHistory("MainWindow::InstrumentLoaded("+QString::number(isp_id)+")");
	if (app()->compiler->Modules.module(isp_id)->Name() == "Robot") {
		emit RoboSig(true);
		{
			//actionRobot->setEnabled(true);
			//actionRobot->setChecked ( false);
//      connect(actionRobot,SIGNAL(triggered()),
//           app()->compiler->Modules.module(isp_id)->instrument(),
//           SLOT(showHideWindow()));
			//app()->compiler->Modules.module(isp_id)->instrument()->showHideWindow(false);
		};
		qDebug("Robosig");
	}
//if (app()->compiler->ispTable->getIspNamebyId(isp_id)=="Draw")emit DrawSig(true);

	if (app()->compiler->Modules.module(isp_id)->Name() == "Draw") {
		emit DrawSig(true);
		{
			//actionDraw->setEnabled(true);
			//actionDraw->setChecked ( false);
//               connect(actionDraw,SIGNAL(triggered(bool)),
//                               app()->compiler->Modules.module(isp_id)->instrument(),
//                               SLOT(showHideWindow()));
			//app()->compiler->Modules.module(isp_id)->instrument()->showHideWindow(false);
		};
		qDebug("Drawsig");
	}


//if (app()->compiler->ispTable->getIspNamebyId(isp_id)=="Files")emit FilesSig(true);
};

void MainWindow::InstrumentUnLoaded(int isp_id)
{
	// app()->addEventToHistory("MainWindow::InstrumentUnLoaded("+QString::number(isp_id)+")");
	if (app()->compiler->Modules.module(isp_id)->Name() == "Robot") {
		//actionRobot->setEnabled(false);
//          disconnect(actionRobot,SIGNAL(triggered()),
//           app()->compiler->Modules.module(isp_id)->instrument(),
//           SLOT(showHideWindow()));
		emit RoboSig(false);
	};
	if (app()->compiler->Modules.module(isp_id)->Name() == "Draw") {
		//actionDraw->setEnabled(false);
//      disconnect(actionDraw,SIGNAL(triggered()),
//                           app()->compiler->Modules.module(isp_id)->instrument(),
//                           SLOT(showHideWindow()));
		emit DrawSig(false);
	}
//if (app()->compiler->ispTable->getIspNamebyId(isp_id)=="Files")emit FilesSig(false);
};

void MainWindow::addIspToText(const QString &name)
{
	qDebug("addIspToText: %s", name.toUtf8().data());
	ProgramTab *pt = qobject_cast<ProgramTab *>(tabWidget->widget(0));
	KumirEdit *editor = pt->editor();
	QTextBlock block = editor->document()->begin();
	bool exists = false;
	int no = -1;
	int b = 0;
	QString use = trUtf8("использовать ");
	for (block = editor->document()->begin(); block != editor->document()->end(); block = block.next()) {
		QString text = block.text();
		while (text.startsWith("|") || text.startsWith(" ")) {
			text.remove(0, 1);
		}
		if (text.startsWith(use)) {
			QString cName;
			bool inString = false;
			QChar quote;
			for (int i = use.length(); i < text.length(); i++) {
				if (inString) {
					if (text[i] == quote) {
						inString = false;
					}
				} else {
					if (text[i] == '\'' || text[i] == '"') {
						quote = text[i];
						inString = true;
					} else if (text[i] == '|') {
						break;
					} else {
						cName += text[i];
					}
				}
			}
			cName = cName.simplified();
			if (cName == name) {
				exists = true;
				no = b;
				break;
			} else {
				if (name.toLower().simplified() == trUtf8("робот") && cName.toLower().simplified() == trUtf8("робот строитель")) {
					exists = true;
					no = b;
					break;
				}
			}
		}
		b++;
	}
	if (!exists) {
		editor->insertLine(0, use + name);
	} else {

		block = editor->document()->begin();
		for (int i = 0; i < no; i++) {
			block = block.next();
		}
		QString text = block.text();
		if (text.startsWith("|")) {
			int c = 0;
			while (text.startsWith("|") || text.startsWith(" ")) {
				if (text.startsWith("|")) {
					c++;
				}
				text.remove(0, 1);
			}
			editor->uncommentLine(no, c);
		}
	}
}

void MainWindow::removeIspFromText(const QString &name)
{
	qDebug("removeIspFromText: %s", name.toUtf8().data());
	ProgramTab *pt = qobject_cast<ProgramTab *>(tabWidget->widget(0));
	KumirEdit *editor = pt->editor();
	QTextBlock block = editor->document()->begin();
	bool exists = false;
	int no = -1;
	int b = 0;
	QString use = trUtf8("использовать ");
	for (block = editor->document()->begin(); block != editor->document()->end(); block = block.next()) {
		QString text = block.text();
		if (text.startsWith(use)) {
			QString cName;
			bool inString = false;
			QChar quote;
			for (int i = use.length(); i < text.length(); i++) {
				if (inString) {
					if (text[i] == quote) {
						inString = false;
					}
				} else {
					if (text[i] == '\'' || text[i] == '"') {
						quote = text[i];
						inString = true;
					} else if (text[i] == '|') {
						break;
					} else {
						cName += text[i];
					}
				}
			}
			cName = cName.simplified();
			if (cName == name) {
				exists = true;
				no = b;
				break;
			} else {
				if (name.toLower().simplified() == trUtf8("робот") && cName.toLower().simplified() == trUtf8("робот строитель")) {
					exists = true;
					no = b;
					break;
				}
			}
		}
		b++;
	}
	if (!exists) {

	} else {
		editor->commentLine(no);
	}
}


void MainWindow::InputOk(const QStringList &texts)
{
//  app()->addEventToHistory ( "MainWindow::InputOk("+text+")" );
	emit inputOk(texts);
//  int oldBlId=oldBlockId;
//  int newBlId=textEdit3->textCursor().blockNumber();
//  if ( oldBlockId <
//          textEdit3->textCursor().blockNumber() )
//  {
//      QTextBlock tBlock=textEdit3->document()->findBlock ( oldCursPos );
//      QString inpText=tBlock.text();
	//
//      waitInput=false;
//      emit inputOk ( inpText ); //TODO Часть текста
//      disconnect ( textEdit3, SIGNAL ( cursorPositionChanged() ), this, SLOT ( InputOk() ) );
//  }
//  else
//  {
//      return;
//  };
}

void MainWindow::showCursorPosition(int row, int col)
{
//	app()->addEventToHistory("MainWindow::showCursorPosition("+QString::number(row)+","+QString::number(col)+")");
	QString r(QString::number(row + 1));
	QString c(QString::number(col + 1));
	sPos->setText(tr("Line") + ": " + r + ", " + tr("Symbol") + ": " + c);
}


void MainWindow::refreshCountErrors(int count)
{
	//app()->addEventToHistory("MainWindow::refreshCountErrors("+QString::number(count)+")");
	// Палитра почему-то слетает. Мигает красным только один раз при запуске
	/*QPalette palette(sCount->palette());
	palette.setColor(QPalette::Window,Qt::red);
	sCount->setPalette(palette);*/
	if (count == -1) {
		count = countErrors;
	} else {
		countErrors = count;
	}
	QString s = (count == 0) ? tr("No errors") : ("<font color=red>" + tr("Errors: ") + QString::number(count) + "</font>");
	sCount->setText(s);
}

void MainWindow::editOutput()
{
	// app()->addEventToHistory("MainWindow::editOutput()");
	fileNewText();
	TextTab *t = dynamic_cast<TextTab *>(tabWidget->widget(tabWidget->count() - 1));
	QString label(tr("Program output"));
	int n = 0;
	bool flag;
	do {
		++n;
		flag = false;
		for (int i = 0; i < tabWidget->count(); ++i) {
			QString s(tabWidget->tabText(i));
			if (s.startsWith(label) && s.endsWith(" " + QString::number(n))) {
				flag = true;
			}
		}
	} while (flag);
	label += " " + QString::number(n);
	tabWidget->setTabText(tabWidget->count() - 1, label);
	setWindowTitle(tr("Program output") + " - " + tr("Kumir"));
	t->setEditorText(textEdit3->toPlainText());
}

void MainWindow::closeTab(int no)
{
	// app()->addEventToHistory("MainWindow::closeTab("+QString::number(no)+")");
	tabWidget->setCurrentIndex(no);
	//fileClose();
}

void MainWindow::showMessage(QString str, int time)
{
	sMessage->setText(str);
	sMessage1->setText(str);
	sMessage1->adjustSize();
	if (sMessage->width() < sMessage1->width()) {
		sMessage->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	} else {
		sMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}

	if (time > 0) {
		timer->start(time);
	} else {
		timer->stop();
	}
}

void MainWindow::clearMessage()
{
	sMessage->setText("");
}

void MainWindow::showSteps()
{
	++countSteps;
	/*if (stepModeRunning)
	{
	    sCount->setText(tr("Steps count: ")+QString::number(countSteps));
	}*/
}

void MainWindow::showStepsAtRun()
{
	sCount->setText(tr("Steps count: ") + QString::number(countSteps));
	//if (app()->variablesWindow)
	//app()->debugDialog->refreshVariables();
}

void MainWindow::findText()
{
	if (!tab(tabWidget->currentIndex())->editor()->find(findPanel->getText(), findPanel->getOptions())) {
		showMessage(tr("Text not found"));
		//QMessageBox::critical(this, tr("Find"), tr("Text not found"));
		tab(tabWidget->currentIndex())->editor()->setFocus();
	}
}

void MainWindow::replaceText()
{
	TextEdit *editor = tab(tabWidget->currentIndex())->editor();
	if (QString::compare(editor->textCursor().selectedText(), findPanel->getText(), findPanel->isCaseSensitivity()) == 0) {
		QString txt(editor->toPlainText());
		int s = editor->textCursor().selectionStart();
		int e = editor->textCursor().selectionEnd();
		txt.replace(s, e - s, findPanel->getNewText());
		editor->changeText(txt);
		QTextCursor qtc(editor->textCursor());
		qtc.setPosition(s);
		qtc.setPosition(s + findPanel->getNewText().length(), QTextCursor::KeepAnchor);
		editor->setTextCursor(qtc);
		if (!tab(tabWidget->currentIndex())->editor()->find(findPanel->getText(), findPanel->getOptions())) {
			showMessage(tr("No more text for replacing"));
		}
		tab(tabWidget->currentIndex())->editor()->setFocus();
	} else {
		showMessage(tr("No found text for replacing"));
	}
}

void MainWindow::replaceAllNext()
{
	TextEdit *editor = tab(tabWidget->currentIndex())->editor();
	QString txt(editor->toPlainText());
	int pos = editor->textCursor().position();
	int s = pos;
	int i;
	int count = 0;
	do {
		i = txt.indexOf(findPanel->getText(), s, findPanel->isCaseSensitivity());
		if (i != -1) {
			txt.replace(i, findPanel->getText().length(), findPanel->getNewText());
			s = i + findPanel->getNewText().length();
			++count;
		}
	} while (i != -1);
	if (count > 0) {
		editor->changeText(txt);
		QTextCursor qtc(editor->textCursor());
		qtc.setPosition(pos);
		editor->setTextCursor(qtc);
	}
	showMessage(tr("Occurrences replaced: ") + QString::number(count));
}

void MainWindow::replaceAllPrev()
{
	TextEdit *editor = tab(tabWidget->currentIndex())->editor();
	QString txt(editor->toPlainText());
	int pos = editor->textCursor().position();
	int s = 0;
	int i;
	int count = 0;
	do {
		i = txt.indexOf(findPanel->getText(), s, findPanel->isCaseSensitivity());
		if (i != -1 && i < pos) {
			txt.replace(i, findPanel->getText().length(), findPanel->getNewText());
			s = i + findPanel->getNewText().length();
			pos += (findPanel->getNewText().length() - findPanel->getText().length());
			++count;
		}
	} while (i != -1 && i < pos);
	if (count > 0) {
		editor->changeText(txt);
		QTextCursor qtc(editor->textCursor());
		qtc.setPosition(pos);
		editor->setTextCursor(qtc);
	}
	showMessage(tr("Occurrences replaced: ") + QString::number(count));
}

void MainWindow::changeAnalysToEdit()
{
	if (m_formMode == "analys") {
		KumirEdit *editor = dynamic_cast<ProgramTab *>(tabWidget->currentWidget())->editor();
		if (editor->errorLine() != -1) { // "Ручное" перерисовавание
			editor->viewport()->update();
//			disconnect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
//			int pos = editor->textCursor().position();
//			for( ; editor->textCursor().blockNumber() > editor->errorLine(); editor->moveCursor(QTextCursor::Up) );
//			for( ; editor->textCursor().blockNumber() < editor->errorLine(); editor->moveCursor(QTextCursor::Down) );
//			editor->moveCursor(QTextCursor::EndOfLine);
//			editor->textCursor().insertText("*");
//			editor->textCursor().deletePreviousChar();
//			QTextCursor qtc(editor->textCursor());
//			qtc.setPosition(pos);
//			editor->setTextCursor(qtc);
//			connect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
		}
		if (editor->runLine() != -1) { // "Ручное" перерисовавание
//			disconnect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
//			int pos = editor->textCursor().position();
//			for( ; editor->textCursor().blockNumber() > editor->runLine(); editor->moveCursor(QTextCursor::Up) );
//			for( ; editor->textCursor().blockNumber() < editor->runLine(); editor->moveCursor(QTextCursor::Down) );
//			editor->moveCursor(QTextCursor::EndOfLine);
//			editor->textCursor().insertText("*");
//			editor->textCursor().deletePreviousChar();
//			QTextCursor qtc(editor->textCursor());
//			qtc.setPosition(pos);
//			editor->setTextCursor(qtc);
//			connect( editor, SIGNAL(textChanged()), this, SLOT(changeAnalysToEdit()) );
			editor->viewport()->update();
		}
		editor->setRunLine(-1);
		editor->setErrorLine(-1);
		editor->viewport()->update();
		changeModeInterface(modeEdit);
		clearMessage();
		refreshCountErrors();
		this->repaint();
	}
}

void MainWindow::addToRecent(const QString &fullPath)
{
	QStringList list = app()->settings->value("RecentFiles").toStringList();
	bool found = list.contains(fullPath);
	if (!found) {
		list.prepend(fullPath);
	} else {
		int index = list.indexOf(fullPath);
		list.removeAt(index);
		list.prepend(fullPath);
	}
	while (list.count() > 5) {
		list.removeLast();
	}
	app()->settings->setValue("RecentFiles", list);

	foreach (QAction *qa, menuOpenRecent->actions()) {
		qa->disconnect(this);
	}
	for (; !menuOpenRecent->actions().isEmpty() ; menuOpenRecent->removeAction(menuOpenRecent->actions().last()));
	//menuOpenRecent->clear();
	int n = 1;
	foreach (QString path, list) {
		QStringList paths = path.split("/");
		menuOpenRecent->addAction("&" + QString::number(n) + " " + paths.last(), this, SLOT(openRecent()));
		n++;
	}
}

void MainWindow::loadRecentList()
{
	QStringList list = app()->settings->value("RecentFiles").toStringList();
	if (list.isEmpty()) {
		return;
	}
	menuOpenRecent->clear();
	int n = 1;
	foreach (QString path, list) {
		if (QFile::exists(path)) {
			QStringList paths = path.split("/");
			if ((app()->kumirStyle() == 'f') || ((app()->kumirStyle() == 't') && (!paths.last().endsWith(".kisp"))) || ((app()->kumirStyle() == 'c') && (!paths.last().endsWith(".kisp") && (!paths.last().endsWith(".txt"))))) {
				QAction *action = menuOpenRecent->addAction("&" + QString::number(n) + " " + paths.last(), this, SLOT(openRecent()));
				action->setToolTip(path);
				connect(action, SIGNAL(hovered()), this, SLOT(showRecentFilePath()));
			}
			n++;
		}
	}
}

void MainWindow::showRecentFilePath()
{
	QString path;
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != NULL) {
		path = action->toolTip();
		showingRecentFilePath = true;
		showMessage(tr("Load") + " " + path, 0);
	}
}

void MainWindow::hideRecentFilePath()
{
	if (showingRecentFilePath) {
		clearMessage();
	}
	showingRecentFilePath = false;
}

void MainWindow::openRecent()
{
	QAction *s = qobject_cast<QAction *>(sender());
	QString txt = s->text();
	txt.remove(0, 1);
	QStringList words = txt.split(" ");
	int no = words[0].toInt();
	QStringList list = app()->settings->value("RecentFiles").toStringList();
	QString fileName = list[no - 1];
	fileOpenInCurrentTab(fileName);
	app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
	curFileInfo = QFileInfo(fileName);
}

void MainWindow::paste()
{
	if (textEdit3->hasFocus()) {
		textEdit3->paste();
	} else {
		int activeTabNo = tabWidget->currentIndex();
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(activeTabNo));
		tab->paste();
	}
}

void MainWindow::showOverwriteMode(bool)
{
	QTextEdit *editor = qobject_cast<QTextEdit *>(sender());
	if (editor != NULL) {
		if (editor->overwriteMode()) {
			sInsert->setText(tr("OVR"));
		} else {
			sInsert->setText(tr("INS"));
		}
	}
}

void MainWindow::changeOverwriteMode()
{
	int tabNo = tabWidget->currentIndex();
	Tab *t = dynamic_cast<Tab *>(tabWidget->widget(tabNo));
	t->editor()->setOverwriteMode(! t->editor()->overwriteMode());
	if (t->editor()->overwriteMode()) {
		sInsert->setText(tr("OVR"));
	} else {
		sInsert->setText(tr("INS"));
	}
}

void MainWindow::stopRunning()
{
	sendMessage(tr("Running is interrupted"), 0);
	if (!runStopMessagePrinted) {
		textEdit3->stop(tr("Running is interrupted"), "gray");
		runStopMessagePrinted = true;
	}
	runErrorFlag = true;
	app()->run->stop_run();
}

void MainWindow::print()
{
	QPrinter *printer = new QPrinter;
	QPrintDialog printDialog(printer, this);
	if (printDialog.exec() == QDialog::Accepted) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(tabWidget->currentIndex()));
		tab->print(printer);
	}
}

void MainWindow::removeModulesFromTable()
{
	qDebug() << app()->compiler->modules()->lastModuleId();
	ProgramTab *ptab = 0;
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "@main") {
			ptab = app()->compiler->modules()->module(i)->Tab();
			break;
		}
	}
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Tab() == ptab && app()->compiler->modules()->module(i)->Name() != "@main" && ptab != NULL) {
			//qDebug() << i << app()->compiler->modules()->module(i)->Name() << " will be deleted from table";
			app()->compiler->modules()->removeModule(i);
		}
	}

}

void MainWindow::arrangeHorizontally()
{
	//int descW=qApp->desktop()->size().width();
	//int descH=qApp->desktop()->size().height();
	int descW = qApp->desktop()->availableGeometry().width();
	int descH = qApp->desktop()->availableGeometry().height();
	bool flagDraw = false, flagRobot = false;

	int robotH = 0;
	int drawW = 0, drawH = 0;
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Draw") {
			if (app()->compiler->modules()->module(i)->isEnabled()) {
				flagDraw = true;
				drawW = app()->compiler->modules()->module(i)->instrument()->getWindowGeometry().width();
				drawH = app()->compiler->modules()->module(i)->instrument()->getWindowGeometry().height();
				app()->compiler->modules()->module(i)->instrument()->setWindowPos(0, descH - drawH - 28);
				app()->compiler->modules()->module(i)->instrument()->showHideWindow(true);
				//              this->activateWindow();
			}
		}
	}
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Robot") {
			if (app()->compiler->modules()->module(i)->isEnabled()) {
				flagRobot = true;
				robotH = app()->compiler->modules()->module(i)->instrument()->getWindowGeometry().height();
				app()->compiler->modules()->module(i)->instrument()->setWindowPos(drawW + 8, descH - robotH - 28);
				app()->compiler->modules()->module(i)->instrument()->showHideWindow(true);
				//              this->activateWindow();
			}
		}
	}
	if (flagDraw || flagRobot) {
		this->move(0, 0);
		this->resize(descW, descH * 2 / 3);
	}
}

void MainWindow::arrangeVertically()
{
	//int descW=qApp->desktop()->size().width();
	//int descH=qApp->desktop()->size().height();
	int descW = qApp->desktop()->availableGeometry().width();
	int descH = qApp->desktop()->availableGeometry().height();
	bool flagDraw = false, flagRobot = false;

	int robotW = 0;
	int drawW = 0, drawH = 0;
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Draw") {
			if (app()->compiler->modules()->module(i)->isEnabled()) {
				flagDraw = true;
				drawW = app()->compiler->modules()->module(i)->instrument()->getWindowGeometry().width();
				drawH = app()->compiler->modules()->module(i)->instrument()->getWindowGeometry().height();
				app()->compiler->modules()->module(i)->instrument()->setWindowPos(descW - drawW - 7, 0);
				app()->compiler->modules()->module(i)->instrument()->showHideWindow(true);
				//              this->activateWindow();
			}
		}
	}
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->modules()->module(i)->Name() == "Robot") {
			if (app()->compiler->modules()->module(i)->isEnabled()) {
				flagRobot = true;
				app()->compiler->modules()->module(i)->instrument()->setWindowPos(descW - robotW - 7, drawH + 28);
				app()->compiler->modules()->module(i)->instrument()->showHideWindow(true);
				//              this->activateWindow();
			}
		}
	}
	if (flagDraw || flagRobot) {
		this->move(0, 0);
		this->resize(descW * 2 / 3, descH);
	}
}


void MainWindow::loadStartEnvironment()
{
	// int err=0;
	QList<KumValueStackElem> arguments;
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->LoadFromFileActivated();
	roboEditor->append2LastFiles(app()->settings->value("Robot/StartField/File", "").toString());
}

void MainWindow::editStartEnvironment()
{
	int err = 0;
	QList<KumValueStackElem> arguments;
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	// robo->runFunc("EditStart",&arguments,&err);

	roboEditor->createRescentMenu();
	actionEditStartEnvironment->setEnabled(false);
	actionLoadEnvironment->setEnabled(false);
	roboEditor->field->editField();
	err = roboEditor->field->loadFromFile(app()->settings->value("Robot/StartField/File", "").toString());
	roboEditor->field->drawField(FIELD_SIZE_SMALL);
	roboEditor->show();
	roboEditor->prepare();
	if (err == 0) {
		roboEditor->setWindowTitleText(app()->settings->value("Robot/StartField/File", "").toString());
	} else {
		roboEditor->setWindowTitleText(trUtf8("Нет файла"));
	}
	robo->setRobotStartField(roboEditor->field);
	roboEditor->showNormal();
	//roboEditor->activateWindow();
}

void MainWindow::showRobot()
{

	app()->compiler->Modules.module(2)->instrument()->showHideWindow(false);
}

void MainWindow::loadDrawing()
{
	KumDraw *draw = (KumDraw *)app()->compiler->Modules.module(3)->instrument();
	draw->LoadFromFileActivated();
}

void MainWindow::clearDrawing()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ClearPicture();
}

void MainWindow::saveDrawing()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->SaveToFileActivated();
}

void MainWindow::printDrawing()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->PrintToPDF();
}

void MainWindow::drawZoomIn()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ZoomUp();
}

void MainWindow::drawZoomOut()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ZoomDown();
}

void MainWindow::drawZoomFit()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ToWindow();
}

void MainWindow::drawZoomRestore()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ToCenter();
}

void MainWindow::drawGrid()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->ShowNetWindow();
}

void MainWindow::drawInformation()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->Info();
}

void MainWindow::drawLayoutHorizontal()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->HorizWindow();
}

void MainWindow::drawLayoutVertical()
{
	KumDraw *draw = static_cast<KumDraw *>(
			app()->compiler->Modules.module(3)->instrument()
		);
	draw->VertWindow();

}

void MainWindow::showDraw()
{
	app()->compiler->Modules.module(3)->instrument()->showHideWindow(false);
}

void MainWindow::saveEnvironment()
{
	qDebug("SaveCurr");
	int err = 0;
	QList<KumValueStackElem> arguments;
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->runFunc("SaveCurrent", &arguments, &err);
	roboEditor->append2LastFiles(app()->settings->value("Robot/StartField/File", "").toString());
}

void MainWindow::showPult()
{
	int err = 0;
	QList<KumValueStackElem> arguments;
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	// app()->kumServer->OpenPort("localhost",4242);
	robo->runFunc("ShowPult", &arguments, &err);
}

void MainWindow::showStartEnvironment()
{
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->reset();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	Q_UNUSED(event);
	if (event->key() == Qt::Key_F12) {
		if (!menubar->hasFocus()) {
			menuBar()->setFocus(Qt::MenuBarFocusReason);
			menuBar()->setActiveAction(menuProgram->menuAction());
			menuBar()->update();
		} else {
			menuBar()->setActiveAction(0);
			KumirEdit *editor = dynamic_cast<ProgramTab *>(tabWidget->widget(0))->editor();
			editor->setFocus(Qt::MouseFocusReason);
		}
		event->accept();
	} else if (event->key() == Qt::Key_Alt) {
		event->accept();

	} else {
		QMainWindow::keyPressEvent(event);
	}
//	if (!event->text().isEmpty())
//		sMessage->setText("");
}

bool MainWindow::event(QEvent *event)
{

	if (event->type() == QEvent::User) {
		KNPEvent *MEvent = dynamic_cast<KNPEvent *>(event);
		QMetaObject::invokeMethod(this, MEvent->str().toLatin1().constData(),
			Qt::QueuedConnection);
		qDebug() << "OK SUPER";
	};
	return QMainWindow::event(event);
}

void MainWindow::showPdfCommon(const QUrl &url)
{
	qDebug() << "MainWindow::showPdfCommon(" << url.toString() << ")";
	QUrl fullPath;
	QString helpBase;
#ifdef Q_OS_MAC
	helpBase = QDir::cleanPath(QApplication::applicationDirPath() + "/../Resources/");
#else
	helpBase = QDir::cleanPath(QApplication::applicationDirPath() + "/Kumir/");
#endif
	if (url.scheme() == "file") {
		fullPath = QUrl::fromLocalFile(QDir(helpBase).absoluteFilePath(url.toLocalFile()));
	} else {
		fullPath = url;
	}
	qDebug() << "Show PDF from: " << fullPath;
	QDesktopServices::openUrl(fullPath);
}

void MainWindow::showPDF()
{
	showPdfCommon(QUrl::fromLocalFile("Help/manual.pdf"));
}

void MainWindow::showPdfFromWorld()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) {
		return;
	}
	const QUrl url = action->data().toUrl();
	showPdfCommon(url);
}


void MainWindow::finishEditingRobo()
{
	actionEditStartEnvironment->setEnabled(true);
	actionLoadEnvironment->setEnabled(true);
}


void MainWindow::printEnvironment()
{
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->Print2PDF();
}

void MainWindow::howToUseMacros()
{
	QMessageBox::information(this, tr("Macros"),
		tr("Macro is a set of keys pressings that is set up on one key. Macros are intended for user maintainability.\n\
								There are two types of macros in Kumir:\n\
								1. System macros. Located in 'Main Menu -> Insert'.\n\
								2. User macros. Located in 'Main Menu -> Edit -> Macros'.")
	);


	/*                                                   QString::fromUtf8(
	"  Макрос - это некоторая последовательность нажатий клавиш, назначенных на одну клавишу. \
	Макросы созданы для удобства пользователя.\n\
	В КуМире существуют два типа макросов:\n\
	1. Системые. Приведены в разделе 'Вставка' главного меню. Пользователь может их использовать, но не может модифицировать.\n\
	2. Пользовательские. Приведены в подразделе меню 'Редактирование'->'Макросы'. Пользователь может их создавать, изменять, удалять."
	)*/
}



void MainWindow::openHyperText()
{
	if (!app()->httpDaemon) {
		app()->httpDaemon = new HttpDaemon(HttpDaemon::defaultPort, app());
		connect(app()->httpDaemon, SIGNAL(setProgramText(const QString &)), this, SLOT(setCurrentEditorText(const QString &)));
		connect(app()->httpDaemon, SIGNAL(setProgramFile(const QString &)), this, SLOT(fileOpenInCurrentTab(QString)));
		connect(app()->httpDaemon, SIGNAL(runProgram()), this, SLOT(debugRun()));
		connect(app()->httpDaemon, SIGNAL(doSTEP()), this, SLOT(shagRun()));
		connect(app()->httpDaemon, SIGNAL(doSmallStep()), this, SLOT(vnutrVizova()));
		connect(app()->httpDaemon, SIGNAL(stop()), this, SLOT(stopRunning()));
		connect(app()->httpDaemon, SIGNAL(runAlg(const QString &, int)), this, SLOT(StartFromAlg(QString, int)));
		connect(app()->httpDaemon, SIGNAL(setRobotEnvironment(const QString &)), (KumRobot *)app()->compiler->Modules.module(2)->instrument(), SLOT(setStartEnvironment(const QString &)));
	}
	QDesktopServices::openUrl(QString("http://localhost:") + QString::number(HttpDaemon::defaultPort));
	actionHT->setCheckable(true);
	actionHT->setChecked(true);
}

void MainWindow::createFriendMenu()
{
	QList<int> friendIdList;
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); i++) {
		//qDebug()<<"Friend:"<<app()->compiler->modules()->module(i)->isFriend();
		if ((app()->compiler->modules()->module(i)->isFriend()) && (!app()->compiler->modules()->module(i)->menu())) {
			friendIdList.append(i);
		}
	};

	if (friendIdList.count() < 1) {
		return;//Vikhodim esle nrchrgo delat
	}
	//friendMenu->show();

	for (int i = 0; i < friendIdList.count(); i++) {
		kumirPluginInterface *plugin = app()->compiler->modules()->module(friendIdList[i])->Plugin();
		const QString name = plugin->name();
		const QIcon icon = QIcon(app()->compiler->modules()->module(friendIdList[i])->iconFile);
		QAction *friendAct = friendMenu->addAction(name);
		friendAct->setIcon(icon);
		bool hasPult = plugin->hasPult();
		if (hasPult) {
			const QString actionName = name + tr(" - Pult");
			QAction *friendPAct = friendMenu->addAction(actionName);
			connect(friendPAct, SIGNAL(triggered()),
				app()->compiler->modules()->module(friendIdList[i]), SLOT(showPult()));
		}

		if (app()->compiler->modules()->module(friendIdList[i])->Plugin()->hasOpt()) {
			QString optText = app()->compiler->modules()->module(friendIdList[i])->Plugin()->optText();
			QAction *friendOAct = friendMenu->addAction(app()->compiler->modules()->module(friendIdList[i])->name + " - " + optText);
			connect(friendOAct, SIGNAL(triggered()), app()->compiler->modules()->module(friendIdList[i]), SLOT(showOpt()));
		}
		app()->compiler->modules()->module(friendIdList[i])->setMenu();//Флаг наличия меню.
		connect(friendAct, SIGNAL(triggered()), app()->compiler->modules()->module(friendIdList[i]), SLOT(showWindow()));

		app()->compiler->modules()->module(friendIdList[i])->Plugin()->connectSignalSendText(this, SLOT(insertText(QString)));
		if (i < friendIdList.count() - 1) {
			friendMenu->addSeparator();
		}
	}


	int c = 0;
	int mas[] = {Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5, Qt::Key_6, Qt::Key_7, Qt::Key_8, Qt::Key_9};
//	QSignalMapper *signalMapper = new QSignalMapper(this);
//	connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(showPdfCommon(QString)));
	for (int i = 0; i <= app()->compiler->modules()->lastModuleId(); ++i) {
		if (app()->compiler->Modules.module(i)->isFriend()) {
			Macro *m = new Macro(this);
			QString name = QString::fromUtf8("использовать ") + app()->compiler->Modules.module(i)->Name();

			uint type = 6;
			int key = 16777249;
			uint modifiers = 0;
			QString txt;
			uint autoRepeat = 0;
			int count = 1;
			m->addCommand(new QKeyEvent((QEvent::Type)(type), key, (Qt::KeyboardModifiers)(modifiers), txt, autoRepeat == 1, count));
			key = 16777232;
			modifiers = 67108864;
			m->addCommand(new QKeyEvent((QEvent::Type)(type), key, (Qt::KeyboardModifiers)(modifiers), txt, autoRepeat == 1, count));
			m->addCommand(new QKeyEvent(QEvent::None, 1048, Qt::NoModifier, name));
			m->addCommand(new QKeyEvent(QEvent::None, 16777220, Qt::NoModifier, QString::fromUtf8("&#xd;"))); // Enter key

			m->setName(name);
			bool flagAlreadyExist;
			do {
				if (!(c < 9)) {
					break;
				}
				flagAlreadyExist = false;
				foreach (Macro *mm, app()->standardMacros) {
					if (mm->keySequence() == QKeySequence(Qt::Key_Escape, mas[c]).toString()) {
						flagAlreadyExist = true;
					}
				}
				if (flagAlreadyExist) {
					++c;
				}
			} while (flagAlreadyExist);
			if (c < 9) {
				m->setKeySequence(QKeySequence(Qt::Key_Escape, mas[c]).toString());
				++c;
			}

			app()->standardMacros.append(m);
			QAction *mAction = menuMacro->addAction(m->name(), this, SLOT(macroPlay()), m->keySequence());
			m->setAction(mAction);

			QUrl pdfUrl = app()->compiler->Modules.module(i)->Plugin()->pdfUrl();
			if (pdfUrl.isValid() && !pdfUrl.isEmpty()) {
				QAction *action = menuModulesManuals->addAction(app()->compiler->Modules.module(i)->Name());
				action->setData(pdfUrl);
				connect(action, SIGNAL(triggered()), this, SLOT(showPdfFromWorld()));
//				signalMapper->setMapping(action, filename);
//				connect (action, SIGNAL(triggered()), signalMapper, SLOT(map()) );
			}
		}
	}
}

void MainWindow::insertText(QString text)
{
	tab(0)->editor()->doInsert(text);
};


void MainWindow::setInsertFirstAlgorhitmAvailable(bool value)
{
	actionInsert_first_algorhitm->setEnabled(value);
}

void MainWindow::insertFirstAlgorhitm()
{
	QString text = app()->compiler->firstAlgorhitmText();
	tab(0)->editor()->doInsertFirst(text);
}


uint  MainWindow::checkModulesTask()
{
	qDebug() << "Check task in mainwindow.";
	int mark = 10;
	int count_i = app()->Task()->isps.count();

	for (int i = 0; i < count_i; i++) {
		QString ispName = app()->TaskControl()->Isp(i);
		int id = app()->compiler->modules()->idByName(ispName); //Получаем id модуля
		if (id < 0) {
			continue;
		}
		if (!app()->compiler->modules()->module(id)->isFriend()) {
			continue;
		}
		qDebug() << "Start script";
		int c_mark = app()->compiler->modules()->module(id)->
			Plugin()->check(app()->Task()->scripts[i]);
		qDebug() << "app()->Task()->scripts[i]" << app()->Task()->scripts[i];
		if (c_mark < mark) {
			mark = c_mark;
		}
	}
	return mark;
}
void MainWindow::startTC()
{
	for (int i = 0; i < tabWidget->count(); ++i) {
		if (dynamic_cast<Tab *>(tabWidget->widget(i))->isModified()) {
			tabWidget->setCurrentIndex(i);
			QMessageBox::StandardButton ans;
			ans = QMessageBox::question(this, tr("Kumir"), tr("This file not saved. Save it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
			if (ans == QMessageBox::Cancel) {

				return;
			} else if (ans == QMessageBox::Yes) {
				if (!fileSave()) {
					return;
				}
			}
		}
	}
	app()->TaskControl()->show();
};
void MainWindow::text2file(QString file)
{
	if (outTextF.isOpen()) {
		outTextF.close();
	}


	qDebug() << "Text2File" << file;
	outTextFile = file;
	if (outTextFile == "") {
		return;
	}
	outTextF.setFileName(outTextFile);
	QFileInfo fi(outTextF);
	if (!fi.isAbsolute()) {
		outTextF.setFileName(app()->settings->value("Directories/IO", "").toString() + "/" + outTextFile);
	};
	if (!outTextF.open(QIODevice::WriteOnly)) {
		// textEdit3->output(trUtf8("Невозможно перенаправить в")+outTextFile);
		emit(setRunError(trUtf8("Невозможно перенаправить в") + outTextF.fileName()));
		outTextFile = "";
	}

};

void MainWindow::textFromFile(QString file)
{

	if (inTextF.isOpen()) {
		inTextF.close();
	}
	inTextFile = file;
	if (inTextFile == "") {
		return;
	}
	inTextF.setFileName(inTextFile);
	QFileInfo fi(inTextF);
	if (!fi.isAbsolute()) {
		inTextF.setFileName(app()->settings->value("Directories/IO", "").toString() + "/" + inTextFile);
	};
	if (!inTextF.open(QIODevice::ReadOnly)) {

		textEdit3->output(trUtf8("Невозможно читать из ") + inTextFile);
		emit(setRunError(trUtf8("Невозможно читать из ") + inTextF.fileName()));
		inTextFile = "";
	}

};

void LabelEx::setText(const QString &txt)
{
//     qDebug() << "LabelEx::setText(" << txt << ")";
	QLabel::setText(txt);
}

void MainWindow::setRecordMacroMode(bool v)
{
	if (v) {
		sRecordMacro->setPixmap(QPixmap(":/recording.png"));
	} else {
		sRecordMacro->setPixmap(QPixmap());
	}
}

#ifdef Q_OS_WIN32
bool MainWindow::winEvent(MSG *message, long *result)
{
	return false;
}

#endif
