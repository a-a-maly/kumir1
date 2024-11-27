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
#include "run.h"
#include "network.h"
#include "int_proga.h"
#include "files_isp.h"
#include "files_p_isp.h"
//#include "strtypes.h"
#include "kassert.h"
#include "config.h"
#include "compiler.h"
#include "kummodules.h"
#include "kumsinglemodule.h"

#include <QTcpSocket>

#define VALUES_REFRESH_TIME 200
#ifdef Q_OS_WIN32
#define MAX_CALL_DEEP 700
#include <time.h>
#elif defined(Q_OS_MAC)
#define MAX_CALL_DEEP 180
#else
#define MAX_CALL_DEEP 1800
#endif

#define REAL_RES_DEBUG false

Kumir_Run::Kumir_Run(QObject *parent)
	: QThread(parent)
{


	i_delay = -1;

	running = false;
	BreakFlag = false;
	child = NULL;
	stoped = false;
	run_stack.clear();
	parentRun = NULL;
	ispErrorCode = 0;
	connect(this, SIGNAL(resetPlugin(int)), app()->compiler->modules(), SLOT(resetPlugin(int)));


};

function_table *Kumir_Run::Cur_Functions()
{
	KumSingleModule *module = Modules->module(curModule);
	function_table *table = module->Functions();
	return table;
}

symbol_table *Kumir_Run::Cur_Symbols()
{
	KumSingleModule *module = Modules->module(curModule);
	symbol_table *table = module->Values();
	return table;
}

void Kumir_Run::StartProgaram()
{
	run();
};

void Kumir_Run::init(RunSetup Setup, int moduleId)
{
	i_delay = -1;
	m_skipHiddenAlgorhitm = false;
	setup = Setup.copy();
	GlubCircle = 0;
	//Proga_Value=(QList<proga>)Setup.Proga_Value;
	Modules = Setup.modules();
	output = Setup.Output();
	debugW = Setup.DebugW();
	main = Setup.Main();
	curModule = moduleId;
	isFunction = false;
	QList<proga> debug = *(Modules->module(curModule)->Proga_Value());
	//qDebug() << "DATA: ===============================";
	//for (int i=0; i<debug.count(); i++ ) {
	//proga p = debug[i];
	//qDebug() << p.line << " " << p.real_line.lineNo;
	//}
	//qDebug() << "END DATA: ===============================";
	mode = Setup.Mode();
	ready = Setup.isReady();
	start_str = Setup.Start();
	ISP_BASE = Setup.ISP_BASE;
	deep = 0;

	running = false;
	syncFlag = true;
	pauseFlag = false;
	deleteChild();
	child = NULL;
	stoped = false;
	run_stack.clear();
	//  leadOut();
}


int Kumir_Run::initForFunc//Инициализация
(RunSetup Setup, int Deep, int func_str, Kumir_Run *parent_Run, int moduleId)
{
	// qDebug("Kumir_Run::initForFunc");
	i_delay = -1;
	parentRun = parent_Run;
	isFunction = true;
	setup = Setup.copy();
	GlubCircle = 0;
	curModule = moduleId;
	Modules = Setup.modules();
	//Proga_Value=(QList<proga>)Setup.Proga_Value;
	//functions=Setup.Functions();
	//symbols=Setup.Symbols();
	output = Setup.Output();
	debugW = Setup.DebugW();
	main = Setup.Main();
	//ispolniteli=Setup.Ispolniteli();

	mode = Setup.Mode();

	ready = Setup.isReady();
	start_str = func_str;
	ISP_BASE = Setup.ISP_BASE;
	deep = Deep;

	running = false;
	syncFlag = true;
	child = NULL;
	stoped = false;
	run_stack.clear();
	return 0;
};


void Kumir_Run::sendText(QString text)
{
	if (isFunction) {
		parentRun->sendText(text);
		return;
	};
	syncFlag = true; //IF SYNC needed false
	// int t;
	// if(t==0)usleep(1);
	emit textMessage(text);
	usleep(5);
	while ((!syncFlag) && (!stoped)) {
		usleep(30);
	};
	//qDebug("Send");
};
void Kumir_Run::sendChangeIn(QString fileName)
{
	if (isFunction) {
		parentRun->sendChangeIn(fileName);
		return;
	};
	syncFlag = true; //IF SYNC needed false

	emit setInFile(fileName);
	usleep(5);
	while ((!syncFlag) && (!stoped)) {
		usleep(30);
	};
	//qDebug("Send");
};
void Kumir_Run::sendChangeOut(QString fileName)
{
	if (isFunction) {
		parentRun->sendChangeOut(fileName);
		return;
	};
	syncFlag = true; //IF SYNC needed false
	//    qDebug()<<"Send Change Out";
	emit setOutFile(fileName);
	usleep(5);
	while ((!syncFlag) && (!stoped)) {
		usleep(30);
	};
	//qDebug("Send");
};
void Kumir_Run::switchIspMode(int mode)
{
	for (int i = 0; i <= Modules->lastModuleId(); i++)
		if (Modules->module(i)->isEnabled()) {
			if (Modules->module(i)->isNetwork()) {
				connect(this,
					SIGNAL(SwitchIspMode(int)), Modules->module(i)->Connection(),
					SLOT(switchMode(int)));
				syncFlag = false;
				emit SwitchIspMode(mode); //1 - run mode
				//usleep(1500);
				waitForSync();
				disconnect(this,
					SIGNAL(SwitchIspMode(int)), Modules->module(i)->Connection(),
					SLOT(switchMode(int)));
				continue;
			};
			if (!Modules->module(i)->Graphics()) {
				continue;
			}
			connect(this,
				SIGNAL(SwitchIspMode(int)), Modules->module(i),
				SLOT(SwitchIspMode(int)));
			syncFlag = false;
			emit SwitchIspMode(mode); //1 - run mode
			waitForSync();
			disconnect(this, SIGNAL(SwitchIspMode(int)), Modules->module(i),
				SLOT(SwitchIspMode(int)));
		};
	//Modules->module(i)->instrument()->showHideWindow(true);
};

void Kumir_Run::Pause(int str, int module_id)
{
	if (Modules->module(module_id)->Proga_Value_Line(str).real_line.tab == NULL || Modules->module(module_id)->Proga_Value_Line(str).real_line.lineNo == -1) {
		return;
	}
	if (isFunction) {
		parentRun->Pause(str, module_id);
		return;
	};
	pauseFlag = false;

	emit pause(str, module_id);
	usleep(5);
	while ((!pauseFlag) && (!stoped)) {
		usleep(100);
	};
	switchIspMode(1);
	//qDebug("Continue");
};

void Kumir_Run::waitForSync()
{
	//    qDebug()<<"Wait for sync";
	//syncFlag=false;
	while ((!syncFlag) && (!stoped)) {
		usleep(20);
	};
	//    qDebug()<<"Wait for sync OK";

};

void Kumir_Run::ispError(QString text)

{

	if (child) {
		child->ispError(text);
		return;
	};
	//    qDebug()<<"Isp Error: " <<text;
	app()->setIspErrorText(text);
	ispErrorCode = ISP_INT_ERROR;
	//StopRun();
	//sendText(trUtf8("\nОшибка исполнителя:")+text);
	//qDebug()<<"Isp Error";
	SyncMessage();



};

void Kumir_Run::sendDebug(QString text, int str, int module_id)
{
	if (Modules->module(module_id)->Proga_Value_Line(str).real_line.tab == NULL) {
		return;
	}
	if (isFunction) {
		parentRun->sendDebug(text, str, module_id);
		return;
	};
	//syncFlag=false;//IF SYNC needed false
	//int t;
	//if(t==0)usleep(1);
	emit debugMessage(text, str, module_id);
	usleep(5);
	int wt;
	wt = 0;
	while ((!syncFlag) && (!stoped)) {
		usleep(5);
		wt = wt + 5;
	};
	//qDebug("Debug lag time %i msec",wt);
};



void Kumir_Run::sendGraphics(QString name, QList<KumValueStackElem> *arguments, int *err)
{
	if (isFunction) {

		parentRun->sendGraphics(name, arguments, err);

		return;
	};
	syncFlag = false; //IF SYNC needed false
	//qDebug()<<"Name in run:"<< name;
	emit GraphIspFunc(name, arguments, err);
	//usleep(1500);
	int sync_lag = 0;
	while ((!syncFlag) && (!stoped)) {
		usleep(30);
		sync_lag = sync_lag + 30;
		//        if(sync_lag<60)
		//            qDebug()<< "Sync";
	};
	//qDebug()<< "Lag time "<<sync_lag;
	//qDebug("Send");
};




void Kumir_Run::sendValues()
{
	if (isFunction) {

		parentRun->sendValues();

		return;
	};
	syncFlag = false; //IF SYNC needed false
	//qDebug()<<"Name in run:"<< name;
	emit updateValues();
	//usleep(1500);
	int sync_lag = 0;
	while ((!syncFlag) && (!stoped)) {
		usleep(10);
		sync_lag = sync_lag + 30;
	};
};



void Kumir_Run::sendNetwork(int module_id, QString name, QList<KumValueStackElem> *arguments, int *err)
{

	if (isFunction) {

		parentRun->sendNetwork(module_id, name, arguments, err);

		return;
	};
	syncFlag = false; //IF SYNC needed false
	//qDebug()<<"Name in run:"<< name;
	connect(this, SIGNAL(sendNetwMessage(QString)), Modules->module(module_id)->Connection(), SLOT(sendCmd(QString)));
	QString message = name + ",";
	for (int i = 0; i < arguments->count(); i++) {
		KumValueStackElem argument = arguments->at(i);

		message = message + argument.asString() + ",";
	};
	syncFlag = false;
	emit sendNetwMessage(message + '\n');
	disconnect(this, SIGNAL(sendNetwMessage(QString)), Modules->module(module_id)->Connection(), SLOT(sendCmd(QString)));
	usleep(50);
	if (ispErrorCode == 0) {
		waitForSync();
	}
	qDebug() << "Wait for sync from sendNetwork";
	*err = ispErrorCode;
	int funct_id = Modules->module(module_id)->Functions()->idByName(name);
	if (Modules->module(module_id)->Functions()->typeById(funct_id) != none) {
		int perem_id = Modules->module(module_id)->Values()->inTable(name, "global");
		if (perem_id == -1) {
			*err = 2;
			return;
		};
		//TODO setValue
		if (Modules->module(module_id)->Functions()->typeById(funct_id) == kumString) {
			Modules->module(module_id)->Values()->symb_table[perem_id].value.setStringValue(Value);
		};
		if (Modules->module(module_id)->Functions()->typeById(funct_id) == integer) {
			bool ok;
			int iValue = Value.toInt(&ok);
			if (!ok) {
				*err = RUN_BAG_TYPE_FROM_ISP;
				return;
			};
			Modules->module(module_id)->Values()->symb_table[perem_id].value.setIntegerValue(iValue);
		};
		if (Modules->module(module_id)->Functions()->typeById(funct_id) == real) {
			bool ok;
			float fValue = Value.toFloat(&ok);
			if (!ok) {
				*err = RUN_BAG_TYPE_FROM_ISP;
				return;
			};
			Modules->module(module_id)->Values()->symb_table[perem_id].value.setFloatValue(fValue);
		};
		if (Modules->module(module_id)->Functions()->typeById(funct_id) == kumBoolean) {
			bool ok = false;
			bool bValue;
			if (Value == trUtf8("да")) {
				bValue = true;
				ok = true;
			};
			if (Value == trUtf8("нет")) {
				bValue = false;
				ok = true;
			};

			if (!ok) {
				*err = RUN_BAG_TYPE_FROM_ISP;
				return;
			};
			Modules->module(module_id)->Values()->symb_table[perem_id].value.setBoolValue(bValue);
		};
		Modules->module(module_id)->Values()->setUsed(perem_id, true);
		qDebug() << "Get Value:" << Value << "Perem_id:" << perem_id;
	};
	//qDebug()<< "Lag time "<<sync_lag;
	//qDebug("Send");
};

void Kumir_Run::sendError(int error, int stroka, int module_id, const QString &sourceText)
{
	if (isFunction) {

		parentRun->sendError(error, stroka, module_id, sourceText);

		return;
	};
	syncFlag = true; //IF SYNC needed false

	emit runError(error, stroka, module_id, sourceText);
	usleep(5);
	while ((!syncFlag) && (!stoped)) {
		usleep(30);
	};
	ispErrorCode = 0;
	//qDebug("Send");
};

void Kumir_Run::sendInput(QString format)
{
	if (isFunction) {
		parentRun->sendInput(format);
		return;
	};
	app()->mainWindow->activateWindow();
	emit waitForInput(format);
	usleep(50);

};


void Kumir_Run::sendStep(void)
{
	if (isFunction) {
		parentRun->sendStep();
		return;
	};

	emit Step();
	usleep(10);

};


int Kumir_Run::initIsps(void)
{
	ispErrorCode = 0;
	app()->setIspErrorText("");
	for (int i = 0; i <= Modules->lastModuleId(); i++) {
		if (!Modules->module(i)->isEnabled()) {
			continue;
		}

		if (Modules->module(i)->isNetwork()) {
			connect(Modules->module(i)->Connection(),
				SIGNAL(GetModuleError(QString)), this,
				SLOT(ispError(QString)));

			connect(Modules->module(i)->Connection(),
				SIGNAL(getOK()), this,
				SLOT(SyncMessage()));
			connect(Modules->module(i)->Connection(),
				SIGNAL(getReturn(QString)), this,
				SLOT(ispReturn(QString)));
			syncFlag = false;
			Modules->module(i)->Connection()->sendCmd("reset\n");
			waitForSync();

			continue;
		};
		if (Modules->module(i)->Graphics()) {
			connect(Modules->module(i),
				SIGNAL(sync()), this,
				SLOT(SyncMessage()));
			Modules->module(i)->connectSig();
			if (Modules->module(i)->isEnabled()) {
				//Modules->module(i)->instrument()->showHideWindow(true);
				qDebug("Show Window!!!");

				connect(this,  SIGNAL(GraphIspFunc(QString, QList<KumValueStackElem> *, int *)), Modules->module(i),
					SLOT(runFunc(QString, QList<KumValueStackElem> *, int *)), Qt::BlockingQueuedConnection);
				qDebug("Connect");

				connect(this,
					SIGNAL(SwitchIspMode(int)), Modules->module(i),
					SLOT(SwitchIspMode(int)));
				//emit SwitchIspMode(1); //1 - run mode
				//waitForSync();
				disconnect(this, SIGNAL(SwitchIspMode(int)), Modules->module(i),
					SLOT(SwitchIspMode(int)));
				emit resetPlugin(i);
			};

			connect(Modules->module(i),
				SIGNAL(IspError(QString)), this,
				SLOT(ispError(QString)));
		}

		if ((Modules->module(i)->isKumIsp()) && (Modules->module(i)->isEnabled())) {
			// qDebug()<<"ModName"<<Modules->module(i)->Name();

			if (Modules->module(i)->Name() == "@main") {
				continue;
			}

			int to = -1;

			if (Modules->module(i)->Functions()->size() > 0) {
				to = Modules->module(i)->Functions()->posById(0);
			}

			if (to > -1) {
				runFromTo(i, 1, to);
			}
		}
	}

	switchIspMode(1);
	return 0;
};


void Kumir_Run::leadOut()
{
	switchIspMode(SEE_MODE);//SEE_MODE
	usleep(100);
	for (int i = 0; i <= Modules->lastModuleId(); i++) {
		if (Modules->module(i)->isNetwork()) {

			disconnect(Modules->module(i)->Connection(),
				SIGNAL(GetModuleError(QString)), this,
				SLOT(ispError(QString)));
			disconnect(Modules->module(i)->Connection(),
				SIGNAL(getOK()), this,
				SLOT(SyncMessage()));
			disconnect(Modules->module(i)->Connection(),
				SIGNAL(getReturn(QString)), this,
				SLOT(ispReturn(QString)));

		};
		if (Modules->isGraphics(i)) {
			disconnect(Modules->module(i),
				SIGNAL(sync()), this,
				SLOT(SyncMessage()));
			Modules->module(i)->disconectSig();
			//if(Modules->module(i)->isEnabled()){
			//  qDebug("----------LeadOut-----------------");
			//Modules->module(i)->instrument()->SwitchMode(3); //1 - analize mode
			//};

			disconnect(this,  SIGNAL(GraphIspFunc(QString, QList<KumValueStackElem> *, int *)), Modules->module(i),
				SLOT(runFunc(QString, QList<KumValueStackElem> *, int *)));
			disconnect(Modules->module(i),
				SIGNAL(IspError(QString)), this,
				SLOT(ispError(QString)));
		};
		if (Modules->module(i)->Name() == "FilesP") {
			Modules->module(i)->instrument()->reset();
		};
	};

};

void Kumir_Run::run()
{
	uint time_seed = QDateTime::currentDateTime().toTime_t();
#ifdef Q_OS_WIN32
	uint clock_seed = clock();
	qsrand(time_seed + clock_seed);
#else
	qsrand(time_seed);
#endif

	if (!ready) {
		output->append("INTERNAL ERROR: RUN NOT READY");
		emit finished();
		return;
	};
	running = true;

	//    RunBreakFlag=false;
	initIsps(); //Инициализация исполнителей
	oldTime = clock_m.currentMSecsSinceEpoch();
	curModule = Modules->idByName("@main");

	if (setup.initMainNeeded()) {


		int to = -1;

		if (Modules->module(curModule)->Functions()->size() > 0) {
			to = Modules->module(curModule)->Functions()->posById(0);
		}
		qDebug() << "Init Main" << to;
		if (to > -1) {
			runFromTo(curModule, 0, to);
		}
	}
	freeStack();
	run_stack.push_back(start_str);
	int rsc = run_stack.count();

	stoped = false;

	bool runStackNotEmpty = run_stack.count() > 0;
	int cnt = Cur_Proga_Value()->count();
	if (cnt < 1) {
		running = false;
		emit finished();
		exit();
		return;
	};

	while (runStackNotEmpty) {
		// qDebug("Main Run Circle");
		int cur_str = pop();
		if (cur_str >= Cur_Proga_Value()->count()) {
			leadOut();
			usleep(300);
			emit finished();
			running = false;
			exit();
			return;
		};
		rsc = run_stack.count();
		bool test = false;
		if (rsc > 0) {
			test = Cur_Line(cur_str).is_break;
		}
		int err = 0;
		QString errSource;
		if (stoped) {
			leadOut();
			usleep(300);
			emit finished();
			exit();
		};
		if ((setup.isPoShagam()) || (test)) {
			if (Cur_Line(cur_str).real_line.tab) {
				Pause(cur_str, curModule);
			}
		};



		if (!stoped) {
			QPair<int, QString> p = do_line(cur_str);
			err = p.first;
			errSource = p.second;
			sendStep();
		} else {
			leadOut();
			usleep(300);
			emit finished();
			exit();
		};




		rsc = run_stack.count();
		if ((err != 0) || (ispErrorCode != 0)) {
			if (ispErrorCode != 0) {
				err = ispErrorCode;
			}
			running = false;
			//sendText("Run Err"+QString::number(err));
			leadOut();
			if (err == -1) {
				return;
			}
			if (err == -2) {
				exit();
				emit finished();
				return;
			};
			if (!stoped) {
				emit runError(err, cur_str, curModule, errSource);
			}
			exit();
			emit finished();
			return;
		};
		runStackNotEmpty = run_stack.count() > 0;
		cnt = Cur_Proga_Value()->count();
		//lineNoInProga = run_stack.last()<cnt;
	};



	running = false;
	leadOut();
	emit finished();
	exit();

};




