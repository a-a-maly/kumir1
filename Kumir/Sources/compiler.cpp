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
#include "compiler.h"
#include "text_analiz.h"
#include "kumiredit.h"
#include "application.h"

#include "mainwindow.h"
#include "programtab.h"
#include "messagesprovider.h"
#include "tools.h"
#include "pushdownautomata.h"
#include "pwrulesnormalizer.h"
#include "config.h"
#include "network.h"
#include "kumsinglemodule.h"
#include "kumrobot.h"
#include "kumdraw.h"
Compiler::Compiler(QObject *parent)
	: QObject(parent)
{
	analizer = new text_analiz();

	analizer->setModules(&Modules);

//    symbols = new symbol_table();
//    functions = new function_table();

	int system_module_id = Modules.appendInternalModule("system", NULL, QUrl::fromLocalFile("system.info.xml"));
	KumSingleModule *system = Modules.module(system_module_id);
//    system->Functions()->free(0);
	Q_CHECK_PTR(system);
	system->Functions()->addInternalFunction(trUtf8("пауза"), none, 0);
	system->Functions()->addInternalFunction(trUtf8("стоп"), none, 0);
	if (!app()->isExamMode()) {
		system->Functions()->addInternalFunction(trUtf8("НАЗНАЧИТЬ ВЫВОД"), none, 1);
		system->Functions()->setArgType(system->Functions()->size() - 1, 0, kumString);
		system->Functions()->addInternalFunction(trUtf8("НАЗНАЧИТЬ ВВОД"), none, 1);
		system->Functions()->setArgType(system->Functions()->size() - 1, 0, kumString);
	};

	system->setType(KumSystem);

	KumInstrument *stdFunc = new KumStandartFunctions(&Modules);
	int internal_id = Modules.appendInternalModule("internal", stdFunc, QUrl::fromLocalFile("internal.info.xml"));
	KumSingleModule *internal = Modules.module(internal_id);
//    main=NULL;

	int c = (((KumStandartFunctions *) stdFunc)->getFunctionList(internal->Functions(), internal->Values()));
	internal->updateDocumentation();
	system->updateDocumentation();
	oldTabNo = -1;



	//Internal Isps
//    qDebug() << "Creating robot...";
	if (QString(qApp->metaObject()->className()) == "QApplication") {
		QWidget *dummy = NULL;
		dummy = new QWidget();
//        qDebug() << "Calling robot constructor...";
		KumInstrument *robot = new KumRobot(dummy);
//        qDebug() << "Robot constructor done.";

		int robot_id = Modules.appendInternalModule("Robot", robot, QUrl::fromLocalFile("robot.info.xml"));
		KumSingleModule *robo_mod = Modules.module(robot_id);
		c = (((KumRobot *) robot)->getFunctionList(robo_mod->Functions(), robo_mod->Values()));
		robo_mod->setEnabled(false);
		robo_mod->setGraphics(true);
		robo_mod->updateDocumentation();

	}

//    qDebug() << "Creating draw...";
	if (QString(qApp->metaObject()->className()) == "QApplication") {
		KumInstrument *draw = new KumDraw();
		int draw_id = Modules.appendInternalModule("Draw", draw, QUrl::fromLocalFile("draw.info.xml"));
		KumSingleModule *draw_mod = Modules.module(draw_id);
		c = (((KumDraw *) draw)->getFunctionList(draw_mod->Functions(), draw_mod->Values()));
		draw_mod->setEnabled(false);
		draw_mod->setGraphics(true);
		draw_mod->updateDocumentation();
	}

//    qDebug() << "End creating internal modules...";

	KumInstrument *strings = new ExtStrings();
	int strings_id = Modules.appendInternalModule("Strings", strings, QUrl::fromLocalFile("strings.info.xml"));
	KumSingleModule *strings_mod = Modules.module(strings_id);
	c = (((ExtStrings *) strings)->getFunctionList(strings_mod->Functions(), strings_mod->Values()));
	strings_mod->setEnabled(false);
	strings_mod->updateDocumentation();

	KumInstrument *pfiles = new KumPFiles();
	int pfiles_id = Modules.appendInternalModule("FilesP", pfiles, QUrl::fromLocalFile("filesp.info.xml"));
	KumSingleModule *pfiles_mod = Modules.module(pfiles_id);
	c = (((KumPFiles *) pfiles)->getFunctionList(pfiles_mod->Functions(), pfiles_mod->Values()));
	pfiles_mod->setEnabled(false);
	pfiles_mod->updateDocumentation();


	Q_UNUSED(c);

//    connect ( this, SIGNAL ( finished() ), this, SLOT ( updateEditor() ) );


	pdAutomata = new PushdownAutomata(this);
	QString rulesPath;
#ifdef Q_OS_MAC
	rulesPath = QDir::cleanPath(QApplication::applicationDirPath() + "/../Resources/");
#else
	rulesPath = QApplication::applicationDirPath() + "/Kumir/PDScripts/";
#endif

	QDir rulesDir = QDir(rulesPath);
//	qDebug()<<"Loading rules:"<<rulesPath;
	QStringList rulesFileList = rulesDir.entryList(QStringList() << "*.rules");
	QBuffer normalizedRules;
	QList<int> priorities;
	for (int i = 0; i < rulesFileList.count(); i++) {
		rulesFileList[i] = rulesDir.absoluteFilePath(rulesFileList[i]);
		priorities << i;
	}

	PWRulesNormalizer::process(rulesFileList, &normalizedRules, priorities);
//    QFile debugFile(rulesPath+"/normalized.out");

	normalizedRules.open(QIODevice::ReadOnly);
	QPoint errCode = pdAutomata->loadRules(&normalizedRules);
	normalizedRules.close();
	normalizedRules.open(QIODevice::ReadOnly);
//    debugFile.open(QIODevice::WriteOnly|QIODevice::Text);
//    debugFile.write(normalizedRules.readAll());
//    debugFile.close();
	normalizedRules.close();

	if (errCode.x() > 0) {
		normalizedRules.open(QIODevice::ReadOnly);
		QTextStream ts(&normalizedRules);
		ts.setCodec("UTF-8");
		QStringList lines = ts.readAll().split("\n", QString::KeepEmptyParts);
		normalizedRules.close();
		qDebug() << "Error processing rules at line " << errCode.y() << ". Error code: " << errCode.x() << ", line: " << lines[errCode.y() - 1];
	}

}

