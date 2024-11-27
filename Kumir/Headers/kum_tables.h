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
#ifndef KUM_TABLES_H
#define KUM_TABLES_H
#include "kassert.h"
//#include "Addons/plugin_interface.h"
#include "enums.h"
#include <QStringList>
#include <QPoint>

class KumInstrument;
class KumValueStackElem;
class ProgramTab;
class KNPConnection;
class KumSingleModule;

struct PeremPrt {
	int mod_id;
	int perem_id;
};

struct Argument {
	PeremType type;
	int dimension;
	bool res;
	bool argres;
	QString name;
	int linkId;
};

struct f_table {
	f_table()
	{
		name = "";
		doc = "";
		str = -1;
		proga_str = -1;
		ends = -1;
		//      for ( int i=0; i<MAX_ARG; i++ )
		//      {
		//          perems[i] = none;
		//          razm[i] = 0;
		//          is_res[i] = false;
		//          is_argRes[i] = false;
		//          arg_perem_id[i] = -1;
		//      }
		type = none;
		disabled = false;
		badalg = false;
		teacher = false;
		is_dummy = false;
	}
	QString name;
	//QString isp;
	QString doc;
	int str;
	int proga_str;
	int ends;
	bool is_dummy;
	//  PeremType perems[MAX_ARG];
	//  int razm[MAX_ARG]; // Размерность аргументор-массивов
	//  bool is_res[MAX_ARG];
	//  bool is_argRes[MAX_ARG];
	//  int nperem;
	//  int arg_perem_id[MAX_ARG];
	//  QString arg_name[MAX_ARG];
	QList<Argument> l_arguments;
	PeremType type;
	bool disabled;
	bool badalg;

	bool runStepInto;
	int outboundAlgorhitmIP;
	bool teacher;
};

/**
 * Таблица функций
 */
class function_table
{
public:
	function_table();

	int size() const;
	bool isBroken(int id) const;
	void setBroken(int id, bool v);
	bool isDummy(int id) const;
	QString docString(int id) const;
	void setDocString(int id, const QString &v);
	bool isRunStepInto(int id) const;

	void addFunction(const QString &name, int str);
	void addInternalFunction(const QString &name, PeremType type, uint nperem);    //добавление встроенной функции
	int append(const QString &name, PeremType returnType, int declPos);
	int appendDummy(const QString &name);
	int appendArg(int funcId, PeremType argType,
		int argDim, int isRes, int isArgRes,
		int argPeremId);
	int argPeremId(int funcId, int argId) const;
	QString nameByPos(int pos) const;   //Возвращает имя алгоритма по номеру строки - global если такого алгоритма нет
	int idByPos(int pos) const;
	int posById(int id) const;
	int bytePosById(int id) const;
	int idByName(const QString &name) const;    //возвращает номер вункции в таблице или -1 если функции нет
	QString nameById(int id) const;

	bool isRes(int id, int arg_id) const;
	void setRes(int id, int arg_id, bool v);
	bool isArgRes(int id, int arg_id) const;

	void free(int base);    //отчистка
	int declPosById(int id) const;    //возвращает номер строки описания функции
	void setEndPos(int id, int pos);    //устанавливает конец алгоритма
	PeremType typeById(int id) const;    //возвращает тип функции
	int argCountById(int id) const;

	PeremType argTypeById(int id, int argNo) const;
	int argDimById(int id, int argNo) const;
	void setArgDim(int id, int argNo, int dim);
	void setArgType(int funcNo, int argId, PeremType argType);
	void setRunStepInto(int funcId, bool v);
	bool isTeacher(int id) const;
	void setTeacher(int id, bool flag);

	QList<f_table> l_table;
private:
};


/**
 * Значение переменной
 */


class perem_value
{
public:
	perem_value();
	perem_value(int id,
		PeremType type,
		int size0_start, int size0_end,
		int size1_start, int size1_end,
		int size2_start, int size2_end,
		int razm = 0);
	void init(int id,
		PeremType type,
		int size0_start, int size0_end,
		int size1_start, int size1_end,
		int size2_start, int size2_end,
		int razm = 0);
public:
	~perem_value();


public:
	int getValueAsInt();
public:
	bool getValueAsBool();
public:
	double getValueAsFloat();
public:
	QString getValueAsString();
	QVariant getValueAsVariant() const;
	QChar getValueAsCharect();
public:
	void  setIntegerValue(int value);
public:
	void  setFloatValue(double value);
public:
	void  setBoolValue(bool value);
public:
	void  setStringValue(QString value);
	void setCharectValue(QChar value);
public:
	int  initMass(int id);
public:
	void  setIntMassValue(int value, int x, int y, int z);
public:
	void  setStringMassValue(QString value, int x, int y, int z);
public:
	void  setBoolMassValue(bool value, int x, int y, int z);
public:
	void  setFloatMassValue(double value, int x, int y, int z);
public:
	void  setCharMassValue(QChar value, int x, int y, int z);
	void setVariantMassValue(const QVariant &value, int x, int y, int z);
public:
	int   getIntMassValue(int x, int y, int z);
	QVariant getVariantMassValue(int x, int y, int z) const;
	QVariant getVariantMassValue2aaa(int col, int row, int slice) const;
public:
	QString  getStringMassValue(int x, int y, int z);
public:
	double   getFloatMassValue(int x, int y, int z);
public:
	int   getBoolMassValue(int x, int y, int z);
public:
	QChar   getCharMassValue(int x, int y, int z);
	bool isReady(int x, int y, int z);
	void setReady(int x, int y, int z, bool value);
	void cleanMass();

