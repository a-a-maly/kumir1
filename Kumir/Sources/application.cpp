/****************************************************************************
**
** Copyright (C) 2004-2009 NIISI RAS. All rights reserved.
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

#define APP_DEBUG 1
#define AUTOSAVE_PERIOD_IN_MINUTES 5

#include "application.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "modulesregistration.h"
#include "compiler.h"
#include "messagesprovider.h"
#include "macro.h"
#include "assistant.h"
#include "network.h"
#include "run.h"
#include "tab.h"
#include "debugdialog.h"
#include "programtab.h"
#include "kassert.h"
#include "config.h"
#include "semaphorewidget.h"
#include "int_proga.h"


#include "subprocess.h"
#include "kumsinglemodule.h"
#include "kumrobot.h"

#include "secondarywindow.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QTimer>
#include <QTranslator>
#include <QPluginLoader>

#ifndef WIN32
#include "unistd.h"
#endif


Application::Application() : QObject()
{
	isExam = false;
	ispMode = false;
	batchMode = false;
	zMode = false;
	zPath = "";
	mainWindow = NULL;
	settingsDialog = NULL;
	settings = NULL;
	modulesRegistration = NULL;
	networkAccessManager = NULL;
	debugDialog = NULL;
	compiler = NULL;
	run = NULL;
	messagesProvider = NULL;
	assistant = NULL;
	kumServer = NULL;
	SigCross = NULL;
	httpDaemon = NULL;
	errorText = QString::fromUtf8("ОШИБКА ИСПОЛНИТЕЛЯ");
	m_python = NULL;
	m_convertorProcess = NULL;
	taskControlLoaded = false;
	startControlScripts = false;
}

void Application::finishBatchInitialization()
{
	batchTimeout->start();
	if (connectIspPort > 0) {
		bachDelayTimer = new QTimer(m_instance);
		bachDelayTimer->setSingleShot(true);
		bachDelayTimer->setInterval(1000);

		connect(bachDelayTimer, SIGNAL(timeout()), m_instance, SLOT(startBach()));
		bachDelayTimer->start();
	} else {
		mainWindow->StartRun();
	}
}

void Application::loadModules()
{


	// Восстанавливаем список исполнителей
	int countMod = settings->value("ModulesRegistration/IspData/Count", 0).toInt();
	for (int i = 0; i < countMod; ++i) {
		QString filename = settings->value("ModulesRegistration/IspData/Filename-" + QString::number(i), "").toString();
		int ID = settings->value("ModulesRegistration/IspData/ID-" + QString::number(i), -1).toInt();
		modulesRegistration -> regModule(filename, ID);
	}
	if (connectIspPort > 0) {
		modulesRegistration->tryToConnect(connectIspPort);
		//        qDebug()<<"Try to connect "<<connectIspPort;
		//if(batchMode)mainWindow->show();
	}
	if (countMod > 0) {
		doCompilation();
	}


}

void Application::createHttpServer()
{

}

void Application::createSettings()
{
	m_kumirStyle = styleText;
	port = 4242;
	port_max = 4252;
	connectIspPort = -1;
	QString settingsPath;
	QString workDir;

	QStringList argv = qApp->arguments();
	int argc = argv.size();

	for (int i = 1; i < argc; i++) {
		QString param(argv[i]);
		if (param.startsWith("-c")) {
			if (i + 1 < argc) {
				connectIspPort = argv[i + 1].toInt();
				i++;
			}
		}

		if (param.startsWith("-e")) {
			isExam = true;
		}

		if (param.startsWith("-b")) {
			batchMode = true;
		}

		if (param == "-sf") {
			m_kumirStyle = styleFull;
		} else if (param == "-sk") {
			m_kumirStyle = styleClassic;
		}

		if (param == "-i") {
			ispMode = true;
			if (i + 1 < argc) {
				port = argv[i + 1].toInt();
				i++;
			}
		}
	}

	foreach (QString arg, qApp->arguments()) {
		if (arg.toLower().startsWith("--config=") || arg.toLower().startsWith("-c=") || arg.toLower().startsWith("/c=")) {
			int equalPos = arg.indexOf("=");
			settingsPath = arg.mid(equalPos + 1);
			if (settingsPath.startsWith("\"") && settingsPath.endsWith("\"")) {
				settingsPath = settingsPath.mid(1, settingsPath.length() - 2);
			}
		}
		if (arg.toLower().startsWith("--workdir=") || arg.toLower().startsWith("-w=") || arg.toLower().startsWith("/w=")) {
			int equalPos = arg.indexOf("=");
			workDir = arg.mid(equalPos + 1);
			if (workDir.startsWith("\"") && workDir.endsWith("\"")) {
				workDir = workDir.mid(1, settingsPath.length() - 2);
			}
		}
	}

	if (!settingsPath.isEmpty()) {

		if (!QFile::exists(settingsPath)) {
			QFile f(settingsPath);
			if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
				f.close();
			}
		}
		QFileInfo fi(settingsPath);
		if (fi.isDir() || !fi.isWritable() || !QFile::exists(settingsPath)) {
			QMessageBox::warning(0, tr("Can't use settings"),
				tr("Can't use settings file %1\n. This option is ignored.").arg(settingsPath)
			);
			settingsPath = "";
		}
	}


	if (!settingsPath.isEmpty()) {
		settings = new QSettings(settingsPath, QSettings::IniFormat);
	} else {
		settings = new QSettings("NIISI RAS", "Kumir 1.9");
	}
	settings->setIniCodec("UTF-8");
	//    qDebug() << settings->fileName();
	if (settings->value("Common/SetDefaultSettingsAtFirstRun", false).toBool()) {
		QStringList keys = settings->allKeys();
		QString key;
		for (int i = 0; i < keys.size(); i++) {
			key = keys[i];
			settings->remove(key);
		}
	}

	s_configPath = settingsPath;
	zMode = !settingsPath.isEmpty();
	zPath = !workDir.isEmpty();

	if (!workDir.isEmpty()) {
		settings->setValue("Directories/StdModules", workDir);
		settings->setValue("Directories/IO", workDir);
	}

	QString defaultPath;
#if QT_VERSION >= 0x050000
	defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
#else
	defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
	if (!QDir(defaultPath).exists()) {
		QDir::root().mkpath(defaultPath);
	}

	if (settings->value("Directories/StdModules", "").toString().isEmpty()) {
		settings->setValue("Directories/StdModules", defaultPath);
	}
	if (settings->value("Directories/IO", "").toString().isEmpty()) {
		settings->setValue("Directories/IO", defaultPath);
	}
}

void Application::createMacros()
{
	loadMacros();
	mainWindow->updateMacros();
}

void Application::createNetworkUtilites()
{
	networkAccessManager = new QNetworkAccessManager(this);
	kumServer = new KNPServer();
	SigCross = new SignalCrosser();
	SigCross->setModulesNRun(compiler->modules(), run);
	if (ispMode) {
		kumServer->OpenPort("localhost", port);
		if (kumServer->isListening()) {}
		else {
			while (port <= port_max) {
				port++;

				kumServer->OpenPort("localhost", port);
				if (kumServer->isListening()) {

					return;
				}

			}
		}
	}
}

void Application::createMessagesProvider()
{
	messagesProvider = new MessagesProvider();
}

void Application::createSecondaryWindows()
{
	qApp->setWindowIcon(QIcon(":/icons/kumir.png"));
	modulesRegistration = new ModulesRegistration(mainWindow, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

	assistant = new Assistant(0);
	assistant->setMinimumSize(600, 400);
	assistantWindow = new SecondaryWindow(assistant, tr("Assistant"));
	assistantWindow->setWindowIcon(QIcon(":/icons/assistant_window.png"));
	assistant->init(compiler->modules());

	debugDialog = new DebugDialog(mainWindow);
	debugDialog->setMinimumSize(500, 200);
	variablesWindow = new SecondaryWindow(debugDialog, tr("Variables"));
	variablesWindow->setWindowIcon(QIcon(":/icons/variables_window.png"));
	debugDialog->init(compiler->modules());


	settingsDialog = new SettingsDialog(mainWindow, Qt::Dialog | Qt::WindowSystemMenuHint);
	keywordsViewer = new QWebView;
	keywordsViewer->setMinimumSize(300, 300);
	keywordsViewer->setHtml(KumTools::instance()->getKeyWordsHTMLList());
	keywordsViewer->setWindowTitle(tr("Keywords"));
	keywordsWindow = new SecondaryWindow(keywordsViewer, tr("KeywordsWindow"));
	keywordsWindow->setWindowIcon(QIcon(":/icons/keywords_window.png"));

}

void Application::finishGUIInitialization()
{

	runDelayTimer = new QTimer(this);
	runDelayTimer->setSingleShot(true);
	runDelayTimer->setInterval(1000);
	QString fileName;
	QString currentArg;

	QStringList argv = qApp->arguments();
	int argc = argv.size();

	for (int i = 1; i < argc; i++) {
		currentArg = argv[i];
		if (
			(currentArg.startsWith("\"") && currentArg.endsWith("\"")) ||
			(currentArg.startsWith("'") && currentArg.endsWith("'"))) {
			currentArg = currentArg.mid(1, currentArg.length() - 2);
		}
		if (currentArg.endsWith(".kum")) {
			fileName = QFileInfo(currentArg).absoluteFilePath();
			break;
		}
	}


	QString startText;
	if (qApp->arguments().contains("-e")) {
		startText = QString::fromUtf8("алг \nнач\n\nкон\n");
	} else {
		startText = QString::fromUtf8("| использовать Робот\nалг \nнач\n\nкон\n");
	}
	startText = settings->value("Start/Text", startText).toString();


	compiler->loadModulesInfo();
	mainWindow->createFriendMenu();
	periodicSaveTimer->start();
	doCompilation();
	if (!emergencyRestoreState()) {
		if (!fileName.isEmpty() && QFile::exists(fileName)) {
			mainWindow->fileOpenInCurrentTab(fileName);

			for (int i = 1; i < argc; i++) {
				if (argv[i].toLower() == "-x" || argv[i].toLower() == "--execute") {
					connect(runDelayTimer, SIGNAL(timeout()), mainWindow, SLOT(StartRun()));
					runDelayTimer->start();
				}
			}
		} else {
			mainWindow->setCurrentEditorText(startText);
			//compiler->loadModulesInfo();
		}
	}
	mainWindow->show();
}

void Application::prepareBatch()
{
	outFile = NULL;
	marginFile = NULL;
	batchTimeout = new QTimer(this);
	int timeout_value = 30000;
	foreach (const QString &arg, qApp->arguments()) {
		if (arg.startsWith("--timeout=")) {
			timeout_value = arg.mid(10).toInt() * 1000;
		}
	}

	batchTimeout->setInterval(timeout_value);
	connect(batchTimeout, SIGNAL(timeout()), this, SLOT(finishBatch()));
	QString kumFile;
	QString outFile_textEdit;
	QString outFile_margin;
	QString outFile_output;
	QString outFile_marginRun;

	QStringList argv = qApp->arguments();
	int argc = argv.size();

	for (int i = 1; i < argc; i++) {
		if (argv[i].startsWith("-k")) {
			kumFile = argv[i];
			kumFile.remove(0, 2);
			mainWindow->fileOpenInFirstTab(kumFile);
			qDebug() << "Batch file = " << kumFile;
		}
		if (argv[i].startsWith("-o")) {
			QString files = argv[i];
			files.remove(0, 2);
			QStringList filesList = files.split(",");
			if (filesList.count() > 0) {
				outFile_textEdit = filesList[0];
			}
			if (filesList.count() > 1) {
				outFile_margin = filesList[1];
			}
			if (filesList.count() > 2) {
				outFile_output = filesList[2];
			}
			if (filesList.count() > 3) {
				outFile_marginRun = filesList[3];
			}
			if (!outFile_textEdit.isEmpty()) {
				QFile of1(outFile_textEdit);
				if (of1.open(QIODevice::WriteOnly)) {
					QTextStream ts1(&of1);
					ts1.setCodec("UTF-8");
					ts1 << mainWindow->currentEditorText(true);
					of1.close();
				}
			}
			if (!outFile_margin.isEmpty()) {
				QFile of2(outFile_margin);
				if (of2.open(QIODevice::WriteOnly)) {
					QTextStream ts2(&of2);
					ts2.setCodec("UTF-8");
					ts2 << mainWindow->currentMarginText();
					of2.close();
				}
			}
			if (!outFile_output.isEmpty()) {
				outFile = new QFile(outFile_output);
			}
			if (!outFile_marginRun.isEmpty()) {
				marginFile = new QFile(outFile_marginRun);
			}
		}
	}
}

void Application::createCrashHelper()
{
	periodicSaveTimer = new QTimer(this);
	periodicSaveTimer->setSingleShot(false);
	periodicSaveTimer->setInterval(100 * 60 * AUTOSAVE_PERIOD_IN_MINUTES);
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(emergencyCleanState()));
	connect(periodicSaveTimer, SIGNAL(timeout()), this, SLOT(emergencySaveState()));

}

void Application::createConnections()
{
	connect(compiler, SIGNAL(updateEditorFinished()), mainWindow->semaphoreWidget, SLOT(setModeGreen()));
	connect(mainWindow->actionModulesRegistration, SIGNAL(triggered()), modulesRegistration, SLOT(showForm()));
	KumRobot *robot = dynamic_cast<KumRobot *>(compiler->Modules.module(2)->instrument());
	ProgramTab *tab = dynamic_cast<ProgramTab *>(mainWindow->tab(0));
	connect(robot, SIGNAL(pultCommand(QString)), tab->editor(), SLOT(insertPultLine(const QString &)));
	connect(mainWindow->actionUserAlgorithms, SIGNAL(triggered()), assistantWindow, SLOT(show()));
	connect(mainWindow->actionVariables, SIGNAL(triggered()), variablesWindow, SLOT(show()));
	connect(mainWindow->actionKeyWords, SIGNAL(triggered()), keywordsWindow, SLOT(show()));

	connect(SigCross, SIGNAL(do_b_shag()), mainWindow, SLOT(shagRun()));
	connect(SigCross, SIGNAL(do_start_run()), mainWindow, SLOT(StartRun()));
	connect(SigCross, SIGNAL(do_stop()), mainWindow, SLOT(stopRunning()));
	connect(SigCross, SIGNAL(do_start_alg(QString, int)), mainWindow, SLOT(StartFromAlg(QString, int)));
	connect(SigCross, SIGNAL(do_message(QString)), mainWindow, SLOT(textMessage(QString)));
	connect(SigCross, SIGNAL(do_Call(QString, QString, int)), run, SLOT(callFunct(QString, QString, int)));
	connect(run, SIGNAL(sendServReplay(QString, int)), kumServer, SLOT(servReplay(QString, int)));
	connect(compiler, SIGNAL(firstAlgorhitmAvailable(bool)), mainWindow, SLOT(setInsertFirstAlgorhitmAvailable(bool)));
}

void Application::createCompiler()
{
	compiler = new Compiler();
#ifdef Q_OS_MAC
	for (int i = 0; i < compiler->modules()->count(); i++) {
		KumSingleModule *mod = compiler->modules()->module(i);
		if (mod->metaObject()->className() == "KumRobot") {
			KumRobot *robot = qobject_cast<KumRobot *>(mod);
			//robot->robotWindow->setParent(mainWindow);
			//          robot->setWindowMenuBar(mainWindow->menuWidget());

		}
	}
#endif
}

void Application::createRuntime()
{
	run = new Kumir_Run();
}

void Application::createMainWindow()
{
	mainWindow = new MainWindow();
	mainWindow->fileNewProgram();
	mainWindow->setCaptionFromTab(0);
	mainWindow->setActionsForTab(0);
}

void Application::finishBatch()
{
	if (!batchMode) {
		return;
	}
	QString marginText = mainWindow->currentMarginText();
	if (marginFile != NULL) {
		if (marginFile->open(QIODevice::WriteOnly)) {
			QTextStream ts(marginFile);
			ts.setCodec("UTF-8");
			ts << marginText;
			marginFile->close();
		}

	}
	if (outFile != NULL) {
		if (outFile->open(QIODevice::WriteOnly)) {
			QTextStream ts(outFile);
			ts.setCodec("UTF-8");
			ts << "1\n-1\n";
			ts << mainWindow->outputText();
			outFile->close();
		}
	}
	qApp->quit();
}


void Application::setLanguage()
{
	QString languageName = settings->value("Common/Language", QString::fromUtf8("Русский")).toString();
#ifndef Q_OS_MAC
	const QString langConfPath = QApplication::applicationDirPath() + "/Kumir/lang.conf";
#endif
#ifdef Q_OS_MAC
	const QString langConfPath = QApplication::applicationDirPath() + "/../Resources/lang.conf";
#endif
	if (QFile::exists(langConfPath)) {
		QFile langConf(langConfPath);
		if (langConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
			languageName = QString::fromUtf8(langConf.readAll()).simplified().trimmed();
			langConf.close();
		}
	}
#ifdef ONLY_RUSSIAN_LANGUAGE
	languageName = QString::fromUtf8("Русский");
#endif
	if (!currentLanguage().isEmpty() && currentLanguage() == languageName) {
		//        qDebug() << "Skip language change";
		return;
	}
	foreach (QTranslator *translator, translators) {
		Q_UNUSED(translator);
		//      removeTranslator(translator);
		//      delete translator;
	}
	//  translators.clear();
	QString appPath = QApplication::applicationDirPath() + "/Kumir/Languages/";
#ifdef Q_OS_MAC
	appPath = QApplication::applicationDirPath() + "/../Resources/";
#endif
	//    qDebug() << "Lang path: " << appPath;
	QDir curDir = QDir(appPath);

	QStringList langFilesFilter;
	langFilesFilter << "*.lang";

	curDir.setNameFilters(langFilesFilter);
	QStringList errfiles;
	QStringList locfiles;
	QStringList posixNames;
	QStringList keyfiles;
	QStringList extraCharsets;
	//    qDebug()<<"Lang files found"<<curDir.entryList().count()<<"DIR "<<appPath;
	foreach (QString fileName, curDir.entryList()) {
		QFile f(appPath + "/" + fileName);
		QTextStream ts(&f);
		ts.setCodec("UTF-8");
		if (f.open(QIODevice::ReadOnly)) {
			QString data = ts.readAll();
			f.close();
			QString name;
			QString errfile;
			QString locfile;
			QString posixname;
			QString extracharset;
			QString keyfile;
			QStringList lines = data.split("\n");
			foreach (QString line, lines) {
				QStringList pair = line.split("=");
				if (pair.count() == 2) {
					if (pair[0].simplified() == "name") {
						name = pair[1].simplified();
					} else if (pair[0].simplified() == "err_file") {
						errfile = pair[1].simplified();
					} else if (pair[0].simplified() == "loc_file") {
						locfile = pair[1].simplified();
					} else if (pair[0].simplified() == "posix_name") {
						posixname = pair[1].simplified();
					} else if (pair[0].simplified() == "extra_charset") {
						extracharset = pair[1].simplified();
					} else if (pair[0].simplified() == "key_file") {
						keyfile = pair[1].simplified();
					}
				}
			}
			m_languageList << name;
			errfiles << errfile;
			locfiles << locfile;
			keyfiles << keyfile;
			posixNames << posixname;
			extraCharsets << extracharset;
		}
	}
	if (languageName.isEmpty()) {
		QStringList envs = QProcess::systemEnvironment();
		QString langEnv;
		foreach (QString env, envs) {
			if (env.startsWith("LANG=")) {
				langEnv = env;
				break;
			}
		}
		QString lang;
		if (!langEnv.isEmpty()) {
			QStringList pair = langEnv.split("=");
			if (pair.count() > 1) {
				QStringList lg = pair[1].split(".");
				lang = lg[0];
			}
		}
		if (!lang.isEmpty()) {
			int index = -1;
			for (int i = 0; i < posixNames.count(); i++) {
				if (lang.trimmed() == posixNames[i]) {
					index = i;
					break;
				}
			}
			if (index != -1) {
				languageName = m_languageList[index];
			}
		}
	}
	if (m_languageList.contains(languageName)) {
		//      if ( translator != NULL )
		//          delete translator;
		//      translator = new QTranslator();
		int i = m_languageList.indexOf(languageName);

		//      K_ASSERT(QDir::setCurrent(appPath));

		QStringList qmFiles = locfiles[i].split(QRegExp("\\s+"));
		foreach (QString qmFile, qmFiles) {
			QTranslator *translator = new QTranslator();
			if (!translator->load(appPath + "/" + qmFile)) {
				qDebug("Warning: localization file \"%s\" not found! Unterface still english and keywords still russian.", qmFile.toUtf8().data());
				delete translator;
			} else {
				qApp->installTranslator(translator);
				translators << translator;


			}
		}
		m_currentLanguage = languageName;
		KumTools::instance()->initNormalizator(appPath + "/" + keyfiles[i]);
		KumTools::instance()->setExtraCharset(extraCharsets[i]);
		messagesProvider->init(errfiles[i]);


	}
	if (mainWindow) {
		mainWindow->updateLanguage();
	}
}



/** Возвращает список доступный языков */
QStringList Application::languageList()
{
	return m_languageList;
}

