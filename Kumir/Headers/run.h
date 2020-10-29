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
#ifndef _KUMIR_RUN_H
#define _KUMIR_RUN_H
#include "int_proga.h"
#include "error.h" // NEW V.Y. 01.09
#include "messagesprovider.h"

struct RunContext {
	inline RunContext()
	{
		IP = 0;
	}
	int IP;
};

struct LoopContext {
	/** Loop begin instruction point */
	int IP_begin;
	/** Loop value or variable ID in symbol table */
	int loopValue;
	/** Loop start value */
	int fromValue;
	/** Loop end value */
	int toValue;
	/** Loop step value */
	int stepValue;
	/** Variable ID */
	int variableID;
};

class Kumir_Run : public QThread
{
	Q_OBJECT
public:
	Kumir_Run(QObject *parent = 0);

	void init(RunSetup Setup, int ModuleId);
	int initForFunc(RunSetup Setup, int deep, int func_str, Kumir_Run *parent_Run, int moduleId);

	void StartProgaram();

	//protected:
	void run(); //запуск потока
	int runForFunc(int func_id, symbol_table    *arguments); //запуск для польз функций
	void setMode(RunMode run_mode);//!Не рекомендуется вызывать из других потоков
	void setSelfMode(RunMode run_mode)// Установить режим выполнения только себе (режим выполнения подвызовов не изменится)
	{
		setup.setMode(run_mode);
	};
public:
	inline void setSkipHiddenAlgorhitm(bool v)
	{
		m_skipHiddenAlgorhitm = v;
	}
	uint getKeyCode();
	bool isRunning() const;
	void stop_run();

private:
	void freeStack();
private:
	int pop();

public slots:
	void inputReady(QStringList input);
	void inputKeyReady(uint code, bool shift, bool control, bool alt, bool meta);
	inline void evaluateDelay(int msec)
	{
		i_delay = msec;
	}

protected:
	inline void deleteChild()
	{
		if (child) {
			child->deleteChild();
			child->deleteLater();
			child = NULL;
		}
	}

protected slots:
	void StopRun();
	void ispError(QString text);
	void SyncMessage();
	void ispReturn(QString value);
	void Continue();
	void DoConcaAlg();
	void setFuncDebugMode(int);
	void callFunct(QString name, QString arg, int client_id);

signals:
	void finished();
	void textMessage(QString text);
	void setOutFile(QString fileName);
	void setInFile(QString fileName);

	void runError(int error, int stroka, int module_id, const QString &source);
	void debugMessage(QString text, int stroka, int module_id);
	void waitForInput(QString format);
	void pause(int str, int module_id);
	void GraphIspFunc(QString name, QList<KumValueStackElem> *arguments, int *err);
	void Step();
	void waitForKey(bool ignoreModifiers);
	void sendMessage(const QString &txt, int);
	void SwitchIspMode(int mode);
	void sendNetwMessage(QString message);
	void sendServReplay(QString  text, int client_id);
	void resetPlugin(int module_id);
	void updateValues();

private:
	Kumir_Run *parentRun;
	bool isFunction;
	int i_delay;
	void sendText(QString text);
	void sendChangeIn(QString text);
	void sendChangeOut(QString text);
	void sendDebug(QString text, int stroka, int module_id);
	void sendError(int error, int stroka, int module_id, const QString &);
	void sendInput(QString format);
	void sendInputKey(bool ignoreModifiers);
	void sendStep();
	void Pause(int str, int module_id);
	void waitForSync();
	void sendGraphics(QString name, QList<KumValueStackElem> *arguments, int *err);
	void sendNetwork(int module_id, QString name, QList<KumValueStackElem> *arguments, int *err);
	void switchIspMode(int);
	void sendValues();//USES SYNC signal

	int initIsps(void);//инициализация исполнителей
	QPair<int, QString> do_line(int str);
	int do_vivod(int str);
	int getOperPrior(QChar oper);
	KumValueStackElem calc_simple_universal(const QStringList &simple_viraj,
		const QStringList &extraExpressions,
		const QString &expression,
		int *error,
		KumValueStackElem *newValue = NULL,
		QString *debugString = NULL);
	int run_circle(int str);
	int initPerem(int str, int module);
	int calcMassSize(QStringList viraj_ids, PeremPrt mass, int str); //Работает с symbol_table;
	int testMassArgs(PeremPrt mass_id, QList<int> args);
	int compareMass(symbol_table *src_table, int src_id,
		symbol_table *dst_table, int dst_id);

	int test_if(int str);
	int do_utv(int str);
	int calc_prisv(int str);

	QList<int> calcMassArgs(int mass_id, int mod_id,
		const QString &mass_args,
		const QStringList &simple_viraj,
		const QStringList &extraSubexprList,
		int *err, QString *debugString = NULL);
	QList<int> calcMassPos(const QString &mass_viraj,
		const QStringList &simple_viraj,
		const QStringList &extraSubexprList,
		int *err);