	//
	//public:int  setFloatValue(double value);

public:
	QVector<QVariant> data;
	// bool *massReady;
	//                QVector<bool> massReady;
public:
	PeremType p_type;
	bool has_read;
private:
	bool initd;
private:
	int size_x;
private:
	int size_y;
private:
	int size_z;
	int start_x;
	int start_y;
	int start_z;
	int razmer;

};



/**
 * Таблица переменных
 */
class symbol_table
{

	//struct s_val
	//{
	//  int int_value;
	//  QString string_value;
	//  double double_value;
	//  int *mass[1024];
	//};

	/**
	     * Данные переменной
	     */
public:
	struct s_table {
		s_table()
		{
			name = "";
			type = none;
			alg = "";
//			value = NULL;
			size0_start = 0;
			size0_end = 0;
			size1_start = 0;
			size1_end = 0;
			size2_start = 0;
			size2_end = 0;
			str = -1;
			razm = 0;
			is_const = false;
			ready = false;
			used = false;
			is_Res = false;
			is_ArgRes = false;
			is_Arg = false;
			is_FunctionReturnValue = false;
			constantValue = QString();
			//int pointer = -1;

		}
		QString name;
		PeremType type;
		QString alg;
		//QString isp;
		QString constantValue;
		perem_value value;
		int size0_start;
		int size0_end;
		int size1_start;
		int size1_end;
		int size2_start;
		int size2_end;
		int str;
		int razm;
		bool is_const;
		bool ready;
		bool used;
		bool is_Res;
		bool is_ArgRes;
		bool is_Arg; // является арг, рез или аргрез-аргументом функции
		//int pointer;
		PeremPrt pointer;
		bool is_FunctionReturnValue;
	};
public:
	int prepareConstValue(int id);
public:
	symbol_table();
	~symbol_table();
public:
	int add_symbol(QString name, int str, PeremType type, QString alg, bool p_Const);
public:
	symbol_table *deepCopy();
public:
	int valueCopy(int id, s_table value);
public:
	int localRestore(QString alg, symbol_table *from);
	//  public: int use_symbol ( QString name,int str,QString alg,s_val value );
public:
	int new_symbol(int id);

public:
	int new_argument(PeremType type);

public:
	double asFloat(int id);
public:
	int asInt(int id);

public:
	int init(uint id);
public:
	int copyMassInt(uint from, uint to);
public:
	int copyMassIntArg(uint from, uint to, symbol_table *arg_table);

public:
	int copyMassString(uint from, uint to);
public:
	int copyMassStringArg(uint from, uint to, symbol_table *arg_table);


public:
	int copyMassCharect(uint from, uint to);
public:
	int copyMassCharectArg(uint from, uint to, symbol_table *arg_table);

public:
	int copyMassBool(uint from, uint to);
public:
	int copyMassBoolArg(uint from, uint to, symbol_table *arg_table);


public:
	int copyMassReal(uint from, uint to);
public:
	int copyMassRealArg(uint from, uint to, symbol_table *arg_table);

	void setTableValues(int id, const symbol_table *fromTable, int fromId);


	void setConstantValue(int id, const QString &cval);

public:
	int drop(int id);
public:
	bool isReady(uint id)
	{
		return symb_table[id].ready;
	};