/** Возвращает текущий язык */
QString Application::currentLanguage()
{
	return m_currentLanguage;
}


void Application::doCompilation()
{

	mainWindow->semaphoreWidget->setModeRed();

	compiler->init(0);
	compiler->start();

	if (assistant && assistant->isVisible()) {
		assistant->updateAlgorhitms();
	}
	if (debugDialog) {
		debugDialog->refreshVariables();
	}
	debugDialog->reset();
}

void Application::startProgram()
{
	run->start(QThread::IdlePriority);
};

void Application::loadMacroFromFile(QString dir, QString fileName, bool editable)
{
	QFile f(dir + QDir::separator() + fileName);
	if (f.open(QIODevice::ReadOnly)) {
		QTextStream ts(&f);
		ts.setCodec("UTF-8");
		QString data = ts.readAll();
		f.close();
		QDomDocument document;
		document.setContent(data);
		if (document.doctype().name() == "KumirMacro-1.5") {
			Macro *m = new Macro(this);
			if (m->load(document)) {
				m->setFileName(fileName);
				//m->setEditable(fi.isWritable());
				if (editable) {
					m->setEditable(true);
					macros << m;
				} else {
					m->setEditable(false);
					standardMacros << m;
				}
			} else {
				delete m;
			}
		}
	}
}