	int getMassIntData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraExprList,
		int *err);
	double getMassFloatData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraExprList,
		int *err);
	bool getMassBoolData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraExprList,
		int *err);
	QChar getMassCharData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraExprList,
		int *err);
	QString getMassStringData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraExprList,
		int *err);

	void setMassIntData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraSubexprList,
		int *err, int value, QString *debugString, QString name);
	void setMassFloatData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraSubexprList,
		int *err, double value, QString *debugString, QString name);
	void setMassBoolData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraSubexprList,
		int *err, bool value, QString *debugString, QString name);
	void setMassCharData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraSubexprList,
		int *err, const QChar &value, QString *debugString, QString name);
	void setMassStringData(PeremPrt mass_id,
		const QString &mass_args,
		const QStringList &viraj,
		const QStringList &extraSubexprList,
		int *err, const QString &value, QString *debugString, QString name);

	int prepare_arguments(const QString &name,
		QString vizov,
		const QStringList &simple_viraj,
		const QStringList &extraSubexprList,
		symbol_table *args
		, QList<KumValueStackElem> *arguments);
	int run_f(const QString &name,
		const QString &vizov,
		const QStringList &simple_viraj,
		const QStringList &extraSubexprList,
		int *err);
	int setArguments(symbol_table *arguments, int func_id);
	void leadOut(void);

	int do_break(int str);
	int do_funct(int str);
	int do_vibor(int str);
	int do_pri(int str);
	int do_nado(int str);
	int do_dano(int str);
	int do_vvod(int str);
	int do_fvvod(int str);
	int do_fvivod(int str);
	int do_use(int str);

	QPair<int, QString> runFromTo(int module_id, int from, int to);
	bool charPrintable(QChar chr)
	{
		if ((chr.unicode() > 32) && (chr.unicode() < 255)) {
			return true;
		} else {
			return false;
		}
	};

	QList<proga> *Cur_Proga_Value();

	proga  Cur_Line(int str);
	function_table  *Cur_Functions();

	symbol_table  *Cur_Symbols();

	QStack<LoopContext> loopStack;


	KumValueStackElem calc_simple_universal_in_Module(int module_id,
		const QStringList &simple_viraj,
		const QStringList &extraExpressions,
		const QString &expression,
		int *error)
	{
		int old_mod_id = curModule;
		curModule = module_id;
		KumValueStackElem result = calc_simple_universal(simple_viraj, extraExpressions, expression, error);
		curModule = old_mod_id;
		return result;
	};

	bool ready;
	//QList<proga> Proga_Value;
	KumModules *Modules;
	//function_table *functions;
	//symbol_table *symbols;
	QTextEdit *output;
	QTextEdit *debugW;
	uint mode;

	RunSetup setup;
	MainWindow *main;
	//isp_table *ispolniteli;
	Kumir_Run *child;
	QList<int> run_stack;
	int ISP_BASE;
	int start_str;
	int status;
	bool stoped;
	bool syncFlag, vvodFlag, pauseFlag;
	QStringList vvod_str;
	QList<bool> Vibor_Flags;
	QList<int> vibor_stack;
	void getFuncParam(QString Vizov, int *func_id, int *module_id)
	{
		*func_id = -1;
		int emp_pos = Vizov.indexOf('&');
		int break_pos = Vizov.indexOf('|');
		if (emp_pos == -1) {
			emp_pos = Vizov.length();
		}
		// выделяем блок до &
		QString temp;
		temp = Vizov.mid(0, break_pos);
		if (temp[0] == '@') {
			temp = temp.mid(1);
		}
		// номер функции в таблице функций
		bool ok;
		*func_id = temp.toInt(&ok);
		if (!ok) {
			qDebug("getFuncParam:cant get func_id");
		};
		temp = Vizov.mid(break_pos + 1, emp_pos - 1 - break_pos);
		*module_id = temp.toInt(&ok);

		if (!ok) {
			qDebug("getFuncParam:cant get module_id");
		};
	};
	qint64 OldTime()
	{
		if (isFunction) {
			return parentRun->OldTime();
		} else {
			return oldTime;
		}

	}

	void SetOldTime(qint64 ot)
	{
		if (isFunction) {
			parentRun->SetOldTime(ot);
		} else {
			oldTime = ot;
		}

	}

	bool running;// выполняется ли программа
	QString Value;//for network modules
	int GlubCircle;
	int CircleBegPosition[MAX_GLUB_CIRCLE];
	int CircleOt[MAX_GLUB_CIRCLE];
	int CircleDo[MAX_GLUB_CIRCLE];
	int CirclePerem[MAX_GLUB_CIRCLE];
	int CircleStep[MAX_GLUB_CIRCLE];
	bool BreakFlag;
	int deep;
	int curModule;
	QDateTime clock_m;
	qint64 oldTime;
	bool keyInputFinished;
	uint keyInputCode;
	bool keyInputShift;
	bool keyInputControl;
	bool keyInputAlt;
	bool keyInputMeta;
	int ispErrorCode;
	static bool m_skipHiddenAlgorhitm;
};
#endif