void Compiler::loadKumirModules()
{
	QString modulesPath =
		app()->settings->
		value("Directories/StdModules", "").toString();
	scanForModules(modulesPath);

}

void Compiler::init(int tabNo)
{

	if (tabNo < 1) {
		singleEditor = true;
		currentTab = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(tabNo));
//        main_id=Modules.idByName("main");
//        if(main_id<0)
//        {
//            Modules.appendMainModule(currentTab);
//            main_id=Modules.lastModuleId();
//        };
//        main=Modules.module(main_id);
		outputWindow = app()->mainWindow->textEdit3;
		currentEditor = currentTab->editor();
		currentMargin = currentTab->margin();
		text = currentEditor->toPlainText();
		normalizedText = currentEditor->normalizedText();
	} else {
		singleEditor = false;
		currentTab = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(tabNo));
		outputWindow = app()->mainWindow->textEdit3;
		currentEditor = currentTab->editor();
		currentMargin = currentTab->margin();
		text = currentEditor->toPlainText();
	}


}


int Compiler::fileTabNo(QString FileName)
{
	//outputWindow->append("fileName:"+FileName);
	int tabs_count = app()->mainWindow->tabWidget->count();

	for (int i = 0; i < tabs_count; i++) {
		ProgramTab *cTab = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(i));
		QString cTabName = cTab->getFileName();
#ifdef WIN32
		cTabName.replace("\\", "/");
#endif
		//outputWindow->append("Tab "+QString::number(i)+": "+cTabName);

		if (cTabName == FileName) {
			return i;
		}
	};
	return -1;
}


QStringList Compiler::getIspText(QString *FileName, int *err, int use_isp_no)
{
	Q_UNUSED(use_isp_no);
	QStringList ispLines;

	QFile *ispFile = new QFile(*FileName);   //Ищем рядом

	if (!ispFile) {
		*err = 2;
		return ispLines;
	}
	if (*FileName == "") {
		*err = ISP_USE_NOFILE;
		return ispLines;
	}
	if (!ispFile->exists()) { //Ищем в стандартных
		*FileName = app()->settings->value("Directories/StdModules", QDir::currentPath()).toString() + "/" + *FileName;
		ispFile->setFileName(*FileName);
	};
	QFileInfo ifinfo(*ispFile);
	*FileName = ifinfo.absoluteFilePath();




	if (!ispFile->exists()) { //В стандартных тоже нет
		*err = ISP_USE_NOFILE;
		delete ispFile;
		return ispLines;
	};


	*FileName = ifinfo.absoluteFilePath();
	ispFile->setFileName(*FileName);
	QString DEBUG = *FileName;
	if (ispFile->open(QIODevice::ReadOnly)) {
		IspFiles.append(ispFile);
		QTextStream ts(ispFile);
		ts.setCodec("UTF-16LE");
		*err = 0;
		ispLines.clear();
		while (!ts.atEnd()) {
			ispLines.append(ts.readLine());
		};
		return ispLines;
	} else {
		*err = ISP_READ_ERROR;
		return ispLines;
	};
	*err = 2;
	return ispLines;
}

void Compiler::appendProgaValue()
{

}
int Compiler::compareLists(QList<IspLine> *now_list, QList<IspLine> *old_list)
{

	int i_count;
	bool wasUser = false;
	int err_start, err_end;
	QString filename_now = "";
	QString filename_old = "";
	QString unloadFileName = "";
	if (now_list->count() > old_list->count()) {
		i_count = old_list->count();
	} else {
		i_count = now_list->count();
	}

	for (int i = 0; i < i_count; i++) {


		int state_old = analizer->parce_use_isp(old_list->at(i).line, i, &filename_old, &err_start, &err_end);
		int state_new = analizer->parce_use_isp(now_list->at(i).line, i, &filename_now, &err_start, &err_end);
		if ((state_old == USER_USE) || (state_new == USER_USE)) {
			if (wasUser) {
				return i;
			}
			wasUser = true;
		}
		//Зплатка пользовательских исполнителей
		if ((state_old == state_new) && (filename_old == filename_now)) {
			continue;    //Ничего не поменялось
		}
		unloadFileName = filename_old;
		if (filename_old == "") {
			unloadFileName = filename_now;
		}

		if ((state_old != state_new)) {

			return i;
		}
		if ((filename_now != filename_old)) {

			return i;
		}

	}

	if (now_list->count() < old_list->count()) {


		//      int state_new=analizer->parce_use_isp ( old_list->at ( now_list->count() ).line,now_list->count(),&filename_now,&err_start,&err_end );
		if (unloadFileName == "") {
			unloadFileName = filename_now;
		}

		return i_count;
	}


	return i_count;
}

/**
 * Загрузка исполнителей
 * @return код ошибки
 */