QString Application::getMacroDir()
{
#ifdef Q_OS_MAC
	return QCoreApplication::applicationDirPath() + "/../Resources/Macro";
#else
#endif
	return QCoreApplication::applicationDirPath() + "/Kumir/Macro";
}

void Application::loadMacros()
{
#ifdef Q_OS_MAC
	macroDir = QCoreApplication::applicationDirPath() + "/../Resources/Macro";
#else
	macroDir = QCoreApplication::applicationDirPath() + "/Kumir/Macro";
#endif

	QDir dir = QDir(macroDir);
	QStringList filters;
	filters << "*.mcr";
	dir.setNameFilters(filters);

	QStringList standartConstr;
	standartConstr << trUtf8("insert_algorhitm.macro");
	standartConstr << trUtf8("insert_if_then_else.macro");
	standartConstr << trUtf8("insert_switch_case_done.macro");
	standartConstr << trUtf8("insert_else.macro");
	standartConstr << trUtf8("insert_loop_raz_end.macro");
	standartConstr << trUtf8("insert_loop_for_end.macro");
	standartConstr << trUtf8("insert_loop_while_end.macro");
	standartConstr << trUtf8("insert_loop_end.macro");
	standartConstr << trUtf8("insert_isp_konisp.macro");
	standartConstr << trUtf8("insert_up.macro");
	standartConstr << trUtf8("insert_right.macro");
	standartConstr << trUtf8("insert_down.macro");
	standartConstr << trUtf8("insert_left.macro");
	standartConstr << trUtf8("insert_paint.macro");

	QStringList standartMacros;
	if (!isExamMode()) {
		standartMacros << trUtf8("insert_use_robot.macro");
		standartMacros << trUtf8("insert_use_draw.macro");
		standartMacros << trUtf8("insert_use_files.macro");
	};

	foreach (QString fileName, standartConstr) {
		loadMacroFromFile(macroDir, fileName, false);
	}
	foreach (QString fileName, standartMacros) {
		loadMacroFromFile(macroDir, fileName, false);
	}
	foreach (QString fileName, dir.entryList()) {
		loadMacroFromFile(macroDir, fileName, true);
	}
}