int Kumir_Run::setArguments(symbol_table *arguments, int func_id)
{
	for (int i = 0; i < Cur_Functions()->argCountById(func_id); i++) {
		int perem_id = Cur_Functions()->argPeremId(func_id, i);
		bool isArgRes = arguments->isArgRes(i);
		bool isRes = arguments->isRes(i);
		if (isArgRes || isRes) {
			qDebug() << "ArgREZ";
			PeremPrt pointer = arguments->getPointer(i);
			Cur_Symbols()->setPointer(perem_id, pointer.mod_id, pointer.perem_id);
			bool used = Modules->module(pointer.mod_id)->Values()->isUsed(pointer.perem_id);
			if (isArgRes) {
				Cur_Symbols()->setUsed(perem_id, used);
			} else {
				qDebug() << "REZ";
				Modules->module(pointer.mod_id)->Values()->setUsed(pointer.perem_id, false);
				Cur_Symbols()->setUsed(perem_id, false);
			};

			continue;
		};
		switch (Cur_Functions()->argTypeById(func_id, i)) {
		case integer: {
			//int value=argValue.asInt();
			Cur_Symbols()->symb_table[perem_id]
			.value.setIntegerValue(arguments->getValueAsInt(i));
			Cur_Symbols()->setUsed(perem_id, true);
		};
		break;

		case real: {
			//float value=argValue.asFloat();
			Cur_Symbols()->symb_table[perem_id]
			.value.setFloatValue(arguments->getValueAsFloat(i));
			Cur_Symbols()->setUsed(perem_id, true);
		};
		break;

		case kumString: {
			//  QString value=argValue.asString();
			Cur_Symbols()->symb_table[perem_id]
			.value.setStringValue(arguments->getValueAsString(i));
			Cur_Symbols()->setUsed(perem_id, true);
		};
		break;

		case charect: {
			//  QString value=argValue.asString();
			Cur_Symbols()->symb_table[perem_id]
			.value.setCharectValue(arguments->getValueAsCharect(i));
			Cur_Symbols()->setUsed(perem_id, true);
		};
		break;
		case kumBoolean: {
			// bool value=argValue.asBool();
			Cur_Symbols()->symb_table[perem_id]
			.value.setBoolValue(arguments->getValueAsBool(i));
			Cur_Symbols()->setUsed(perem_id, true);
		};
		break;


		case mass_bool:
		case mass_integer:
		case mass_real:
		case mass_string:
		case mass_charect: {
			symbol_table *myTable = Cur_Symbols();
			int strNo = Cur_Functions()->posById(func_id);
			const QString line = Modules->module(curModule)->Proga_Value_Line(strNo).line;
			int start = line.indexOf("(");
			int end = line.lastIndexOf(")");
			const QString varDeclLine = line.mid(start, end - start).split(",")[i];
			const QStringList exprs = varDeclLine.split("#");
			PeremPrt p;
			p.mod_id = curModule;
			p.perem_id = perem_id;
			int locError = calcMassSize(exprs, p, strNo);
			if (locError) {
				return locError;
			}
			if (myTable->symb_table[perem_id].razm >= 1) {
				int myStart = myTable->symb_table[perem_id].size0_start;
				int myEnd = myTable->symb_table[perem_id].size0_end;
				int argStart = arguments->symb_table[i].size0_start;
				int argEnd = arguments->symb_table[i].size0_end;
				if (myStart < argStart || myEnd > argEnd) {
					return RUN_ARRAY_BOUND;
				}
			}
			if (myTable->symb_table[perem_id].razm >= 2) {
				int myStart = myTable->symb_table[perem_id].size1_start;
				int myEnd = myTable->symb_table[perem_id].size1_end;
				int argStart = arguments->symb_table[i].size1_start;
				int argEnd = arguments->symb_table[i].size1_end;
				if (myStart < argStart || myEnd > argEnd) {
					return RUN_ARRAY_BOUND;
				}
			}
			if (myTable->symb_table[perem_id].razm >= 3) {
				int myStart = myTable->symb_table[perem_id].size2_start;
				int myEnd = myTable->symb_table[perem_id].size2_end;
				int argStart = arguments->symb_table[i].size2_start;
				int argEnd = arguments->symb_table[i].size2_end;
				if (myStart < argStart || myEnd > argEnd) {
					return RUN_ARRAY_BOUND;
				}
			}
			myTable->symb_table[perem_id].value.init(perem_id
				, Cur_Functions()->argTypeById(func_id, i)
				, myTable->symb_table[perem_id].size0_start
				, myTable->symb_table[perem_id].size0_end
				, myTable->symb_table[perem_id].size1_start
				, myTable->symb_table[perem_id].size1_end
				, myTable->symb_table[perem_id].size2_start
				, myTable->symb_table[perem_id].size2_end
				, myTable->symb_table[perem_id].razm
			);
			myTable->setTableValues(perem_id, arguments, i);
			myTable->setUsed(perem_id, true);
		};
		break;

		//        case mass_integer:
		//            {
		//                arguments->copyMassIntArg ( i,perem_id,Cur_Symbols() );
		//                Cur_Symbols()->setUsed(perem_id,true);
		//            };
		//            break;

		//        case mass_real:
		//            {
		//                arguments->copyMassRealArg ( i,perem_id,Cur_Symbols() );
		//                Cur_Symbols()->setUsed(perem_id,true);
		//            };
		//            break;

		//        case mass_string:
		//            {
		//                arguments->copyMassStringArg ( i,perem_id,Cur_Symbols() );
		//                Cur_Symbols()->setUsed(perem_id,true);
		//            };
		//            break;
		//        case mass_charect:
		//            {
		//                arguments->copyMassCharectArg ( i,perem_id,Cur_Symbols() );
		//                Cur_Symbols()->setUsed(perem_id,true);
		//            };
		//            break;
		default:
			break;
		};

	};
	return 0;
};



int Kumir_Run::runForFunc(int func_id, symbol_table *arguments)
{

	status = 0;
	if (ready) {
		// qDebug("Run4Func");
		freeStack();
		if (setup.isPoShagam()) {
			setup.func_mode = debug;
		} else {
			setup.func_mode = neprerivno;
		}
		run_stack.push_back(start_str);
		int rsc = run_stack.count();
		running = true;
		int err = setArguments(arguments, func_id);
		if (err) {
			return err;
		}

		while (run_stack.count() > 0) {
			int cur_str = pop();
			rsc = run_stack.count();
			// run_mutex.lock();
			if ((setup.isPoShagam())) {
				Pause(cur_str, curModule);
			};



			int err = 0;
			QString errSource;
			if (!stoped) {
				QPair<int, QString> p = do_line(cur_str);
				err = p.first;
				errSource = p.second;
				sendStep();
			} else {
				usleep(20);
				return status;
			};

			// BuffOut();
			// qDebug("FuncBuffOut_OK.");

			// run_mutex.unlock();

			rsc = run_stack.count();
			if ((err != 0) || (ispErrorCode != 0)) {
				if (ispErrorCode != 0) {
					err = ispErrorCode;
				}
				if ((Cur_Line(cur_str).real_line.tab) && (err != -2)) { //Строка отображена пользователю
					sendError(err, cur_str, curModule, errSource);
					return -2;
				};
				status = err;

				return status;
			};
		};



		Modules->module(curModule)->Values()->resetInitFlag(Modules->module(curModule)->Functions()->nameById(func_id));


	};
	return status;
}



void Kumir_Run::setMode(RunMode run_mode) //Function
{
	if (child) {
		child->setMode(run_mode);
	}
	setup.setMode(run_mode);
	mode = run_mode;
};

void Kumir_Run::setFuncDebugMode(int run_mode) //Slot
{
	if (child) {
		child->setFuncDebugMode(run_mode);
	}
	if (run_mode == 1) {
		setup.func_mode = po_shagam;
	}
	if (run_mode == 2) {
		setup.func_mode = neprerivno;
	}
	if (run_mode == 4) {
		setup.func_mode = debug;
	}
};


void Kumir_Run::Continue() //Slot
{
	pauseFlag = true;
};

void Kumir_Run::DoConcaAlg() //Slot
{
	if (child) {
		child->DoConcaAlg();
		pauseFlag = true;
		return;
	}
	setup.func_mode = debug;
	setup.setMode(debug);
	if (parentRun) {
		parentRun->setSelfMode(po_shagam);
	}
	pauseFlag = true;
};

bool Kumir_Run::isRunning() const
{
	return running;
}

void Kumir_Run::stop_run() //FUNCTION
{

	freeStack();
	stoped = true;
	if (child) {
		child->stop_run();
	}
};

void Kumir_Run::StopRun() //SLOT
{
	stop_run();
};




void Kumir_Run::SyncMessage() //SLOT
{
	//qDebug("Sync");
	if (child) {
		child->SyncMessage();
	}
	syncFlag = true;
};

void Kumir_Run::ispReturn(QString value) //SLOT
{
	//qDebug("Sync");
	if (child) {
		child->ispReturn(value);
	}
	Value = value;
	syncFlag = true;
};

int Kumir_Run::pop()
{
	if (run_stack.count() > 0) {
		int temp = run_stack.last();
		run_stack.pop_back();

		return temp;
	};
	return -1;


};

void Kumir_Run::freeStack()
{
	if (!run_stack.empty()) {
		run_stack.clear();
	}
};


//Выполнение программы
QPair<int, QString> Kumir_Run::do_line(int str)
{
	if ((OldTime() + VALUES_REFRESH_TIME) < clock_m.currentMSecsSinceEpoch()) {
		//qDebug()<<clock.currentMSecsSinceEpoch()<<" OLD"<<OldTime();
		sendValues();
		SetOldTime(clock_m.currentMSecsSinceEpoch());
	};
	//qDebug("Run:do_line %i",str);
	if (setup.isPoShagam()) {
		emit sendMessage("", 0);
	}
	int err = 0;
	QString errSource;
	bool skipInitVariables = false;

	proga cl = Cur_Line(str);
	if (cl.forceJump != -1 && !m_skipHiddenAlgorhitm) {
		cl = Cur_Line(cl.forceJump);
		m_skipHiddenAlgorhitm = true;
		skipInitVariables = true;
	}

	errSource = cl.source;

	if (cl.error > 0) {


		//     OutBuff->appendErrMsg( QString::fromUtf8("ОШИБКА ВЫПОЛНЕНИЯ: ")+run_err(Cur_Line(str).error),Cur_Line(str).real_line);


		return QPair<int, QString>(cl.error, cl.source);

	};
	//int str_type=Cur_Line(str).str_type;
	int nextLine;
	switch (cl.str_type) {
	case Kumir::Empty:
		run_stack.push_back(str + 1);
		break;
	case Kumir::UseModule:
		run_stack.push_back(str + 1);
		break;
	case Kumir::Doc:
		run_stack.push_back(str + 1);
		break;
	case Kumir::AlgDecl: {
//        if (!skipInitVariables)
//            err=initPerem(str,curModule);
		nextLine = cl.else_pos;
		run_stack.push_back(nextLine);
		break;
	}
	case Kumir::AlgBegin:
		run_stack.push_back(str + 1);
		break;


	case Kumir::LoopBegin:
		err = run_circle(str);
		break;
	case Kumir::LoopEnd: {
		int NC = pop();
		pop();
		run_stack.push_back(NC);
		break;
	};
	case Kumir::If:
		err = test_if(str);
		break;
	case Kumir::Fin:
		run_stack.push_back(str + 1);
		Vibor_Flags.pop_back();
		break;
	case Kumir::Then:
		run_stack.push_back(str + 1);
		break;
	case Kumir::Else:
		run_stack.push_back(Cur_Line(str).else_pos + 1);
		break;
	case Kumir::Exit: {
		pop();
		//CircleBegPosition[GlubCircle]=0;
		GlubCircle--;
		break;
	};
	case Kumir::Assign:
		err = calc_prisv(str);
		break;
	case Kumir::Output:
		err = do_vivod(str);
		break;
	case Kumir::Input:
		err = do_vvod(str);
		break;
	case Kumir::FileInput:
		err = do_fvvod(str);
		break;
	case Kumir::FileOutput:
		err = do_fvivod(str);
		break;
	case Kumir::Switch:
		err = do_vibor(str);
		break;
	case Kumir::Case:
		err = do_pri(str);
		break;
	case Kumir::Assert:
		err = do_utv(str);
		break;
	case Kumir::LoopEndCond:
		err = do_break(str);
		break;

	case Kumir::Pre:
		if (Cur_Line(str).VirajList.isEmpty()) {
			err = 0;
			run_stack.push_back(Cur_Line(str).else_pos);
		} else {
			err = do_dano(str);
		}
		break;
	case Kumir::Post:
		if (Cur_Line(str).VirajList.isEmpty()) {
			err = 0;
			break;
		} else {
			err = 0;
			err = do_nado(str);
			if (Cur_Line(str).else_pos != -1) {
				run_stack.push_back(Cur_Line(str).else_pos);
			}
			break;
		};

	case Kumir::CallAlg:
		err = do_funct(str);
		break;
	case Kumir::AlgEnd:
		if (Cur_Line(str).else_pos > 0) {
			run_stack.push_back(Cur_Line(str).else_pos);
		}
		break;
	default:
		break;
	}

	if (Cur_Line(str).str_type == Kumir::VarDecl) {
		if (!skipInitVariables) {
			err = initPerem(str, curModule);
		}
		run_stack.push_back(str + 1);
		if (!Cur_Line(str).generatedLinesAfter.isEmpty()) {
			QList<proga> extra = Cur_Line(str).generatedLinesAfter;
			int baseStr = str;
			int subStr;
			int strIndex;
			for (int j = extra.count() - 1; j >= 0; j--) {
				subStr = j + 1;
				strIndex = - (baseStr * 10 + subStr);
				run_stack.push_back(strIndex);
			}
		}
	};

	if (err != 0) {
		//     qDebug("Kumir_Run::do_line(%d)::err==%d",str,err);

		// OutBuff->appendErrMsg( QString::fromUtf8("ОШИБКА ВЫПОЛНЕНИЯ: ")+run_err(err),Cur_Line(str).real_line);


		return QPair<int, QString>(err, errSource);
	};
	if (err == 0) {
		//OutBuff->appendCounterMsg(1);
	}
	if (i_delay > 0) {
		msleep(i_delay);
		i_delay = -1;
	}
	return QPair<int, QString>(err, errSource);
};


int Kumir_Run::do_vivod(int str)
{


	//KumValueStackElem test(10);

	// QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
	QString out = "";
	//  int dbg=Cur_Line(str).VirajList.count();
	for (int i = 0; i < Cur_Line(str).VirajList.count(); i++) {
		if (Cur_Line(str).VirajTypes[i] == integer) {
			int error = 0;
			int res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asInt();
			if (error != 0) {
				//        if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};
			if (stoped) {
				return 0;
			}
			sendText(QString::number(res));
		};

		if ((Cur_Line(str).VirajTypes[i] == kumString) || (Cur_Line(str).VirajTypes[i] == charect)) { //09-03 ???
			int err;
			QString res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&err).asString();
			//QString testrazb=Cur_Line(str).VirajList[i][2];
			if (err != 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return err;
			};
			if (stoped) {
				return 0;
			}
			sendText(res);
		};


		if (Cur_Line(str).VirajTypes[i] == real) {
			int error = 0;
			double res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asFloat();
			if (error != 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};

			QString to_out = QString::number(res, 'g', 7);
			if (stoped) {
				return 0;
			}
			sendText(to_out);
		};

		if (Cur_Line(str).VirajTypes[i] == kumBoolean) {
			int error = 0;
			bool res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asBool();
			if (error != 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};

			if (res) {
				sendText(trUtf8("да"));
			} else {
				sendText(trUtf8("нет"));
			}
		};

		if (Cur_Line(str).VirajTypes[i] == none) {
			QStringList sList = Cur_Line(str).VirajList[i];
			if (
				(sList.count() == 1)
				&&
				(sList[0] == QString(QChar(KS_NS)))
			)
				if (stoped) {
					return 0;
				}
			sendText("\n");
		};




	};



	run_stack.push_back(str + 1);
	return 0;

};