int Compiler::scanIsps()
{

	useIspErrors.clear();

	proga Str_Data;
	//  bool reb_flag=false;
	Str_Data.line = ""; // текст выполнимого псевдо-кода
	RealLine rl;
	Str_Data.error = 0;
	int k_id = Modules.idByName(trUtf8("Кузнечик"));
	if ((k_id > 0) && (Modules.module(k_id)->isEnabled())) {
		QStringList algHeaders_k;
		for (int i = 0; i < Modules.module(k_id)->Plugin()->algList().count(); i++) {
			algHeaders_k << Modules.module(k_id)->Plugin()->algList()[i].kumirHeader;
		};
		Modules.module(k_id)->Clean();
		app()->compileAlgList(k_id, algHeaders_k);
	};

	QList<IspLine> cur_list = UsesList(); //Списоск строк исп
	QList<int> enabledNow; //Номера включенных сейчас
	for (int i = 0; i < cur_list.count(); i++) {
		IspLine ispLine = cur_list[i];
		int id = -1;
		if (cur_list[i].type == ROBO_USE) {
			id = Modules.idByName("Robot");
		}
		if (cur_list[i].type == DRAW_USE) {
			id = Modules.idByName("Draw");
		}
		if (cur_list[i].type == STRINGS_USE) {
			id = Modules.idByName("Strings");
		}
		if (cur_list[i].type == FILES_P_USE) {
			id = Modules.idByName("FilesP");
		}
		bool kuzn_zaplat = false;
		QString kuzn_init = ""; //Заплата на кузнечика
		if (cur_list[i].type == 0) {
			QString name = cur_list[i].line.mid(2, cur_list[i].line.length()).trimmed();
			//Заплата на кузнечика
			QRegExp kuznec(trUtf8("\\b(Кузнечик)\\s+(\\d+)\\s+(\\d+)$"));
			kuzn_zaplat = kuznec.exactMatch(name);

			if (kuzn_zaplat) {
				kuzn_init = name.mid(8, name.length() - 8);
				name = trUtf8("Кузнечик");
			};
//            qDebug()<<"Kuzn init"<<kuzn_init;
			//Конец заплаты
			id = Modules.idByName(name);
		}
		if (id > -1) {
			if (kuzn_zaplat) {
				Modules.module(id)->Plugin()->setParameter("Shagi", QVariant(kuzn_init));
				QStringList shagi = kuzn_init.split(" ");
				if (shagi[0].toInt() > 999 || shagi[1].toInt() > 999) {
					proga p = cur_list[i].p;
					p.error = ISP_NOT_EXISTS;
					p.err_start = 2;
					p.err_length = p.line.length() - 2;
					useIspErrors.append(p);
					continue;
				};
				QStringList algHeaders;
				for (int i = 0; i < Modules.module(id)->Plugin()->algList().count(); i++) {
					algHeaders << Modules.module(id)->Plugin()->algList()[i].kumirHeader;
				};
				Modules.module(id)->Clean();
				app()->compileAlgList(id, algHeaders);
			};
			if (!Modules.module(id)->isEnabled()) {
				if (!app()->isExamMode()) {
					Modules.module(id)->setEnabled(true);
				}

				if (Modules.module(id)->isInternal()) {
					emit kumInstrumentLoaded(id);
				}
			}

			if (Modules.module(id)->isBroken()) {
				proga p = cur_list[i].p;
				p.error = ISP_BROKEN ;
				p.err_start = 2;
				p.err_length = p.line.length() - 2;
				useIspErrors.append(p);
			}

			if (app()->isExamMode()) {

				proga p = cur_list[i].p;
				p.error = ISP_EXAM_MODE ;
				p.err_start = 2;
				p.err_length = p.line.length() - 2;
				useIspErrors.append(p);
			};
			if (enabledNow.indexOf(id) < 0) {
				enabledNow.append(id);
			} else {
				qDebug() << "Many ISPS!";
				proga p = cur_list[i].p;
				p.error = ISP_RELOAD ;
				p.err_start = 2;
				p.err_length = p.line.length() - 2;
				useIspErrors.append(p);
			};
			bool NameConflict = !Modules.testNameConflict(id);
			if (NameConflict) {
				proga p = cur_list[i].p;
				p.error = ISP_NAME_CONFLICT ;
				p.err_start = 2;
				p.err_length = p.line.length() - 2;
				useIspErrors.append(p);
//                qDebug()<<"NAME CONFLICT";
			}


		} else {
			if (cur_list[i].type == 0) {
				const QString moduleName = cur_list[i].line.mid(2).trimmed();
				int mod_id = Modules.idByName(moduleName);
				bool exists_but_broken = false;
				if (mod_id > -1) {
					exists_but_broken = Modules.module(mod_id)->isBroken();
				}
//                qDebug() << "Isp not found!";
				proga p = cur_list[i].p;
				p.error = exists_but_broken ? ISP_BROKEN : ISP_NOT_EXISTS;
				p.err_start = 2;
				p.err_length = p.line.length() - 2;
				useIspErrors.append(p);

			}
		}
	}
	//Unload unused isps
//    for ( int i=2;i<=Modules.lastModuleId();i++ ) //0,1 - System and internal functions
//    {
//        if ( ( enabledNow.indexOf ( i ) ==-1 ) && ( i!=main_id ) && ( Modules.module ( i )->Tab() !=dynamic_cast<ProgramTab*> ( app()->mainWindow->tab ( 0 ) ) ) )
//        {

//            Modules.module ( i )->setEnabled ( false );

//            if ( Modules.module ( i )->isInternal() )
//            {
//                //                        if(Modules.module(i)->instrument())
//                //                              Modules.module(i)->instrument()->showHideWindow(false);
//                emit kumInstrumentUnLoaded ( i );
//            }
//        }
//    }
//    updateMainId();
	for (int i = 2; i < Modules.count(); i++) {
		if (Modules.module(i)->isEnabled() && !enabledNow.contains(i)) {
			Modules.module(i)->setEnabled(false);
		}
	}
	return cur_list.count();
}

QList<IspLine> Compiler::UsesList()
{
	QList<IspLine> toRet;
	IspLine useLine;
	toRet.empty();
	QString FileName;
	int err_start, err_end;
	//QList<ProgaText> *Proga_Text=main->Proga_Text();
	QList< proga > uses = pdAutomata->loads();
	foreach (proga p, uses) {
		int state = analizer->parce_use_isp(p, &FileName, &err_start, &err_end);
		if (state == -10) {
			break;
		}
		useLine.line = p.line;
		useLine.type = state;
		useLine.error = 0;
		useLine.p = p;
		toRet << useLine;
	}

	return toRet;
}




//int Compiler::compileUserModules() //� азбор исполнителей пользователя
//{
//    //userModIds.clear();
//    QList<int> done;
//    done.clear();

//    QList<Isp> allModules;
//    allModules += pdAutomata->modules();
//    //  allModules += pdAutomata->brokenModules();
//    int debug1 = pdAutomata->modules().count();
//    int debug2 = pdAutomata->brokenModules().count();
//    Q_UNUSED(debug1);   Q_UNUSED(debug2);
////    bool mainFound = false;
//    for ( int i=0; i<allModules.count(); i++ ) {
//        Isp current = allModules[i];
//        if ( current.isEmpty() ) {
////            int id = Modules.idByName("main");
////            if (id==-1) {
////                id=Modules.appendModuleFromTab ( "main",main->Tab(),normalizedText[i] );
////            }
////            mainFound = true;
//            continue;
//        }
//        K_ASSERT ( current.count() >= 2 );
//        K_ASSERT ( current[0].str_type == ISP );