	bool isReady(uint id, int x, int y, int z);


public:
	bool isUsed(uint id);

public:
	bool isRes(uint id)
	{
		return symb_table[id].is_Res;
	};

public:
	bool isArgRes(uint id)
	{
		return symb_table[id].is_ArgRes;
	};

public:
	PeremPrt pointTo(uint id)
	{
		return symb_table[id].pointer;
	};

public:
	void setUsed(uint id, bool use);
public:
	void setPointer(uint id, int module, int point)
	{

		//          if(id<0)
		//              {
		//              qDebug()<< module<<point;
		//              return;
		//
		//              };
		K_ASSERT(point > -1);
		K_ASSERT(module > -1);
		symb_table[id].pointer.mod_id = module;
		symb_table[id].pointer.perem_id = point;
	};

public:
	PeremPrt getPointer(uint id)
	{
		return symb_table[id].pointer;
	};
public:
	int getRazmById(uint id)
	{
		return symb_table[id].razm;
	};

public:
	int free(int base_symb);
public:
	bool isConst(int id);
public:
	void setConst(int id, bool isconst);

public:
	int inTable(const QString &name, const QString &alg);    //возвращает ID переменной
public:
	int getStrByID(int id);    //возвращает номер строки описания переменной
public:
	PeremType getTypeByID(int id);
public:
	QList<s_table> symb_table;
//public: int count;


public:
	QString getValueAsString(int id)
	{
		return symb_table[id].value.getValueAsString();
	};

public:
	QChar getValueAsCharect(int id)
	{
		return symb_table[id].value.getValueAsCharect();
	};

public:
	int getValueAsInt(int id)
	{
		return symb_table[id].value.getValueAsInt();
	};

public:
	bool getValueAsBool(int id)
	{
		return symb_table[id].value.getValueAsBool();
	};


public:
	double getValueAsFloat(int id)
	{
		return symb_table[id].value.getValueAsFloat();
	};

public:
	void resetInitFlag(const QString &alg_name)
	{

		for (int i = 0; i < symb_table.size(); i++) {
			if ((symb_table[i].alg == alg_name) && (!symb_table[i].is_const)) {
				symb_table[i].ready = false;
			}
		}
	}


private:
	//isp_table* isp_Table_ptr;
};




struct RealLine {
	RealLine()
	{
		fileName = "";
		tab = NULL;
		lineNo = 0;
	}
	QString fileName;
	ProgramTab *tab;
	int lineNo;
	int termNo;
};


class proga
{
public:

	proga();

	/**Нормализованная строка
	     */
	QString line;

	/**Тип строки
	     */
	//  int str_type;
	Kumir::InstructionType str_type;

	/**Счетчик проходов по строке
	     */
	int counter;

	/**Строка прыжка
	     */
	int else_pos;

	/** Ссылка на реальную строку в тексте
	     */
	RealLine real_line;
	int real_line_begin;
	int error;
	int err_start;
	int err_length;
	int forceJump;



	/**Список выражений в строке
	     */
	QList<QStringList> VirajList;
	/**Типы выражений
	     */
	QList<PeremType> VirajTypes;
	bool is_break;
	QList<proga> generatedLinesAfter;
	QStringList extraSubExpressions;
	QString source;

	int curError(int pos)
	{
		Q_UNUSED(pos);
		return error;
	};
};

class ProgaText
{
public:

	ProgaText(const ProgaText &cp)
	{
		source = cp.source;
		Text = cp.Text;
		stroka = cp.stroka;
		x_offset = cp.x_offset;
		Base_type = cp.Base_type;
		Error = cp.Error;
		P = cp.P;
		L = cp.L;
		File = cp.File;
		modify = cp.modify;
		position = cp.position;
		y_offset = cp.y_offset;
		errstart = cp.errstart;
		errlen = cp.errlen;
		indentRank = cp.indentRank;

	}
	ProgaText &operator= (const ProgaText &cp)
	{
		source = cp.source;
		Text = cp.Text;
		stroka = cp.stroka;
		position = cp.position;
		x_offset = cp.x_offset;
		Base_type = cp.Base_type;
		Error = cp.Error;
		P = cp.P;
		L = cp.L;
		File = cp.File;
		modify = cp.modify;
		y_offset = cp.y_offset;
		errstart = cp.errstart;
		errlen = cp.errlen;
		indentRank = cp.indentRank;
		return (*this);
	}
	bool equals(const ProgaText &other)
	{
		return Text == other.Text &&
			x_offset == other.x_offset &&
			Base_type == other.Base_type &&
			Error == other.Error &&
			errstart == other.errstart &&
			indentRank == other.indentRank &&
			errlen == other.errlen;
	}
	ProgaText();
	QString text()
	{
		return Text;
	};

	int error()
	{
		return Error;
	};
	void setError(int error)
	{
		Error = error;
	};
	int base_type()
	{
		return Base_type;
	};
	inline void setErrorRange(int start, int len)
	{
		errstart = start;
		errlen = len;
	}
	inline void getErrorRange(int *start, int *len)
	{
		*start = errstart;
		*len = errlen;
	}

	QString Text;
	int stroka; // номер реальной строки
	int position; // номер инструкции в строке
	int x_offset; // кол-во ксимволов до её начала
	// не используются
	int y_offset;
	Kumir::InstructionType Base_type;
	int Error;
	QString File;
	bool modify;
	QList<int> P, L;
	int errstart, errlen;
	QPoint indentRank;
	QString source;
};

#endif