int Kumir_Run::do_fvivod(int str)
{



	int filesIspId = Modules->idByName("FilesP");
	if (filesIspId == -1) {
		return 2;
	}
	KumPFiles *filesInstreument = (KumPFiles *)Modules->Instrument(filesIspId);
	QString out = "";
	//  int dbg=Cur_Line(str).VirajList.count();
	int err = 0;
	int key = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&err).asInt();
	int pos = 0;
	if (err != 0) {
		return err;
	};

	for (int i = 1; i < Cur_Line(str).VirajList.count(); i++) {
		if (Cur_Line(str).VirajTypes[i] == integer) {
			int error = 0;
			int res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asInt();
			if (error != 0) {
				//        if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};

			error = filesInstreument->writeText(key, QString::number(res), &pos);
			if (error != 0) {
				return error;
			};

		};

		if ((Cur_Line(str).VirajTypes[i] == kumString) || (Cur_Line(str).VirajTypes[i] == charect)) { //09-03 ???
			int error;
			QString res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asString();
			//QString testrazb=Cur_Line(str).VirajList[i][2];
			if (error != 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};
			error = filesInstreument->writeText(key, res, &pos);
			if (error != 0) {
				return error;
			};
		};


		if (Cur_Line(str).VirajTypes[i] == real) {
			int error = 0;
			double res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asFloat();
			if (error > 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};

			error = filesInstreument->writeText(key, QString::number(res, 'g', 6), &pos);
			if (error > 0) {
				//                 if(!haltOnError)run_stack.push_back(str+1);
				return error;
			};
		};

		if (Cur_Line(str).VirajTypes[i] == kumBoolean) {
			int error = 0;
			bool res = calc_simple_universal(Cur_Line(str).VirajList[i],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[i].last(),
					&error).asBool();
			if (error > 0) {
				return error;
			};

			if (res) {
				error = filesInstreument->writeText(key, trUtf8("да"), &pos);
			} else {
				error = filesInstreument->writeText(key, trUtf8("нет"), &pos);
			}
			if (error > 0) {
				return error;
			};
		};

		if (Cur_Line(str).VirajTypes[i] == none) {
			QStringList sList = Cur_Line(str).VirajList[i];
			if (
				(sList.count() == 1)
				&&
				(sList[0] == QString(QChar(KS_NS)))
			) {
				err = filesInstreument->writeText(key, QString(QChar(13)) + "\n", &pos);    //нс TUT NS!!!!!
			}
			if (err > 0) {
				return err;
			};
		};




	};


	if (!filesInstreument->flushFile(key)) {
		return 2;
	}
	run_stack.push_back(str + 1);
	return 0;

};




/**
 * Определение приоритета опреатора
 * @param oper Оператор
 * @return Приоритет от 0 до 100
 */
int Kumir_Run::getOperPrior(QChar oper)
{
	if (oper == '0') {
		return 99;
	}
	if (oper == '!') {
		return 100;
	}
	if (oper == '(') {
		return 90;
	}
	if (oper == ')') {
		return 90;
	}
	// begin NEW V.Y.
	if ((oper == QChar(UNARY_MINUS)) || oper == (QChar(UNARY_PLUS))) {
		return 4;
	}
	// end NEW V.Y.
	if (oper == QChar(KS_STEPEN)) {
		return 2;    //Mordol 08-31
	}
	if (oper == '*') {
		return 3;
	}
	if (oper == '/') {
		return 3;
	}
	if (oper == '+') {
		return 4;
	}
	if (oper == '-') {
		return 4;
	}
	if (oper == '>') {
		return 5;
	}
	if (oper == '<') {
		return 5;
	}
	if (oper == '=') {
		return 5;
	}
	if (oper == QChar(KS_BOLSHE_RAVNO)) {
		return 5;
	}
	if (oper == QChar(KS_MENSHE_RAVNO)) {
		return 5;
	}
	if (oper == QChar(KS_NE_RAVNO)) {
		return 5;
	}

	if (oper == QChar(KS_OR)) {
		return 12;
	}
	if (oper == QChar(KS_AND)) {
		return 11;
	}
	if (oper == QChar(KS_NOT)) { //MR-07-B
		return 10;
	}



	return 0;
};





/**
 * Выполнение заголовков цикла
 * @param str номер строки
 * @return Код ошибки
 */
int Kumir_Run::run_circle(int str)
{
	if (stoped) {
		return 0;
	}
	QString line = Cur_Line(str).line;
	int ras_pos = line.indexOf(QChar(KS_RAZ));
	int circle_type;
	if (line.length() > 1) {
		if (ras_pos > 2) {
			circle_type = RAZ_CIRCLE;
		}
		if (line[2] == QChar(KS_DLYA)) {
			circle_type = FOR_CIRCLE;
		}
		if (line[2] == QChar(KS_POKA)) {
			circle_type = WHILE_CIRCLE;
		}
	} else {

		bool FirstFlag = true;

		for (int iCircle = 0; iCircle < GlubCircle; iCircle++) {
			if (CircleBegPosition[iCircle] == str) {
				FirstFlag = false;
				break;
			}
		}
		if (FirstFlag) {
			CircleBegPosition[GlubCircle] = str;
			GlubCircle++;

		}
		run_stack.push_back(Cur_Line(str).else_pos + 1);
		run_stack.push_back(str);
		run_stack.push_back(str + 1);
		return 0;
	}
	int error = 0;


	if (circle_type == RAZ_CIRCLE) {
		int iCircle;
		bool FirstFlag = true;

		for (iCircle = 0; iCircle < GlubCircle; iCircle++) {
			if (CircleBegPosition[iCircle] == str) {
				FirstFlag = false;
				break;
			}
		}
		if (FirstFlag) {
			int res = calc_simple_universal(Cur_Line(str).VirajList[0],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[0].last(),
					&error).asInt();
			BreakFlag = false;
			if (error > 0) {
				return error;
			}
			proga p = Cur_Line(str);
			if (res <= 0) { //не выполняется ни разу
				run_stack.push_back(p.else_pos + 1);
				return 0;
			}

			if (GlubCircle >= MAX_GLUB_CIRCLE) { //превышение максимальной глубины
				return RUN_BAD_DEFINITION; // NEW V.Y.
			}
			CircleBegPosition[GlubCircle] = str; //запоминаем параметры цикла
			CircleOt[GlubCircle] = 1;
			CircleDo[GlubCircle] = res;
			CirclePerem[GlubCircle] = 1;
			CircleStep[GlubCircle] = 1;
			GlubCircle++;
			qDebug() << "Glub Circle" << GlubCircle;
			run_stack.push_back(p.else_pos + 1); // засовываем в стек номер строки после цикла
			run_stack.push_back(str); //голова цикла
			run_stack.push_back(str + 1); //
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				sendDebug(QString::fromUtf8("1 раз"), str, curModule);
			}

		} else {
			// не в первый раз
			CirclePerem[iCircle] += CircleStep[iCircle];

			if (CirclePerem[iCircle] > CircleDo[iCircle] || BreakFlag) { //конец?
				CircleBegPosition[iCircle] = 0;
				GlubCircle--;
				BreakFlag = false;
				run_stack.push_back(Cur_Line(str).else_pos + 1); //BUG_FIX #21 Mordol
				return 0;
			}
			//CirclePerem[iCircle] > CircleDo
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			run_stack.push_back(str);
			run_stack.push_back(str + 1);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				sendDebug(QString::number(CirclePerem[iCircle]) + QString::fromUtf8(" раз"), str, curModule);
			}
		}
		//ТОDO  if ( (str>0) && toBreak )
		// OutBuff->appendDebugMsg(QString::number(CirclePerem[iCircle]),str);
	}
	//raz_circle
	if (circle_type == FOR_CIRCLE) {
		qDebug() << "Glub" << GlubCircle;
		int iCircle;
		bool FirstFlag = true;
		int perem_t_id;
		for (iCircle = 0; iCircle < GlubCircle; iCircle++) {
			if (CircleBegPosition[iCircle] == str) {
				FirstFlag = false;
				break;
			}
		}
		//for

		int ot_pos = line.indexOf(QChar(KS_OT));
		QString perem = line.mid(5, ot_pos - 5);
		perem_t_id = perem.toInt();
		PeremPrt point2;
		point2.perem_id = perem_t_id;
		point2.mod_id = curModule;
		if (Cur_Symbols()->isArgRes(perem_t_id) || Cur_Symbols()->isRes(perem_t_id)) {
			point2 = Cur_Symbols()->pointTo(perem_t_id);
		}


		if (FirstFlag) {
			BreakFlag = false;
			int ot = calc_simple_universal(Cur_Line(str).VirajList[0],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[0].last(),
					&error).asInt();
			int do_v = calc_simple_universal(Cur_Line(str).VirajList[1],
					Cur_Line(str).extraSubExpressions,
					Cur_Line(str).VirajList[1].last(),
					&error).asInt();
			Modules->setUsed(point2, true);
			if (error != 0) {
				return error;
			}
			int shag = 1;
			Modules->setUsed(point2, true);
			//int res,shag=-1;

			int StepPosition = Cur_Line(str).line.indexOf(QChar(KS_SHAG));

			if (StepPosition > -1) {
				shag = calc_simple_universal(Cur_Line(str).VirajList[2],
						Cur_Line(str).extraSubExpressions,
						Cur_Line(str).VirajList[2].last(),
						&error).asInt();
				if (error != 0) {
					return error;
				}
				//  res=(abs(do_v-ot)+1)/abs(shag);
			}
			if (shag > 0) {
				if (ot > do_v) {
					run_stack.push_back(Cur_Line(str).else_pos + 1);
					return 0;
				}
			} else {
				if (ot < do_v) {
					run_stack.push_back(Cur_Line(str).else_pos + 1);
					return 0;
				}

			}

			if (GlubCircle >= MAX_GLUB_CIRCLE) {
				return RUN_BAD_DEFINITION;
			}
			CircleBegPosition[GlubCircle] = str;
			CircleOt[GlubCircle] = ot;
			CircleDo[GlubCircle] = do_v;
			CircleStep[GlubCircle] = shag;
			CirclePerem[GlubCircle] = ot;
			GlubCircle++;
			qDebug() << "Glub++";
			Modules->setIntegerValue(point2, ot);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				sendDebug(Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(ot), str, curModule);
			}
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			run_stack.push_back(str);
			run_stack.push_back(str + 1);
		}
		//FirstFlag
		else {
			CirclePerem[iCircle] += CircleStep[iCircle];
			if (CircleStep[iCircle] > 0) {
				if (CirclePerem[iCircle] > CircleDo[iCircle] || BreakFlag) {
					CircleBegPosition[iCircle] = 0;
					GlubCircle--;
					BreakFlag = false;
					proga p = Cur_Line(str);
					int nextInstruction = p.else_pos + 1;
					run_stack.push_back(nextInstruction);
					return 0;
				}
			} else {
				if (CirclePerem[iCircle] < CircleDo[iCircle] || BreakFlag) { //конец
					CircleBegPosition[iCircle] = 0;
					GlubCircle--;
					BreakFlag = false;
					run_stack.push_back(Cur_Line(str).else_pos + 1); //09-08-Mordol!!
					return 0;
				}
			}
			//CirclePerem[iCircle] > CircleDo

			Modules->setIntegerValue(point2, CirclePerem[iCircle]);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				sendDebug(Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(CirclePerem[iCircle]), str, curModule);
			}
			//TODO debug            if ( (str>0) && toBreak )
			//        OutBuff->appendDebugMsg(symbols->symb_table[perem_id].name+"="+QString::number(CirclePerem[iCircle]),str);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				sendDebug(Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(CirclePerem[iCircle]), str, curModule);
			}
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			run_stack.push_back(str);
			run_stack.push_back(str + 1);
		}

	}
	// for_circle

	if (circle_type == WHILE_CIRCLE) {
		int err = 0;
		bool FirstFlag = true;
		qDebug() << "GLUB in WHILE" << GlubCircle;
		for (int iCircle = 0; iCircle < GlubCircle; iCircle++) {
			if (CircleBegPosition[iCircle] == str) {
				FirstFlag = false;
				break;
			}
		}
		if (FirstFlag) {

			CircleBegPosition[GlubCircle] = str;
			GlubCircle++;
		}
		bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
				Cur_Line(str).extraSubExpressions,
				Cur_Line(str).VirajList[0].last(),
				&err).asBool();
		if (err != 0) {
			return err;
		}
		if (!res  || BreakFlag) {
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			BreakFlag = false;
			GlubCircle--;
		} else {
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			run_stack.push_back(str);
			run_stack.push_back(str + 1);
		}
		QString out;
		if (res) {
			out = trUtf8("да");
		} else {
			out = trUtf8("нет");
		}
		if ((setup.isPoShagam()) || (setup.isDebug())) {
			sendDebug(out, str, curModule);
		}
		return 0;
	}

	return 0;

}