//        int err_start = 0;
//        int err_length = current[0].line.length();
//        QString ispName;
//        int error = analizer->parce_isp(current[0].line,NULL,NULL,0,&err_start,&err_length,"",&ispName);
//        if ( error > 0 ) {
//            if ( current[0].error == 0 ) {
//                current[0].error = error;
//                current[0].err_start = err_start;
//                current[0].err_length = err_length;
//                allModules[i] = current;
//                currentEditor->setError(current[0].real_line.lineNo,current[0].real_line.termNo,error, ErrorPosition(err_start, err_length));
//            }
//            continue;
//        }

//        int id = Modules.idByName(ispName);
//        if ( id == -1 ) {
//            id=Modules.appendModuleFromTab ( ispName,main->Tab(),normalizedText[i] );
//            // ???? DENIS
//            Modules.module(id)->Clean();
//            Modules.module(id)->setProgaValue(current);
//        }
//        else {
//            if ( ( !Modules.module ( id )->isKumIsp() ) || ( Modules.module ( id )->Tab() !=main->Tab() ) || ( done.indexOf ( id ) >-1 ) )
//            {
//                current[0].error = ISP_RELOAD;
//                current[0].err_start = 0;
//                current[0].err_length = current[0].line.length();
//                allModules[i] = current;
//                currentEditor->setError(current[0].real_line.lineNo,current[0].real_line.termNo,ISP_RELOAD, ErrorPosition(0, current[0].line.length()));
//                continue;
//            }
//            Modules.module(id)->Clean();
//            Modules.module(id)->setProgaValue(current);
//        }

//        done << id;
////        userModIds << id;
//    }



////    int k=2;
////    while ( k<=Modules.lastModuleId() )
////    {
////        if ( ( Modules.module ( k )->isKumIsp() ) && ( Modules.module ( k )->Tab() ==main->Tab() ) && ( userModIds.indexOf ( k ) ==-1 ) ) {qDebug() <<k;Modules.disableModule ( k );};
////        k++;
////    };
//    for ( int j=0;j<Modules.count();j++ )
//    {
//        if ( !Modules.module(j)->isKumIsp() ||
//             Modules.module(j)->isProtected() )
//        {
//            continue;
//        }

//        //      KumSingleModule* curModule=Modules.module ( userModIds[j] );

//        analizer->buildTablesForModule(&Modules, j);

//        analizer->USER_BASE_ALG_ID=0;


//        if ( isNeedToStop() )
//            return 0;

//        Modules.module ( j )->setEnabled ( true );

//        analizer->analizeModule(j);

//        Modules.module ( j )->removeDummyLines();

////        currentEditor->setErrorsForModules(userModIds);


//        Modules.module ( j )->setEnabled ( false );
//    }
////    k=0;
////    while ( k<=Modules.lastModuleId() )
////    {
////        if (  Modules.module ( k )->isKumIsp()  &&
////              Modules.module ( k )->Tab()==main->Tab() &&
////              userModIds.indexOf ( k ) ==-1  &&
////              Modules.module(k)->name!="main")
////        {
////            qDebug() <<k;Modules.removeModule ( k );
////            break;
////        }
////        k++;
////    }

////    for ( int i=0;i<userModIds.count();i++ )
////    {
////        if ( userModIds[i]>k ) userModIds[i]--;
////    }
////    for ( int i=0;i<done.count();i++ )
////    {
////        if ( done[i]>k ) done[i]--;
////        Modules.module ( done[i] )->setEnabled ( true );
////    }
//    Modules.Debug();
//    updateMainId();
//    return 0;

//}

void Compiler::compileUserModules(QList<QList<proga> > rawModules)
{
	// clean modules table
	QList<int> toRemove;
	for (int i = 0; i < Modules.count(); i++) {
		if (Modules.module(i)->isKumIsp() && !Modules.module(i)->isProtected()) {
//            Modules.removeModule(i);
			toRemove << i;
		}
	}
	while (toRemove.size() > 0) {
		int index = toRemove.first();
		toRemove.pop_front();
		Modules.removeModule(index);
		for (int i = 0; i < toRemove.size(); i++) {
			toRemove[i] --;
		}
	}
//    Modules.Debug();
	ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
	int userModStart = Modules.count();
	// create modules skeletons
	for (int i = 0; i < rawModules.size(); i++) {
		QString moduleName = "@main";
		if (!rawModules[i].isEmpty()) {
			proga p = rawModules[i][0];
			int err_start, err_length;
			if (p.str_type == Kumir::ModuleBegin && p.error == 0) {
				int error = analizer->parce_isp(rawModules[i][0].line, NULL, NULL, 0, &err_start, &err_length, "", &moduleName);
				if (error != 0) {
					moduleName = "@main";
					rawModules[i][0].error = error;
					rawModules[i][0].err_start = err_start;
					rawModules[i][0].err_length = err_length;
				}

			}
		}
		Modules.appendUserModule(moduleName, rawModules[i]);
		Modules.lastModule()->setEnabled(true);
		Modules.lastModule()->setTab(pt);
	}
//    Modules.Debug();
	// compile modules
	QList<int> userModIds;
	for (int i = userModStart; i < Modules.count(); i++) {
		analizer->nestedAlg = 0;
		analizer->ispName = Modules.module(i)->name;
		analizer->firstAlg = true;
		analizer->firstAlgorhitmText = "";
		analizer->buildTablesForModule(&Modules, i);
		userModIds << i;
	}
//    Modules.Debug();
	for (int i = userModStart; i < Modules.count(); i++) {
		analizer->analizeModule(i);
	}
	// check is we have non-empty main module
	int mainId = Modules.idByName("@main");
	if (mainId == -1) {
		userModIds << Modules.appendUserModule("@main", QList<proga>());
		Modules.lastModule()->setTab(pt);

	}
//    Modules.Debug();
	// update editor
	currentEditor->setErrorsForModules(userModIds);
}

