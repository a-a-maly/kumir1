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
#ifndef TEXT_ANALIZ_H
#define TEXT_ANALIZ_H

#include <QVariant>

#include "int_proga.h"
#include "compiler.h"
#include "tools.h"
#include "config.h"


enum StructType {
	// Types od structures
	s_none,
	constant,
	simple_var,
	array,
	array_elem,
	string_elem,
	substring,
	func,
	complex
};


struct ExpressionElement {
	bool isOper;
	PeremType type;
	QChar oper;
};

class text_analiz
{
public:
	enum ArgModifer {
		arg,
		res,
		argres
	};
	text_analiz();

	QString ispName;
	bool debugMode;
	bool firstAlg;
	int nestedAlg;
	QString algName;
	QString firstAlgorhitmText;
	int parceFuncList(int module_id, QStringList funcList);
	void buildTablesForModule(KumModules *modules, int module_id, QString fileName = QString("\'USER"));
	void analizeModule(int module_id, QString fileName = QString("\'USER"));

	static QVariant createConstValue(const QString &str, const PeremType pt);


	PeremType parceExpression(QString viraj,
		function_table *table,
		symbol_table *symbols,
		QString algorithm,
		int *err,
		QStringList *,
		StructType *virStruct,
		ErrorPosition *err_pos,
		QStringList &extraSubExpressions,
		bool isLeft = false);

	PeremType parceExpression2(QString viraj,
		function_table *table,
		symbol_table *symbols,
		QString algorithm,
		int *err,
		QStringList *parsedViraj,
		ErrorPosition *err_pos,
		QStringList &extraSubExpressions,
		bool isLeft = false);
	//  {
	//      StructType virStruct;
	//      return NewGetVirajType(viraj,table,symbols,algorithm,err,parsedViraj,&virStruct,err_pos,isLeft,root);
	//  }


	PeremType resType(int *err, PeremType type1, PeremType type2, QChar oper);

	QString dropSpace(QString viraj, QList<int> &P, QList<int> &L);//,QList<int> *skobka,QList<int> *sk_type);

	int parceAlgorhitmHeader(proga &pv, int str, symbol_table *symbols, function_table *functions);
	int parceVariables(QString source,
		int ispId,
		QString algName,
		ArgModifer mod,
		int str,
		symbol_table *symbols,
		function_table *functions,
		ErrorPosition &err_pos,
		QList<int> &ids,
		QList<QStringList> &exprList,
		QStringList &extraSubExpressions,
		bool isArg,
		QList<proga> &pv
	);
	int parceVariablesDeclaration(proga &pv, int str, QString alg, symbol_table *symbols, function_table *table);
	int parceAssignment(proga &pv, function_table *table, symbol_table *symbols, int str);
	//  int new_parce_prisv(QString prisv,function_table *table,symbol_table *symbols,int str,int *err_start,int *err_end);
	int parceLoopBegin(proga &pv, symbol_table *symbols, function_table *functions, int str);
	int parce_if(proga &pv, symbol_table *symbols,
		function_table *functions, int str);
	int parceOutput(proga &pv, function_table *table, symbol_table *symbols, int str);
	int parceFileOutput(proga &pv, function_table *table, symbol_table *symbols, int str);
	int parce_utv(proga &pv, function_table *table, symbol_table *symbols, int str);

	int parce_kc_pri(proga &pv, function_table *table, symbol_table *symbols, int str);

	int parce_dano(proga &pv, function_table *table, symbol_table *symbols, int str);
	int parce_nado(proga &pv, function_table *table, symbol_table *symbols, int str);

	int parceInput(proga &pv, function_table *table, symbol_table *symbols, int str);

	int parceFileInput(proga &pv, function_table *table, symbol_table *symbols, int str);

	int parce_vibor(QString utv, function_table *table, symbol_table *symbols, int str);
	int parce_pri(proga &pv, function_table *table, symbol_table *symbols, int str);
	int parce_isp(QString instr, function_table *table, symbol_table *symbols, int str,              int *err_start, int *err_end, QString file, QString *returnName = NULL);
	int parce_endisp(QString instr);
	int parce_doc(QString instr, function_table *functions, symbol_table *symbols, int str);

	int parce_use_isp(QString instr, int str, QString *fileName, int *err_start, int *err_end);
	int parce_use_isp(proga pv, QString *fileName, int *err_start, int *err_end);

	PeremType retUnMassType(PeremType inType);
	QStringList splitMassArgs(QString mass_args, QList<int> &starts, int *err);


	PeremType getObjType(
		QString                 work_block, // In: Obj descriptor
		int                     *err,       // Out:Error code (0 - OK)
		function_table          *table,     // In: Function table
		symbol_table            *symbols,   // In: Symbol table
		QList<PeremType>    type_list,  // In: Types of subexpressions
		// MR-New
		QList<StructType>,          // In: Descriptors of subexpressions
		QStringList exprList,
		StructType              *StrNew,
		int &err_start,
		int &err_end, bool &include_bounds_in_error
	);

	int Err_start;
	int Err_length;
	QTextEdit *outPutWindow;
	int USER_BASE_ALG_ID;
	void setModules(KumModules *modules_table)
	{
		modules = modules_table;
	};

private:
	QList<QRegExp> Omega;
	QList<proga> m_hiddenAlgorhitm;
	QString Delim;
	QString DelimNOT;
	QString Breaks;
	QRegExp operators;
	QRegExp operatorsNOT;
	QRegExp brks;
	int m_denyVariableDeclaration;
	void generateEntryPoint(const QStringList &argGroups,
		const QList<ArgModifer> &argMods,
		symbol_table *symbols,
		function_table *functions,
		const QString &callName,
		const PeremType &callReturnType,
		RealLine realLine);
	void generateEntryPoint2(const QStringList &argGroups,
		const QList<ArgModifer> &argMods,
		symbol_table *symbols,
		function_table *functions,
		const QString &callName,
		const PeremType &callReturnType,
		RealLine realLine);
	int parceAlgorhitmCall(proga &pv, function_table *table, symbol_table *symbols, int str);
	int parceConstant(const QString &constant, const PeremType pt, int &maxDim, QString &value, int &errPos, int &errLen);

	int shift;
	int prev_str;
	int L(QString s);
	PeremType G(QString s,
		int *err,
		function_table *table,
		symbol_table *symbols,
		const QList<PeremType> &type_list,
		const QList<StructType> &struct_list,
		const QStringList &expr_list,
		StructType *strNew,
		int *err_start,
		int *err_len,
		bool &include_bounds_in_error);

	void F(const QString &s, QStringList *EL, int *err, int *err_pos, int *err_pos_end, QList< QList<int> > &P, QList< QList<int> > &L);
	bool isRecursive(QString block, QStringList exprList, int moduleNo, int funcNo, bool firstCall, int &exprNo);
	QString splitLogicalExpressions(const QString &s, QStringList &exprList);

public:

	int parce_to(QString instr);
	int parce_inache(QString instr);
	int parce_vse(QString instr);
	int parceLoopEnd(const QString &instr);
	int parce_kon(QString instr);
	int parceAlgorhitmBegin(QString instr);
	int parce_exit(QString instr);



public:



private:
	int m_firstAlgorhitmIP;
	KumModules *modules;
	int moduleId;
};


#endif