KumValueStackElem Kumir_Run::calc_simple_universal(const QStringList &simple_viraj,
	const QStringList &extraExpressions,
	const QString &expression,
	int *error,
	KumValueStackElem *newValue,
	QString *debugString) // VY 07-16
{
	//                                                      A. Пролог
	//                                  A1. Описание переменных
	// Знаки операций
	//  QString opers_string="[+|*|\\-|)|(|/|>|<|=|"+QString(QChar(KS_OR))+"|"+QString(QChar(KS_NE_RAVNO))
	//          +"|"+QString(QChar(KS_BOLSHE_RAVNO))+"|"+QString(QChar(KS_MENSHE_RAVNO))+"|"+QString(QChar(KS_NOT))+"|"+QString(QChar(KS_AND))+QString(QChar(UNARY_PLUS))+"|"+
	//          QString(QChar(UNARY_MINUS))+"|"+
	//          QString(QChar(KS_STEPEN))+"]";


	QString opers_string = "[+*\\-)(/><=" + QString(QChar(KS_OR)) + QString(QChar(KS_NE_RAVNO))
		+ QString(QChar(KS_BOLSHE_RAVNO)) + QString(QChar(KS_MENSHE_RAVNO)) + QString(QChar(KS_NOT)) + QString(QChar(KS_AND)) + QString(QChar(UNARY_PLUS)) +
		QString(QChar(UNARY_MINUS)) +
		QString(QChar(KS_STEPEN)) + "]";
	//  Они же - как регулярное выражение
	QRegExp oper_rx = QRegExp(opers_string);
	// Стек значений переменных
	QList<KumValueStackElem>         perems;
	// Стек операций
	QList<QChar>                               opers;
	//  Текущая позиция в исходной строке
	int pos = 0;                    // 0 - начало
	//  Текущий позиция оператора
	int  oper = 0;                      //  0 - начало
	//                              A2.  Инициализация переменных.
	perems.clear();

	pos = 0;
	oper = 0;
	opers.append('0');            // заносим признак начала в стек операторов
	*error = 0;
	//qDebug()<<"VVERKHNIY VARIANT"<< "LOg op";
	int op_id = -1;
	bool optimizFlag = false;
	//                               A3.   Основной цикл
	//  Цикл прерывается, когда больше не найдено операторов
	while (oper > -1) {
		QString perem; // подстрока, содержащая операнд
		// Ищем оператор начиная со строки pos
		oper = oper_rx.indexIn(expression, pos);
		//        qDebug()<<"expression"<<expression;
		if ((oper == -1) && (pos == 0)) {
			// если оператор не найден и не достигнут конец строки,
			// то подразумевается, что все выражение состоит из операнда
			perem = expression;
		} else {
			// Если оператор найден, то
			// выделяем операнд (perem) как вырезку из строки выражения [pos,oper)
			perem = expression.mid(pos, oper - pos);
		}

		// ---- Определение текущего оператора

		// текущий оператор
		QChar operatorS;
		// если оператор не был найден,
		// то он равен псевдооператору '!', означающему "конец вычислений"
		if (oper == -1) {
			operatorS = '!';
		} else {
			// если оператор был найден, то operatorS равен символу на позиции oper
			operatorS = expression[oper];

		}

		// ---------------------------------------- разбираемся с тем, что находится перед оператором

		// проверка на пустой операнд
		if (perem.isEmpty()) {
			pos = oper + 1;
			opers.append(operatorS);
			continue;
		}
		// QByteArray test=perem.toAscii();
		// ------ ??????????????
		if (optimizFlag) {
			perems.append(KumValueStackElem(dummy, 0));
		}

		// Переменная или константа
		if ((perem[0] == '^') && (!optimizFlag)) {
			// выделяем номер в таблице символов
			int perem_t_id = perem.mid(1).toInt();
			PeremPrt point2;
			point2.perem_id = perem_t_id;
			point2.mod_id = curModule;

			if ((Cur_Symbols()->isArgRes(perem_t_id)) || (Cur_Symbols()->isRes(perem_t_id))) {
				point2 = Cur_Symbols()->pointTo(perem_t_id);
			};

			if (newValue != NULL) {
				int p_id;
				symbol_table *st;
				//              if((Cur_Symbols()->isArgRes(perem_t_id))||(Cur_Symbols()->isRes(perem_t_id))) {
				//                  st = Modules->module(point2.mod_id)->Values();
				//                  p_id = point2.perem_id;
				//              }
				//              else {
				st = Modules->module(point2.mod_id)->Values();
				p_id = point2.perem_id;
				//              }
				switch (newValue->getType()) {
				case integer:
					st->symb_table[p_id].value.setIntegerValue(newValue->asInt());
					break;
				case real:
					st->symb_table[p_id].value.setFloatValue(newValue->asFloat());
					break;
				case bool_or_num:
				case bool_or_lit:
				case kumBoolean:
					st->symb_table[p_id].value.setBoolValue(newValue->asBool());
					break;
				case charect:
					st->symb_table[p_id].value.setCharectValue(newValue->asCharect());
					break;
				case kumString:
					st->symb_table[p_id].value.setStringValue(newValue->asString());
					break;
				default:
					*error = RUN_NO_VALUE;
					break;
				}
				st->setUsed(p_id, true);
				if (debugString != NULL) {
					if (REAL_RES_DEBUG) { //Вывод на поля настоящей переменной
						*debugString = st->symb_table[p_id].name + "=";
					} else { // ссылка
						*debugString = Cur_Symbols()->symb_table[perem_t_id].name + "=";
					}

					if (newValue->getType() == kumString || newValue->getType() == charect) {
						*debugString += "\"" + newValue->asString() + "\"";
					} else {
						*debugString += newValue->asString();
					}
					if (st->symb_table[p_id].type == real && !debugString->contains(".")) {
						*debugString += ".0";
					}
				}
				return KumValueStackElem(0);
			} else {
				// проверяем на наличие значения
				KumValueStackElem elem;
				if (!(Modules->isUsed(point2))) {
					qDebug("P_id:%i not used!", perem_t_id);
					elem = KumValueStackElem(dummy, RUN_NO_VALUE);
				} else { //Есть ли значение у переменной
					// если не отругались на ошибку ранее, то добавляем в стек значение перем.
					if ((Cur_Symbols()->isArgRes(perem_t_id)) || (Cur_Symbols()->isRes(perem_t_id))) {
						elem = KumValueStackElem(Modules->module(point2.mod_id)->Values(),
								point2.perem_id);
					} else {
						elem = KumValueStackElem(Cur_Symbols(), perem_t_id);
					}
				}

				perems.append(elem);
				if (operatorS == QChar(KS_OR) && elem.asBool()) {
					return elem;
				}
				if (operatorS == QChar(KS_AND) && (!elem.asBool())) {
					return elem;
				}
			}

		};

		// Подвыражения
		if ((perem[0] == '&') && (!optimizFlag)) {
			// код ошибки
			int err = 0;
			// выделяем номер в таблице символов
			int index = perem.mid(1).toInt();

			// ======== Блок, от которого хотим избавиться.
			// ======== Здесь определяется тип подвыражения,
			// ======== и в зависимости от типа вызываем
			// ======== соответствующий calc_simple_*.
			/*
			PeremType exprType = getSimpleExprType(simple_viraj,perem_id);
			// Заплатка :( По идее лучше чтоб calc_simple_bool возвращал
			// не bool а KumValueStackElem и с ним и работать без разбора типов.
			if ( exprType == real )
			perems.append(calc_simple_double(simple_viraj,perem_id,  &err));
			if ( exprType == kumBoolean )
			perems.append(calc_simple_bool(simple_viraj,perem_id,  &err));
			if ( exprType == integer )
			perems.append(calc_simple_int(simple_viraj,perem_id,  &err));
			if ( (exprType == kumString) || (exprType == charect)  )
			perems.append(calc_simple_string(simple_viraj,perem_id,  &err));
			*/
			// ======== Конец блока.

			//  Здесь мы рекурсивно вызываем себя для вычисления подвыражения
			//    с номером perem_id и добавляем в стек результат вычисления.
			KumValueStackElem elem = calc_simple_universal(simple_viraj, extraExpressions, simple_viraj[index], &err);
			if (err) {
				*error = err;
				return elem;
			}
			if (elem.Error() != 0) {
				*error = elem.Error();
				return elem;
			};
			perems.append(elem);
			if (operatorS == QChar(KS_OR) && elem.asBool()) {
				return elem;
			}
			if (operatorS == QChar(KS_AND) && (!elem.asBool())) {
				return elem;
			}

		};

		if (perem[0] == '#') {
			// Подвыражения из списка extraExpressions
			// (дополнительные псевдо-скобки для операций "или" и "и")
			int err = 0;
			int index = perem.mid(1).toInt();
			KumValueStackElem elem = calc_simple_universal(simple_viraj, extraExpressions, extraExpressions[index], &err);
			if (elem.Error() != 0) {
				*error = elem.Error();
				return elem;
			};
			if (err != 0) {
				*error = err;
				return KumValueStackElem(0);
			};
			perems.append(elem);
			if (operatorS == QChar(KS_OR) && elem.asBool()) {
				return elem;
			}
			if (operatorS == QChar(KS_AND) && (!elem.asBool())) {
				return elem;
			}
		}

		//Массивы
		if ((perem[0] == '%') && (!optimizFlag)) {
			int err = 0;
			// Выделяем аргументы массива
			int emp_pos = perem.indexOf('&');
			if (emp_pos == -1) {
				emp_pos = perem.length();
			}
			// выделяем блок до &
			QString temp;
			temp = perem.mid(1, emp_pos - 1);
			// номер массива в таблице символов
			int mass_t_id = temp.toInt();
			PeremPrt point2;
			// проверяем на инициализированность
			point2.perem_id = mass_t_id;
			point2.mod_id = curModule;
			if ((Cur_Symbols()->isArgRes(mass_t_id)) || (Cur_Symbols()->isRes(mass_t_id))) {
				point2 = Cur_Symbols()->pointTo(mass_t_id);
			}
			// TODO реализовать проверку каждого элемента массива
			//if(!(Cur_Symbols()->isUsed(mass_id))){*error=RUN_NO_VALUE; return 0;};
			// далее выделяем элемент массива и заносим его в стек

			if (newValue != NULL) {
				const QString mass_args = perem.mid(emp_pos, perem.length() - emp_pos);
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_integer) {
					setMassIntData(point2, mass_args, simple_viraj, extraExpressions, &err, newValue->asInt(), debugString, Cur_Symbols()->symb_table[mass_t_id].name);
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_real) {
					setMassFloatData(point2, mass_args, simple_viraj, extraExpressions, &err, newValue->asFloat(), debugString, Cur_Symbols()->symb_table[mass_t_id].name);
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_string) {
					setMassStringData(point2, mass_args, simple_viraj, extraExpressions, &err, newValue->asString(), debugString, Cur_Symbols()->symb_table[mass_t_id].name);
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_bool) {
					setMassBoolData(point2, mass_args, simple_viraj, extraExpressions, &err, newValue->asBool(), debugString, Cur_Symbols()->symb_table[mass_t_id].name);
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_charect) {
					setMassCharData(point2, mass_args, simple_viraj, extraExpressions, &err, newValue->asCharect(), debugString, Cur_Symbols()->symb_table[mass_t_id].name);
				}
				//              Cur_Symbols()->setUsed(mass_t_id,true);
				Modules->setUsed(point2, true);
				if (err > 0) {
					*error = err;
				}

				return KumValueStackElem(0);
			} else {
				KumValueStackElem value;
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_integer) {
					value = KumValueStackElem(getMassIntData(point2, perem.mid(emp_pos, perem.length() - emp_pos), simple_viraj, extraExpressions, &err));
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_real) {
					value = KumValueStackElem(getMassFloatData(point2, perem.mid(emp_pos, perem.length() - emp_pos), simple_viraj, extraExpressions, &err));
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_string) {
					value = KumValueStackElem(getMassStringData(point2, perem.mid(emp_pos, perem.length() - emp_pos), simple_viraj, extraExpressions, &err));
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_bool) {
					value = KumValueStackElem(getMassBoolData(point2, perem.mid(emp_pos, perem.length() - emp_pos), simple_viraj, extraExpressions, &err));
				}
				if (Cur_Symbols()->getTypeByID(mass_t_id) == mass_charect) {
					value = KumValueStackElem(getMassCharData(point2, perem.mid(emp_pos, perem.length() - emp_pos), simple_viraj, extraExpressions, &err));
				}
				perems.append(value);
				if (operatorS == QChar(KS_OR) && value.asBool()) {
					return value;
				}
				if (err != 0) {
					*error = err;
					return KumValueStackElem(0);
				}
			}


			// Если при выделении из массива возникла ошибка -- выход

			if (err > 0) {
				perems.pop_back();
				perems.append(KumValueStackElem(dummy, err));
				//*error = err;
				//return 0;
			}
		}

		// Функции
		if ((perem[0] == '@') && (!optimizFlag)) {

			// номер функции в таблице функций
			int funct_id;
			int mod_id;
			getFuncParam(perem, &funct_id, &mod_id);
			// определяем имя функции
			QString f_name = Modules->module(mod_id)->Functions()->nameById(funct_id);
			// функция возвращает свое значение в одноименную глобальную перем.,
			// поэтому определяем ID этой переменной
			//int isp_id=ispolniteli->inTable(Cur_Functions()->func_table[funct_id].isp);

			//if(Modules->isIntsrument(mod_id))mod_id=0;
			int f_perem_id = Modules->module(mod_id)->Values()->inTable(f_name, "global");

			if (f_perem_id < 0) {
				qDebug("RUN::calc_simple_universal:f_perem_id<0!!!!");
			}
			int err = 0;
			run_f("@" + QString::number(funct_id) + "|" + QString::number(mod_id), perem, simple_viraj, extraExpressions, &err); //TODO CHEK ME
			//qDebug("After Run F,%i",f_perem_id);
			// Если возникла ошибка -- выход
			if (err != 0) {
				*error = err;
				return KumValueStackElem(0);
			};
			// Если функция ничего не вернула, то ошибка
			if (!(Modules->module(mod_id)->Values()->isUsed(f_perem_id))) {
				*error = RUN_NO_VALUE;
				return KumValueStackElem(0);
			};
			// Добавляем значение в стек
			KumValueStackElem value = KumValueStackElem(Modules->module(mod_id)->Values(), f_perem_id);
			perems.append(value);
		}

		//Вырезка
		if ((perem[0] == QChar(TYPE_SKOBKA)) && (!optimizFlag)) {
			// Количество подвыражений
			// если 1 -- то отдельный символ
			// если 2 -- то подстрока
			int diez_count = perem.count("&");

			//      Выделяем блок до &
			int diez_pos = perem.indexOf("&");
			// Определяем ID переменной строки
			QString id_str = perem.mid(1, diez_pos - 1);
			int perem_t_id = id_str.toInt();
			PeremPrt point2;
			point2.mod_id = curModule;
			point2.perem_id = perem_t_id;
			// Проверка на наличие значения
			if ((Cur_Symbols()->isArgRes(perem_t_id)) || (Cur_Symbols()->isRes(perem_t_id))) {
				point2 = Cur_Symbols()->pointTo(perem_t_id);
			};
			if (!(Modules->isUsed(point2)) && !newValue) {
				*error = RUN_NO_VALUE;
				return KumValueStackElem("");
			}

			// temp_string -- значение исходной строки
			QString temp_string =
				Modules->getStringValue(point2);

			// индексы вырезки
			int index1, index2;

			//Обращение к конкретному символу.
			if (diez_count == 1) {
				// определяем ID подвыражения для первого аргумента &
				id_str = perem.mid(diez_pos + 1, perem.length() - diez_pos - 1);
				int var_id = id_str.toInt();
				//Подсчет индекса
				int err = 0;
				index1 = calc_simple_universal(simple_viraj, extraExpressions, simple_viraj[var_id], &err).asInt();
				// Если ошибка -- выход
				if (err > 0) {
					perems.append(KumValueStackElem(dummy, err));
				} else { // тип None?

					int templ = temp_string.length();

					if (//Проверка правильности индекса
						(index1 > templ)
						||
						(index1 < 1)
					) {

						perems.append(KumValueStackElem(dummy, RUN_STRING_BOUND_1));
					} else
						// Добавляем в стек символ из строки
						if (newValue) {
							temp_string.replace(index1 - 1, 1, newValue->asString());
							Modules->setStringValue(point2, temp_string);
							if (debugString) {
								*debugString = Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].name + "=\"" + temp_string + "\"";
							}
							return KumValueStackElem(0);
						} else {
							KumValueStackElem value = KumValueStackElem(QChar(temp_string[index1 - 1]));
							perems.append(value);
						}
				};
			} else { //Вырезка
				// ищем второе вхождение &
				int next_diez_pos = perem.indexOf("&", diez_pos + 1);
				// выделяем блок между & и &
				id_str = perem.mid(diez_pos + 1, next_diez_pos - diez_pos - 1);
				// ID первого индекса вырезки
				int var_id = id_str.toInt();
				//Начало вырезки
				int err = 0;
				index1 = calc_simple_universal(simple_viraj, extraExpressions, simple_viraj[var_id], &err).asInt();

				// выделяем блок после второго &
				id_str = perem.mid(next_diez_pos + 1, perem.length() - next_diez_pos - 1);
				// ID второго индекса вырезки
				var_id = id_str.toInt();
				//Конец вырезки

				index2 = calc_simple_universal(simple_viraj, extraExpressions, simple_viraj[var_id], &err).asInt();
				if (err > 0) {
					perems.append(KumValueStackElem(dummy, err));
				} else {
					int templ = temp_string.length();
					if ( //Проверка правильности индексов
						(index2 > templ)
						||
						(index1 < 1)
						||
						(index1 > index2)
					) {
						perems.append(KumValueStackElem(dummy, RUN_STRING_BOUND_2));
					} else
						// Добавляем в стек подстроку [index1,index2]
						if (newValue) {
							temp_string.replace(index1 - 1, index2 - index1 + 1, newValue->asString());
							Modules->setStringValue(point2, temp_string);
							if (debugString) {
								*debugString = Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].name + "=\"" + temp_string + "\"";
							}
							return KumValueStackElem(0);
						} else {
							KumValueStackElem value = KumValueStackElem(temp_string.mid(index1 - 1, index2 - index1 + 1));
							perems.append(value);
						}
				};
			}
		}

		if (operatorS == QChar(KS_AND) && (!perems.last().asBool()) && !optimizFlag && (perems.last().getType() == kumBoolean)) {
			optimizFlag = true;
			op_id = opers.count();
		};
		if (operatorS == QChar(KS_OR) && (perems.last().asBool()) && !optimizFlag && (perems.last().getType() == kumBoolean)) {
			if (opers.last() != QChar(KS_NOT)) {
				optimizFlag = true;
				op_id = opers.count();
			}
		};
		// --------------------------------------------- конец разбора операнда

		// getOperPrior возвращает порядок вычисления оператора,
		// бОльший порядок вычисления соответствует мЕньшему приоритету.

		// getOperPrior(operatorS) -- порядок текущего оператора
		// getOperPrior(opers.last()) -- порядок последнего оператора в стеке

		if (getOperPrior(operatorS) >= getOperPrior(opers.last())) {

			// Если текущий оператор менее приоритетный, чем последний в стеке.

			// Вычисление содержимого стека до тех пор, пока
			// там не останется операторов, менее приоритетных,
			// чем текущий
			while (getOperPrior(operatorS) >= getOperPrior(opers.last())) {
				// Поледней оператор из стека
				*error = 0;
				QChar cur_op = opers.last();
				opers.pop_back();

				if (cur_op == '0') {
					break;    //Стек закончился.
				}

				// Последнее значение в стеке
				KumValueStackElem firstP = perems.last();
				if (perems.empty()) {
					*error = 2;
					return KumValueStackElem(99);
				}
				perems.pop_back();

				// Предпоследнее значение в стеке
				KumValueStackElem secondP;

				// Берем значение из стека только в том случае,
				// если оператор бинарный
				if ((cur_op != QChar(UNARY_MINUS))
					&& (cur_op != QChar(UNARY_PLUS))
					&& (cur_op != QChar(KS_NOT))) { //Для унарных операторов
					secondP = perems.last();
					perems.pop_back();
				};

				//Вычисление.
				// В зависимости от оператора вызывается
				// соответствующая функция класса
				// KumValueStackElem, которая сама разбирается,
				// что делать с конкретными значениями различных типов данных.
				// На этом этапе предполагается, что соответствие типов
				// является корректным, т.к. было проверено на этапе разбора.

				if (cur_op == '+') {
					perems.append(KumValueStackElem::Summ(firstP, secondP, error));
				}
				if (cur_op == QChar(UNARY_MINUS)) {
					KumValueStackElem value = KumValueStackElem::Minus(KumValueStackElem(0), firstP, error);
					perems.append(value);
				}
				if (cur_op == QChar(UNARY_PLUS)) {
					perems.append(firstP);
				}
				if (cur_op == '-') {
					perems.append(KumValueStackElem::Minus(secondP, firstP, error));
				}
				if (cur_op == '*') {
					perems.append(KumValueStackElem::Multi(firstP, secondP, error));
				}
				if (cur_op == '/') {
					if ((firstP.Error() > 0) && (!optimizFlag)) {
						*error = firstP.Error();
						return KumValueStackElem(0);
					}
					//                  if((firstP.asFloat()==0)&&(!optimizFlag)) {
					//                     perems.append(KumValueStackElem(dummy,RUN_DIV_ZERO));
					//               }
					if (secondP.Error() > 0) {
						perems.append(KumValueStackElem(dummy, secondP.Error()));
					} else if (firstP.Error() > 0) {
						perems.append(KumValueStackElem(dummy, firstP.Error()));
					} else {
						perems.append(KumValueStackElem::Div(secondP, firstP, error));
					}
				}
				if (cur_op == QChar(KS_STEPEN)) {
					perems.append(KumValueStackElem::Pow(secondP, firstP, error));
				}
				if (cur_op == '=') {
					perems.append(KumValueStackElem::compare(secondP, firstP));
				}

				if (cur_op == '>') {
					perems.append(KumValueStackElem::More(secondP, firstP));
				}

				if (cur_op == '<') {
					perems.append(KumValueStackElem::Less(secondP, firstP));
				}

				if (cur_op == QChar(KS_NE_RAVNO)) {
					perems.append(KumValueStackElem::NotEq(secondP, firstP));
				}


				if (cur_op == QChar(KS_BOLSHE_RAVNO)) {
					perems.append(KumValueStackElem::GEQ(secondP, firstP));
				}

				if (cur_op == QChar(KS_MENSHE_RAVNO)) {
					perems.append(KumValueStackElem::LEQ(secondP, firstP));
				}


				if (cur_op == QChar(KS_OR)) {
					KumValueStackElem value = KumValueStackElem::Or(secondP, firstP) ;

					perems.append(value);
					bool secondValue = secondP.asBool();
					bool firstIsDummy = firstP.getType() == dummy;

					if (!secondValue && firstIsDummy) {
						*error = firstP.Error();
						return KumValueStackElem(0);
					};
					if (op_id == opers.count()) {
						optimizFlag = false;
						op_id = -1;
					};
				}

				if (cur_op == QChar(KS_AND)) {
					KumValueStackElem value = KumValueStackElem(KumValueStackElem::And(secondP, firstP));

					perems.append(value);
					if (secondP.asBool() && (firstP.getType() == dummy)) {
						*error = firstP.Error();
						return KumValueStackElem(0);
					};
					if (op_id == opers.count()) {
						optimizFlag = false;
						op_id = -1;
					};

				}

				if (cur_op == QChar(KS_NOT)) {
					bool value = firstP.asBool();
					value = ! value;
					perems.append(KumValueStackElem(value));
				}

				if (*error != 0) {
					return KumValueStackElem(0);
				}
			};
			// конец вычисления
			opers.append(operatorS);
		} else {
			// Случай, когда текущий оператор более приоритетный,
			// чем последний в стеке.
			// Добавляем оператор в стек. Он будет вычислен позже
			// (когда встретится (псевдо-)оператор менее приоритетный)
			opers.append(operatorS);
		}
		// Устанавливаем текущую позицию в строке на след. после
		// оператора символ
		pos = oper + 1;

	}//Конец основного цикла

	KumValueStackElem calc_result = perems.last();
	if (calc_result.getType() == dummy) {
		*error = calc_result.Error();
	}
	return calc_result; // ( без приведения к какому-либо конкретному типу )
	// Было:  return calc_result.asBool();
};


/**
 * Инициализация переменных
 * @param str
 * @return
 */