void Compiler::scanForModules(const QString &modulesPath)
{
	QList<int> toRemove;
	for (int i = 0; i < Modules.count(); i++) {
		KumSingleModule *mod = Modules.module(i);
		if (mod->isKumIsp() && mod->isProtected()) {
			toRemove << i;
			break;
		}
	}
	for (int i = 0; i < toRemove.size(); i++) {
		Modules.removeModule(toRemove.first());
	}

	QList<Compiler::RawModule> allExternalModules;
	QDir modulesDir(modulesPath);
	QStringList l = modulesDir.entryList(QStringList() << "*.kum");
	foreach (const QString &s, l) {
		const QString fullPath = modulesDir.absoluteFilePath(s);
		const QUrl url = QUrl::fromLocalFile(fullPath);
		QFile f(fullPath);
		if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QList<Compiler::RawModule> ml = extractModulesFromFile(url, f.readAll());
			for (int i = 0; i < ml.size(); i++) {
				ml[i].id = Modules.appendExternalFileModule(ml[i].name, ml[i].content);
				Modules.module(ml[i].id)->setProtected(true);
				if (ml[i].errors.size() > 0) {
					Modules.module(ml[i].id)->setBroken(true);
				}
			}
			allExternalModules += ml;
			f.close();
		}
	}
	buildTablesForModules(allExternalModules);
	analizeModules(allExternalModules);
//    Modules.Debug();
}

QList<Compiler::ModuleError> Compiler::buildTablesForModules(QList<RawModule> &modules)
{
	QList<Compiler::ModuleError> errors;
	for (int i = 0; i < modules.size(); i++) {
		RawModule m = modules[i];
		if (m.errors.isEmpty()) {
			analizer->buildTablesForModule(&Modules,
				m.id,
				m.url.toString());
		} else {
			Modules.module(m.id)->setBroken(true);
		}
		if (Modules.module(m.id)->isBroken()) {
//            qDebug() << "Module is broken: " << m.url.toString();
		}
	}
	return errors;
}

QList<Compiler::ModuleError> Compiler::analizeModules(QList<RawModule> &modules)
{
	QList<Compiler::ModuleError> errors;
	for (int i = 0; i < modules.size(); i++) {
		RawModule m = modules[i];
		if (m.errors.isEmpty()) {
			Modules.module(m.id)->setEnabled(true);
			analizer->analizeModule(m.id, m.url.toString());
			Modules.module(m.id)->setEnabled(false);
		} else {
			Modules.module(m.id)->setBroken(true);
		}
		if (Modules.module(m.id)->isBroken()) {
//            qDebug() << "Module is broken: " << m.url.toString();
		}
	}
	return errors;
}

QList<Compiler::RawModule> Compiler::extractModulesFromFile(const QUrl &url, const QByteArray &data) const
{
	KumTools *t = KumTools::instance();
	const QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
	const QString kumText = codec->toUnicode(data);
	QList<ProgaText> normalizedText;
	const QStringList lines = kumText.split("\n", QString::KeepEmptyParts);
	foreach (const QString &line, lines) {
		QPoint errPos;
		int err = t->splitUserLine(line, errPos, &normalizedText);
		Q_UNUSED(err);
	}
	for (int i = 0; i < normalizedText.size(); i++) {
		normalizedText[i].Base_type = t->getStringType(normalizedText[i].Text);
	}
	PushdownAutomata *pd = app()->compiler->getPdAutomata();
	pd->init(normalizedText, url.toString(), 0);
	if (pd->process() == 1) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(" (при компиляции файла: ") + url.toString() + " )";
		qDebug() << QString::fromUtf8(">> Не хватает правил описания ошибочных ситуаций!");
		qDebug() << "===========================================================";
	} else {
		if (!pd->postProcess()) {
			qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
			qDebug() << QString::fromUtf8(" (при компиляции файла: ") + url.toString() + " )";
			qDebug() << QString::fromUtf8(">> Ошибка в одном из скриптов!");
			qDebug() << "===========================================================";
		}
	}
	const QList<proga> uses = pd->loads();
	const QList<Isp> imodules = pd->modules();
	QStringList requires;
	QList<RawModule> rmodules;
	for (int i = 0; i < imodules.size(); i++) {
		RawModule m;
		m.content = imodules[i];
		bool hasName = false;
		for (int j = 0 ; j < m.content.size(); j++) {
			if (m.content[j].error) {
				ModuleError e;
				e.from = 0;
				e.length = m.content[j].line.length();
				e.code = m.content[j].error;
				m.errors << e;
			}
			if (m.content[j].str_type == Kumir::ModuleBegin && !hasName) {
				const QString moduleName = m.content[j].line.mid(2).trimmed();
				hasName = true;
				m.name = moduleName;
			}
		}
		m.uses = requires;
		rmodules += m;
	}
	return rmodules;
}

void Compiler::run()
{

//    userModIds.clear();

	// Инициализируем анализатор
	analizer->debugMode = false;
	analizer->nestedAlg = 0;
	analizer->ispName = "@main";
	analizer->firstAlg = true;
	analizer->firstAlgorhitmText = "";


	QList<ProgaText> pt = currentEditor->completeNormalizedText();

	int debug = pt.count();

	pdAutomata->init(pt, "\'USER", currentTab);
	int errorCode = pdAutomata->process();
	if (errorCode == 1) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(">> Не хватает правил описания ошибочных ситуаций!");
		qDebug() << "===========================================================";
		return;
	}

	if (errorCode == 2) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(">> Слишком много ошибок!");
		qDebug() << "===========================================================";
		return;
	}

	if (!pdAutomata->postProcess()) {
		qDebug() << "============ PD AUTOMATA EXITED WITH ERROR ================";
		qDebug() << QString::fromUtf8(">> Ошибка в одном из скриптов!");
		qDebug() << "===========================================================";
		return;
	}

	debug = pdAutomata->mainModule().count();
	debug = pdAutomata->loads().count();
	scanIsps();



	currentEditor->setErrorsAndRanks(pdAutomata->text());

//    int zdvig=0;

//    if ( oldTabNo<1 )
//    {
//        if ( main->Proga_Text()->count() >0 ) zdvig=scanIsps (  );
//        if(zdvig==-1){
//            main->Clean();
//            zdvig=scanIsps (  );};

//    }
	// Построение таблиц
	QList< QList< proga> > rawModules = pdAutomata->modules();
	compileUserModules(rawModules);
