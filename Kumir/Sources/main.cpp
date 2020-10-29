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

#include "application.h"
#include "compiler.h"
#include <iostream>
#include <QMessageBox>
#include <QSplashScreen>

#include "tools.h"
int main(int argc, char *argv[])
{


	QString msgLoadingSettings = QString::fromUtf8("Чтение настроек...");
	QString msgSettingLanguage = QString::fromUtf8("Установка языка...");
	QString msgPreparingCompiler = QString::fromUtf8("Подготовка компилятора...");
	QString msgPreparingRuntime = QString::fromUtf8("Подготовка среды выполнения...");
	QString msgPreparingMainWindow = QString::fromUtf8("Подготовка главного окна...");
	QString msgPreparingChildWindows = QString::fromUtf8("Подготовка дополнительных окон...");
	QString msgLoadingModules = QString::fromUtf8("Загрузка исполнителей...");
	QString msgPreparingNetwork = QString::fromUtf8("Подготовка сетевого окружения...");
	QString msgSettingInterconnections = QString::fromUtf8("Установление внутренних связей...");
	QString msgLoadingMacros = QString::fromUtf8("Загрузка макросов...");
	QString msgLoadKumirModules = QString::fromUtf8("Загрузка исполнителей учителя...");
	QString msgStartingEmergencyRestore = QString::fromUtf8("Подготовка аварийного восстановления...");
	if (QLocale::system().language() != QLocale::Russian) {
		msgLoadingSettings = "Loading settings...";
		msgSettingLanguage = "Selecting language...";
		msgPreparingCompiler = "Preparing compiler...";
		msgPreparingRuntime = "Preparing runtime...";
		msgPreparingMainWindow = "Preparing main window...";
		msgPreparingChildWindows = "Preparing secondary windows...";
		msgLoadingModules = "Loading modules...";
		msgPreparingNetwork = "Preparing network connections...";
		msgSettingInterconnections = "Setting application interconnections...";
		msgLoadingMacros = "Loading macros...";
		msgStartingEmergencyRestore = "Starting emergency restore...";
		msgLoadKumirModules = "Loading teacher's modules...";
	}
	bool headless = false;
	bool showHelpAndExit = false;
	QString message;
	for (int i = 0; i < argc; i++) {
		QString param = QString(argv[i]);
		if (param == "-batch" || param == "--batch" || param == "-b") {
			headless = true;
		} else if (param == "-h" || param == "--help") {
			message += QString::fromUtf8("Система программирования КуМир\n");
			message += QString::fromUtf8("\n");
			message += QString::fromUtf8("Использование: kumir [-h] [--config=путь_к_файлу] [--workdir=путь_к_каталогу]\n");
			message += QString::fromUtf8("\n");
			message += QString::fromUtf8("-h, --help\tПоказать это сообщение и завершить работу\n");
			message += QString::fromUtf8("\n");
			message += QString::fromUtf8("--config=путь_к_файлу\tХранить настройки в файле \"путь_к_файлу\"\n");
			message += QString::fromUtf8("--workdir=путь_к_каталогу\tиспользовать каталог \"путь_к_каталогу\" в качестве рабочего\n");
			std::cout << message.toUtf8().data();
#ifndef Q_OS_WIN32
			exit(0);
#endif
			showHelpAndExit = true;
		}
	}
	QCoreApplication *application;
	QSplashScreen *splash = NULL;
	if (headless) {
		// TODO: Make QCoreApplication instead of QApplication!!!!!
		application = new QApplication(argc, argv);
		//      application = new QCoreApplication(argc, argv);
	} else {
		application = new QApplication(argc, argv);
	}
	if (showHelpAndExit) {
		QMessageBox *w_usage = new QMessageBox(QMessageBox::Information, "Kumir", message);
		w_usage->setMinimumWidth(900);
		QObject::connect(w_usage, SIGNAL(buttonClicked(QAbstractButton *)), application, SLOT(quit()));
		w_usage->show();
		return application->exec();
	}
	if (!headless) {
#ifndef Q_OS_MAC
		splash = new QSplashScreen(QCoreApplication::applicationDirPath() + "/Kumir/Images/splashscreen.png");
#endif
#ifdef Q_OS_MAC
		splash = new QSplashScreen(QCoreApplication::applicationDirPath() + "/../Resources/Images/splashscreen.png");
#endif
		splash->show();
	}
	QCoreApplication::setOrganizationName(QString::fromUtf8("НИИСИ РАН"));
	QCoreApplication::setApplicationName(QString::fromUtf8("Кумир"));
	QCoreApplication::setApplicationVersion("1.8.1");
	Application::createInstance();
	if (splash) {
		splash->showMessage(msgLoadingSettings, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createSettings();
	if (splash) {
		splash->showMessage(msgSettingLanguage, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createMessagesProvider();
	Application::instance()->setLanguage();
	if (splash) {
		splash->showMessage(msgPreparingCompiler, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createCompiler();
	if (splash) {
		splash->showMessage(msgPreparingRuntime, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createRuntime();
	if (splash) {
		splash->showMessage(msgLoadKumirModules, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->compiler->loadKumirModules();
//	Application::instance()->compiler->updateMainId();
	if (splash) {
		splash->showMessage(msgPreparingMainWindow, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->loadTaskControlPlugin();
	Application::instance()->createMainWindow();
	if (splash) {
		splash->showMessage(msgPreparingChildWindows, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createSecondaryWindows();
	if (splash) {
		splash->showMessage(msgLoadingModules, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->loadModules();
//	Application::instance()->compiler->updateMainId();
	if (splash) {
		splash->showMessage(msgPreparingNetwork, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createNetworkUtilites();
	if (splash) {
		splash->showMessage(msgSettingInterconnections, Qt::AlignBottom | Qt::AlignHCenter);
	}
	Application::instance()->createConnections();

	if (headless) {
		Application::instance()->prepareBatch();
		Application::instance()->finishBatchInitialization();
	} else {
		if (splash) {
			splash->showMessage(msgLoadingMacros, Qt::AlignBottom | Qt::AlignHCenter);
		}
		Application::instance()->createMacros();
		if (splash) {
			splash->showMessage(msgStartingEmergencyRestore, Qt::AlignBottom | Qt::AlignHCenter);
		}
		Application::instance()->createCrashHelper();
		if (splash) {
			splash->clearMessage();
		}
		Application::instance()->finishGUIInitialization();
		if (splash) {
			splash->hide();
		}
	}
	return application->exec();
}