int Kumir_Run::initPerem(int str, int module)
{

	KumSingleModule *mod = Modules->module(module);
	//    qDebug() << "initPerem(" << str << "," << module << ")";

	QStringList perems;
	QStringList viraj_ids;
	int cur_cr_pos = 0;

	proga pv = mod->Proga_Value_Line(str);
	QString work_stri = pv.line;
	//mainform->writeMain("LINE:"+Cur_Line(str).line);
	int o_sk_pos = work_stri.indexOf('(');
	int z_sk_pos = work_stri.indexOf(')');
	if (o_sk_pos != -1 && z_sk_pos != -1) {
		work_stri = work_stri.mid(o_sk_pos + 1, z_sk_pos - o_sk_pos - 1);
		perems = work_stri.split(',', QString::SkipEmptyParts);
	}
	if (work_stri.contains("^")) {
		perems = work_stri.split(',', QString::SkipEmptyParts);
	}
	// mainform->writeMain("work_str:"+work_str);
	for (int j = 0; j < perems.count(); j++) {
		QString  work_str = perems[j];
		cur_cr_pos = 0;
		while (cur_cr_pos > -1) {
			cur_cr_pos = work_str.indexOf('^', cur_cr_pos);
			int diez_pos = work_str.indexOf('#', cur_cr_pos);
			QString p_id_str = work_str.mid(cur_cr_pos + 1, diez_pos - cur_cr_pos - 1);
			// mainform->writeMain("P_id:"+p_id_str);
			int perem_id = p_id_str.toInt();
			PeremType type = Modules->module(module)->Values()->getTypeByID(perem_id);
			if ((type == mass_integer) || (type == mass_string) || (type == mass_charect) || (type == mass_real) ||
				(type == mass_bool)) {

				if (perem_id == 15) {
					qDebug("init 15");
				};
				viraj_ids = work_str.split('#', QString::SkipEmptyParts);
				// mainform->writeMain("IDS_COUNT:"+QString::number(viraj_ids.count()));
				//  Modules->module(module)->Values()->symb_table[perem_id].size0_start=0;
				// Modules->module(module)->Values()->symb_table[perem_id].size1_start=0;
				// Modules->module(module)->Values()->symb_table[perem_id].size2_start=0;

				// Modules->module(module)->Values()->symb_table[perem_id].size0_end=-1;
				// Modules->module(module)->Values()->symb_table[perem_id].size1_end=-1;
				// Modules->module(module)->Values()->symb_table[perem_id].size2_end=-1;


				PeremPrt mass_p;
				mass_p.mod_id = module;
				mass_p.perem_id = perem_id;
				int err = calcMassSize(viraj_ids, mass_p, str);
				if (err != 0) {
					return err;
				}
				err = Modules->module(module)->Values()->prepareConstValue(perem_id);
				if (err) {
					return err;
				}

			};






			if (cur_cr_pos > -1) {
				cur_cr_pos++;
			}
			if (!Modules->module(module)->Values()->isReady(perem_id)) {
				Modules->module(module)->Values()->new_symbol(perem_id);
			}
		};
	};

	//run_stack.push_back(str+1);
	return 0;
};



int Kumir_Run::testMassArgs(PeremPrt mass_id, QList<int> args)
{
	if ((args[0] < 0)) {
		return RUN_NEG_INDEX;
	};

	int max_arg = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size0_end
		-  Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size0_start;
	if ((args[0] > max_arg)) {
		return RUN_ARRAY_BOUND;
	};

	if (Modules->module(mass_id.mod_id)->Values()->getRazmById(mass_id.perem_id) >= 2) { //MR-07

		if ((args[1] < 0)) {
			return RUN_NEG_INDEX_2;
		};
		//    int arg_debug=args[1];
		max_arg = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size1_end
			-  Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size1_start;
		if ((args[1] > max_arg)) {
			return RUN_ARRAY_BOUND_2;
		};

	};

	if (Modules->module(mass_id.mod_id)->Values()->getRazmById(mass_id.perem_id) == 3) {

		if ((args[2] < 0)) {
			return RUN_NEG_INDEX_3;
		};

		max_arg = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size2_end     //MR-07-D-2 Bylo: size1
			-  Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].size2_start; //MR-07-D-3 Bylo: size1
		if ((args[2] > max_arg)) {
			return RUN_ARRAY_BOUND_3;
		};

	};




	return 0;

};


int  Kumir_Run::compareMass(symbol_table *src_table, int src_id,
	symbol_table *dst_table, int dst_id)
{

	if (!((src_table->getTypeByID(src_id) == mass_integer) ||
			(src_table->getTypeByID(src_id) == mass_real) ||
			(src_table->getTypeByID(src_id) == mass_bool) ||
			(src_table->getTypeByID(src_id) == mass_string) ||
			(src_table->getTypeByID(src_id) == mass_charect))) {
		return 0;
	}

	int sourceDim = src_table->getRazmById(src_id);
	int destinationDim = dst_table->getRazmById(dst_id);

	if (sourceDim != destinationDim) {
		return RUN_ARRAY_BOUND;
	}

	if (src_table->symb_table[src_id].size0_start != dst_table->symb_table[dst_id].size0_start) {
		return RUN_ARRAY_BOUND;
	}

	if (src_table->symb_table[src_id].size0_end != dst_table->symb_table[dst_id].size0_end) {
		return RUN_ARRAY_BOUND;
	}

	if (sourceDim == 1) {
		return 0;
	}

	if (src_table->symb_table[src_id].size1_start != dst_table->symb_table[dst_id].size1_start) {
		return RUN_ARRAY_BOUND;
	}

	if (src_table->symb_table[src_id].size1_end != dst_table->symb_table[dst_id].size1_end) {
		return RUN_ARRAY_BOUND;
	}

	if (sourceDim == 2) {
		return 0;
	}

	if (src_table->symb_table[src_id].size2_start != dst_table->symb_table[dst_id].size2_start) {
		return RUN_ARRAY_BOUND;
	}

	if (src_table->symb_table[src_id].size2_end != dst_table->symb_table[dst_id].size2_end) {
		return RUN_ARRAY_BOUND;
	}

	return 0;
}


int Kumir_Run::calc_prisv(int str)
{
	// сслылки на подвыражения в левой и правой частях
	int leftIndex, rightIndex;
	QString line;
	line = Cur_Line(str).line;
	QStringList links = line.split(QChar(KS_PRISV));
	K_ASSERT(links.count() == 2);
	bool ok;
	leftIndex = links[0].mid(1).toInt(&ok);
	K_ASSERT(ok);
	rightIndex = links[1].mid(1).toInt(&ok);
	K_ASSERT(ok);


	// значение правой части выражения
	KumValueStackElem rightValue;

	// вычисляем значение правой части выражения
	int local_error = 0;
	rightValue = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0][rightIndex],
			&local_error);

	if (stoped) {
		return 0;
	}
	if (local_error != 0) {
		return local_error;
	}

	// устанавливаем новое значение в левой части

	QString debugString;
	calc_simple_universal(Cur_Line(str).VirajList[0],
		Cur_Line(str).extraSubExpressions,
		Cur_Line(str).VirajList[0][leftIndex],
		&local_error,
		&rightValue,
		&debugString);

	if (local_error > 0) {
		return local_error;
	}

	if (setup.isPoShagam() || setup.isDebug()) {
		sendDebug(debugString, str, curModule);
	}

	run_stack.push_back(str + 1);

	return 0;
}


/**
 * Вычисление размера массива
 * @param viraj_ids  номера подвыражений
 * @param perem_id номер переменной в symbol_table
 * @param str строка описания
 * @return код ошибки
 */
int Kumir_Run::calcMassSize(QStringList viraj_ids, PeremPrt mass, int str)
{
	if (stoped) {
		return 0;
	}
	KumSingleModule *massModule = Modules->module(mass.mod_id);
	int perem_id = mass.perem_id;
	//  int debug=viraj_ids.count();
	for (int i = 1; i < viraj_ids.count(); i++) {
		bool ok;
		int viraj_id = viraj_ids[i].toInt(&ok);
		if (!ok) {
			return INT_R_10;
		}

		QStringList test = massModule->Proga_Value_Line(str).VirajList[viraj_id];
		QStringList extraSubexprList = massModule->Proga_Value_Line(str).extraSubExpressions;
		// mainform->writeMain("MASS_ARG_VIRAJ:"+test[0]);
		int error = 0;
		int res = calc_simple_universal_in_Module(mass.mod_id,
				test,
				extraSubexprList,
				test.last(),
				&error).asInt();

		if (error != 0) {
			return error;
		}


		//  mainform->writeMain("MASS_ARG:"+QString::number(res));
		switch (i) {
		case 1:
			massModule->Values()->symb_table[perem_id].size0_start = res;
			break;
		case 2:
			massModule->Values()->symb_table[perem_id].size0_end = res;
			if (massModule->Values()->symb_table[perem_id].size0_end < massModule->Values()->symb_table[perem_id].size0_start) {
				return RUN_END_BEFORE_START;
			}
			break;

		case 3:
			massModule->Values()->symb_table[perem_id].size1_start = res;
			break;
		case 4:
			massModule->Values()->symb_table[perem_id].size1_end = res;
			if (massModule->Values()->symb_table[perem_id].size1_end < massModule->Values()->symb_table[perem_id].size1_start) {
				return RUN_END_BEFORE_START_2;
			}
			break;

		case 5:
			massModule->Values()->symb_table[perem_id].size2_start = res;
			break;
		case 6:
			massModule->Values()->symb_table[perem_id].size2_end = res;
			if (massModule->Values()->symb_table[perem_id].size2_end < massModule->Values()->symb_table[perem_id].size2_start) {
				return RUN_END_BEFORE_START_3;
			}
			break;
		case 7:
			return RUN_BAD_BOUND_DEF;
		};
	};



	return 0;
}


QList<int> Kumir_Run::calcMassPos(const QString &mass_viraj,
	const QStringList &simple_viraj,
	const QStringList &extraSubexprList,
	int *err)
{
	Q_UNUSED(err);
	QList<int> temp;
	int link_pos = mass_viraj.indexOf("&");
	// mainform->writeMain("MASS_VIRAJ="+mass_viraj);
	int mvLength = mass_viraj.length();
	while (link_pos < mvLength) {
		int pos = mass_viraj.indexOf("&", link_pos + 1);
		if (pos == -1) {
			pos = mass_viraj.length();
		}
		QString str_id = mass_viraj.mid(link_pos + 1, pos - link_pos - 1);
		// mainform->writeMain("MASS_ID="+str_id);
		int error;
		int mass_pos = calc_simple_universal(simple_viraj,
				extraSubexprList,
				simple_viraj[str_id.toInt()],
				&error).asInt();
		temp.append(mass_pos);

		// mainform->writeMain("MASS_POS="+QString::number(mass_pos));

		link_pos = pos;
	};
	return temp;

};





QList<int> Kumir_Run::calcMassArgs(int mass_id, int mod_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, QString *debugString)
{
	QStringList args = mass_args.split('&', QString::SkipEmptyParts);

	QList<int> int_id, int_args;
	int_args.clear();
	for (int i = 0; i < args.count(); i++) {
		QString test = args[i];
		int_id.append(args[i].toInt());

	};
	//  int test=int_id[0];
	int iz = 0;
	//MR-07-E
	for (int i = 0; i < int_id.count(); i++) {

		iz = calc_simple_universal(viraj,
				extraSubexprList,
				viraj[int_id[i]],
				err).asInt();
		if (debugString != NULL) {
			if (i == 0) {
				*debugString += "[";
			}
			*debugString += QString::number(iz);
			if (/*i>0 &&*/ i < int_id.count() - 1) {
				*debugString += ",";
			}
			if (i == int_id.count() - 1) {
				*debugString += "]";
			}
		}
		if (*err > 0) {
			return int_args;
		}
		int base = 0;
		if (i == 0) {
			base = Modules->module(mod_id)->Values()->symb_table[mass_id].size0_start;
		}
		if (i == 1) {
			base = Modules->module(mod_id)->Values()->symb_table[mass_id].size1_start;
		}
		if (i == 2) {
			base = Modules->module(mod_id)->Values()->symb_table[mass_id].size2_start;
		}
		iz -= base;
		int_args.append(iz);
	}
	//RM


	int args_count = int_args.count();
	if (args_count != Modules->module(mod_id)->Values()->symb_table[mass_id].razm) {
		*err = RUN_BAD_COUNT;
	}
	if (args_count < 2) {
		int_args.append(-1);
		int_args.append(-1);
	};
	if (args_count < 3) {
		int_args.append(-1);
	}




	return int_args;
};

void Kumir_Run::setMassIntData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, int value,
	QString *debugString,
	QString name
) {
	if (debugString != NULL) {
		if (REAL_RES_DEBUG) {
			*debugString = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].name;
		} else {
			*debugString = name;
		}
	}

	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraSubexprList, err, debugString);
	if (*err != 0) {
		return;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return;
	};
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setReady(int_args[0], int_args[1], int_args[2], true);
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setIntMassValue(value, int_args[0], int_args[1], int_args[2]);
	if (debugString != NULL) {
		*debugString += "=" + QString::number(value);
	}
};

void Kumir_Run::setMassBoolData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, bool value, QString *debugString, QString name)
{
	if (debugString != NULL) {
		if (REAL_RES_DEBUG) {
			*debugString = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].name;
		} else {
			*debugString = name;
		}
	}
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraSubexprList, err, debugString);
	if (*err != 0) {
		return;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return;
	};
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setReady(int_args[0], int_args[1], int_args[2], true);
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setBoolMassValue(value, int_args[0], int_args[1], int_args[2]);
	if (debugString != NULL) {
		*debugString += "=" + (value ? trUtf8("да") : trUtf8("нет"));
	}
};

void Kumir_Run::setMassFloatData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, double value, QString *debugString, QString name)
{
	if (debugString != NULL) {
		if (REAL_RES_DEBUG) {
			*debugString = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].name;
		} else {
			*debugString = name;
		}
	}
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraSubexprList, err, debugString);
	if (*err != 0) {
		return;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return;
	};
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setReady(int_args[0], int_args[1], int_args[2], true);
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setFloatMassValue(value, int_args[0], int_args[1], int_args[2]);
	if (debugString != NULL) {
		*debugString += "=" + QString::number(value, 'g', 7);
	}
	if (!debugString->contains(".")) {
		*debugString += ".0";
	}
};

void Kumir_Run::setMassStringData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, const QString &value, QString *debugString, QString name)
{
	if (debugString != NULL) {
		if (REAL_RES_DEBUG) {
			*debugString = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].name;
		} else {
			*debugString = name;
		}
	}
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraSubexprList, err, debugString);
	if (*err != 0) {
		return;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return;
	};
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setReady(int_args[0], int_args[1], int_args[2], true);
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setStringMassValue(value, int_args[0], int_args[1], int_args[2]);
	if (debugString != NULL) {
		*debugString += "=\"" + value + "\"";
	}
};

void Kumir_Run::setMassCharData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraSubexprList,
	int *err, const QChar &value, QString *debugString, QString name)
{
	if (debugString != NULL) {
		if (REAL_RES_DEBUG) {
			*debugString = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].name;
		} else {
			*debugString = name;
		}
	}
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraSubexprList, err, debugString);
	if (*err != 0) {
		return;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return;
	};
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setReady(int_args[0], int_args[1], int_args[2], true);
	Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.setCharMassValue(value, int_args[0], int_args[1], int_args[2]);
	if (debugString != NULL) {
		*debugString += "=\"" + QString(value) + "\"";
	}
}


int Kumir_Run::getMassIntData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraExprList,
	int *err)
{
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraExprList, err);
	if (*err != 0) {
		return 0;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return 0;
	};
	if (!Modules->module(mass_id.mod_id)->Values()->isReady(mass_id.perem_id, int_args[0], int_args[1], int_args[2])) {
		*err = RUN_NO_MASS_VALUE;
		return 0;
	}

	return Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.getIntMassValue(int_args[0], int_args[1], int_args[2]);

};

bool Kumir_Run::getMassBoolData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraExprList,
	int *err)
{
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraExprList, err);
	if (*err != 0) {
		return 0;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return 0;
	};
	if (!Modules->module(mass_id.mod_id)->Values()->isReady(mass_id.perem_id, int_args[0], int_args[1], int_args[2])) {
		*err = RUN_NO_MASS_VALUE;
		return 0;
	}

	return Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.getBoolMassValue(int_args[0], int_args[1], int_args[2]);

};



double Kumir_Run::getMassFloatData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraExprList,
	int *err)
{
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraExprList, err);
	if (*err != 0) {
		return 0;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return 0;
	};
	KumSingleModule *mod = Modules->module(mass_id.mod_id);
	symbol_table *st = mod->Values();
	if (!st->isReady(mass_id.perem_id, int_args[0], int_args[1], int_args[2])) {
		*err = RUN_NO_MASS_VALUE;
		return 0;
	}

	return Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.getFloatMassValue(int_args[0], int_args[1], int_args[2]);

};



QString Kumir_Run::getMassStringData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraExprList,
	int *err)
{
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraExprList, err);
	if (*err != 0) {
		return 0;
	};
	int error = testMassArgs(mass_id, int_args);
	if (error != 0) {
		*err = error;
		return 0;
	};
	if (!Modules->module(mass_id.mod_id)->Values()->isReady(mass_id.perem_id, int_args[0], int_args[1], int_args[2])) {
		*err = RUN_NO_MASS_VALUE;
		return 0;
	}

	return Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.getStringMassValue(int_args[0], int_args[1], int_args[2]);

};

QChar Kumir_Run::getMassCharData(PeremPrt mass_id,
	const QString &mass_args,
	const QStringList &viraj,
	const QStringList &extraExprList,
	int *err)
{
	QChar result;
	QList<int> int_args = calcMassArgs(mass_id.perem_id, mass_id.mod_id, mass_args, viraj, extraExprList, err);
	int error = testMassArgs(mass_id, int_args);
	if (error > 0) {
		*err = error;
		return QChar('0');
	};
	if (!Modules->module(mass_id.mod_id)->Values()->isReady(mass_id.perem_id, int_args[0], int_args[1], int_args[2])) {
		*err = RUN_NO_MASS_VALUE;
		return '!';
	}
	result = Modules->module(mass_id.mod_id)->Values()->symb_table[mass_id.perem_id].value.getCharMassValue(int_args[0], int_args[1], int_args[2]);
	return result;
}

/**
 * кц_при
 * @return код ошибки
 */
int Kumir_Run::do_break(int str) //15-09-Mordol
{

	int error = 0;


	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();//12345
	// outBuff->append("URA VIZIVAETSYA! res "+QString::number(res));
	if (error != 0) {
		//outBuff->append("Error! kc_pri");
		return error;
	};
	if (str > 0 && ((setup.isPoShagam()) || (setup.isDebug()))) {
		sendDebug(res ? trUtf8("да") : trUtf8("нет"), str, curModule);
	}
	if (!res) {
		int NC = pop();
		pop();
		run_stack.push_back(NC);

	}
	if (res) {
		pop();
		pop();
		run_stack.push_back(str + 1);
	};

	return 0;
}