//    main = Modules.module(Modules.idByName("main"));
//    main->setProgaValue(pdAutomata->mainModule());
//    analizer->buildTablesForModule(&Modules,main_id);
//    analizer->USER_BASE_ALG_ID=0;


//    Modules.Debug();

//    analizer->analizeModule(main_id);
//    analizer->USER_BASE_ALG_ID=0;

//    main->removeDummyLines();

//    currentEditor->setErrorsForModules(QList<int>() << main_id);

	QApplication::restoreOverrideCursor();

}



//void Compiler::updateMainId()
//{
//    main_id = Modules.idByName("main");
//}

Compiler::~Compiler()
{
}


void Compiler::updateEditor()
{

	foreach (proga p, useIspErrors) {
		RealLine rl = p.real_line;
		LineProp *tp = KumirEdit::linePropByLineNumber(currentEditor->document(), rl.lineNo);
		QList<ProgaText> pts = tp->normalizedLines;
		ProgaText pt = pts[rl.termNo];
		pt.Error = p.error;
		pt.setErrorRange(2, pt.Text.length() - 2);
		pts[rl.termNo] = pt;
		tp->normalizedLines = pts;
	}

	int i = 0;
	QString errorMessages;
	int countErrors = 0;
	for (QTextBlock block = currentEditor->document()->begin(); block != currentEditor->document()->end(); block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		Q_CHECK_PTR(lp);
		lp->error = 0;
		lp->error_count = 0;
		bool hasHiddenLines = false;
		bool hiddenHasErrors = false;
		foreach (ProgaText pt, lp->normalizedLines) {
			if (pt.Error > 0 && lp->error == 0) {
				lp->error = pt.Error;
			}
			if (pt.Error > 0) {
				lp->error_count++;
			}
			if (pt.stroka == -1) {
				hasHiddenLines = true;
				if (pt.Error) {
					hiddenHasErrors = true;
				}
			}
		}
		if (lp->normalize_error > 0) {
			lp->error = lp->normalize_error;
			lp->error_count = 1;
		}
		if (!hasHiddenLines) {
			lp->errorMask = KumTools::instance()->recalculateErrorPositions(block.text().mid(lp->indentCount * 2), lp->normalizedLines);
			QPoint indRang = KumTools::instance()->getIndentRang(lp->normalizedLines);
			lp->indentRang = indRang.x();
			lp->indentRangNext = indRang.y();
		} else {
			lp->errorMask = QVector<bool>(block.text().mid(lp->indentCount * 2).size(), hiddenHasErrors);
			lp->indentRang = 0;
			lp->indentRangNext = 0;
		}

		i++;
		if (lp->error > 0) {
			QString line;
			bool isInclude = false;
			for (int j = 0; j < lp->error_count - 1; j++) {
				line += "> ";
			}
			for (int j = 0; j < lp->normalizedLines.size(); j++) {
				if (lp->normalizedLines.at(j).stroka == -1) {
					isInclude = true;
					break;
				}
			}
			if (isInclude) {
				line = app()->messagesProvider->errorMessage(INCLUDE_CONTAINS_ERRORS) + "\n";
			} else {
				line = app()->messagesProvider->errorMessage(lp->error) + "\n";
			}
			errorMessages += line;
		} else if (block.isValid() && block.next().isValid()) {
			errorMessages += "\n";
		}
		countErrors += lp->error_count;
	}

	QStringList algNames;
	for (int m = 0; m <= Modules.lastModuleId(); m++) {
		if (!Modules.module(m)->isEnabled()) {
			continue;
		}
		function_table *curFT = Modules.module(m)->Functions();
		for (int i = 0; i < curFT->size(); i++) {
			QString curAlgName = curFT->nameById(i);
			algNames << curAlgName;
			if (curAlgName.contains(" ") && curFT->typeById(i) == kumBoolean) {
				QStringList nameParts = curAlgName.split(" ");
				for (int j = 0; j < nameParts.count(); j++) {
					QString notName;
					for (int k = 0; k < j; k++) {
						notName += nameParts[k];
					}
					notName += QString(trUtf8("не")) + " ";
					for (int k = j; k < nameParts.count(); k++) {
						notName += nameParts[k];
						if (k < (nameParts.count() - 1)) {
							notName += " ";
						}
					}
					algNames << notName;
				}
			}
		}
	}
	currentEditor->setSHAlgNames(algNames);
	currentEditor->applyIndentation();
	currentEditor->rehighlight();
	currentTab->setMarginText(errorMessages);
	qDebug() << "ED.LINES: " << currentEditor->document()->blockCount() << " M.LINES: " << currentMargin->document()->blockCount();
	emit changeCountErrors(countErrors);
	emit updateEditorFinished();
	if (analizer->firstAlgorhitmText.isEmpty()) {
		emit firstAlgorhitmAvailable(false);
	} else {
		emit firstAlgorhitmAvailable(true);
	}
}

QString Compiler::firstAlgorhitmText() const
{
	return analizer->firstAlgorhitmText;
}



