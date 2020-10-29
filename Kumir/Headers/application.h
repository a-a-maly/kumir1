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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtCore>
#include "../../TaskControl/taskControlInterface.h"
#include "../../TaskControl/csInterface.h"
#include "enums.h"
class QTcpSocket;

class MainWindow;
class SettingsDialog;
class ModulesRegistration;
class KumirEdit;
class Compiler;
class Kumir_Run;
class MessagesProvider;
class Macro;
class Assistant;
class DebugDialog;
class KNPServer;
class SignalCrosser;
class HttpDaemon;
class QNetworkAccessManager;
class PythonInterpreter;
class KumirPythonInterface;

class fromTCInterface: public CSInterface
{
public:
	// fromTCInterface(){};
	bool startNewTask(QStringList isps);
	QString getText();
	bool setPreProgram(QVariant param);
	bool setTesting(QString param);
	void startProgram(QVariant param);
	void setParam(QString paramname, QString value);
	int  stepsCount()
	{
		return 0;
	}
	QVariant runInternalFunc(QString, QVariant)
	{
		return 0;
	}


};

/** Класс приложения КУМИР */
class Application
	: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QStringList languageList READ languageList)
	Q_PROPERTY(QString currentLanguage READ currentLanguage)
public:
	/**
	* Конструктор приложения
	* @param argc количество аргументов командной строки
	* @param argv[] аргументы командной строки
	 */
	Application();
	static void createInstance();
public slots:
	void createMainWindow();
	void createCompiler();
	void createRuntime();
	void createSettings();
	void createNetworkUtilites();
	void createSecondaryWindows();
	void createHttpServer();
	void createMessagesProvider();
	void createConnections();
	void createMacros();
	void prepareBatch();
	void loadModules();
//                void startPython();
//                void loadConvertors(bool status);
	void createCrashHelper();
	void finishGUIInitialization();
	void finishBatchInitialization();
	QString configPath() const;
public:
	static Application *instance();
	/** Ссылка на главное окно приложения */
	MainWindow *mainWindow;
	/** Ссылка на окно настроек */
	SettingsDialog *settingsDialog;
	/** Ссылка на структуру настроек приложения */
	QSettings *settings;
	/** Ссылка на окно регистрации исполнителей */
	ModulesRegistration *modulesRegistration;
	QNetworkAccessManager *networkAccessManager;
	/** Ссылка на окно значений величин */
	DebugDialog *debugDialog;
	/** Возвращает список доступный языков */
	QStringList languageList();
	/** Возвращает текущий язык */
	QString currentLanguage();
	/**
	 * Инициализирует язык приложения в соответствии с настройками.
	 * При этом вызывает установку языка для всех компонентов
	 */
	void setLanguage();
	/** Ссылка на объект компилятора */
	Compiler *compiler;
	/** Ссылка на объект компилятора Kumir_Run */
	Kumir_Run *run;
	/** Ссылка на поставщик текстов сообщений об ошибках */
	MessagesProvider *messagesProvider;
	/** Список ссылок на доступные макросы */
	QList<Macro *> macros;
	QList<Macro *> standardMacros;
	/** Ссылка на окно помошника */
	Assistant *assistant;
	QWidget *assistantWindow;
	QWidget *variablesWindow;
	/** Ссылка на окно со списком ключевых слов */
	class QWebView *keywordsViewer;
	QWidget *keywordsWindow;
	/** Ссылка на поток вывода для пакетного режима */
	QFile *outFile;
	QFile *marginFile;
	/** В контейнер для протоколирования */
	void addEventToHistory(QString str);
	QChar kumirStyle();
	bool isZMode();
	bool isBatchMode();
	QString getZPath();
	QString getMacroDir();
	/** Ппроверяет наличие другого kumir-процесса
	 * @return true, если выполняется kumir с другими PID и PPID (Unix) или два экземпляра kumir.exe (Win32)
	 */
	bool isOtherKumirRunning();
	bool isTeacherMode()
	{
		return QCoreApplication::arguments().contains("-t") || QCoreApplication::arguments().contains("--teacher");
	}

	QStringList ExtIspsList()
	{
		QString data = settings->value("Isps", "").toString();
		return data.split(';');
	};
	void RemoveAtExtIspsList(int id)
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
		};
	};
	void AppendExtIspsToList(QString name, uint port)
	{
		QString data = settings->value("Isps", "").toString();
		data = data + ";" + name + "," + QString::number(port);
		settings->setValue("Isps", data);
	};
	void AppendExtIspsToList(QString name, uint port, QString url)
	{
		QString data = settings->value("Isps", "").toString();
		data = data + ";" + name + "," + QString::number(port) + ',' + url;
		settings->setValue("Isps", data);
	};
	QString ispErrorText()
	{
		return errorText;
	};
	void setIspErrorText(QString text)
	{
		errorText = text;
	};

	int kumPort()
	{
		return port;
	};
	bool isExamMode()
	{
		return isExam;
	};
	//Парсировка и добавление в таблицы алгоритмов исполнителя. Исполнитель должен существовать.
	void compileAlgList(int module_id, QStringList algList);
	void checkFriendConnections();//Проверка соединения с друзьями и их запск по необходимости.
	QList<QWidget *> secondaryWindows;
	KNPServer *kumServer;
	SignalCrosser *SigCross;
	QList<QTcpSocket *> Connections;
	bool ispMode;
	HttpDaemon *httpDaemon;
	QString errorText;
	void debugMsg(QString msg);
	taskControlInterface *TaskControl()
	{
		return taskControl;
	}
	fromTCInterface *CSInterface()
	{
		return csInterface;
	}
	bool isTaskControlLoaded()
	{
		return taskControlLoaded;
	}
	/** Загрузка библиотеки контроля заданий. Вызывается при запуске КУМИРа.*/
	void loadTaskControlPlugin();
	void startCheck();
	void continueCheck();

	void endCheck()
	{
		startControlScripts = false;
	};
	bool isScriptTesting()
	{
		return startControlScripts;
	}

	KumZadanie *Task()
	{
		return taskControl->Task();
	}

	QStringList taskIsps;