/**
 * Выполнение оператора если
 * @param str Номер строки
 * @return Код ошибки
 */
int Kumir_Run::test_if(int str)
{
	Vibor_Flags.append(false);
	//outBuff->append("Viraj:"+Cur_Line(str).VirajList[0][0]);
	int error = 0;
	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();
	if (error != 0) {
		return error;
	}
	if (!res) {
		run_stack.push_back(Cur_Line(str).else_pos + 1);
	}
	if (res) {
		//run_stack.push_back(Cur_Line(str).else_pos+1);
		run_stack.push_back(str + 1);
	};
	QString out;
	if (res) {
		out = trUtf8("да");
	} else {
		out = trUtf8("нет");
	}
	if ((setup.isPoShagam()) || (setup.isDebug())) {
		sendDebug(out, str, curModule);
	}



	return 0;
};




int Kumir_Run::do_utv(int str)
{
	QStringList vl = Cur_Line(str).VirajList[0];
	//   QString dbg = Cur_Line(str).VirajList[0];
	//   qDebug("dddddd %s",dbg.utf8().data());
	int error = 0;
	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();

	if (error != 0) {
		return error;
	}
	if ((setup.isPoShagam()) || (setup.isDebug())) {
		if (!res) {
			sendDebug(trUtf8("нет"), str, curModule);
		} else {
			sendDebug(trUtf8("да"), str, curModule);
		}
	};

	if (!res) {
		return RUN_FALSE_UTV;    // NEW V.Y. 01.09
	}
	if (res) {
		run_stack.push_back(str + 1);
	};

	return 0;
};




/**
 * Вычисление аргументов функции или алгоритма
 * @param name имя функции или алгоритма
 * @param vizov выражение вызова
 * @param simple_viraj выржения аргументов
 * @param args Таблица переменных для записи результатов
 * @param arguments  Список KumValueStackElem для записи результатов
 * @return код ошибки
 */
int Kumir_Run::prepare_arguments(const QString &name,
	QString vizov,
	const QStringList &simple_viraj,
	const QStringList &extraSubexprList,
	symbol_table *args,
	QList<KumValueStackElem> *arguments)
{
	//qDebug() << simple_viraj;
	QString f_name = name.mid(1);
	//  int break_pos=f_name.indexOf('|');

	int f_id;
	int module_id;
	getFuncParam(f_name, &f_id, &module_id);
	KumSingleModule *funcModule = Modules->module(module_id);
	if (funcModule->Functions()->argCountById(f_id) == 0) {
		return 0;
	}
	if (funcModule->Functions()->typeById(f_id) == none) {
		vizov = simple_viraj[simple_viraj.count() - 1];
	}
	args->free(0);
	arguments->clear();
	QStringList f_arg_list_link = vizov.mid(vizov.indexOf('&') + 1).split("&", QString::SkipEmptyParts);
	QList<int> arg_id_list;
	//  int test_count=f_arg_list_link.count();

	//   vizov=simple_viraj[simple_viraj.count()-1];

	QList<int> vizov_id_list;

	for (int i = 0; i < f_arg_list_link.count(); i++) {
		vizov_id_list.append(f_arg_list_link[i].toInt());    //Список ссылок на выражения
	}

	if (f_arg_list_link.count() != vizov_id_list.count()) {
		return 2;    //Internal Error
	}
	int func_str = funcModule->Functions()->bytePosById(f_id);
	QString func_line = "";
	if (funcModule->isKumIsp()) {
		func_line = funcModule->Proga_Value_Line(func_str).line;
	}

	int point_id = func_line.indexOf("^");

	QList<bool> toDelete;
	for (int i = 0; i < simple_viraj.count() - 1; i++) {
		toDelete.append(false);
	}

	QRegExp rxAmpersand = QRegExp("&(\\d+)");
	QStringList d;
	for (int i = 0; i < simple_viraj.count() - 1; i++) {
		int pos = rxAmpersand.indexIn(simple_viraj[i]);
		while (pos != -1) {
			d = rxAmpersand.capturedTexts();
			QString sLnk = d[1];
			int lnk = sLnk.toInt();
			toDelete[lnk] = true;
			pos = rxAmpersand.indexIn(simple_viraj[i], pos + 1);
		}
	}

	QStringList links;

	for (int i = 0; i < simple_viraj.count() - 1; i++) {

		if (!toDelete[i]) {
			links.append(simple_viraj[i]);
		}
	}

	//  for ( int i=0; i<simple_viraj.count(); i++ ) qDebug("aaa: %s",simple_viraj[i].utf8().data());
	//  for ( int i=0; i<links.count(); i++ ) qDebug("bbb: %s",links[i].utf8().data());

	QList<uint> exprLinks;
	{
		int pos = vizov.indexOf("&");
		if (pos != -1) {
			QString s = vizov.mid(pos + 1);
			QStringList sl = s.split("&");
			foreach (QString t, sl) {
				exprLinks << t.toInt();
			}
		}
	}
	QStringList viraj_ids;
	for (int i = 0; i < funcModule->Functions()->argCountById(f_id); i++) { //Main Circle

		QString testVr = simple_viraj[exprLinks[i]];
		//        PeremType argType=funcModule->Functions()->func_table[f_id].perems[i]; //Тип аргумента

		bool isArgRes = funcModule->Functions()->isArgRes(f_id, i);
		bool isRes = funcModule->Functions()->isRes(f_id, i);
		int id = funcModule->Functions()->argPeremId(f_id, i);

		int zpt_pos = func_line.indexOf(",", point_id);
		if (zpt_pos == -1) {
			zpt_pos = func_line.indexOf(")");
		}
		QString point = func_line.mid(point_id + 1, zpt_pos - point_id - 1);
		QString mass_pos;
		int diez_pos = point.indexOf("#");
		if (diez_pos > 0) {
			mass_pos = point.mid(diez_pos, point.length());
			point = point.left(diez_pos);
		};
		// int perem_id=0;
		//perem_id=point.toInt();

		//int arg_id=0;
		//arg_id=point.toInt();

		if (diez_pos > 0) {
			PeremPrt mass_p;
			mass_p.perem_id = id;
			mass_p.mod_id = module_id;
			QString spls = "dummy" + mass_pos;
			viraj_ids = spls.split('#', QString::SkipEmptyParts);



			int error = calcMassSize(viraj_ids, mass_p, func_str);
			if (error != 0) {
				return error;
			};

		};




		if (isArgRes || isRes) { //� езы И аргрезы

			QString link = simple_viraj[exprLinks[i]];
			//          if(links.count()>i)
			//             link = links[i];//TODO Check Me.Zachem udalayaem elementi? Dlya massivov
			link = link.mid(1);
			int point2 = link.toInt(); //N perem v tekushey tablice simvolov
			//            function_table *table = Cur_Functions();
			function_table *table = funcModule->Functions();


			//            if (f_id < table->size()) {
			PeremType argType = table->argTypeById(f_id, i);
			if ((argType == mass_string) ||
				(argType == mass_charect) ||
				(argType == mass_real) ||
				(argType == mass_bool) ||
				(argType == mass_integer)) {
//                initPerem(table->posById(f_id),module_id);//TODO Chekme


				//                    int error=compareMass(table->argPeremId(f_id,i),module_id,point2);
//                int error = compareMass(Cur_Symbols(), point2,
//                                        funcModule->Values(), table->argPeremId(f_id, i));
//                if(error>0){
//                    return error;
//                }
			}
			PeremPrt ptr;
			ptr.perem_id = point2;
			ptr.mod_id = curModule;
			args->new_argument(integer);//Добавление аргмента в таблицу аргументов

			args->symb_table[i].is_ArgRes = isArgRes;
			args->symb_table[i].is_Res = isRes;
			if (Cur_Symbols()->isRes(point2) || Cur_Symbols()->isArgRes(point2)) {
				ptr = Cur_Symbols()->getPointer(point2);
			};
			args->symb_table[i].value.setIntegerValue(point2);
			args->setPointer(i, ptr.mod_id, ptr.perem_id);



			if ((isArgRes) && (!Modules->isUsed(ptr))) {
				return RUN_NO_VALUE;
			}

			//if(args->isArgRes(id))args->setUsed(id,Cur_Symbols()->isUsed(point2));
			if (args->isArgRes(i)) {
				args->setUsed(i, Cur_Symbols()->isUsed(point2));
			}
			// if(args->isRes(i))args->setUsed(i,Cur_Symbols()->isUsed(point2));

			//if(args->isArgRes(id))args->setUsed(id,args->isUsed( point2));

			//if(args->isRes(id)){args->setUsed(id,false);Cur_Symbols()->setUsed(point2,false); };
			if (args->isRes(i)) {
				args->setUsed(i, false);
				Cur_Symbols()->setUsed(point2, false);
			};
			//args->setUsed(id,true);
			args->setPointer(i, ptr.mod_id, ptr.perem_id);
			// ??????????????????????????????????????????????????????????????????????
			arguments->append(KumValueStackElem(ptr.perem_id));
			arguments->append(KumValueStackElem(ptr.mod_id));

			continue;
		}

		PeremType argumentType = funcModule->Functions()->argTypeById(f_id, i);

		if (argumentType == kumBoolean) {

			int error = 0;
			args->new_argument(kumBoolean);
			bool res = calc_simple_universal(simple_viraj,
					extraSubexprList,
					simple_viraj[vizov_id_list[i]],
					&error).asBool();
			if (error != 0) {
				return error;
			};

			if (Modules->module(module_id)->isKumIsp()) {
				args->symb_table[i].value.setBoolValue(res);
				//PeremPrt copy;
				//copy.perem_id = id;
				//copy.mod_id = module_id;
				//                Modules->setBoolValue(copy,res);
				//                Modules->setUsed(copy,true);

			};
			args->setUsed(i, true);
			arguments->append(KumValueStackElem(res));
		}


		if (argumentType == kumString) {

			int error = 0;
			args->new_argument(kumString);
			QString res = calc_simple_universal(simple_viraj,
					extraSubexprList,
					simple_viraj[vizov_id_list[i]],
					&error).asString();
			if (error != 0) {
				return error;
			};
			if (Modules->module(module_id)->isKumIsp()) {
				args->symb_table[i].value.setStringValue(res);
			}
			if (Modules->module(module_id)->isKumIsp()) {
				//PeremPrt copy;
				//copy.perem_id = id;
				//copy.mod_id = module_id;
				//                Modules->setStringValue(copy,res);
				//                Modules->setUsed(copy,true);
			};
			args->setUsed(i, true);
			arguments->append(KumValueStackElem(res));

		}

		if (argumentType == charect) {
			int error = 0;
			QChar res = calc_simple_universal(simple_viraj,
					extraSubexprList,
					simple_viraj[vizov_id_list[i]],
					&error).asCharect();
			if (error != 0) {
				return error;
			};
			args->new_argument(charect);

			if (Modules->module(module_id)->isKumIsp()) {
				args->symb_table[i].value.setCharectValue(res);
			}
			args->setUsed(i, true);
			args->symb_table[i].type = charect;
			arguments->append(KumValueStackElem(res));
		}

		if (argumentType == integer) {
			int error = 0;
			int res = calc_simple_universal(simple_viraj,
					extraSubexprList,
					simple_viraj[vizov_id_list[i]],
					&error).asInt();
			if (error != 0) {
				return error;
			};
			args->new_argument(integer);
			if (Modules->module(module_id)->isKumIsp()) {
				args->symb_table[i].value.setIntegerValue(res);

				//PeremPrt copy;
				//copy.perem_id = id;
				//copy.mod_id = module_id;
				//                Modules->setIntegerValue(copy,res);
				//                Modules->setUsed(copy,true);
			};
			args->setUsed(i, true);
			arguments->append(KumValueStackElem(res));
		}

		if (argumentType == real) {
			int error = 0;
			double res = calc_simple_universal(simple_viraj,
					extraSubexprList,
					simple_viraj[vizov_id_list[i]],
					&error).asFloat();
			if (error != 0) {
				return error;
			};
			args->new_argument(real);
			if (Modules->module(module_id)->isKumIsp()) {
				args->symb_table[i].value.setFloatValue(res);
			}
			args->setUsed(i, true);
			arguments->append(KumValueStackElem(res));
		}

		if (argumentType == mass_bool || argumentType == mass_real ||
			argumentType == mass_integer || argumentType == mass_charect ||
			argumentType == mass_string) {
			//            initPerem(funcModule->Functions()->posById(f_id),module_id);
			QString v_mass_str = simple_viraj[exprLinks[i]];

			//            int error=0;
			v_mass_str = v_mass_str.right(vizov.length() - 1);
			int a = v_mass_str.indexOf("&");
			if (a == -1) {
				a = v_mass_str.length();
			}

			QString b = v_mass_str.mid(1, a - 1);
			int pointer_to = b.toInt();
			if (pointer_to == -1) {
				return INT_R_07;
			}
			//            symbol_table *moduleSymbols = funcModule->Values();
			symbol_table *mySymbols = Cur_Symbols();
			//            error = compareMass(mySymbols, pointer_to,
			//                                moduleSymbols, id);
			//            if (error>0) {
			//                return error;
			//            }

			args->new_argument(argumentType);
			PeremPrt from_p;
			from_p.mod_id = curModule;
			from_p.perem_id = pointer_to;
			bool is_res = mySymbols->isRes(pointer_to);
			bool is_argres = mySymbols->isArgRes(pointer_to);
			if (is_res || is_argres) {
				from_p = Cur_Symbols()->getPointer(pointer_to);
			}
			//            if (argumentType==mass_bool)
			//                error=Modules->module(from_p.mod_id)->Values()->copyMassBoolArg(from_p.perem_id,i,args);
			//            else if (argumentType==mass_integer)
			//                error=Modules->module(from_p.mod_id)->Values()->copyMassIntArg(from_p.perem_id,i,args);
			//            else if (argumentType==mass_real)
			//                error=Modules->module(from_p.mod_id)->Values()->copyMassRealArg(from_p.perem_id,i,args);
			//            else if (argumentType==mass_string)
			//                error=Modules->module(from_p.mod_id)->Values()->copyMassStringArg(from_p.perem_id,i,args);
			//            else if (argumentType==mass_charect)
			//                Modules->module(from_p.mod_id)->Values()->copyMassCharectArg(from_p.perem_id,i,args);
			//            if (error>0) {
			//                return error;
			//            }
			args->symb_table[i].size0_start = mySymbols->symb_table[from_p.perem_id].size0_start;
			args->symb_table[i].size0_end = mySymbols->symb_table[from_p.perem_id].size0_end;
			args->symb_table[i].size1_start = mySymbols->symb_table[from_p.perem_id].size1_start;
			args->symb_table[i].size1_end = mySymbols->symb_table[from_p.perem_id].size1_end;
			args->symb_table[i].size2_start = mySymbols->symb_table[from_p.perem_id].size2_start;
			args->symb_table[i].size2_end = mySymbols->symb_table[from_p.perem_id].size2_end;
			args->symb_table[i].razm = mySymbols->symb_table[from_p.perem_id].razm;
			args->symb_table[i].value = mySymbols->symb_table[from_p.perem_id].value;
			args->setUsed(i, Cur_Symbols()->isUsed(id));
		}

		point_id = func_line.mid(zpt_pos + 1).count("^");

	} //End Of main Circle


	return 0;
};




/**
 * Запуск функции при вычислении выражения
 * � езультаты функции записываются в одноименную переменную.
 *
 * @param name имя функции
 * @param simple_viraj список выражений-аргументов
 * @return целочисленный эквивалент вычисленной функции(используется для отладки)
 */
int Kumir_Run::run_f(const QString &name,
	const QString &vizov,
	const QStringList &simple_viraj,
	const QStringList &extraSubexprList,
	int *err)
{

	ispErrorCode = 0;
	if (stoped) {
		return 0;
	}
	QList<KumValueStackElem> arguments;
	//QList<void *> args;

	//  args.clear();
	QString f_name = name.mid(1);

	int f_id, mod_id;
	getFuncParam(vizov, &f_id, &mod_id);
	QStringList f_arg_list_link = vizov.mid(vizov.indexOf('&') + 1).split("&", QString::SkipEmptyParts);
	QList<int> arg_id_list;


	symbol_table *f_arguments = new symbol_table();

	//  int test_count=f_arg_list_link.count();

	for (int i = 0; i < f_arg_list_link.count(); i++) {
		arg_id_list.append(f_arg_list_link[i].toInt());
	}


	int func_str = Modules->module(mod_id)->Functions()->bytePosById(f_id);
	bool forceStepMode = Modules->module(mod_id)->Functions()->isRunStepInto(f_id);

	if (Modules->module(mod_id)->isKumIsp()) {
		Modules->module(mod_id)->saveValues();
	}

	int error = prepare_arguments(name,
			vizov,
			simple_viraj,
			extraSubexprList,
			f_arguments,
			&arguments);

	if (error > 0) {
		*err = error;
		return error;
	}


	if (!Modules->module(mod_id)->isKumIsp()) { // NEW 2007-06-26//TODO ISPTYPES
		int error = 0;

		if (!Modules->module(mod_id)->Graphics() && !Modules->module(mod_id)->isNetwork()) {

			Modules->module(mod_id)->instrument()->runFunc(
				Modules->module(mod_id)->Functions()->nameById(f_id),
				&arguments, &error);

			if (error > 0) {
				*err = error;    //09-06-Mordol
				return error;
			}
		} else { //Исполнители с графикой
			int localError = 0;
			QString name_ts = Modules->module(mod_id)->Functions()->nameById(f_id);
			//qDebug()<<"Before emit name:"<<name_ts;
			syncFlag = false;
			if (!Modules->module(mod_id)->isNetwork()) {
				sendGraphics(name_ts, &arguments, &localError);
			} else
				sendNetwork(mod_id,
					Modules->module(mod_id)->Functions()->nameById(f_id),
					&arguments,
					&localError);

			//qDebug("After");

			if (localError != 0) {
				*err = localError;
				return localError;
			}

			// qDebug("After disconnect");
			// sendText("Error:"+QString::number(err));
			if (localError != 0) {
				*err = localError;
				return localError;
			}
		};

		if (error != 0) {
			*err = error;
			return 0;
		};//09-06-Mordol
	} else {

		deep++;
		Kumir_Run *f_run = new Kumir_Run();

		//        Modules->module(mod_id)->CopyValues();

		f_run->initForFunc(setup, deep, func_str, this, mod_id);
		RunMode m = (RunMode)mode;
		if (forceStepMode && m == po_shagam) {
			f_run->setMode(po_shagam);
		} else {
			f_run->setMode(setup.func_mode);
		}
		child = f_run;
		if (deep > MAX_CALL_DEEP) {
			return RUN_MANY_CALLS;
		}
#ifdef WIN32
		if (deep > MAX_CALL_DEEP) {
			*err = RUN_MANY_CALLS;
			return RUN_MANY_CALLS;
		};
#endif
		//qDebug("Deep:%i",deep);
		int child_err = f_run->runForFunc(f_id, f_arguments);


		deep--;
		if (stoped) {
			if (f_arguments) {
				delete f_arguments;
			}
			return 0;
		};
		//        Modules->module(mod_id)->localRestoreValues(
		//                Modules->module(mod_id)->Functions()->nameById(f_id)
		//                );
		Modules->module(mod_id)->restoreValues();
		if (f_run) {
			f_run->deleteLater();
		}
		if (child) {
			deleteChild();
		}
		child = NULL;
		if (child_err != 0) {
			*err = child_err;
		}


	};
	//    f_id = f_name.toInt();
	//  int isp_id=ispolniteli->inTable(Cur_Functions()->func_table[f_id].isp);

	int perem_id = Modules->FuncPerm(f_id, mod_id);
	if (perem_id == -1) {
		perem_id = f_name.toInt();
	}; //09-06-Mordol
	// qDebug("Before getValue %i",perem_id);
	int to_ret = 0;
	// qDebug("After getValue");
	if (f_arguments) {
		delete f_arguments;
	}
	return to_ret;

};