QString Compiler::refactorText(const QString &source, QMap<QString, QString> dict)
{
	QRegExp rxBreaks(trUtf8(
			"\\b(цел[\\s]+таб|цел[\\s]+|целтаб)\\b|\\bцел\\b|\\b(вещ[\\s]+таб|вещ|вещтаб)\\b|\\b(лит[\\s]+таб|лит|литтаб)\\b|\\b(сим[\\s]+таб|сим|симтаб)\\b|\\b(лог[\\s]+таб|лог|логтаб)\\b|<=|>=|:=|#|\\*\\*|\\*|:|;|=|�� |<|≤|≥|>|<>|\\[|\\]|\\(|\\)|,|-|/|\\+|\\bвсе\\b|\\bалг\\b|\\bесли\\b|\\bто\\b|\\bиначе\\b|\\bнач\\b|\\bкон\\b|\\bшаг\\b|\\bнц\\b|\\bкц\\b|\\bдля\\b|\\bраз\\b|\\bпока\\b|\\bдо\\b|\\bот\\b|\\bарг\\b|\\bрез\\b|\\bввод\\b|\\bвывод\\b|\\bутв\\b|\\bдано\\b|\\bнадо\\b|\\bкц_при\\b|\\bвыбор\\b|\\bи\\b|\\bили\\b|\\bпри\\b|\\bнс\\b|;"
		));

	QString result;
	QStringList sl = source.split("\n", QString::KeepEmptyParts);

	for (int i = 0; i < sl.count(); i++) {
		QString line = sl[i];
		int pos = 0;
		int ppos = rxBreaks.indexIn(line, 0);
		if (ppos != -1) {
			QString part = line.mid(0, ppos);
			QString token = part.simplified();
			if (! token.isEmpty()) {
				if (dict.keys().contains(token)) {
					if (!KumTools::instance()->inLit(line, ppos)) {
						line.replace(0, ppos, dict[token]);
						pos = dict[token].length();
					}
				}
			}
			ppos += rxBreaks.matchedLength();
		} else {
			ppos = 0;
		}

		while (pos != -1) {
			pos = rxBreaks.indexIn(line, ppos);
			if (pos != -1) {
				QString part = line.mid(ppos, pos - ppos);
				QString token = part.simplified();
				if (! token.isEmpty()) {
					if (!KumTools::instance()->inLit(line, ppos)) {
						if (dict.keys().contains(token)) {
							line.replace(ppos, pos - ppos, dict[token] + " ");
							int shift = pos - ppos;
							shift -= dict[token].length() + 1;
							pos += shift;
						}
					}
				}
				ppos = pos + rxBreaks.matchedLength();
			} else {
				QString part = line.mid(ppos, line.length() - ppos);
				QString token = part.simplified();
				if ((!token.isEmpty()) && (! KumTools::instance()->inLit(line, ppos))) {
					if (dict.keys().contains(token)) {
						line.replace(ppos, line.length() - ppos, " " + dict[token] + " ");
					}
				}
				break;
			}
		}
		sl[i] = line;
	}
	result = sl.join("\n");
	return result;
}

QStringList Compiler::extractNames(const QString &source)
{
	QRegExp rxBreaks(trUtf8(
			"\\b(цел[\\s]+таб|цел[\\s]+|целтаб)\\b|\\bцел\\b|\\b(вещ[\\s]+таб|вещ|вещтаб)\\b|\\b(лит[\\s]+таб|лит[\\s]+|литтаб)\\b|\\bлит\\b|\\b(сим[\\s]+таб|сим|симтаб)\\b|\\b(лог[\\s]+таб|лог|логтаб)\\b|<=|>=|:=|#|\\*\\*|\\*|:|;|=|�� |<|≤|≥|>|<>|\\[|\\]|\\(|\\)|,|-|/|\\+|\\bвсе\\b|\\bалг\\b|\\bесли\\b|\\bто\\b|\\bиначе\\b|\\bнач\\b|\\bкон\\b|\\bшаг\\b|\\bнц\\b|\\bкц\\b|\\bдля\\b|\\bраз\\b|\\bпока\\b|\\bдо\\b|\\bот\\b|\\bарг\\b|\\bрез\\b|\\bввод\\b|\\bвывод\\b|\\bутв\\b|\\bдано\\b|\\bнадо\\b|\\bкц_при\\b|\\bвыбор\\b|\\bи\\b|\\bили\\b|\\bпри\\b|\\bнс\\b|;"
		));


	QStringList result;
	QStringList sl = source.split("\n", QString::KeepEmptyParts);

	for (int i = 0; i < sl.count(); i++) {
		QString line = sl[i];
		int pos = 0;
		int ppos = rxBreaks.indexIn(line, 0);
		if (ppos != -1) {
			QString token = line.mid(0, pos).simplified();
			if (! token.isEmpty()) {
				if (!KumTools::instance()->inLit(line, ppos)) {
					if (! result.contains(token))
						if (! token[0].isDigit()) {
							result << token;
						}
				}
			}
			ppos += rxBreaks.matchedLength();
		} else {
			ppos = 0;
		}

		while (pos != -1) {
			pos = rxBreaks.indexIn(line, ppos);
			if (pos != -1) {
				QString part = line.mid(ppos, pos - ppos);
				QString token = part.simplified();
				if (! token.isEmpty()) {
					if (!KumTools::instance()->inLit(line, ppos)) {
						if (! result.contains(token))
							if (! token[0].isDigit()) {
								result << token;
							}
					}
				}
				ppos = pos + rxBreaks.matchedLength();
			} else {
				QString token = line.mid(ppos, line.length() - ppos).simplified();
				if ((!token.isEmpty()) && (! KumTools::instance()->inLit(line, ppos))) {
					if (!result.contains(token))
						if (! token[0].isDigit()) {
							result << token;
						}
				}
				break;
			}
		}
	}
	return result;
}

int Compiler::compileModule(int module_id)
{
	KumSingleModule *curModule = Modules.module(module_id);
	Modules.tempDisable();
	curModule->Functions()->free(0);
	analizer->buildTablesForModule(&Modules, module_id);
	analizer->USER_BASE_ALG_ID = 0;
	Modules.module(module_id)->setEnabled(true);
	Modules.tempEnable();
	analizer->analizeModule(module_id);
	Modules.module(module_id)->setEnabled(false);
	return 0;
}

int Compiler::loadModulesInfo()
{
	if (app()->isExamMode()) {
		return 0;
	}
	QString resorcesPath;
	QString suffix;
#ifdef Q_OS_UNIX
	modulesInfoPath = QApplication::applicationDirPath() + "/Addons/";
	suffix = ".so";
#endif

#ifdef Q_OS_MAC
	modulesInfoPath = QApplication::applicationDirPath() + "/../Resources/Addons/";
	suffix = ".dylib";
#endif

#ifdef Q_OS_WIN32
	modulesInfoPath = QApplication::applicationDirPath() + "/Addons/";
	suffix = ".dll";
#endif

//    qDebug()<<"Modules info path:"<<modulesInfoPath;
	QDir infoDir = QDir(modulesInfoPath);

//    qDebug()<<"From load mdules info"<<modules()->lastModuleId();
	int plugin_count = modules()->scanPlugins(infoDir, "*" + suffix);
//    qDebug()<<"Plugin count:"<<plugin_count;

	//app()->mainWindow->createFriendMenu();

	return 0;
}