void Application::addEventToHistory(QString str)
{
	eventsHistory.append(QDate::currentDate().toString("dd.MM.yyyy") + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " " + str);
	//  qDebug(eventsHistory.takeLast().toUtf8());
}

void Application::saveEventsHistory(QString str)
{
	QFile file(str);
	if (!file.open(QIODevice::Append | QIODevice::Text)) {
		QMessageBox::critical(0, tr("Kumir"), tr("Unable to save file") + " " + str);
		return;
	}
	QTextStream out(&file);
	out << "** Kumir protocol 1.5.0 **" << endl;
	foreach (QString qs, eventsHistory) {
		out << qs << endl;
	}
	file.close();
	eventsHistory.clear();
}

QChar Application::kumirStyle()
{
	switch (m_kumirStyle) {
	case (styleText):
		return QChar('t');
	case (styleFull):
		return QChar('f');
	case (styleClassic):
		return QChar('c');
	default:
		return QChar('f');
	}
}

bool Application::isBatchMode()
{
	return batchMode;
}

bool Application::isZMode()
{
	return zMode;
}

QString Application::getZPath()
{
	return zPath;
}

QString Application::configPath() const
{
	return s_configPath;
}

bool Application::isOtherKumirRunning()
{
	bool otherKumirRunning = false;
#ifndef WIN32
	QProcess *ps = new QProcess(this);
	ps->start("ps", QStringList() << "-A" << "-f");
	ps->waitForFinished(500);
	QStringList psLines = QString::fromLocal8Bit(ps->readAllStandardOutput().data()).split("\n");
	QProcess *whoami = new QProcess(this);
	whoami->start("whoami");
	whoami->waitForFinished(500);
	QString username = QString::fromLocal8Bit(whoami->readAllStandardOutput().data()).trimmed();
	QString debug;
	foreach (QString processInfo, psLines) {
		debug = processInfo;
		if (processInfo.startsWith(username)) {
			QStringList proc = processInfo.split(" ", QString::SkipEmptyParts);
			if (proc.count() > 8) {
				for (int j = 8; j < proc.count(); j++) {
					proc[7] += proc[j] + " ";
				}
			}
			if (proc.count() > 7) {
				QRegExp rxKumir("\\bkumir\\b");

				if (rxKumir.indexIn(proc[7]) != -1) {
					Q_PID pid = proc[1].toULong();
					Q_PID ppid = proc[2].toULong();
					pid_t currentPid = getpid() - 1; // Qt Bug???
					if (pid == currentPid || ppid == currentPid) {
					} else {
						otherKumirRunning = true;
						break;
					}
				}
			}
		}
	}
#else
	QProcess tasklist(this);
	tasklist.start("tasklist");
	tasklist.waitForFinished(500);
	QStringList proc = QString::fromLocal8Bit(tasklist.readAllStandardOutput().data()).split("\n");
	int kumirCount = 0;
	foreach (QString procLine, proc) {
		if (procLine.startsWith("kumir.exe")) {
			kumirCount++;
		}
	}
	otherKumirRunning = kumirCount > 1;
#endif
	return otherKumirRunning;
}