int Kumir_Run::do_funct(int str)
{
	//qDebug("Run:do_funct:begins");
	ispErrorCode = 0;
	QList<KumValueStackElem> *arguments = new QList<KumValueStackElem>();
	QString line = Cur_Line(str).line;
	//  int skobk_pos=line.indexOf("(");
	QString inskobk, name;
	name = line;
	name = name.trimmed();

	int funct_id, mod_id;
	getFuncParam(name, &funct_id, &mod_id);
	symbol_table *f_arguments = new symbol_table();
	//  int func_str=Modules->module(mod_id)->Functions()->func_table[funct_id].proga_str;


	QString vizov = "@" + line;
	for (int i = 0; i < Modules->module(mod_id)->Functions()->argCountById(funct_id); i++) {
		vizov = vizov + "&" + QString::number(i);
	}

	if (Modules->module(mod_id)->isSystem()) {
		//sendText("Debug:"+Modules->module(mod_id)->Functions()->nameById(funct_id));
		if (Modules->module(mod_id)->Functions()->nameById(funct_id) == trUtf8("пауза")) {
			if (Cur_Line(str).real_line.tab) {
				setup.setMode(po_shagam);
			}

			Pause(str, curModule);
			run_stack.push_back(str + 1);
			return 0;
		};
		if (Modules->module(mod_id)->Functions()->nameById(funct_id) == trUtf8("стоп")) {

			//int error=prepare_arguments("@"+name,"",viraj,&f_arguments,&arguments);
			QString errText = trUtf8("\nСТОП.");
			sendText(errText);

			return -2;
		};
		if (Modules->module(mod_id)->Functions()->nameById(funct_id) == trUtf8("НАЗНАЧИТЬ ВЫВОД")) {

			QStringList viraj;
			if (Cur_Line(str).VirajList.count() > 0) {
				viraj = Cur_Line(str).VirajList[0];
			}
			int error = prepare_arguments("@" + name, "", viraj, Cur_Line(str).extraSubExpressions, f_arguments, arguments);
			if (error > 0) {
				return error;
			}
			KumValueStackElem el = arguments->at(0);
			QString fileN = el.asString();
			emit sendChangeOut(fileN);
			qDebug() << "Vivod !!" << fileN;

			if (f_arguments) {
				delete f_arguments;
			}
			run_stack.push_back(str + 1);
			return 0;
		};
		if (Modules->module(mod_id)->Functions()->nameById(funct_id) == trUtf8("НАЗНАЧИТЬ ВВОД")) {

			QStringList viraj;
			if (Cur_Line(str).VirajList.count() > 0) {
				viraj = Cur_Line(str).VirajList[0];
			}
			int error = prepare_arguments("@" + name, "", viraj, Cur_Line(str).extraSubExpressions, f_arguments, arguments);
			if (error > 0) {
				return error;
			}
			KumValueStackElem el = arguments->at(0);
			QString fileN = el.asString();
			emit sendChangeIn(fileN);
			qDebug() << "Vivod !!" << fileN;

			if (f_arguments) {
				delete f_arguments;
			}
			run_stack.push_back(str + 1);
			return 0;
		};
		if (f_arguments) {
			delete f_arguments;
		}
		return 2;
	} else if ((!Modules->module(mod_id)->isInternal()) && (!Modules->module(mod_id)->isNetwork()) && (!Modules->module(mod_id)->isFriend())) { //Исполнители написанные на екимр
		if (Modules->module(mod_id)->isNetwork()) {
			qDebug() << "do_funct:Module Is Network!!!";
		}
		deep++;
		if (deep > MAX_CALL_DEEP) {
			qDebug("Deep recursion");
			return RUN_MANY_CALLS;
		};
#ifdef WIN32
		if (deep > MAX_CALL_DEEP) {
			return RUN_MANY_CALLS;
		}
#endif
		//        Modules->module(mod_id)->CopyValues();
		if (Modules->module(mod_id)->isKumIsp()) {
			Modules->module(mod_id)->saveValues();
		}
		QStringList viraj;
		if (Cur_Line(str).VirajList.count() > 0) {
			viraj = Cur_Line(str).VirajList[0];
		}
		int error = prepare_arguments("@" + name, "", viraj, Cur_Line(str).extraSubExpressions, f_arguments, arguments);
		if (error != 0) {
			if (f_arguments) {
				delete f_arguments;
			}
			return error;
		};





		run_stack.push_back(str + 1);

		Kumir_Run *f_run = new Kumir_Run();
		bool forceStepMode =
			Modules->module(mod_id)->Functions()->isRunStepInto(funct_id);
		RunMode m = (RunMode)mode;

		f_run->initForFunc(setup,
			deep,
			Modules->module(mod_id)->Functions()->posById(funct_id),
			this,
			mod_id);

		if (forceStepMode && m == po_shagam) {
			f_run->setMode(po_shagam);
		} else {
			f_run->setMode(setup.func_mode);
		}

		child = f_run;
		error = f_run->runForFunc(funct_id, f_arguments);

		//if(deep>96)return RUN_MANY_CALLS;

		//        Modules->module(mod_id)->localRestoreValues(
		//                Modules->module(mod_id)->Functions()->nameById(funct_id)
		//                );
		Modules->module(mod_id)->restoreValues();
		if (deep > 10) {
			usleep(10);
		}
		deep--;
		if (f_run) {
			f_run->deleteLater();
		};
		if (f_arguments) {
			delete f_arguments;
		}
		deleteChild();
		child = NULL;
		return error;


	} else { //internal
		int err = 0;
		QStringList viraj;
		if (Modules->module(mod_id)->Functions()->argCountById(funct_id) > 0) {
			viraj = Cur_Line(str).VirajList[0];
			err = prepare_arguments("@" + name, "  ", viraj, Cur_Line(str).extraSubExpressions, f_arguments, arguments);
			if (err != 0) {
				return err;
			}
		};

		if (!Modules->module(mod_id)->isKumIsp()) { // NEW 2007-06-26//TODO ISPTYPES
			int error = 0;
			if (!Modules->module(mod_id)->Graphics() && !Modules->module(mod_id)->isNetwork()) {
				Modules->module(mod_id)->instrument()->runFunc(
					Modules->module(mod_id)->Functions()->nameById(funct_id),
					arguments,
					&error);

				if (error > 0) {
					return error;    //09-06-Mordol
				}
			} else { //Исполнители с графикой
				int err = 0;


				syncFlag = false;
				//qDebug("Run:do_funct:emit:args %i",arguments.count());
				QString name_ts = Modules->module(mod_id)->Functions()->nameById(funct_id);
				//qDebug()<<"Before emit name:"<<name_ts;
				syncFlag = false;
				if (!Modules->module(mod_id)->isNetwork()) {
					sendGraphics(name_ts, arguments, &err);
				} else {
					sendNetwork(mod_id, Modules->module(mod_id)->
						Functions()->nameById(funct_id),
						arguments,
						&err);

					if (stoped) {
						return -2;
					}
				}

				if (err != 0) {
					if (f_arguments) {
						delete f_arguments;
					}
					return err;
				}

				if (err != 0) {
					return err;
				}
			}
			if (str >= 0) {
				run_stack.push_back(str + 1);
			}
		}
	}
	if (f_arguments) {
		delete f_arguments;
	}
	delete arguments;
	return 0;
}

/**
 * Выполнение оператора выбор
 * @param str
 * @return
 */
int Kumir_Run::do_vibor(int str)
{
	/*int perem_id=Cur_Line(str).line.toInt();*/
	Vibor_Flags.append(false);
	vibor_stack.append(1);
	run_stack.push_back(Cur_Line(str).else_pos);
	return 0;
};


/**
 * Выполнение оператора при.
 * @param str
 * @return
 */
int Kumir_Run::do_pri(int str)
{


	//int perem_id=vibor_stack.last();
	int elsepos = Cur_Line(str).else_pos;
	//qDebug("VF count: %i",Vibor_Flags.count());
	if (Vibor_Flags[Vibor_Flags.count() - 1]) {

		//ZAPLATKA DLYA POSHAGOGO VIPOLNENIYA
		int pos = Cur_Line(str).else_pos;

		while (Cur_Line(pos).str_type == Kumir::Case) {
			pos = Cur_Line(pos).else_pos;
		}
		//KONEC ZAPLATKI

		run_stack.push_back(pos);
		//qDebug("count: %i",vibor_stack.count());
		//vibor_stack.pop_back();
		return 0;
	};
	//if(Proga_Value[elsepos].str_type==IF_ZAKR)
	//                {
	//             vibor_stack.pop_back();
	//            };
	QStringList virajList = Cur_Line(str).VirajList[0];

	//PeremType test=Cur_Symbols()->typeById(perem_id);
	// PeremType test2=Cur_Line(str).VirajTypes[0];



	//virajList[0]="^"+QString::number(perem_id)+"=("+virajList[0]+")";
	//outBuff->append("viraj:"+virajList[0]);

	int error = 0;
	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();

	if (error != 0) {
		return error;
	}
	if (!res) {
		if ((setup.isPoShagam()) || (setup.isDebug())) {
			sendDebug(QString::fromUtf8("нет"), str, curModule);
		}
		if ((Cur_Line(elsepos).str_type == Kumir::Else) &&
			(vibor_stack[vibor_stack.count() - 1] == 1)) {
			run_stack.push_back(Cur_Line(str).else_pos + 1);
			vibor_stack.pop_back();
		} else {
			run_stack.push_back(Cur_Line(str).else_pos);
		};
	};
	if (res) {
		if ((setup.isPoShagam()) || (setup.isDebug())) {
			sendDebug(trUtf8("да"), str, curModule);
		}
		run_stack.push_back(str + 1);
		vibor_stack[vibor_stack.count() - 1] = 0;
		Vibor_Flags[Vibor_Flags.count() - 1] = true;
	};


	return 0;
};



int Kumir_Run::do_dano(int str)
{
	QStringList vl = Cur_Line(str).VirajList[0];
	//   QString dbg = Cur_Line(str).VirajList[0];
	//   qDebug("dddddd %s",dbg.utf8().data());
	int error = 0;
	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();

	if (error != 0) {
		return error;
	}
	if (str > 0) {
		if ((setup.isPoShagam()) || (setup.isDebug())) {
			if (!res) {
				sendDebug(QString::fromUtf8(" нет"), str, curModule);
			} else {
				sendDebug(QString::fromUtf8(" да"), str, curModule);
			}
		};
	};
	if (!res) {
		return RUN_BAD_DANO;    // NEW V.Y. 01.09
	}
	if (res) {
		qDebug("Dano pos:%i", Cur_Line(str).else_pos);
		run_stack.push_back(Cur_Line(str).else_pos);
	};

	return 0;
};


int Kumir_Run::do_nado(int str)
{
	QStringList vl = Cur_Line(str).VirajList[0];
	//   QString dbg = Cur_Line(str).VirajList[0];
	//   qDebug("dddddd %s",dbg.utf8().data());
	int error = 0;
	bool res = calc_simple_universal(Cur_Line(str).VirajList[0],
			Cur_Line(str).extraSubExpressions,
			Cur_Line(str).VirajList[0].last(),
			&error).asBool();

	if (error != 0) {
		return error;
	}
	if (str > 0) {
		if ((setup.isPoShagam()) || (setup.isDebug())) {
			if (!res) {
				sendDebug(QString::fromUtf8(" Ложно"), str, curModule);
			} else {
				sendDebug(QString::fromUtf8(" Истинно"), str, curModule);
			}
		};
	};
	if (!res) {
		return RUN_BAD_NADO;    // NEW V.Y. 01.09
	}


	return 0;
};

void Kumir_Run::inputReady(QStringList input)//SLOT
{
	vvod_str = input;
	vvodFlag = true;
	if (child != NULL) {
		child->inputReady(input);
	}

};

/**
 * Выполнение ввода.
 * @param str
 * @return
 */