int Compiler::parceModulesInfoFile(QIODevice *source)
{
	QTextStream ts(source);
	ts.setCodec("UTF-8");
	QStringList lines = ts.readAll().split("\n");
//    qDebug()<<"Lines Count:"<<lines.count();
	int pos = 0;

	//Выделяем имя
	while ((pos < lines.count()) && (lines[pos].isEmpty() || lines[pos].isNull())) {
		pos++;
	}
	if (lines[pos].indexOf("[NAME]") < 0) {
		qDebug() << "No name section in isp file!";
		return -1;
	};
	QString name = "";
	while ((pos < lines.count()) && (lines[pos].indexOf("[/NAME]") < 0)) {
		name += lines[pos];
		pos++;
	};
	name += lines[pos];
	pos++;
	name.replace("[/NAME]", "");
	name.replace("[NAME]", "");
	name.replace("\n", " ");
	int tn_start = 0, tn_len = 0;
	if (KumTools::instance()->test_name(name, tn_start, tn_len) > 0) {
		qDebug() << "Bad ISP name!";
		return -2;
	};
//    qDebug()<<"Isp name "<<name;

	//Выделяем имя бинарник
	while ((pos < lines.count()) && (lines[pos].isEmpty() || lines[pos].isNull())) {
		pos++;
	}
	if (lines[pos].indexOf("[BINARY]") < 0) {
		qDebug() << "No [BINARY] section in isp file!";
		return -1;
	};
	QString binary = "";
	while ((pos < lines.count()) && (lines[pos].indexOf("[/BINARY]") < 0)) {
		binary += lines[pos];
		pos++;
	};
	binary += lines[pos];
	pos++;
	binary.replace("[/BINARY]", "");
	binary.replace("[BINARY]", "");
	binary.replace("\n", "");
	QFile binFile(modulesInfoPath + binary);
	if (!binFile.exists()) {
		qDebug() << "Module " << name << " binary not found!";
		return -2;
	};



	//Выделяем URL
	while ((pos < lines.count()) && (lines[pos].isEmpty() || lines[pos].isNull())) {
		pos++;
	}
	if (lines[pos].indexOf("[URL]") < 0) {
		qDebug() << "No [URL] section in isp file!";
		return -1;
	};
	QString url = "";
	while ((pos < lines.count()) && (lines[pos].indexOf("[/URL]") < 0)) {
		url += lines[pos];
		pos++;
	};
	url += lines[pos];
	pos++;
	url.replace("[/URL]", "");
	url.replace("[URL]", "");
	url.replace("\n", "");
	if (url.isEmpty()) {
		qDebug() << "Module " << name << " url not found!";
		return -3;
	};
	QStringList urlSplit = url.split(":");
	if (urlSplit.count() < 2) {
		return -3;
	}
	bool ok;
	int port = urlSplit[1].toInt(&ok);
	Q_UNUSED(port);
	if (!ok) {
		return -3;
	}

	//Выделяем список алгоритмов
	while ((pos < lines.count()) && (lines[pos].isEmpty() || lines[pos].isNull())) {
		pos++;
	}
	if (lines[pos].indexOf("[ALG_DESC]") < 0) {
		qDebug() << "No [ALG_DESC] section in isp file!" << lines[pos];
		return -1;
	};
	QStringList algs;
	while ((pos < lines.count()) && (lines[pos].indexOf("[/ALG_DESC]") < 0)) {
		algs.append(lines[pos]);
		pos++;
	};
	algs.append(lines[pos]);
	pos++;
	algs[algs.count() - 1] = algs[algs.count() - 1].replace("[/ALG_DESC]", "");
	algs[algs.count() - 1] = algs[algs.count() - 1].replace("[ALG_DESC]", "");
	algs[algs.count() - 1] = algs[algs.count() - 1].replace("\n", "");


	//Выделяем Icon
	while ((pos < lines.count()) && (lines[pos].isEmpty() || lines[pos].isNull())) {
		pos++;
	}
	if (lines[pos].indexOf("[ICON]") < 0) {
		qDebug() << "No [ICON] section in isp file!";
		return -1;
	};
	QString icon = "";
	while ((pos < lines.count()) && (lines[pos].indexOf("[/ICON]") < 0)) {
		icon += lines[pos];
		pos++;
	};
	icon += lines[pos];
	pos++;
	icon.replace("[/ICON]", "");
	icon.replace("[ICON]", "");
	icon.replace("\n", "");
	qDebug() << "Icon" << icon;
	if (icon.isEmpty()) {
		qDebug() << "Module " << modulesInfoPath + name << " icon not found!";
	};



	return 0;
};

void Compiler::ispReady()
{
	//emit ispStateChanged();
	KNPConnection *extModule = qobject_cast<KNPConnection *>(sender());
//    qDebug()<<"Isp "<<extModule->name <<" Ready!";
	int  module_id = modules()->idByName(extModule->name);

	modules()->module(module_id)->setConnection(extModule);


};
void Compiler::compileAlgList(int module_id, QStringList algList)
{
	if (!analizer) {
		qDebug() << "Analizer not ready!";
		return;
	};
	//analizer->modules=modules();
	analizer->parceFuncList(module_id, algList);
};

bool Compiler::isEmpty(int moduleNo)
{
	KumSingleModule *module = Modules.module(moduleNo);
	QList<proga> *data = module->Proga_Value();
	return data->size() == 0;
}

proga Compiler::compiledLine(int str, int module_id)
{
	return Modules.module(module_id)->Proga_Value_Line(str);
}

int Compiler::StartPos()
{
	int result = mainModule()->initialIP();
//    int realLine = mainModule()->Proga_Value()->at(result).real_line.lineNo;
//    for (int i=0; i<Modules.count(); i++) {
//        KumSingleModule * module = Modules.module(i);
//        if (module->isKumIsp() && module->Proga_Value()->count()>0) {
//            int rl = module->Proga_Value()->at(0).real_line.lineNo;
//            int error
//        }
//    }
	return result;
}

KumSingleModule *Compiler::mainModule()
{
	int mainId = Modules.idByName("@main");
	Q_ASSERT(mainId >= 0);
	KumSingleModule *mainMod = Modules.module(mainId);
	Q_CHECK_PTR(mainMod);
	return mainMod;
}