public slots:
	/**
	 * Компиляция текста программы из определенной вкладки.
	 * Если номер вкладки не указан или равен -1, то выполняется
	 * компиляция всех текстов.
	 * @param tabNo номер вкладки (нумерация с 0).
	 */
	void doCompilation();
	void startProgram();
	void finishBatch();
	/** Записать протокол действий пользователя на диск */
	void saveEventsHistory(QString str = "history.log");
	void emergencySaveState();
	bool emergencyRestoreState();
	void emergencyCleanState();
	void startBach();
//                void sendToConvertor();

protected slots:
	void handleSubprocessFinished(const QString &name, const QByteArray &reply, int errorCode);

protected:
	/** Чтение настроек приложения. Вызывается при запуске КУМИРа.*/
	void loadSettings();

	/** Чтение макросов с диска. Выполняется при запуске КУМИРа. */
	void loadMacros();
	void loadMacroFromFile(QString dir, QString fileName, bool editable);
	/** Список доступных языков */
	QStringList m_languageList;
	/** Текущий язык КУМИРа */
	QString m_currentLanguage;
	/** Ссылка на объект переводчика */
	QList<QTranslator *> translators;
	QTimer *batchTimeout;
	QStringList eventsHistory;
	/** Флаг пакетного режима */
	bool batchMode;
	bool zMode;
	QString zPath;
	QString s_configPath;
	enum { styleFull, styleText, styleClassic };
	int m_kumirStyle;
	QTimer *periodicSaveTimer;
	QTimer *runDelayTimer;
	QTimer *bachDelayTimer;
	QString macroDir;
	static Application *m_instance;
	int port;
	int port_max;

	int connectIspPort;
	QList<struct Convertor *> m_convertors;
	PythonInterpreter *m_convertorProcess;
	KumirPythonInterface *m_python;

	bool taskControlLoaded;
	taskControlInterface *taskControl;
	fromTCInterface *csInterface;
	bool startControlScripts;
	KumZadanie task;
	int curFieldId;
	bool isExam;


};

Application *app();

#endif