int Kumir_Run::do_vvod(int str)
{
	//outPutWindow->setStop();





	QString arg_line;
	QString line = Cur_Line(str).line;
	QString perem_str = line.mid(1);
	int emp_mass_pos = perem_str.indexOf('&');
	int perem_t_id = Cur_Line(str).VirajList.count();
	QString format;
	//qDebug("Vvod count:%i",Cur_Line(str).VirajList.count());
	for (int j = 0; j < Cur_Line(str).VirajList.count(); j++) {
		//  qDebug("Cur_Line(str).VirajList["+QString::number(j)+"].last()="+Cur_Line(str).VirajList[j].last());
		QString perem_str = Cur_Line(str).VirajList[j].last();
		emp_mass_pos = perem_str.indexOf('&');
		if (emp_mass_pos < 0) {
			// perem_str=perem_str.mid(1);
			perem_t_id = perem_str.mid(1, 999).toInt();
		} else {
			//QString mass_str=perem_str.mid(1,emp_mass_pos-1);
			perem_t_id = perem_str.mid(1, emp_mass_pos - 1).toInt();
			arg_line = perem_str.mid(emp_mass_pos);
		};
		PeremPrt  point2;
		point2.mod_id = curModule;
		point2.perem_id = perem_t_id;
		//QString vvod_perem=Cur_Line(str).line;

		if (Cur_Symbols()->isRes(perem_t_id) || (Cur_Symbols()->isArgRes(perem_t_id))) {
			point2 = Cur_Symbols()->pointTo(perem_t_id);
		}
		vvodFlag = false;
		vvod_str.clear();

		//      if ( Cur_Symbols()->isConst ( perem_id ) )
		//          return RUN_CONST_INPUT;
		//      Cur_Symbols()->setUsed ( perem_id,true );
		PeremType perType = Cur_Symbols()->getTypeByID(perem_t_id);
		if (perType == kumString && Cur_Symbols()->isConst(perem_t_id)) {
			QString text = Modules->getStringValue(point2);
			text.replace("\\", "\\\\");
			text.replace("%", "\\%");
			format += text;
		} else if (perType == integer || perType == mass_integer) {
			format += "%i";
		} else if (perType == real  || perType == mass_real) {
			format += "%f";
		} else if (perType == kumBoolean || perType == mass_bool) {
			format += "%b";
		} else if (perType == kumString || perType == mass_string) {
			format += "%s";
		} else if (perType == charect || perType == mass_charect) {
			format += "%c";
		}
	}
	// if ( ( perType != charect ) && ( perType != mass_charect ) )
	//  sendInput(false);
	//else
	//      sendInput(perType);

	sendInput(format);

	while ((!vvodFlag) && (!stoped) && (ispErrorCode != ISP_INT_ERROR)) {
		usleep(50);
		if (stoped) {
			return 0;
		}
	};
	if (ispErrorCode == ISP_INT_ERROR) {
		return ISP_INT_ERROR;
	}
	K_ASSERT(vvod_str.count() == Cur_Line(str).VirajList.count());
	QStringList msgTexts;
	for (int j = 0; j < Cur_Line(str).VirajList.count(); j++) {
		//  qDebug("Cur_Line(str).VirajList["+QString::number(j)+"].last()="+Cur_Line(str).VirajList[j].last());
		QString perem_str = Cur_Line(str).VirajList[j].last();
		emp_mass_pos = perem_str.indexOf('&');
		if (emp_mass_pos < 0) {
			// perem_str=perem_str.mid(1);
			perem_t_id = perem_str.mid(1, 999).toInt();
		} else {
			//QString mass_str=perem_str.mid(1,emp_mass_pos-1);
			perem_t_id = perem_str.mid(1, emp_mass_pos - 1).toInt();
			arg_line = perem_str.mid(emp_mass_pos);
		};


		PeremPrt  point2;
		point2.mod_id = curModule;
		point2.perem_id = perem_t_id;
		//QString vvod_perem=Cur_Line(str).line;

		if (Cur_Symbols()->isRes(perem_t_id) || (Cur_Symbols()->isArgRes(perem_t_id))) {
			point2 = Cur_Symbols()->pointTo(perem_t_id);
		}
		if ((Cur_Symbols()->isArgRes(perem_t_id))) {
			point2 = Cur_Symbols()->pointTo(perem_t_id);
		}
		//QString vvod_perem=Cur_Line(str).line;


		vvodFlag = false;
		//          vvod_str.clear();

		//      if ( Cur_Symbols()->isConst ( perem_id ) )
		//          return RUN_CONST_INPUT;
		Cur_Symbols()->setUsed(perem_t_id, true);

		//          if ( Cur_Symbols()->isRes ( perem_t_id ) )
		//          {
		//                  return RUN_INPUT_RES;
		//          };


		PeremType perType = Cur_Symbols()->getTypeByID(perem_t_id);

		switch (perType) {
		case integer: {
			bool ok;
			int value = vvod_str[j].toInt(&ok);
			if (!ok) {
				return RUN_INPUT_INT;
			}

			Modules->setIntegerValue(point2, value);
			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+QString::number ( value ),str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(value);
			Modules->setUsed(point2, true);
			break;
		};

		case mass_string: {
			int error = 0;


			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}

			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}

			QString  res = vvod_str[j];
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setStringMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if (error > 0) {
				return error;
			}
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + vvod_str[j] + "\"";
			}
			break;
		};



		case mass_integer: {

			int error = 0;
			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}

			bool ok;

			int res = vvod_str[j].toInt(&ok);
			if (!ok) {
				return RUN_INPUT_INT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setIntMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if (error > 0) {
				return error;
			}
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + vvod_str[j] + "\"";
			}
			break;
		};

		case mass_real: {
			int error = 0;

			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}

			bool ok;

			double res = vvod_str[j].toDouble(&ok);
			if (!ok) {
				return RUN_INPUT_FLOAT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setFloatMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + vvod_str[j] + "\"";
			}

			if (error > 0) {
				return error;
			}



			break;
		};


		case mass_bool: {
			int error = 0;

			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}
			QString tmp = vvod_str[j].toUpper();
			bool res;
			if (tmp == trUtf8("ДА") || tmp == trUtf8("НЕТ")) {
				res = (tmp == trUtf8("ДА"));
			} else {
				return RUN_INPUT_BOOL;
			}


			if (error > 0) {
				return error;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setBoolMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + vvod_str[j].toLower() + "\"";
			}


			break;
		};


		case mass_charect: {
			int error = 0;

			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}


			int code = vvod_str[j].toInt();
			QChar chr = QChar(code);

			if (error > 0) {
				return error;
			}
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setCharMassValue(chr, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString p;
				p = "\"" + QString(chr) + "\"";
				//                  if ( charPrintable ( chr ) ) //TODO ??????
				//                      p = "\""+QString ( chr ) +"\"";
				//                  else
				//                  {
				//                      if ( code == 4100 ) // Enter
				//                          p = "Enter";
				//                      if ( code == 4114 ) // Left
				//                          p = " " + QString ( QChar ( 0x2190 ) ) + " ";
				//                      if ( code == 4115 ) // Up
				//                          p = " " + QString ( QChar ( 0x2191 ) ) + " ";
				//                      if ( code == 4116 ) // Right
				//                          p = " " + QString ( QChar ( 0x2192 ) ) + " ";
				//                      if ( code == 4117 ) // Down
				//                          p = " " + QString ( QChar ( 0x2193 ) ) + " ";
				//                      if ( code == 4103 ) // Delete
				//                          p = "Delete";
				//                      if ( code == 4099 ) // Backspace
				//                          p = " " + QString ( QChar ( 0x21D0 ) ) + " ";
				//                      if ( code == 4102 ) // Insert
				//                          p = "Insert";
				//                      p += QString::fromUtf8 ( " (код " ) +vvod_str+")";
				//                  }
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"="+p,str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=" + p;
			}

			break;
		};

		case kumString:

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setStringValue(vvod_str[j]);

			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"=\""+vvod_str[j]+"\"",str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=\"" + vvod_str[j] + "\"";
			Modules->setUsed(point2, true);
			break;

		case charect: {
			QString ch;

			ch = vvod_str[j];
			K_ASSERT(ch.length() == 1);
			QChar chr = QChar(ch[0]);

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setCharectValue(chr);
			Modules->setUsed(point2, true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString p;
				p = "\"" + QString(chr) + "\"";
				//                  if ( charPrintable ( chr ) )
				//                      p = "\""+QString ( chr ) +"\"";
				//                  else
				//                  {
				//                      if ( code == 4100 ) // Enter
				//                          p = "Enter";
				//                      if ( code == 4114 ) // Left
				//                          p = " " + QString ( QChar ( 0x2190 ) ) + " ";
				//                      if ( code == 4115 ) // Up
				//                          p = " " + QString ( QChar ( 0x2191 ) ) + " ";
				//                      if ( code == 4116 ) // Right
				//                          p = " " + QString ( QChar ( 0x2192 ) ) + " ";
				//                      if ( code == 4117 ) // Down
				//                          p = " " + QString ( QChar ( 0x2193 ) ) + " ";
				//                      if ( code == 4103 ) // Delete
				//                          p = "Delete";
				//                      if ( code == 4099 ) // Backspace
				//                          p = " " + QString ( QChar ( 0x21D0 ) ) + " ";
				//                      if ( code == 4102 ) // Insert
				//                          p = "Insert";
				//                      p += QString::fromUtf8 ( " (код " ) +vvod_str+")";
				//                  }
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+p,str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + p;
			}
		}
		break;

		case real: {
			bool ok;
			double value = vvod_str[j].toDouble(&ok);
			if (!ok) {
				return RUN_INPUT_FLOAT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setFloatValue(value);
			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+QString::number ( value ),str );
			QString dbgMsg = Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(value);
			if (!dbgMsg.contains('.')) {
				dbgMsg = dbgMsg + ".0";
			}
			msgTexts << dbgMsg;
			Modules->setUsed(point2, true);
		}
		break;

		case kumBoolean: {

			bool value = false;
			QString tmp = vvod_str[j].toUpper();
			if ((tmp != trUtf8("ДА")) && (tmp != trUtf8("НЕТ"))) {
				return RUN_INPUT_BOOL;
			}
			if (tmp == trUtf8("ДА")) {
				value = true;
			}
			if (tmp == trUtf8("НЕТ")) {
				value = false;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setBoolValue(value);
			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+vvod_str[j],str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + vvod_str[j].toLower();
			Modules->setUsed(point2, true);
		}
		break;

		default:
			break;

		};
	};
	if ((setup.isPoShagam()) || (setup.isDebug())) {
		QString msg = msgTexts.join(", ");
		sendDebug(msg, str, curModule);
	}
	run_stack.push_back(str + 1);

	//outPutWindow->cleanStop();
	if (setup.isPoShagam()) {
		emit sendMessage(QApplication::translate("KumIOArea", "Input finished."), 0);
	}
	return 0;

};



/**
 * Выполнение ввода.из файла
 * @param str
 * @return
 */
int Kumir_Run::do_fvvod(int str)
{
	//outPutWindow->setStop();




	int filesIspId = Modules->idByName("FilesP");
	if (filesIspId == -1) {
		return 2;
	}
	KumPFiles *filesInstreument = (KumPFiles *)Modules->module(filesIspId)->instrument();
	QString arg_line;
	QString line = Cur_Line(str).line;
	QString perem_str = line.mid(1);
	int emp_mass_pos = perem_str.indexOf('&');
	int perem_t_id = Cur_Line(str).VirajList.count();
	QString format;
	QString key_str = Cur_Line(str).VirajList[0].last();
	int key_id = key_str.mid(1, 999).toInt();
	if ((key_id < 0)) {
		return 2;
	}
	int key = Cur_Symbols()->getValueAsInt(key_id);
	if (key < 0) {
		return RT_FILE_NO_KEY;
	};



	QStringList msgTexts;
	for (int j = 1; j < Cur_Line(str).VirajList.count(); j++) {

		QString perem_str = Cur_Line(str).VirajList[j].last();
		emp_mass_pos = perem_str.indexOf('&');
		if (emp_mass_pos < 0) {
			// perem_str=perem_str.mid(1);
			perem_t_id = perem_str.mid(1, 999).toInt();
		} else {
			//QString mass_str=perem_str.mid(1,emp_mass_pos-1);
			perem_t_id = perem_str.mid(1, emp_mass_pos - 1).toInt();
			arg_line = perem_str.mid(emp_mass_pos);
		};
		PeremPrt  point2;
		point2.mod_id = curModule;
		point2.perem_id = perem_t_id;
		if ((Cur_Symbols()->isArgRes(perem_t_id))) {
			point2 = Cur_Symbols()->pointTo(perem_t_id);
		}

		if (Cur_Symbols()->isConst(perem_t_id)) {
			return RUN_CONST_INPUT;
		}

		if (Cur_Symbols()->isRes(perem_t_id)) {
			return RUN_INPUT_RES;
		};
		Modules->setUsed(point2, true);




		PeremType perType = Cur_Symbols()->getTypeByID(perem_t_id);
		int err = 0;
		int pos = 0;

		QString lexem = filesInstreument->getLexem(perType, key, &err, &pos);
		if (err > 0) {
			return err;
		}
		switch (perType) {
		case integer: {
			bool ok;
			int value = lexem.toInt(&ok);
			if (!ok) {
				return RUN_INPUT_INT;
			}

			Modules->setIntegerValue(point2, value);
			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+QString::number ( value ),str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(value);

			break;
		};

		case mass_string: {
			int error = 0;
			;
			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}


			QString  res = lexem;
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setStringMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if (error > 0) {
				return error;
			}
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + lexem + "\"";
			}
			break;
		};



		case mass_integer: {
			int error = 0;


			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}


			bool ok;

			int res = lexem.toInt(&ok);
			if (!ok) {
				return RUN_INPUT_INT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setIntMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if (error > 0) {
				return error;
			}
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + lexem + "\"";
			}
			break;
		};

		case mass_real: {
			int error = 0;


			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}


			bool ok;

			double res = lexem.toFloat(&ok);
			if (!ok) {
				return RUN_INPUT_FLOAT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setFloatMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + lexem + "\"";
			}

			if (error > 0) {
				return error;
			}



			break;
		};


		case mass_bool: {
			int error = 0;

			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}

			QString tmp = lexem.toUpper();
			bool res;
			if (tmp == trUtf8("ДА") || tmp == trUtf8("НЕТ")) {
				res = (tmp == trUtf8("ДА"));
			} else {
				return RUN_INPUT_BOOL;
			}


			if (error > 0) {
				return error;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setBoolMassValue(res, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"=\""+vvod_str[j]+"\"",str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=\"" + lexem + "\"";
			}


			break;
		};


		case mass_charect: {
			int error = 0;

			QList<int> pos_mas = calcMassArgs(point2.perem_id,
					point2.mod_id,
					arg_line,
					Cur_Line(str).VirajList[j],
					Cur_Line(str).extraSubExpressions,
					&error);

			if (error > 0) {
				return error;
			}
			error = testMassArgs(point2, pos_mas);
			if (error > 0) {
				return error;
			}




			int code = lexem.toInt();
			QChar chr = QChar(code);

			if (error > 0) {
				return error;
			}
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setCharMassValue(chr, pos_mas[0], pos_mas[1], pos_mas[2]);
			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setReady(pos_mas[0], pos_mas[1], pos_mas[2], true);
			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString p;
				p = "\"" + QString(chr) + "\"";
				//                  if ( charPrintable ( chr ) ) //TODO ??????
				//                      p = "\""+QString ( chr ) +"\"";
				//                  else
				//                  {
				//                      if ( code == 4100 ) // Enter
				//                          p = "Enter";
				//                      if ( code == 4114 ) // Left
				//                          p = " " + QString ( QChar ( 0x2190 ) ) + " ";
				//                      if ( code == 4115 ) // Up
				//                          p = " " + QString ( QChar ( 0x2191 ) ) + " ";
				//                      if ( code == 4116 ) // Right
				//                          p = " " + QString ( QChar ( 0x2192 ) ) + " ";
				//                      if ( code == 4117 ) // Down
				//                          p = " " + QString ( QChar ( 0x2193 ) ) + " ";
				//                      if ( code == 4103 ) // Delete
				//                          p = "Delete";
				//                      if ( code == 4099 ) // Backspace
				//                          p = " " + QString ( QChar ( 0x21D0 ) ) + " ";
				//                      if ( code == 4102 ) // Insert
				//                          p = "Insert";
				//                      p += QString::fromUtf8 ( " (код " ) +vvod_str+")";
				//                  }
				QString massStr;
				if (pos_mas[0] != -1) {
					massStr += QString::number(pos_mas[0] + 1);
				}
				if (pos_mas[1] != -1) {
					massStr += "," + QString::number(pos_mas[1] + 1);
				}
				if (pos_mas[2] != -1) {
					massStr += "," + QString::number(pos_mas[2] + 1);
				}
				massStr = "[" + massStr + "]";
				//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+massStr+"="+p,str );
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + massStr + "=" + p;
			}

			break;
		};

		case kumString:


			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setStringValue(lexem);

			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"=\""+vvod_str[j]+"\"",str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=\"" + lexem + "\"";
			break;

		case charect: {
			QString ch;
			if (lexem.length() > 1) {
				return RUN_INPUT_CHAR;
			}
			ch = lexem;

			//int code = lexem.toInt();
			//QChar chr = QChar(code);
			if (lexem.isEmpty()) {
				return RUN_INPUT_CHAR;
			}
			Modules->setCharValue(point2, lexem[0]);
			//Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setCharectValue ( chr );

			if ((setup.isPoShagam()) || (setup.isDebug())) {
				QString p;
				p = "\"" + lexem[0] + "\"";
				//
				msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + p;
			}
		}
		break;

		case real: {
			bool ok;
			double value = lexem.toDouble(&ok);
			if (!ok) {
				return RUN_INPUT_FLOAT;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setFloatValue(value);


			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + QString::number(value);

		}
		break;

		case kumBoolean: {

			bool value = false;
			QString tmp = lexem.toUpper();
			qDebug() << "Lexem" << tmp;
			if ((tmp != trUtf8("ДА")) && (tmp != trUtf8("НЕТ"))) {
				return RUN_INPUT_BOOL;
			}
			if (tmp == trUtf8("ДА")) {
				value = true;
			}
			if (tmp == trUtf8("НЕТ")) {
				value = false;
			}

			Modules->module(point2.mod_id)->Values()->symb_table[point2.perem_id].value.setBoolValue(value);
			//              if ( ( setup.isPoShagam() ) || ( setup.isDebug() ) )
			//                  sendDebug ( Cur_Symbols()->symb_table[perem_id].name+"="+vvod_str[j],str );
			msgTexts << Cur_Symbols()->symb_table[perem_t_id].name + "=" + lexem;

		}
		break;

		default:
			break;

		};
	};
	if ((setup.isPoShagam()) || (setup.isDebug())) {
		QString msg = msgTexts.join(", ");
		sendDebug(msg, str, curModule);
	}
	run_stack.push_back(str + 1);

	//outPutWindow->cleanStop();
	return 0;

};


int Kumir_Run::do_use(int str)
{
	Q_UNUSED(str);
	return 0;
};



QPair<int, QString> Kumir_Run::runFromTo(int module_id, int from, int to)
{
	int err = 0;
	QString errSource;
	// if((from<0)||(from>Proga_Value.count())||(to<0)||(to>Proga_Value.count())||(from>to))return 2;//TODO raskoment
	curModule = module_id;
	if (ready) {

		if (child) {
			child = NULL;
		}
		freeStack();

		run_stack.push_back(from);
		int rsc = run_stack.count();
		running = true;
		while (run_stack.count() > 0) {
			int cur_str = pop();
			rsc = run_stack.count();
			if (cur_str == to) {
				break;
			}
			if ((setup.isPoShagam())) {
				//             if(Proga_Value[cur_str].real_line.tab)Pause(cur_str);//TODO raskoment
			};
			if (!stoped) {
				QPair<int, QString> p = do_line(cur_str);
				err = p.first;
				errSource = p.second;
			} else {
				usleep(300);
				emit finished();
				exit();
			};
			if (err != 0) {
				return QPair<int, QString> (err, errSource);
			};
		};
	};


	return QPair<int, QString> (err, errSource);;
};

uint Kumir_Run::getKeyCode()
{
	keyInputFinished = false;
	sendInputKey(false);
	while ((!keyInputFinished) && (!stoped)) {
		usleep(50);
		if (stoped) {
			return 0;
		}
	};
	uint result = keyInputCode;

	return result;
}

void Kumir_Run::sendInputKey(bool ignoreModifiers)
{
	if (isFunction) {
		parentRun->sendInputKey(ignoreModifiers);
		return;
	}
	app()->mainWindow->activateWindow();
	emit waitForKey(ignoreModifiers);
	usleep(50);
}

void Kumir_Run::inputKeyReady(uint code, bool shift, bool control, bool alt, bool meta)
{
	keyInputShift = shift;
	keyInputControl = control;
	keyInputAlt = alt;
	keyInputMeta = meta;
	keyInputCode = code;
	keyInputFinished = true;
}
void Kumir_Run::callFunct(QString name, QString arg, int client_id)
{
	QTcpSocket *tcpSocket = app()->Connections[client_id];

	if (!running) {
		tcpSocket->write(QString::fromUtf8("ZZZZZZZZZ,ERROR,Run NOT Runing" + '\n').toUtf8());
		return;
	};
	if (child) {
		child->callFunct(name, arg, client_id);
		return;
	};
	int mod_id = Modules->idByName("@main");
	int func_id, m_id;
	Modules->FuncByName(name, &m_id, &func_id);
	if (func_id < 0) {
		tcpSocket->write("ERROR,no function" + '\n');
		return;
	};
	if (mod_id != m_id) {
		tcpSocket->write("ERROR,not main module" + '\n');
		return;
	};
	symbol_table *f_arguments = new symbol_table();
	f_arguments->new_argument(kumString);
	f_arguments->symb_table[0].value.setStringValue(arg);
	//    Modules->module(mod_id)->CopyValues();
	Modules->module(mod_id)->saveValues();


	Kumir_Run *f_run = new Kumir_Run();

	f_run->initForFunc(setup,
		deep,
		Modules->module(mod_id)->Functions()->posById(func_id),
		this,
		mod_id);

	child = f_run;

	int error = f_run->runForFunc(func_id, f_arguments);

	//    Modules->module(mod_id)->localRestoreValues(
	//            Modules->module(mod_id)->Functions()->nameById(func_id)
	//            );
	Modules->module(mod_id)->restoreValues();

	if (deep > 10) {
		usleep(10);
	}
	deep--;

	if (f_run) {
		f_run->deleteLater();
	}

	if (f_arguments) {
		delete f_arguments;
	}

	deleteChild();

	child = NULL;

	QString replay = "OK\n";

	if (error > 0)
		replay = QString::fromUtf8("ZZZZZZ,ERROR,Error ") +
			QString::number(error) + "\n";

	tcpSocket->write(replay.toUtf8());
};

bool Kumir_Run::m_skipHiddenAlgorhitm = false;

QList<proga> *Kumir_Run::Cur_Proga_Value()
{
	return Modules->module(curModule)->Proga_Value();
}

proga Kumir_Run::Cur_Line(int str)
{
	if (str >= 0) {
		return Modules->module(curModule)->Proga_Value_Line(str);
	} else {
		str = -str;
		int baseStr = str / 10;
		int subStr = (str % 10) - 1;
		return Modules->module(curModule)->Proga_Value_Line(baseStr).generatedLinesAfter[subStr];
	}
}