bool Application::emergencyRestoreState()
{
	bool result = false;
	if (!settings->value("Emergency/Clean", true).toBool() && !isOtherKumirRunning()) {
		if (QMessageBox::question(0, tr("Recover files - Kumir"), tr("It seems Kumir was failed last time. Try to recover it's state?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
			mainWindow->semaphoreWidget->fastRepaint = false;
			QStringList openedFiles = settings->value("Emergency/Data", QStringList()).toStringList();
			int i = 0;
			while (i < openedFiles.count()) {
				int tabNo = i / 3;
				QString fileName = openedFiles[i];
				i++;
				QString fileType = openedFiles[i];
				i++;
				QString data = openedFiles[i];
				i++;
				if (mainWindow->tabWidget->count() > tabNo) {
					Tab *tab = dynamic_cast<Tab *>(mainWindow->tabWidget->widget(tabNo));
					tab->editor()->setText(data);
					tab->setFileName(fileName);

				} else {
					if (fileType == "Program") {
						mainWindow->fileNewProgram();
					}
					//mainWindow->fileNewModule(); - bylo
					else {
						mainWindow->fileNewText();
					}
					Tab *tab = dynamic_cast<Tab *>(mainWindow->tabWidget->widget(tabNo));
					tab->editor()->setText(data);
					KumirEdit *editor = dynamic_cast<KumirEdit *>(tab->editor());
					editor->initLineProps(editor->document());
					editor->applyIndentation();
					tab->setFileName(fileName);
				}
			}
			int curTab = settings->value("Emergency/ActiveTab", 0).toInt();
			if (curTab < mainWindow->tabWidget->count()) {
				mainWindow->tabWidget->setCurrentIndex(curTab);
			}
			for (int i = mainWindow->tabWidget->count() - 1; i >= 0 ; i--) {
				doCompilation();
			}
			result = true;
			mainWindow->semaphoreWidget->fastRepaint = false;
		}
	}
	settings->setValue("Emergency/Clean", false);
	return result;
}

void Application::emergencyCleanState()
{
	if (!isOtherKumirRunning()) {
		settings->setValue("Emergency/Clean", true);
		settings->remove("Emergency/Data");
		settings->remove("Emergency/ActiveTab");
	}
}

void Application::emergencySaveState()
{
	settings->setValue("Emergency/Clean", false);
	QStringList openedFiles;

	Q_CHECK_PTR(mainWindow->tabWidget);

	for (int i = 0; i < mainWindow->tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(mainWindow->tabWidget->widget(i));
		QString fileName = tab->getFileName();
		QString fileType = tab->isProgram() ? "Program" : "Text";
		QString data = tab->editor()->toPlainText();
		QStringList openedFile;
		openedFiles << fileName << fileType << data;
	}
	settings->setValue("Emergency/Data", openedFiles);
	settings->setValue("Emergency/ActiveTab", mainWindow->tabWidget->currentIndex());
}
void Application::startBach()
{
	doCompilation();
	mainWindow->StartRun();
};

Application *Application::instance()
{
	Q_ASSERT(m_instance != NULL);
	return m_instance;
}

Application *Application::m_instance = NULL;

void Application::createInstance()
{
	m_instance = new Application();
}

Application *app()
{
	return Application::instance();
}

void Application::checkFriendConnections()
{
	for (int i = 1; i <= compiler->modules()->lastModuleId(); i++) {
		if (!compiler->modules()->module(i)->isFriend()) {
			continue;
		}
		if (compiler->modules()->module(i)->Process()->state() == QProcess::NotRunning) {
			compiler->modules()->module(i)->startProcess(kumPort());
		};
	};
};
void Application::debugMsg(QString msg)
{
	mainWindow->textMessage(msg);
};
void Application::compileAlgList(int module_id, QStringList algList)
{
	compiler->compileAlgList(module_id, algList);
};


void Application::handleSubprocessFinished(
	const QString &name,
	const QByteArray &reply,
	int errorCode
) {
	Q_UNUSED(name);
	Q_UNUSED(reply);
	Q_UNUSED(errorCode);
	//    qDebug() << "Subprocess " << name << " finished with exitcode " << errorCode;
	//    qDebug() << "Reply is: " << reply;
}

void Application:: loadTaskControlPlugin()
{
	QString path;
#ifndef Q_OS_MAC
	path = QApplication::applicationDirPath() + "/TaskControl/";
	//    qDebug()<<"TC path"<<path;
#endif
	QString prefix, suffix;
#ifdef Q_OS_UNIX
	prefix = "lib";
	suffix = ".so";
#endif
#ifdef Q_OS_WIN32
	suffix = ".dll";
#endif
	QString taskPath;
#ifndef Q_OS_MAC
	taskPath = QApplication::applicationDirPath() + "/TaskControl";
#endif
#ifdef Q_OS_MAC
	path = QApplication::applicationDirPath() + "/../Resources/";
	suffix = ".dylib";
#endif
	QString taskPluginPath = path + prefix + "taskControl" + suffix;
	if (QFile::exists(taskPluginPath)) {
		QPluginLoader *taskLoader = new QPluginLoader(taskPluginPath, this);
		Q_CHECK_PTR(taskLoader);
		QObject *obj = taskLoader->instance();
		if (!obj) {
			qDebug() << "Err:" << taskLoader->errorString();
			return;
		}
		//  Q_CHECK_PTR ( obj );
		csInterface = new fromTCInterface();
		taskControl = qobject_cast<taskControlInterface *>(obj);


		taskControl->setCSinterface(csInterface);
		taskControl->start(trUtf8("Кумир"));
		if (QCoreApplication::arguments().contains("-t")) {
			taskControl->setCSmode(5);
		}
		//    qDebug()<<"taskLoader loaded.";
		taskControlLoaded = true;
	} else {
		qDebug() << "taskControl not loaded:" << taskPluginPath;
	}

}

void Application::startCheck()
{
	startControlScripts = true;
	curFieldId = 0;
	for (int i = 0; i < app()->taskIsps.count(); i++) {
		int id = app()->compiler->modules()->idByName(app()->taskIsps.at(i));


		if (app()->compiler->modules()->module(id)->name == "Robot") {
			app()->mainWindow->textEdit3->append("ROBOT LOAD:" + app()->Task()->field(app()->taskIsps.at(i), curFieldId));
			KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(id)->instrument();
			robo->setStartEnvironment(app()->Task()->field(app()->taskIsps.at(i), curFieldId));
		}

		if (app()->compiler->modules()->module(id)->name == trUtf8("Водолей")) {
			app()->compiler->modules()->module(id)->Plugin()->setParameter("environment",
				QVariant(app()->Task()->field(app()->taskIsps.at(i), curFieldId)));
		}
	}
	app()->mainWindow->startTesting();
}

void Application::continueCheck()
{
	curFieldId++;
	if ((app()->taskIsps.count() == 0) && (curFieldId > 0)) {
		endCheck();
		return;
	}
	for (int i = 0; i < app()->taskIsps.count(); i++) {
		int id = app()->compiler->modules()->idByName(app()->taskIsps.at(i));
		if (curFieldId >= Task()->fieldsCount(taskIsps.at(i))) {
			endCheck();
			return;
		}

		if (app()->compiler->modules()->module(id)->name == "Robot") {
			app()->mainWindow->textEdit3->append("ROBOT LOAD:" + app()->Task()->field(app()->taskIsps.at(i), curFieldId));
			KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(id)->instrument();
			robo->setStartEnvironment(app()->Task()->field(app()->taskIsps.at(i), curFieldId));
		}

		if (app()->compiler->modules()->module(id)->name == trUtf8("Водолей")) {
			app()->compiler->modules()->module(id)->Plugin()->setParameter("environment",
				QVariant(app()->Task()->field(app()->taskIsps.at(i), curFieldId)));
		}
	}
	app()->mainWindow->startTesting();
}

bool Application::isTeacherMode()
{
	return QCoreApplication::arguments().contains("-t") ||
		QCoreApplication::arguments().contains("--teacher");
}

QStringList Application::ExtIspsList()
{
	QString data = settings->value("Isps", "").toString();
	return data.split(';');
}

void Application::RemoveAtExtIspsList(int id)
{
	QStringList data = settings->value("Isps", "").toString().split(';');
	if (data.count() > id) {
		data.removeAt(id);
		QString toSett = "";
		for (int i = 0; i < data.count(); i++) {
			toSett += data[i] + ";";
		}
		settings->setValue("Isps", toSett);
	} else {
		qDebug() << "RemoveAtExtIspsList:Bad id";
	}
}

void Application::AppendExtIspsToList(QString name, uint port)
{
	QString data = settings->value("Isps", "").toString();
	data = data + ";" + name + "," + QString::number(port);
	settings->setValue("Isps", data);
}

void Application::AppendExtIspsToList(QString name, uint port, QString url)
{
	QString data = settings->value("Isps", "").toString();
	data = data + ";" + name + "," + QString::number(port) + ',' + url;
	settings->setValue("Isps", data);
}


void fromTCInterface::setParam(QString paramname, QString value)
{
	qDebug() << paramname << " " << value;
	app()->mainWindow->setCurFile(value);
}

bool fromTCInterface::startNewTask(QStringList isps)
{
	for (int i = 0; i < isps.count(); i++) {
		int id = app()->compiler->modules()->idByName(isps[i]);
		//        qDebug()<<"Module need:"<<isps[i];
		if (id < 0) {
			return false;
		}
		if (app()->compiler->modules()->module(id)->isFriend()) {
			//            qDebug()<<"Set Module fields:"<<app()->Task()->field(isps[i],0);

			app()->compiler->modules()->module(id)->Plugin()->setParameter("environment",
				QVariant(app()->Task()->field(isps[i], 0)));
		}
		if (app()->compiler->modules()->module(id)->name == "Robot") {
			app()->mainWindow->textEdit3->append("ROBOT LOAD:" + app()->Task()->field(isps[i], 0));
			KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(id)->instrument();
			robo->setStartEnvironment(app()->Task()->field(isps[i], 0));
		}
	}
	app()->taskIsps = isps;
	app()->mainWindow->setWindowTitle(app()->Task()->name + QString::fromUtf8(" - Кумир"));
	return true;
}

bool fromTCInterface::setPreProgram(QVariant param)
{
	QString program_file = param.toString();
	// qDebug()<<"Program"<<app()->mainWindow->setWindowTitle(app()->Task()->name);
	if (program_file.right(4) == ".kum") {
		app()->mainWindow->fileOpenInFirstTab(program_file);
		app()->mainWindow->setWindowTitle(app()->Task()->name + QString::fromUtf8(" - Кумир"));
	} else {
		app()->mainWindow->setCurrentEditorText(program_file);
		app()->mainWindow->setWindowTitle(app()->Task()->name + QString::fromUtf8(" - Кумир"));
		if (!app()->isTeacherMode()) {
			app()->mainWindow->actionSave->setEnabled(false);
		}
	}
	return true;
}

void fromTCInterface::startProgram(QVariant param)
{
	//    qDebug()<<"StartProg";
	QString program_file = param.toString();
	app()->startCheck();
}


QString fromTCInterface::getText()
{
	QString text = app()->mainWindow->getPrgText();
	return text;
}

bool fromTCInterface::setTesting(QString param)
{
	app()->compiler->curEditor()->setPostScriptumText(param);
	// qDebug()<<"Set testing alg"<<param;
	return true;
}

