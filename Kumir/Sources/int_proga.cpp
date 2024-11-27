/****************************************************************************
**
** Copyright (C) 2004-2008 IMPB RAS. All rights reserved.
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

#include <math.h>
#include "int_proga.h"
#include "kum_tables.h"
#include "error.h"
#include "tools.h"
#include "integeroverflowchecker.h"
#include "doubleoverflowchecker.h"
#include "config.h"
#include "application.h"
#include <QFile>
#include <QDir>


ProgaText::ProgaText()
{

	position = 0;

	stroka = -1;
	x_offset = 0;
	y_offset = 0;
	Base_type = Kumir::Empty;
	Error = 0;
	//  err_start = 0;
	//  err_end = 0;
	//  err_start_incl =0;
	//  err_end_incl=0;
	errstart = 0;
	errlen = 0;
	modify = true;
}

proga::proga()
{
	str_type = Kumir::Empty;
	counter = 0;
	else_pos = 0;
	//  real_line = 0;
	real_line.lineNo = -1;
	real_line.termNo = 0;
	real_line.tab = NULL;
	real_line_begin = 0;
	error = 0;
	VirajList.clear();
	VirajTypes.clear();
	is_break = false;
	err_start = 0;
	err_length = 0;
	forceJump = -1;
}

//
/*===================================== class function_table ==================================*/

function_table::function_table()
{

}

int function_table::size() const
{
	return l_table.size();
}




/**
 * Поиск функции в таблице
 * @param name Имя функции
 * @return Номер в таблице (-1 если отсутствует)
 */
int function_table::idByName(const QString &name) const
{
	for (int i = 0; i < l_table.size(); i++) {
		if (l_table[i].name == name) {
			return i;
		}
	}
	return -1;
}

PeremType function_table::typeById(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].type;
}

int function_table::argCountById(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].l_arguments.size();
}

/**
 * Определение строки с описанием функции
 * @param id номер функции
 * @return номер строки (-1 если функции нет)
 */
int function_table::posById(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].str;
}

/**
 * Уустановка конца для функции
 * @param id Номер функции
 * @param pos Строка конца
 * @return 0 если все нормально.
 */

void function_table::setTeacher(int id, bool flag)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	l_table[id].teacher = flag;
}

bool function_table::isTeacher(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].teacher;
}

void function_table::setEndPos(int id, int pos)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	l_table[id].ends = pos;
}

void function_table::setRes(int id, int arg_id, bool v)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < l_table[id].l_arguments.size());
	l_table[id].l_arguments[arg_id].res = v;

}

void function_table::setArgDim(int id, int arg_id, int dim)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < l_table[id].l_arguments.size());
	l_table[id].l_arguments[arg_id].dimension = dim;
}

bool function_table::isRunStepInto(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].runStepInto;
}

void function_table::setRunStepInto(int id, bool v)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	l_table[id].runStepInto = v;
}

int function_table::bytePosById(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].proga_str;
}

PeremType function_table::argTypeById(int id, int arg_id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	QList<Argument> arguments = l_table[id].l_arguments;
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < arguments.size());
	return arguments[arg_id].type;
}

bool function_table::isArgRes(int id, int arg_id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	QList<Argument> arguments = l_table[id].l_arguments;
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < arguments.size());
	return arguments[arg_id].argres;
}

int function_table::argPeremId(int id, int arg_id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	QList<Argument> arguments = l_table[id].l_arguments;
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < arguments.size());
	return arguments[arg_id].linkId;
}

QString function_table::docString(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].doc;
}

void function_table::setDocString(int id, const QString &v)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	l_table[id].doc = v;
}


void function_table::setBroken(int id, bool v)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	l_table[id].badalg = v;
}

bool function_table::isBroken(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].badalg;
}

int function_table::argDimById(int id, int arg_id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < l_table[id].l_arguments.size());
	return l_table[id].l_arguments[arg_id].dimension;
}

void function_table::addInternalFunction(const QString &name, PeremType type, uint nperem)
{
	f_table f;
	f.name = name;
	f.type = type;
	f.str = -1;
	f.ends = -1;
	f.disabled = false;
	f.proga_str = 0;
	for (uint i = 0; i < nperem; i++) {
		Argument arg;
		arg.argres = arg.res = false;
		arg.type = none;
		arg.linkId = -1;
		f.l_arguments << arg;
	}
	l_table << f;
}


void function_table::addFunction(const QString &name, int str)
{
	f_table f;
	f.name = name;
	f.str = str;
	f.disabled = false;
	l_table << f;
}


void function_table::free(int base)
{
	int table_size = l_table.size();
	while (table_size > base) {
		l_table.pop_back();
		table_size = l_table.size();
	}
}

QString function_table::nameByPos(int pos) const
{
	if (pos == -1) {
		return "";
	}
	for (int i = l_table.size() - 1; i >= 0; i--) {
//        if((func_table[i].str<0)&&((func_table[i].str>33000)))
//        {
//            return "function_table::getfunctByPos func_table ERR";
//        };
//        Q_ASSERT ( l_table[i].str>=0 );
		if ((l_table[i].str >= 0) && (l_table[i].str <= pos) && (l_table[i].ends >= pos)) {
			return l_table[i].name;
		}
	}
	return "global";
}

int function_table::idByPos(int pos) const
{
	for (int i = l_table.size() - 1; i >= 0; i--) {
//        Q_ASSERT ( l_table[i].str>=0 );
		if ((l_table[i].str >= 0) && (l_table[i].str <= pos) && (l_table[i].ends >= pos)) {
			return i;
		}
	}
	return -1;
}

int function_table::appendArg(int id, PeremType argType,
	int argDim, int isRes, int isArgRes,
	int argPeremId)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Argument arg;
	arg.type = argType;
	arg.dimension = argDim;
	arg.res = isRes;
	arg.argres = isArgRes;
	arg.linkId = argPeremId;
	l_table[id].l_arguments << arg;
	return l_table[id].l_arguments.size() - 1;
}

/*===================================== class symbol_table ==================================*/

symbol_table::symbol_table()
{


//  for (int i=0; i<512; i++) {
//      symb_table[i].value = new perem_value(i, none,
//                                            0, 0,
//                                              0, 0,
//                                             0, 0,
//                                             0);
//   }
//   return;
}

symbol_table::~symbol_table()
{
//   for (int i=0; i<512; i++) {
//       delete symb_table[i].value;
//    }
}

int symbol_table::getStrByID(int id)
{
	if (id < 0) {
		qDebug("symbols:getStrByID: id<0!!!");
		return -1;
	};
	int str = symb_table[id].str;
//    if (id<=count) return  str; else return 99;
	return str;
};


/**
 * Поиск переменной в таблице
 * @param name Имя переменной
 * @param alg Алгоритм для которого описанна
 * @return Номер переменной (-1 если нет)
 */
int symbol_table::inTable(const QString &name, const QString &alg)
{
	QString znach_const = QString::fromUtf8("знач");
	if (app()->currentLanguage() == "english") {
		znach_const = "value";
	}
	QString searchName = name;
	if (name == znach_const) {
		searchName = alg;
	}

	//qDebug("+++++++++++Name:"+name+" Alg:"+alg);
	for (int i = 0; i < symb_table.size(); i++) {
		bool comp = symb_table[i].name == searchName;
		bool comp2 = symb_table[i].alg == alg;
		//if(alg!="global"){
		// if(comp)qDebug("ALG"+ symb_table[i].alg + "OK:"+name+" "+symb_table[i].name);else qDebug("ALG"+ symb_table[i].alg + "BAD:"+name+" "+symb_table[i].name);};

		if ((comp) && (comp2)) {
			return i;
		}
		// if (comp2!=0){return 7;};
	};

	return -1;
};


/**
 * Тип переменной
 * @param id Номер
 * @return Тип переменной (none в случае если нет)
 */
PeremType symbol_table::getTypeByID(int id)
{
	if (id < 0) {
		return none;
	}

//    if(id>count)return none;
	return symb_table[id].type;
};


int symbol_table::init(uint id)
{

	if ((symb_table[id].type == mass_string) || (symb_table[id].type == mass_integer) ||
		(symb_table[id].type == mass_real) || (symb_table[id].type == mass_bool) ||
		(symb_table[id].type == mass_charect)) {


	};
	return 0;
};

int symbol_table::drop(int id)
{
//    count=id;
	while (symb_table.size() > id) {
		symb_table.pop_back();
	}
	return 0;
};


int symbol_table::valueCopy(int id, s_table value)
{
	symb_table[id].type = value.type;
	symb_table[id].name = value.name;
	symb_table[id].alg = value.alg;

	symb_table[id].size0_start = value.size0_start;
	symb_table[id].size0_end = value.size0_end;
	symb_table[id].size1_start = value.size1_start;
	symb_table[id].size1_end = value.size1_end;
	symb_table[id].size2_start = value.size2_start;
	symb_table[id].size2_end = value.size2_end;
	symb_table[id].str = value.str;
	symb_table[id].razm = value.razm;
	symb_table[id].is_const = value.is_const;
	symb_table[id].ready = value.ready;
	symb_table[id].is_Res = value.is_Res;
	symb_table[id].pointer = value.pointer;
	symb_table[id].value = value.value;
//    if(value.type==integer)
//    {
//        symb_table[id].ready=false;
//        new_symbol(id);
//        symb_table[id].value.setIntegerValue(value.value.getValueAsInt());
//    };


//    if(value.type==kumString)
//    {
//        symb_table[id].ready=false;
//        new_symbol(id);
//        symb_table[id].value.setStringValue(value.value.getValueAsString());
//    };

//    if(value.type==real)
//    {
//        symb_table[id].ready=false;
//        new_symbol(id);
//        symb_table[id].value.setFloatValue(value.value.getValueAsFloat());
//    };

//    if(value.type==kumBoolean)
//    {
//        symb_table[id].ready=false;
//        new_symbol(id);
//        symb_table[id].value.setBoolValue(value.value.getValueAsBool());
//    };

//    if(value.type==charect)
//    {
//        symb_table[id].ready=false;
//        new_symbol(id);
//        symb_table[id].value.setCharectValue(value.value.getValueAsCharect());
//    };

	return 0;
};


symbol_table *symbol_table::deepCopy()
{
	symbol_table *temp_tab = new symbol_table();
	//  temp_tab=*this;

//    temp_tab->count = count;
	for (int i = 0; i < symb_table.size(); i++) {
		s_table ss = symb_table[i];
		temp_tab->symb_table.append(ss);
//        temp_tab->symb_table.append(ss);
//        temp_tab->symb_table[i].used = symb_table[i].used;
//        temp_tab->symb_table[i].is_ArgRes = symb_table[i].is_ArgRes;
//        temp_tab->symb_table[i].is_Res = symb_table[i].is_Res;

//        if (isReady(i)){
//            temp_tab->valueCopy(i,symb_table[i]);
//        };
	}
	return temp_tab;
};


int symbol_table::localRestore(QString alg, symbol_table *from)
{
//    if(count!=from->count)return 99;
	for (int i = 0; i < symb_table.size(); i++) {

		if (((from->symb_table[i].alg) == alg) && (from->symb_table[i].name != "!KUM_CONST") && (!from->symb_table[i].is_ArgRes) && (!from->symb_table[i].is_Res)) { //09-01-Mordol

			valueCopy(i, from->symb_table[i]);
			symb_table[i].used = from->symb_table[i].used;
		}
	};
	return 0;

};


/**
 * Добавление переменной в таблицу.
 * @param name Имя
 * @param str Номер строки описания
 * @param type Тип
 * @param alg Алгоритм для которого описанна
 * @param p_Const Константа или нет
 * @return Код ошибки (0 если все хорошо)
 */
int symbol_table::add_symbol(QString name, int str, PeremType type, QString alg, bool p_Const = false)
{

	int intbl = inTable(name, alg);
	if ((intbl != -1) && (name[0] != '!')) {
		return 5;    // 04.09 ERR MESG to text_analiz.cpp
	}
	//intbl=inTable(name,"global",ispId);
	//if((intbl!=-1)&&(name[0]!='!')) return 5; // 04.09 ERR MESG to text_analiz.cpp
//    count++; // TODO вставить проверку
	s_table ss;
	ss.name = name;
	ss.str = str;
	ss.type = type;
	ss.alg = alg;
	ss.is_const = p_Const;
	ss.ready = false;
	ss.is_Res = false;
	ss.is_ArgRes = false;
	ss.pointer.perem_id = -1;
	ss.pointer.mod_id = -1;
	ss.used = p_Const;
	ss.size0_start = 0;
	ss.size0_end = -1;
	ss.size1_start = 0;
	ss.size1_end = -1;
	ss.size2_start = 0;
	ss.size2_end = -1;
	symb_table.append(ss);

	// MR 06.09
	//if (type==string)symb_table[count].s_value.string_value="";
	//if (type==integer)symb_table[count].s_value.int_value=0;
	//if (type==real)symb_table[count].s_value.double_value=0;


	return 0;

}

bool symbol_table::isConst(int id)
{
	if (id == -1) {
		return false;
	}
	return symb_table[id].is_const;
};

void symbol_table::setConst(int id, bool isconst)
{
	symb_table[id].is_const = isconst;
};

int symbol_table::free(int base_symb)
{
//    for(int i=base_symb+1;i<=count;i++)
//    {
//        if(symb_table[i].ready)
//            if(symb_table[i].value)
//            {
//            delete symb_table[i].value;
//            symb_table[i].ready=false;
//    };
//    };

//    count=base_symb;
	while (symb_table.size() > base_symb) {
		symb_table.pop_back();
	}
	return 0;
};

/**
 * Инициализация переменной
 * @param id Номер переменной
 * @return Код ошибки
 */
int symbol_table::new_symbol(int id)
{
//    if(id>count) return SET_PARAM_ERR;
	if (!symb_table[id].ready) {
//        perem_value *data=new perem_value(id,symb_table[id].type,symb_table[id].size0_start,symb_table[id].size0_end,
//                                          symb_table[id].size1_start,symb_table[id].size1_end,
//                                          symb_table[id].size2_start,symb_table[id].size2_end,symb_table[id].razm);
//        symb_table[id].value=data;
		symb_table[id].value.init(id, symb_table[id].type,
			symb_table[id].size0_start, symb_table[id].size0_end,
			symb_table[id].size1_start, symb_table[id].size1_end,
			symb_table[id].size2_start, symb_table[id].size2_end,
			symb_table[id].razm);
		symb_table[id].ready = true;
	};
	return 0;
}

int symbol_table::new_argument(PeremType type)
{
	s_table ss;
	ss.name = "!KUM_ARGUMENT";
	ss.str = 0;
	ss.type = type;
	ss.alg = "argumen";
	ss.is_const = true;
	ss.ready = false;
	ss.is_Res = false;
	ss.is_ArgRes = false;
	ss.pointer.perem_id = -1;
	ss.pointer.mod_id = -1;
	ss.used = true;


	if (!((type == mass_bool) || (type == mass_real) || (type == mass_integer)
			|| (type == mass_string) || (type == mass_charect))) {
		if (!ss.ready) {
			perem_value data = perem_value(symb_table.size(), ss.type, ss.size0_start, ss.size0_end,
					ss.size1_start, ss.size1_end,
					ss.size2_start, ss.size2_end, ss.razm);
			ss.value = data;
			ss.ready = true;
		}
	}
	symb_table.append(ss);
	return 0;
}




double symbol_table::asFloat(int id)
{
	if (symb_table[id].ready) {
		return symb_table[id].value.getValueAsFloat();
	}
	return 99;
};


int symbol_table::asInt(int id)
{
	if (symb_table[id].ready) {
		return symb_table[id].value.getValueAsInt();
	}
	return 99;
};

int symbol_table::copyMassInt(uint from, uint to)
{
	if (!isReady(to)) {
		return 2;
	}
	symb_table[to].value.data = symb_table[from].value.data;
	return 0;
};




int symbol_table::copyMassReal(uint from, uint to)
{

	if (!isReady(to)) {
		return 2;
	}
	symb_table[to].value.data = symb_table[from].value.data;
	return 0;
};




int symbol_table::copyMassString(uint from, uint to)
{
	if (!isReady(to)) {
		return 2;
	}
	symb_table[to].value.data = symb_table[from].value.data;
	return 0;
};

int symbol_table::copyMassCharect(uint from, uint to)
{
	if (!isReady(to)) {
		return 2;
	}
	symb_table[to].value.data = symb_table[from].value.data;
	return 0;
}
void symbol_table::setUsed(uint id, bool use)
{

	//  if((id==39))
	//  {
	//  qDebug("ID 39 set.");
	//  };
	symb_table[id].used = use;

	if (!use &&
		(
			(symb_table[id].type == mass_bool) ||
			(symb_table[id].type == mass_integer) ||
			(symb_table[id].type == mass_real) ||
			(symb_table[id].type == mass_string) ||
			(symb_table[id].type == mass_charect))) {

		if (!use) {
			symb_table[id].value.cleanMass();
		}
	};
};
int symbol_table::copyMassBool(uint from, uint to)
{
	if (!isReady(to)) {
		return 2;
	}
	symb_table[to].value.data = symb_table[from].value.data;
	return 0;
}


int symbol_table::copyMassBoolArg(uint from, uint to, symbol_table *args)
{
	if (symb_table[from].type != mass_bool) {
		return GOT_RECURSIVE_TABLE;
	}

	int razm = symb_table[from].razm;
	args->symb_table[to].razm = razm;

#if 0
	int x, y, z, size;
	x = symb_table[from].size0_end - symb_table[from].size0_start + 1;
	y = symb_table[from].size1_end - symb_table[from].size1_start + 1;
	z = symb_table[from].size2_end - symb_table[from].size2_start + 1;
	switch (razm) {
	case 1:
		size = x;
		break;
	case 2:
		size = x * y;
		break;
	case 3:
		size = x * y * z;
		break;
	}
#endif

	args->symb_table[to].size0_end = symb_table[from].size0_end;
	args->symb_table[to].size0_start = symb_table[from].size0_start;

	args->symb_table[to].size1_end = symb_table[from].size1_end;
	args->symb_table[to].size1_start = symb_table[from].size1_start;

	args->symb_table[to].size2_end = symb_table[from].size2_end;
	args->symb_table[to].size2_start = symb_table[from].size2_start;
	args->new_symbol(to);

	if (!args->isReady(to)) {
		return 2;
	}

	if (!isReady(from)) {
		qDebug("copyMassBoolArg::From Not Ready");
		return 2;
	}

	args->symb_table[to].value.data = symb_table[from].value.data;

	return 0;
}





int symbol_table::copyMassIntArg(uint from, uint to, symbol_table *args)
{
	if (symb_table[from].type != mass_integer) {
		return GOT_RECURSIVE_TABLE;
	}

	int razm = symb_table[from].razm;
	args->symb_table[to].razm = razm;

#if 0
	int x, y, z, size;
	x = symb_table[from].size0_end - symb_table[from].size0_start + 1;
	y = symb_table[from].size1_end - symb_table[from].size1_start + 1;
	z = symb_table[from].size2_end - symb_table[from].size2_start + 1;
	switch (razm) {
	case 1:
		size = x;
		break;
	case 2:
		size = x * y;
		break;
	case 3:
		size = x * y * z;
		break;
	}
#endif

	args->symb_table[to].size0_end = symb_table[from].size0_end;
	args->symb_table[to].size0_start = symb_table[from].size0_start;

	args->symb_table[to].size1_end = symb_table[from].size1_end;
	args->symb_table[to].size1_start = symb_table[from].size1_start;

	args->symb_table[to].size2_end = symb_table[from].size2_end;
	args->symb_table[to].size2_start = symb_table[from].size2_start;
	args->new_symbol(to);

	if (!args->isReady(to)) {
		return 2;
	}
	if (!isReady(from)) {
		qDebug("copyMassIntlArg::From Not Ready");
		return 2;
	}
	args->symb_table[to].value.data = symb_table[from].value.data;

	return 0;
}

int symbol_table::copyMassRealArg(uint from, uint to, symbol_table *args)
{

	if (symb_table[from].type != mass_real) {
		return GOT_RECURSIVE_TABLE;
	}

	int razm = symb_table[from].razm;
	args->symb_table[to].razm = razm;

#if 0
	int x, y, z, size;
	x = symb_table[from].size0_end - symb_table[from].size0_start + 1;
	y = symb_table[from].size1_end - symb_table[from].size1_start + 1;
	z = symb_table[from].size2_end - symb_table[from].size2_start + 1;
	switch (razm) {
	case 1:
		size = x;
		break;
	case 2:
		size = x * y;
		break;
	case 3:
		size = x * y * z;
		break;

	}
#endif

	args->symb_table[to].size0_end = symb_table[from].size0_end;
	args->symb_table[to].size0_start = symb_table[from].size0_start;

	args->symb_table[to].size1_end = symb_table[from].size1_end;
	args->symb_table[to].size1_start = symb_table[from].size1_start;

	args->symb_table[to].size2_end = symb_table[from].size2_end;
	args->symb_table[to].size2_start = symb_table[from].size2_start;
	args->new_symbol(to);

	if (!args->isReady(to)) {
		return 2;
	}
	if (!isReady(from)) {
		qDebug("copyMassRealArg::From Not Ready");
		return 2;
	}
	args->symb_table[to].value.data = symb_table[from].value.data;
	return 0;
}

void symbol_table::setTableValues(int id, const symbol_table *fromTable, int fromId)
{
	int dim = symb_table[id].razm;
	if (dim == 1) {
		for (int x = fromTable->symb_table[fromId].size0_start; x <= fromTable->symb_table[fromId].size0_end; x++) {
			int sourceX = x - fromTable->symb_table[fromId].size0_start;
			int targetX = x - symb_table[id].size0_start;
			QVariant val = fromTable->symb_table[fromId].value.getVariantMassValue(sourceX, 0, 0);
			symb_table[id].value.setVariantMassValue(val, targetX, 0, 0);
		}
	} else if (dim == 2) {
		for (int y = fromTable->symb_table[fromId].size1_start; y <= fromTable->symb_table[fromId].size1_end; y++) {
			int sourceY = y - fromTable->symb_table[fromId].size1_start;
			int targetY = y - symb_table[id].size1_start;
			for (int x = fromTable->symb_table[fromId].size0_start; x <= fromTable->symb_table[fromId].size0_end; x++) {
				int sourceX = x - fromTable->symb_table[fromId].size0_start;
				int targetX = x - symb_table[id].size0_start;
				QVariant val = fromTable->symb_table[fromId].value.getVariantMassValue(sourceX, sourceY, 0);
				symb_table[id].value.setVariantMassValue(val, targetX, targetY, 0);
			}
		}
	} else if (dim == 3) {
		for (int z = fromTable->symb_table[fromId].size2_start; z <= fromTable->symb_table[fromId].size2_end; z++) {
			int sourceZ = z - fromTable->symb_table[fromId].size2_start;
			int targetZ = z - symb_table[id].size2_start;
			for (int y = fromTable->symb_table[fromId].size1_start; y <= fromTable->symb_table[fromId].size1_end; y++) {
				int sourceY = y - fromTable->symb_table[fromId].size1_start;
				int targetY = y - symb_table[id].size1_start;
				for (int x = fromTable->symb_table[fromId].size0_start; x <= fromTable->symb_table[fromId].size0_end; x++) {
					int sourceX = x - fromTable->symb_table[fromId].size0_start;
					int targetX = x - symb_table[id].size0_start;
					QVariant val = fromTable->symb_table[fromId].value.getVariantMassValue(sourceX, sourceY, sourceZ);
					symb_table[id].value.setVariantMassValue(val, targetX, targetY, targetZ);
				}
			}
		}
	}
	symb_table[id].ready = true;
}


int symbol_table::copyMassStringArg(uint from, uint to, symbol_table *args)
{
	if (symb_table[from].type != mass_string) {
		return GOT_RECURSIVE_TABLE;
	}

	int razm = symb_table[from].razm;
	args->symb_table[to].razm = razm;
#if 0
	int x, y, z, size;
	x = symb_table[from].size0_end - symb_table[from].size0_start + 1;
	y = symb_table[from].size1_end - symb_table[from].size1_start + 1;
	z = symb_table[from].size2_end - symb_table[from].size2_start + 1;
	switch (razm) {
	case 1:
		size = x;
		break;
	case 2:
		size = x * y;
		break;
	case 3:
		size = x * y * z;
		break;
	}
#endif
	args->symb_table[to].size0_end = symb_table[from].size0_end;
	args->symb_table[to].size0_start = symb_table[from].size0_start;

	args->symb_table[to].size1_end = symb_table[from].size1_end;
	args->symb_table[to].size1_start = symb_table[from].size1_start;

	args->symb_table[to].size2_end = symb_table[from].size2_end;
	args->symb_table[to].size2_start = symb_table[from].size2_start;
	args->new_symbol(to);

	if (!args->isReady(to)) {
		return 2;
	}

	args->symb_table[to].value.data = symb_table[from].value.data;
	return 0;
}

int symbol_table::copyMassCharectArg(uint from, uint to, symbol_table *args)
{
	int razm = symb_table[from].razm;
	args->symb_table[to].razm = razm;
#if 0
	int x, y, z, size;
	x = symb_table[from].size0_end - symb_table[from].size0_start + 1;
	y = symb_table[from].size1_end - symb_table[from].size1_start + 1;
	z = symb_table[from].size2_end - symb_table[from].size2_start + 1;
	switch (razm) {
	case 1:
		size = x;
		break;
	case 2:
		size = x * y;
		break;
	case 3:
		size = x * y * z;
		break;
	}
#endif
	args->symb_table[to].size0_end = symb_table[from].size0_end;
	args->symb_table[to].size0_start = symb_table[from].size0_start;

	args->symb_table[to].size1_end = symb_table[from].size1_end;
	args->symb_table[to].size1_start = symb_table[from].size1_start;

	args->symb_table[to].size2_end = symb_table[from].size2_end;
	args->symb_table[to].size2_start = symb_table[from].size2_start;
	args->new_symbol(to);

	if (!args->isReady(to)) {
		return 2;
	}
	args->symb_table[to].value.data = symb_table[from].value.data;

	return 0;
}

//===================================================
//===================================================
// CLASS perem_value

perem_value::perem_value()
{

}

perem_value::perem_value(int id, PeremType type, int size0_start, int size0_end, int size1_start, int size1_end, int size2_start, int size2_end, int razm)
{
	init(id, type, size0_start, size0_end, size1_start, size1_end, size2_start, size2_end, razm);
}

void perem_value::init(int id, PeremType type, int size0_start, int size0_end, int size1_start, int size1_end, int size2_start, int size2_end, int razm)
{
	has_read = false;
	start_x = size0_start;
	start_y = size1_start;
	start_z = size2_start;
	Q_UNUSED(id);
	p_type = type;
	if (type == integer || type == real || type == kumBoolean || type == kumString || type == charect) {
		data = QVector<QVariant>(1);
	}

	if ((type == mass_string) || (type == mass_integer) ||
		(type == mass_real) || (type == mass_bool) ||
		(type == mass_charect)) {
		int x = 0;
		int y = 0;
		int z = 0;
		int size = 0;
		x = size0_end - size0_start + 1;
		y = size1_end - size1_start + 1;
		z = size2_end - size2_start + 1;
		size_x = x;
		size_y = y;
		size_z = z;


		switch (razm) {
		case 1:
			size = x;
			break;
		case 2:
			size = x * y;
			break;
		case 3:
			size = x * y * z;
			break;

		};
		razmer = razm;

		data = QVector<QVariant>(size);


		initd = true;
	} else {
		size_x = 0;
		size_y = 0;
		size_z = 0;
		initd = true;
	};

};

perem_value::~perem_value()
{
	data.clear();
};



int perem_value::getIntMassValue(int x, int y, int z)
{

	int razm = razmer;
	int offset;



	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};
	return data[offset].toInt();
}

QVariant perem_value::getVariantMassValue(int x, int y, int z) const
{
	int razm = razmer;
	int offset;

	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};
	if (offset > data.count() - 1) {
		qDebug() << "Internal ERR:" << offset << data.count();
		return QVariant();
	};
	return data[offset];
}

QVariant perem_value::getVariantMassValue2aaa(int col, int row, int slice) const
{
	int dim = razmer;
	int offset = 0;
	int offsetCol = 0;
	int offsetRow = 0;
	int offsetSlice = 0;

	if (dim == 1) {
		offsetCol = col;
		offset = offsetCol;
	} else if (dim == 2) {
		offsetCol = col;
		offsetRow = row;
		offset = size_x * offsetRow + offsetCol;
	} else if (dim == 3) {
		offsetCol = col;
		offsetRow = row;
		offsetSlice = slice;
		offset = size_x * size_y * offsetCol + size_x * offsetRow + offsetSlice;
	}
	return data[offset];
}

QChar perem_value::getCharMassValue(int x, int y, int z)
{
	int razm = razmer, offset;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	return   data[offset].toChar();
};


int perem_value::getBoolMassValue(int x, int y, int z)
{
	int razm = razmer, offset;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};


	bool v = data[offset].toBool();
	if (v) {
		return 1;
	} else {
		return 0;
	}
};


double perem_value::getFloatMassValue(int x, int y, int z)
{
	int razm = razmer, offset;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	if (p_type == mass_real) {
		return   data[offset].toDouble();
	} else if (p_type == mass_integer) {
		int resultInt = data[offset].toInt();
		return (double)resultInt;
	} else {
		return 0;
	}
};


QString perem_value::getStringMassValue(int x, int y, int z)
{

	int razm = razmer, offset;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	return data[offset].toString();
};


void perem_value::setCharMassValue(QChar value, int x, int y, int z)
{
	has_read = false;
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	data[offset] = value;
};

void perem_value::setVariantMassValue(const QVariant &value, int x, int y, int z)
{
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	if (offset < data.size()) {
		data[offset] = value;
	}
}


void perem_value::setBoolMassValue(bool value, int x, int y, int z)
{
	has_read = false;
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};
	data[offset] = value;
};



void perem_value::setStringMassValue(QString value, int x, int y, int z)
{
	has_read = false;
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};


	data[offset] = value;

};



void perem_value::setFloatMassValue(double value, int x, int y, int z)
{
	has_read = false;
	int razm = razmer, offset = 0;



	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	qDebug() << data.size();

	data[offset] = value;
};





void perem_value::setIntMassValue(int value, int x, int y, int z)
{
	has_read = false;
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	};

	data[offset] = value;
};




void perem_value::setIntegerValue(int value)
{
	has_read = false;
	data = QVector<QVariant>(1);
	if (p_type == integer) {
		data[0] = value;
	};
	if (p_type == real) {
		double val = (double)value;
		data[0] = val;
	}
};


void perem_value::setBoolValue(bool value)
{
	has_read = false;
	data = QVector<QVariant>(1);
	data[0] = value;
}




void perem_value::setFloatValue(double value)
{
	has_read = false;
	data = QVector<QVariant>(1);
	data[0] = value;
}


void perem_value::setStringValue(QString value)
{
	has_read = false;
	data = QVector<QVariant>(1);
	data[0] = value;
}

void perem_value::setCharectValue(QChar value)
{
	has_read = false;
	data = QVector<QVariant>(1);
	data[0] = value;
}


double perem_value::getValueAsFloat()
{
	//if(!ready)return 99;
	if (p_type == real) {
		return data[0].toDouble();
	}
	if (p_type == integer) {
		int temp = data[0].toInt();
		return (double)temp;
	};

	if (p_type == kumBoolean) {
		if (data[0].toBool()) {
			return 1;
		} else {
			return 0;
		}
	};
	return 0;
};



int perem_value::getValueAsInt()
{
	if (p_type == integer) {
		return data[0].toInt();
	}
	if (p_type == real) {
		double temp = data[0].toDouble();
		return (int)temp;
	};
	if (p_type == kumBoolean) {
		return (data[0].toInt()) == 1;
	}
	return 0;
};

bool perem_value::getValueAsBool()
{
	return data[0].toBool();
};

QVariant perem_value::getValueAsVariant() const
{
	if (p_type == integer ||
		p_type == real ||
		p_type == kumBoolean ||
		p_type == charect ||
		p_type == kumString) {
		return data[0];
	} else {
		QList<QVariant> serialization;
		serialization << razmer;
		if (razmer >= 1) {
			serialization << start_x << size_x;
		}
		if (razmer >= 2) {
			serialization << start_y << size_y;
		}
		if (razmer >= 3) {
			serialization << start_z << size_z;
		}
		serialization += data.toList();
		return QVariant(serialization);
	}
}


QString perem_value::getValueAsString()
{
	if (p_type == kumString) {
		return data[0].toString();
	}
	if (p_type == charect) {
		return data[0].toChar();
	}
	if (p_type == integer) {
		return QString::number(data[0].toInt());
	}
	if (p_type == real) {
		return QString::number(data[0].toDouble());
	}
	if (p_type == kumBoolean) {
		if (data[0].toBool()) {
			return QString(QString::fromUtf8("Да"));
		} else {
			return QString(QString::fromUtf8("Нет"));
		}
	};

	return "";
}



QChar perem_value::getValueAsCharect()
{
	if (p_type == charect) {
		return data[0].toChar();
	}
	return ' ';
}

bool perem_value::isReady(int x, int y, int z)
{
	int razm = razmer, offset = 0;


	switch (razm) {
	case 1:
		offset = x;
		break;
	case 2:
		offset = y * size_x + x;
		break;
	case 3:
		offset = z * size_y * size_x + y * size_x + x;
		break;
	}
	return data[offset].isValid();
}
void perem_value::cleanMass()
{
	has_read = false;
	qDebug() << "CleanVector";
	for (int i = 0; i < data.size(); i++) {

		data[i] = QVariant();
	};

};
void perem_value::setReady(int x, int y, int z, bool value)
{
	has_read = false;
	Q_UNUSED(x);
	Q_UNUSED(y);
	Q_UNUSED(z);
	Q_UNUSED(value);
//        int razm=razmer,offset=0;

//        switch ( razm )
//        {
//                case 1:
//                        offset = x;
//                        break;
//                case 2:
//                        offset = y * size_x + x;
//                        break;
//                case 3:
//                        offset = z * size_y * size_x + y * size_x + x;
//                        break;
//        }
//        data[offset].isValid() = value;
}

//    ======   class isp_table =======================================
isp_table::isp_table()
{
	count = -1; //!!!!!!!!!!!!!!ChekMe

};

int isp_table::addIsp(QString isp_name, int isp_impl_str, IspType type, QString file)
{
	if (count >= MAX_ISP) {
		return 2;
	}
	count++;
	isp_Table[count].NetworkServer = NULL;
	isp_Table[count].name = isp_name;
	isp_Table[count].isp_begins = isp_impl_str;
	if ((type == instrument) || (type == network)) {
		isp_Table[count].isp_begins = -1;
	}
	isp_Table[count].isp_ends = 65535;
	if ((type == instrument) || (type == network)) {
		isp_Table[count].isp_ends = -1;
	}
	isp_Table[count].type = type;
	//isp_Table[count].firstPeremId=0;
	isp_Table[count].file = file;
	//isp_Table[count].Instrument=NULL;
	isp_Table[count].graphics = false;
	return 0;
};

int isp_table::inTable(QString name)
{
	for (int i = 0; i < count + 1; i++) {
		if (isp_Table[i].name == name) {
			return i;
		}
	};

	return -1;
};




int isp_table::getIspIdbyStr(int pos)
{
	for (int i = count; i > -1; i--) {
		if (isIntsrument(i)) {
			continue;
		}
		if ((isp_Table[i].isp_begins < 0) && ((isp_Table[i].isp_ends > 33000))) {
			return -1;
		};

		if ((isp_Table[i].isp_begins <= pos) && (isp_Table[i].isp_ends >= pos)) {
			return  i;
		};
	};

	return -1;
};

void isp_table::resetInstruments()
{
	for (int i = 0; i <= count; i++) {
		if (isp_Table[i].type == instrument) {
			//         isp_Table[i].Instrument->reset();
			qDebug("Reset inst");
		};
	};
};

void isp_table::closeInstruments()
{
	for (int i = 0; i <= count; i++) {
		if (isp_Table[i].type == instrument) {
			//         isp_Table[i].Instrument->CloseInst();

		};
	};
};

void isp_table::clean(int last)
{
	for (int i = last + 1; i <= count; i++) {
		if (isp_Table[i].type == instrument) {
			//                                         if(isp_Table[i].Instrument)
			//                          isp_Table[i].Instrument->CloseInst();
		};
	};
	count = last;
};
void isp_table::lockInstrumentsGUI()
{
	for (int i = 0; i <= count; i++) {
		if (isp_Table[i].type == instrument) {
			//         isp_Table[i].Instrument->lockControls();

		};
	};
};


//    ======   class file_table =======================================
/**
 * Конструктор класса. Почти ничего не делает.
 */
file_table::file_table()
{
	currentDirInitialized = true;
}

QString file_table::getFullName(QString name) const
{
	QString normName = name.trimmed();

	bool already_full = false;
#ifndef WIN32
	already_full = normName.startsWith("/");
#else
	QRegExp diskName("[a-z]\\:\\\\", Qt::CaseInsensitive);
	already_full = (diskName.indexIn(normName) == 0);
#endif

	if (already_full) {
		return normName;
	} else {
		return currentDir + "/" + normName;
	}
}


/**
 * Открывает файл и помещает его в таблицу файлов.
 * @param fileName имя файла относительно директории currentDir
 * @param fileMode режим открытия - ReadOnly или WriteOnly
 * @param fileType КУМИ� -тип элементов файла
 * @param err код ошибки или 0, если нет ошибок
 */
void file_table::openFile(QString fileName, FileMode fileMode, PeremType fileType, int *err)
{
	//                          1. Check for possible errors

	//   a) no current dir initialized
	/*
	if ( ! currentDirInitialized )
	{
	    *err = RT_ERR_NO_CUR_DIR;
	    return;
	}
	*/

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	//   b) file no exists

	QString fullName = getFullName(fileName);

	if (
		(! QFile::exists(fullName))
		&&
		(fileMode == ReadOnly)
	) {
		*err = RT_ERR_FILE_NO_EXISTS;
		return;
	}

	//   c) file already opened

	if (fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_REOPEN;
		return;
	}

	//                          2. Try to open file

	file f;
	f.data = new QFile(fullName);
	f.name = fullName;
	f.mode = fileMode;
	f.p_type = fileType;

	QFile::OpenMode modeFlag;

	if (fileMode == ReadOnly) {
		modeFlag = QIODevice::ReadOnly;
	} else {
		modeFlag = QIODevice::WriteOnly;
	}

	if (fileType != kumString) {
		modeFlag = modeFlag | QIODevice::Unbuffered;
	}


	if (! f.data->open(modeFlag)) {
		*err = RT_ERR_FILE_NO_ACCESS;
		return;
	}


	//                          3. If all OK than append new entry to file map

	fileMap[fullName] = f;


	//                          DONE

	*err = 0;
}

/**
 * Закрывает файл и удаляет его из таблицы файлов.
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::closeFile(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//                          2. Close file

	fileMap[fullName].data->close();

	//                          3. Remove entry from file map

	fileMap.remove(fullName);

	//                          DONE

	*err = 0;
}

/**
 * Устанавливает позицию чтения на начало файла.
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::setToBegin(QString fileName, int *err)
{
	//                          0. Prolog

	//    KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString normName = fileName.trimmed();
	QString fullName;

	if (normName.startsWith("/")) {
		fullName = normName;
	} else {
		fullName = currentDir + "/" + normName;
	}

	//                          1. Check for possible errors

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//                          2. Set to begin

	fileMap[fullName].data->reset();

	//                          DONE

	*err = 0;
}

/**
 * Возвращает признак окончания файла.
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return true, если в файле есть ещё данные или false, если конец файла
 */
bool file_table::hasData(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return 0;
	}

	//                          2. Check for data

	bool atEnd = fileMap[fullName].data->atEnd();

	//                          DONE

	*err = 0;
	return !atEnd;
}

/**
 * Читает один целочисленный элемент из файла
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return очередной элемент файла
 */
int file_table::readIntegerElem(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return 0;
	}

	//   b) end of file

	QDataStream stream(fileMap[fullName].data);

	if (stream.atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return 0;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != integer) {
		//              *err = RT_ERR_EXPECTED_INTEGER;
		return 0;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode != ReadOnly) {
		*err = RT_ERR_OPENED_FOR_WRITE_ONLY; // internal error
		return 0;
	}

	//                         2. Read data

	qint32 elem; // 32-bit signed integer value
	stream >> elem;
	int result = (int)elem;

	//                         DONE

	*err = 0;
	return result;
}

/**
 * Читает один вещественный элемент из файла
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return очередной элемент файла
 */
double file_table::readFloatElem(QString fileName, int *err)
{
	//                          0. Prolog

	//    KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return 0;
	}

	//   b) end of file

	QDataStream stream(fileMap[fullName].data);

	if (stream.atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return 0;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != real) {
		//              *err = RT_ERR_EXPECTED_REAL;
		return 0;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode != ReadOnly) {
		*err = RT_ERR_OPENED_FOR_WRITE_ONLY; // internal error
		return 0;
	}

	//                         2. Read data

	double elem;
	stream >> elem;

	//                         DONE

	*err = 0;
	return elem;
}

/**
 * Читает один символьный элемент из файла
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return очередной элемент файла
 */
QChar file_table::readCharectElem(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return 0;
	}

	//   b) end of file

	QTextStream stream(fileMap[fullName].data);
	stream.setCodec("UTF-8");

	if (stream.atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return 0;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != charect) {
		//          *err = RT_ERR_EXPECTED_CHARECT; // internal error
		return 0;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode != ReadOnly) {
		*err = RT_ERR_OPENED_FOR_WRITE_ONLY; // internal error
		return 0;
	}

	//                         2. Read data


	stream.setCodec("UTF-8");

	QChar elem; // unicode charect
	stream >> elem;

	//                         DONE

	*err = 0;
	return elem;
}

/**
 * Читает одну строку из файла
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return очередная строка файла
 */
QString file_table::readStringElem(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return 0;
	}

	//   b) end of file

	QTextStream stream(fileMap[fullName].data);
	//      stream.setEncoding(QTextStream::UnicodeUTF8);

	if (stream.atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return 0;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != kumString) {
		//          *err = RT_ERR_EXPECTED_STRING; // internal error
		return 0;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode != ReadOnly) {
		*err = RT_ERR_OPENED_FOR_WRITE_ONLY; // internal error
		return 0;
	}

	//                         2. Read data


	stream.setCodec("UTF-8");

	QString elem; // unicode string
	elem = stream.readLine();

	//                         DONE

	*err = 0;
	return elem;
}

/**
 * Читает один логический элемент из файла
 * @param fileName имя файла относительно currentDir
 * @param err код ошибки или 0, если ошибок нет
 * @return очередной элемент файла
 */
bool file_table::readBooleanElem(QString fileName, int *err)
{
	Q_UNUSED(fileName);
	*err = 888; // NOT IMPLEMENTED YET
	return false;
}

void file_table::clearFile(QString fileName, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode != WriteOnly) {
		*err = RT_ERR_OPENED_FOR_READ_ONLY; // internal error
		return;
	}

	//                          2. Close and remove file

	fileMap[fullName].data->close();

	if (! fileMap[fullName].data->remove()) {
		*err = RT_ERR_FILE_NO_ACCESS;
		return;
	}

	//                          3. Reopen/reclose file for create at length of zero

	fileMap[fullName].data->open(QIODevice::WriteOnly);
	fileMap[fullName].data->close();

	//                          4. Reopen file

	fileMap[fullName].data->open(QIODevice::WriteOnly);

	//                          DONE

	*err = 0;
}

/**
 * Пишет один целочисленный элемент в файл
 * @param fileName имя файла относительно currentDir
 * @param value очередной элемент
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::writeIntegerElem(QString fileName, int value, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != integer) {
		//        *err = RT_ERR_EXPECTED_INTEGER; // internal error
		return;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode == ReadOnly) {
		*err = RT_ERR_OPENED_FOR_READ_ONLY;
		return;
	}

	//                         2. Write data

	QDataStream stream(fileMap[fullName].data);

	stream << (qint32)value;

	//                         DONE

	*err = 0;
}

/**
 * Пишет один вещественный элемент в файл
 * @param fileName имя файла относительно currentDir
 * @param value очередной элемент
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::writeFloatElem(QString fileName, double value, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != real) {
		//        *err = RT_ERR_EXPECTED_REAL; // internal error
		return;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode == ReadOnly) {
		*err = RT_ERR_OPENED_FOR_READ_ONLY;
		return;
	}

	//                         2. Write data

	QDataStream stream(fileMap[fullName].data);

	stream << value;

	//                         DONE

	*err = 0;
}

/**
 * Пишет один символ в файл
 * @param fileName имя файла относительно currentDir
 * @param value очередной символ
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::writeCharectElem(QString fileName, QChar value, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");

	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != charect) {
		//        *err = RT_ERR_EXPECTED_CHARECT; // internal error
		return;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode == ReadOnly) {
		*err = RT_ERR_OPENED_FOR_READ_ONLY;
		return;
	}

	//                         2. Write data

	QTextStream stream(fileMap[fullName].data);
	stream.setCodec("UTF-8");
	stream << value;

	//                         DONE

	*err = 0;
}

/**
 * Пишет одну строку в файл
 * @param fileName имя файла относительно currentDir
 * @param value очередная строка
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::writeStringElem(QString fileName, QString value, int *err)
{
	//                          0. Prolog

	//     KumSettings settings;
	//     settings.setPath("IMPB","Kumir");
	//
	//     currentDir = settings.readEntry("/Kumir/Directories/KumCurrent","");


	QString fullName = getFullName(fileName);

	//                          1. Check for possible errors

	//   a) file not opened

	if (! fileMap.contains(fullName)) {
		*err = RT_ERR_FILE_NOT_OPENED;
		return;
	}

	//   c) type mismatch

	if (fileMap[fullName].p_type != kumString) {
		//        *err = RT_ERR_EXPECTED_STRING; // type mismatch
		return;
	}

	//   d) wrong mode opened

	if (fileMap[fullName].mode == ReadOnly) {
		*err = RT_ERR_OPENED_FOR_READ_ONLY;
		return;
	}

	//                         2. Write data

	QTextStream stream(fileMap[fullName].data);
	stream.setCodec("UTF-8");

	stream << value;
	stream << "\n";

	//                         DONE

	*err = 0;
}

/**
 * Пишет один логический элемент в файл
 * @param fileName имя файла относительно currentDir
 * @param value очередной элемент
 * @param err код ошибки или 0, если ошибок нет
 */
void file_table::writeBooleanElem(QString fileName, bool value, int *err)
{
	Q_UNUSED(fileName);
	Q_UNUSED(value);
	*err = 888; // NOT IMPLEMENTED YET
}

/**
 * Возвращает текущий каталог таблицы файлов.
 * @return абсолютный путь к текущему каталогу
 */
QString file_table::getCurrentDir()
{
	return currentDir;
}

/**
 * Устанавливает текущий каталог таблицы файлов.
 * @param dirName абсолютный путь каталога
 * @param err код ошибки или 0, если успешно
 */
void file_table::setCurrentDir(QString dirName, int *err)
{
	QDir dir(dirName);
	if (! dir.exists()) {
		*err = 0;
		return;
	}
	currentDir = dirName;
	currentDirInitialized = true;
	//     QSettings settings(QSettings::Ini);
	//     settings.setPath("IMPB","Kumir");
	//     settings.beginGroup("/Kumir");
	//     settings.beginGroup("Directories");
	//     settings.writeEntry("KumCurrent",dirName);
}


/**
 * Закрывает все файлы, освобождает память, занятую QFile'ами и очищает таблицу.
 */
void file_table::clear()
{
	foreach (const file f, fileMap.values()) {
		f.data->close();
		delete f.data;
	}
	fileMap.clear();
}

/**
 * Деструктор класса. Просто вызывает свой метод clear()
 */
file_table::~file_table()
{
	this->clear();
}




/*
Kumir Output buffer
*/

KumOutBuffElem::KumOutBuffElem()
{
	TYPE = 0;
	str = 0;

};

KumOutBuffElem::KumOutBuffElem(int tpy, int stri, QString texta)
{
	TYPE = tpy;
	str = stri;
	text = texta;
};


KumOutBuff::KumOutBuff()
{

	updated = false;
	locked = false;
};

void KumOutBuff::appendMainMsg(QString text)
{

	if (!locked) {
		Buffer.append(KumOutBuffElem(1, 0, text));
	}

	updated = true;
};

void KumOutBuff::appendDebugMsg(QString text, int str)
{

	if (!locked) {
		Buffer.append(KumOutBuffElem(2, str, text));
	}

	updated = true;
};

void KumOutBuff::appendErrMsg(QString text, int str)
{

	if (!locked) {
		Buffer.append(KumOutBuffElem(3, str, text));
	}
	locked = true;
	updated = true;
};

void KumOutBuff::appendCounterMsg(int increment)
{
	//  if(!locked)Buffer.append(KumOutBuffElem(4,increment,""));
	Buffer.append(KumOutBuffElem(4, increment, ""));
	//  locked=true;
	updated = true;
}

KumOutBuffElem KumOutBuff::GetFirst()
{

	if (Buffer.count() > 0) {
		KumOutBuffElem tmpe = Buffer.first();
		Buffer.pop_front();
		return tmpe;
	};
	return KumOutBuffElem(1, 0, "Kumir internal error:out buffer problem");
};

KumOutBuffElem KumOutBuff::first()
{

	if (Buffer.count() > 0) {
		KumOutBuffElem tmpe = Buffer[0];
		return tmpe;
	};
	return KumOutBuffElem(1, 0, "Kumir internal error:out buffer problem");
};

/**
* Удаляет устаревшие сообщения DebugMsg и RunLineMsg,
* оставляя только последние сообщения для каждой строки
*/
void KumOutBuff::removeUnactualMessages()
{

	QMap<int, bool> map; // номера нормализованных строк
	int count = 0;
	bool count_initialized = false;

	// TODO port to Qt4 commented block
	//  for ( int i=Buffer.count()-1; i>=0; i-- )
	//  {
	//      KumOutBuffElem elem = Buffer[i];
	//      // Удаляем устаревший вывод на поля
	//      if ( elem.TYPE == 2 )
	//      {
	//          if ( map.contains(elem.str) )
	//          {
	//              Buffer.remove(Buffer.at(i));
	//          }
	//          else
	//          {
	//              map[elem.str] = true;
	//          }
	//      }
	//      // Пересчитываем количество проделанных шагов
	//      if ( elem.TYPE == 4 )
	//      {
	//          if ( ! count_initialized )
	//          {
	//              count = elem.str;
	//              count_initialized = true;
	//              Buffer.remove(Buffer.at(i));
	//          }
	//          else
	//          {
	//              count += elem.str;
	//              Buffer.remove(Buffer.at(i));
	//          }
	//      }
	//  } // конец цикла по сообщениям
	if (count_initialized) {
		appendCounterMsg(count);
	}
}




/**
 *class KumValueStackElem. Представление элемента стека для подсчета выражений
 */
KumValueStackElem::KumValueStackElem()
{
	ready = false;
	valid = false;
	type = none;
};

KumValueStackElem::KumValueStackElem(int value)
{
	ready = true;
	valid = true;
	type = integer;
	m_value = QVariant(value);
	error = 0;

};


KumValueStackElem::KumValueStackElem(double value)
{
	ready = true;
	valid = true;
	type = real;
	m_value = QVariant(value);
	error = 0;

};

KumValueStackElem::KumValueStackElem(PeremType typ, int err)
{
	ready = true;
	valid = true;
	type = typ;
	m_value = QVariant();
	error = err;

};
KumValueStackElem::KumValueStackElem(bool value)
{
	ready = true;
	valid = true;
	type = kumBoolean;
	m_value = QVariant(value);
	error = 0;

};


KumValueStackElem::KumValueStackElem(QString value)
{
	ready = true;
	valid = true;
	type = kumString;
	m_value = QVariant(value);
	error = 0;

};

KumValueStackElem::KumValueStackElem(QChar value)
{
	ready = true;
	valid = true;
	type = charect;
	m_value = QVariant(value);
	error = 0;

};

KumValueStackElem::KumValueStackElem(bool b, double d)
{
	ready = true;
	valid = true;
	type = bool_or_num;
	m_value = QVariant(d);
	m_boolValue = b;
	error = 0;

}

KumValueStackElem::KumValueStackElem(bool b, int i)
{
	ready = true;
	valid = true;
	type = bool_or_num;
	m_value = QVariant(i);
	m_boolValue = b;
	error = 0;

}

KumValueStackElem::KumValueStackElem(bool b, QString s)
{
	ready = true;
	valid = true;
	type = bool_or_lit;
	m_value = QVariant(s);
	m_boolValue = b;
	error = 0;

}

KumValueStackElem::KumValueStackElem(bool b, QChar c)
{
	ready = true;
	valid = true;
	type = bool_or_lit;
	m_value = QVariant(c);
	m_boolValue = b;
	error = 0;

}


KumValueStackElem::KumValueStackElem(symbol_table *symbols, int id)
{
	ready = symbols->isReady(id);
	valid = true;
	QChar value;
	type = symbols->getTypeByID(id);
	if (type == integer) {
		m_value = QVariant(symbols->symb_table[id].value.getValueAsInt());
	}
	if (type == kumString) {
		m_value = QVariant(symbols->symb_table[id].value.getValueAsString());
	};
	if (type == real) {
		m_value = QVariant(symbols->symb_table[id].value.getValueAsFloat());
	}
	if (type == kumBoolean) {
		m_value = QVariant(symbols->symb_table[id].value.getValueAsBool());
	}
	if (type == charect) {
		m_value = QVariant(symbols->symb_table[id].value.getValueAsCharect());
	}
	error = 0;
}



KumValueStackElem::~KumValueStackElem()
{
	ready = false;
	valid = false;
	type = none;
}



int KumValueStackElem::asInt()
{
	if (!ready) {
		return 9999;
	}
	if (type == dummy) {
		return 0;
	}
	int test = type;
	Q_UNUSED(test);
	switch (type) {
	case integer: {
		return m_value.toInt();
	};
	break;

	case real: {
		return static_cast<int>(m_value.toDouble());
	}
	break;

	case kumString:
		//valid=false;
		return  99;
		break;

	case kumBoolean:
		//valid=false;
		return 100;
		break;

	case bool_or_num:
		return 9999;
		break;

	case bool_or_lit:
		return 9999;
		break;

	case charect:
		return 9999;
		break;

	case mass_integer:
		return 9999;
		break;

	case mass_charect:
		return 9999;
		break;

	case mass_string:
		return 9999;
		break;

	case mass_real:
		return 9999;
		break;

	case mass_bool:
		return 9999;
		break;

	case none:
		return 9999;
		break;

	case dummy:
		return 0;
		break;

	};

	return -1;
};

double KumValueStackElem::asFloat()
{
	if (!ready) {
		return 9999;
	}
	if (type == dummy) {
		return 0;
	}
	int test = type;
	Q_UNUSED(test);
	switch (type) {
	case integer: {
		return static_cast<double>(m_value.toInt());
	}
	break;

	case real:
		return m_value.toDouble();
		break;

	case kumString:
		//valid=false;
		return  99;
		break;

	case kumBoolean:
		//valid=false;
		return 100;
		break;

	case bool_or_num:
		return 9999;
		break;

	case bool_or_lit:
		return 9999;
		break;

	case charect:
		return 9999;
		break;

	case mass_integer:
		return 9999;
		break;

	case mass_charect:
		return 9999;
		break;

	case mass_string:
		return 9999;
		break;

	case mass_real:
		return 9999;
		break;

	case mass_bool:
		return 9999;
		break;

	case none:
		return 9999;
		break;

	case dummy:
		return 0;
		break;
	};

	return -1;
};


bool KumValueStackElem::asBool()
{
	if (!ready) {
		return false;
	}
	if (type == dummy) {
		return false;
	}
	int test = type;
	Q_UNUSED(test);
	switch (type) {
	case dummy:
		return 0;
		break;

	case integer:
		return m_value.toInt() != 0;
		break;

	case real:
		return m_value.toInt() != 0.0;
		break;

	case kumString:
		//valid=false;
		return  false;
		break;

	case kumBoolean:
		//valid=false;
		return m_value.toBool();
		break;

	case bool_or_num: {
		return m_boolValue;
	}
	break;

	case bool_or_lit: {
		return m_boolValue;
	}
	break;

	case charect:
		return false;
		break;

	case mass_integer:
		return false;
		break;

	case mass_charect:
		return false;
		break;

	case mass_string:
		return false;
		break;

	case mass_real:
		return false;
		break;

	case mass_bool:
		return false;
		break;

	case none:
		return false;
		break;
	};

	return false;
};


QString KumValueStackElem::asString()
{
	if (!ready) {
		return "INTERNAL ERROR KumValueStackElem:not ready!";
	}
	if (type == dummy) {
		return "";
	}
	switch (type) {
	case integer:
		return QString::number(m_value.toInt());
		break;

	case real: {
		QString result = QString::number(m_value.toDouble(), 'g', 7);
		return result;
		break;
	}

	case kumString:
		//valid=false;
	{
		return m_value.toString();
		break;
	}
	case charect:
		return QString(m_value.toChar());
		break;

	case kumBoolean: {
		bool value = m_value.toBool();
		return value ? QString::fromUtf8("да") : QString::fromUtf8("нет");
		break;
	}

	case bool_or_num: {

		return m_boolValue ? QString::fromUtf8("да") : QString::fromUtf8("нет");
		break;
	}

	case bool_or_lit: {


		return m_boolValue ? QString::fromUtf8("да") : QString::fromUtf8("нет");
		break;
	}

	case mass_integer:
		return "INTERNAL ERROR KumValueStackElem";
		break;

	case mass_charect:
		return "INTERNAL ERROR KumValueStackElem";
		break;

	case mass_string:
		return "INTERNAL ERROR KumValueStackElem";
		break;

	case mass_real:
		return "INTERNAL ERROR KumValueStackElem";
		break;

	case mass_bool:
		return "INTERNAL ERROR KumValueStackElem";
		break;

	case none:
		return "INTERNAL ERROR KumValueStackElem";
		break;
	case dummy:
		return 0;
		break;
	};

	return "";
};


QChar KumValueStackElem::asCharect()
{
	if (!ready) {
		return QChar(0);
	}
	if (type == dummy) {
		return ' ';
	}
	switch (type) {
	case dummy:
		return ' ';
		break;
	case integer:
		return QChar(0);
		break;

	case real:
		return QChar(0);
		break;

	case kumString:
		return QChar(0);
		break;
	case charect:
		return  m_value.toChar();
		break;

	case kumBoolean:
		return QChar(0);
		break;

	case bool_or_num:
		return QChar(0);
		break;

	case bool_or_lit:
		return QChar(0);
		break;

	case mass_integer:
		return QChar(0);
		break;

	case mass_charect:
		return QChar(0);
		break;

	case mass_string:
		return QChar(0);
		break;

	case mass_real:
		return QChar(0);
		break;

	case mass_bool:
		return QChar(0);
		break;

	case none:
		return QChar(0);
		break;
	};
	return QChar(0);
};

QVariant KumValueStackElem::asVariant()
{
//    if(!ready)return QVariant(0);
//    if(type==dummy)return QVariant(' ');
//    switch(type)
//    {
//    case dummy:
//        return QVariant(' ');
//        break;
//    case integer:
//        return QVariant(*(int*)elem_ptr);
//        break;
//
//    case real:
//        return QVariant(*(double*)elem_ptr);
//        break;
//
//    case kumString:
//        return QVariant(*(QString*)elem_ptr);
//        break;
//    case charect:
//        return  QVariant(*(QChar*)elem_ptr);
//        break;
//
//    case kumBoolean:
//        return QVariant(asBool());
//        break;
//
//    case bool_or_num:
//        return QVariant(asBool());
//        break;
//
//    case bool_or_lit:
//        return QVariant(asBool());
//        break;
//
//    case mass_integer:
//        return QVariant("ARRAY");
//        break;
//
//    case mass_charect:
//        return QVariant("ARRAY");
//        break;
//
//    case mass_string:
//        return QVariant("ARRAY");
//        break;
//
//    case mass_real:
//        return QVariant("ARRAY");
//        break;
//
//    case mass_bool:
//        return QVariant("ARRAY");
//        break;
//
//    case none:
//        return QVariant("NONE");
//        break;
//    };
//    return QVariant(0);
	if (type == bool_or_lit || type == bool_or_num) {
		return QVariant(m_boolValue);
	} else {
		return m_value;
	}

}


KumValueStackElem KumValueStackElem::compare(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(false);
	}
	if (!second.ready) {
		return KumValueStackElem(false);
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			int fst = first.asInt();
			int snd = second.asInt();
			bool result = fst == snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			int fst = first.asInt();
			double snd = second.asFloat();
			bool result = fabs(((double)fst) - snd) <= REAL_COMPARE_PRECISION;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst == sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toInt();
				double fstv = ((double)fst);
				double diff = fabs(fstv - sndv);
				bool result = diff <= REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.type == integer) {
			double fst = first.asFloat();
			int snd = second.asInt();
			bool result = fabs(fst - ((double)snd)) <= REAL_COMPARE_PRECISION;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double fst = first.asFloat();
			double snd = second.asFloat();
			double diff = fst - snd;
			double adiff = fabs(diff);
			bool result = adiff <= REAL_COMPARE_PRECISION;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fabs(fst - ((double)sndv)) <= REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				double fstv = fst;
				Q_UNUSED(fstv);
				double diff = fabs(fst - sndv);
				bool result = diff <= REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString fst = first.asString();
			QString snd = second.asString();
			bool result = fst == snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QString fst = first.asString();
			QChar snd = second.asCharect();
			bool result = fst == QString(snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QString fst = first.asString();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fst == sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst == QString(sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar fst = first.asCharect();
			QString snd = second.asString();
			bool result = QString(fst) == snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar fst = first.asCharect();
			QChar snd = second.asCharect();
			bool result = fst == snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QChar fst = first.asCharect();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = QString(fst) == sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst == sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case bool_or_lit: {

		QString fstv;
		if (first.m_value.type() == QVariant::String) {
			fstv = first.m_value.toString();
		}
		if (first.m_value.type() == QVariant::Char) {
			fstv = first.m_value.toChar();
		}

		if (second.type == kumString) {
			QString snd = second.asString();
			bool result = fstv == snd;
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar snd = second.asCharect();
			bool result = fstv == QString(snd);
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fstv == sndv;
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fstv == QString(sndv);
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() == snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = fabs(first.m_value.toDouble() - (static_cast<double>(snd))) <= REAL_COMPARE_PRECISION;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = (static_cast<double>(first.m_value.toInt())) == snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = fabs(first.m_value.toDouble() - snd) <= REAL_COMPARE_PRECISION;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() == sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = fabs(first.m_value.toDouble() - ((double)sndv)) <= REAL_COMPARE_PRECISION;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = fabs((static_cast<double>(first.m_value.toInt())) - sndv) <= REAL_COMPARE_PRECISION;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = fabs(first.m_value.toDouble() - sndv) <= REAL_COMPARE_PRECISION;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case kumBoolean: {
		bool fst = first.asBool();
		bool snd = second.asBool();
		bool result = fst == snd;
		return KumValueStackElem(result);
	}
	break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};



KumValueStackElem KumValueStackElem::Summ(KumValueStackElem first, KumValueStackElem second, int *err)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {

		if (second.type == integer) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double test=FT+ST;
			//              int result;
			//              if ( (test>1073741824) || (test<(-1073741824)) )
			//              {
			//                  *err=CALC_OVERFLOW;
			//                  result = 0;
			//              }
			//              else
			//              {
			//                  result = (int)test;
			//              }
			if (!IntegerOverflowChecker::checkSumm(first.asInt(), second.asInt())) {
				*err = CALC_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asInt() + second.asInt());
			}

			//              return KumValueStackElem ( result );
		}
		if (second.type == real) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double test=FT+ST;
			//
			//              double result = first.asInt() + second.asFloat();
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkSumm(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() + second.asFloat());
			}
		}
		//          if ( ( second.type==integer ) || ( second.type==real ) )
		//              return KumValueStackElem ( first.asInt() +second.asInt() );
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if ((second.type == integer) || (second.type == real)) {
			//              double result = first.asFloat() +second.asFloat();
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkSumm(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() + second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};

	break;
	case kumString: {
		if (second.type == kumString) {
			QString FST = first.asString();
			QString SND = second.asString();
			QString result = SND + FST;
			return KumValueStackElem(result);
		}
		if (second.type == charect) {
			QString FST = first.asString();
			QChar SND = second.asCharect();
			QString result = QString(SND) + FST;
			return KumValueStackElem(result);
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar FST = first.asCharect();
			QString SND = second.asString();
			QString result = SND + QString(FST);
			return KumValueStackElem(result);
		}
		if (second.type == charect) {
			QChar FST = first.asCharect();
			QChar SND = second.asCharect();
			QString result = QString(SND) + QString(FST);
			return KumValueStackElem(result);
		}
		return KumValueStackElem(false);
	};
	break;



	case kumBoolean:
		return KumValueStackElem(false);
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};

KumValueStackElem KumValueStackElem::Multi(KumValueStackElem first, KumValueStackElem second, int *err)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROjR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double test=FT*ST;
			//              int result;
			//              if ( (test>1073741824) || (test<(-1073741824)) )
			//              {
			//                  *err=CALC_OVERFLOW;
			//                  result = 0;
			//              }
			//              else
			//              {
			//                  result = (int)test;
			//              }
			//              return KumValueStackElem ( result );
			if (!IntegerOverflowChecker::checkProd(first.asInt(), second.asInt())) {
				*err = CALC_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asInt() * second.asInt());
			}
		}
		if (second.type == real) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double result=FT*ST;
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkProd(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() * second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if ((second.type == integer) || (second.type == real)) {
			//              double result = first.asFloat() *second.asFloat();
			//              if ( (result>1E30) || (result<(-1E30)) )
			//          {
			//              *err=CALC_FLOAT_OVERFLOW;
			//              result = 0;
			//          }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkProd(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() * second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if ((second.type == kumString)) {
			return KumValueStackElem("INTERNAL ERROR:KumValueStackElem bad oper");
		}
		return KumValueStackElem(false);
	};
	break;


	case kumBoolean:
		return KumValueStackElem(false);
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};




KumValueStackElem KumValueStackElem::Div(KumValueStackElem first, KumValueStackElem second, int *err)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROjR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}

	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			if (second.asInt() == 0) {
				return KumValueStackElem(dummy, RUN_DIV_ZERO);
			}
			if (!IntegerOverflowChecker::checkProd(first.asInt(), second.asInt())) {
				*err = CALC_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				double a, b;
				a = (double)first.asInt();
				b = (double)second.asInt();
				return KumValueStackElem(a / b);
			}
		}
		if (second.type == real) {
			if (second.asFloat() == 0) {
				return KumValueStackElem(dummy, RUN_DIV_ZERO);
			}
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double result=FT*ST;
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkProd(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() / second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.asFloat() == 0) {
			return KumValueStackElem(dummy, RUN_DIV_ZERO);
		}
		if ((second.type == integer) || (second.type == real)) {
			//              double result = first.asFloat() *second.asFloat();
			//              if ( (result>1E30) || (result<(-1E30)) )
			//          {
			//              *err=CALC_FLOAT_OVERFLOW;
			//              result = 0;
			//          }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkProd(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() / second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if ((second.type == kumString)) {
			return KumValueStackElem("INTERNAL ERROR:KumValueStackElem bad oper");
		}
		return KumValueStackElem(false);
	};
	break;


	case kumBoolean:
		return KumValueStackElem(false);
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};



KumValueStackElem KumValueStackElem::Minus(KumValueStackElem first, KumValueStackElem second, int *err)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double test=FT-ST;
			//              int result;
			//              if ( (test>1073741824) || (test<(-1073741824)) )
			//              {
			//                  *err=CALC_OVERFLOW;
			//                  result = 0;
			//              }
			//              else
			//              {
			//                  result = (int)test;
			//              }
			if (!IntegerOverflowChecker::checkDiff(first.asInt(), second.asInt())) {
				*err = CALC_OVERFLOW;
				return KumValueStackElem(0);
			}

			return KumValueStackElem(first.asInt() - second.asInt());
		}
		if (second.type == real) {
			//              double FT=first.asFloat();
			//              double ST=second.asFloat();
			//              double result=FT-ST;
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkDiff(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() - second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if ((second.type == integer) || (second.type == real)) {
			//              double result = first.asFloat() - second.asFloat();
			//              if ( (result>1E30) || (result<(-1E30)) )
			//          {
			//              *err=CALC_FLOAT_OVERFLOW;
			//              result = 0;
			//          }
			//              return KumValueStackElem ( result );
			if (!DoubleOverflowChecker::checkDiff(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(first.asFloat() - second.asFloat());
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if ((second.type == kumString)) {
			return KumValueStackElem("INTERNAL ERROR:KumValueStackElem bad oper");
		}

	};
	break;


	case kumBoolean:
		return KumValueStackElem(false);
		break;

	default:
		return KumValueStackElem(false);
	};

	return KumValueStackElem(false);

};


KumValueStackElem KumValueStackElem::More(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(false);
	}
	if (!second.ready) {
		return KumValueStackElem(false);
	}

	if (second.type == dummy) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			int fst = first.asInt();
			int snd = second.asInt();
			bool result = fst > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			int fst = first.asInt();
			double snd = second.asFloat();
			bool result = ((double)fst) > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = (static_cast<double>(fst)) > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.type == integer) {
			double fst = first.asFloat();
			int snd = second.asInt();
			bool result = fst > ((double)snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double fst = first.asFloat();
			double snd = second.asFloat();
			bool result = fst > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst > (static_cast<double>(sndv));
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = fst > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString fst = first.asString();
			QString snd = second.asString();
			bool result = fst > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QString fst = first.asString();
			QChar snd = second.asCharect();
			bool result = fst > QString(snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QString fst = first.asString();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fst > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst > QString(sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar fst = first.asCharect();
			QString snd = second.asString();
			bool result = QString(fst) > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar fst = first.asCharect();
			QChar snd = second.asCharect();
			bool result = fst > snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QChar fst = first.asCharect();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = QString(fst) > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst > sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case bool_or_lit: {

		QString fstv;
		if (first.m_value.type() == QVariant::String) {
			fstv = first.m_value.toString();
		}
		if (first.m_value.type() == QVariant::Char) {
			fstv = QString(first.m_value.toChar());
		}

		if (second.type == kumString) {
			QString snd = second.asString();
			bool result = fstv > snd;
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar snd = second.asCharect();
			bool result = fstv > QString(snd);
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fstv > sndv;
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fstv > QString(sndv);
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() > snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() > (static_cast<double>(snd));
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = (static_cast<double>(first.m_value.toInt())) > snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() > snd;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() > sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() > (static_cast<double>(sndv));
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = (static_cast<double>(first.m_value.toInt())) > sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() > sndv;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;


	case kumBoolean:
		return KumValueStackElem(first.asBool() == second.asBool());
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};

KumValueStackElem KumValueStackElem::Less(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(false);
	}
	if (!second.ready) {
		return KumValueStackElem(false);
	}
	if (second.type == dummy) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			int fst = first.asInt();
			int snd = second.asInt();
			bool result = fst < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			int fst = first.asInt();
			double snd = second.asFloat();
			bool result = ((double)fst) < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = ((double)fst) < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.type == integer) {
			double fst = first.asFloat();
			int snd = second.asInt();
			bool result = fst < ((double)snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double fst = first.asFloat();
			double snd = second.asFloat();
			bool result = fst < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst < ((double)sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = fst < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString fst = first.asString();
			QString snd = second.asString();
			bool result = fst < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QString fst = first.asString();
			QChar snd = second.asCharect();
			bool result = fst < QString(snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QString fst = first.asString();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fst < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst < QString(sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar fst = first.asCharect();
			QString snd = second.asString();
			bool result = QString(fst) < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar fst = first.asCharect();
			QChar snd = second.asCharect();
			bool result = fst < snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QChar fst = first.asCharect();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = QString(fst) < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst < sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case bool_or_lit: {

		QString fstv;
		if (first.m_value.type() == QVariant::String) {
			fstv = first.m_value.toString();
		}
		if (first.m_value.type() == QVariant::Char) {
			fstv = QString(first.m_value.toChar());
		}

		if (second.type == kumString) {
			QString snd = second.asString();
			bool result = fstv < snd;
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar snd = second.asCharect();
			bool result = fstv < QString(snd);
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fstv < sndv;
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fstv < QString(sndv);
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() < snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() < ((double)snd);
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = ((double)(first.m_value.toInt())) < snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() < snd;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() < sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() < ((double)sndv);
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = ((double)(first.m_value.toInt())) < sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() < sndv;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;


	case kumBoolean:
		return KumValueStackElem(first.asBool() == second.asBool());
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};

KumValueStackElem KumValueStackElem::GEQ(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(false);
	}
	if (!second.ready) {
		return KumValueStackElem(false);
	}
	if (second.type == dummy) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			int fst = first.asInt();
			int snd = second.asInt();
			bool result = fst >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			int fst = first.asInt();
			double snd = second.asFloat();
			bool result = ((double)fst) >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = ((double)fst) >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.type == integer) {
			double fst = first.asFloat();
			int snd = second.asInt();
			bool result = fst >= ((double)snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double fst = first.asFloat();
			double snd = second.asFloat();
			bool result = fst >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst >= ((double)sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = fst >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString fst = first.asString();
			QString snd = second.asString();
			bool result = fst >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QString fst = first.asString();
			QChar snd = second.asCharect();
			bool result = fst >= QString(snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QString fst = first.asString();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fst >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst >= QString(sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar fst = first.asCharect();
			QString snd = second.asString();
			bool result = QString(fst) >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar fst = first.asCharect();
			QChar snd = second.asCharect();
			bool result = fst >= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QChar fst = first.asCharect();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = QString(fst) >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst >= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case bool_or_lit: {

		QString fstv;
		if (first.m_value.type() == QVariant::String) {
			fstv = first.m_value.toString();
		}
		if (first.m_value.type() == QVariant::Char) {
			fstv = QString(first.m_value.toChar());
		}

		if (second.type == kumString) {
			QString snd = second.asString();
			bool result = fstv >= snd;
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar snd = second.asCharect();
			bool result = fstv >= QString(snd);
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fstv >= sndv;
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fstv >= QString(sndv);
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() >= snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() >= ((double)snd);
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = ((double)(first.m_value.toInt())) >= snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() >= snd;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() >= sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() >= ((double)sndv);
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = ((double)(first.m_value.toInt())) >= sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() >= sndv;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;


	case kumBoolean:
		return KumValueStackElem(first.asBool() == second.asBool());
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};

KumValueStackElem KumValueStackElem::LEQ(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(false);
	}
	if (!second.ready) {
		return KumValueStackElem(false);
	}
	qDebug() << "second type" << second.type << "second ready" << second.ready << "second error=" << second.Error();
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			int fst = first.asInt();
			int snd = second.asInt();
			bool result = fst <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			int fst = first.asInt();
			double snd = second.asFloat();
			bool result = ((double)fst) <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = ((double)fst) <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if (second.type == integer) {
			double fst = first.asFloat();
			int snd = second.asInt();
			bool result = fst <= ((double)snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double fst = first.asFloat();
			double snd = second.asFloat();
			bool result = fst <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst <= ((double)sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = fst <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString fst = first.asString();
			QString snd = second.asString();
			bool result = fst <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QString fst = first.asString();
			QChar snd = second.asCharect();
			bool result = fst <= QString(snd);
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QString fst = first.asString();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fst <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst <= QString(sndv);
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case charect: {
		if (second.type == kumString) {
			QChar fst = first.asCharect();
			QString snd = second.asString();
			bool result = QString(fst) <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar fst = first.asCharect();
			QChar snd = second.asCharect();
			bool result = fst <= snd;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {
			QChar fst = first.asCharect();

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = QString(fst) <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fst <= sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case bool_or_lit: {

		QString fstv;
		if (first.m_value.type() == QVariant::String) {
			fstv = first.m_value.toString();
		}
		if (first.m_value.type() == QVariant::Char) {
			fstv = QString(first.m_value.toChar());
		}

		if (second.type == kumString) {
			QString snd = second.asString();
			bool result = fstv <= snd;
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == charect) {
			QChar snd = second.asCharect();
			bool result = fstv <= QString(snd);
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_lit) {

			if (second.m_value.type() == QVariant::String) {
				QString sndv = second.m_value.toString();
				bool result = fstv <= sndv;
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Char) {
				QChar sndv = second.m_value.toChar();
				bool result = fstv <= QString(sndv);
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() <= snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() <= ((double)snd);
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = ((double)(first.m_value.toInt())) <= snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = first.m_value.toDouble() <= snd;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() <= sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() <= ((double)sndv);
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = ((double)(first.m_value.toInt())) <= sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = first.m_value.toDouble() <= sndv;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;


	case kumBoolean:
		return KumValueStackElem(first.asBool() == second.asBool());
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};


KumValueStackElem KumValueStackElem::Pow(KumValueStackElem first, KumValueStackElem second, int *err)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (second.Error() > 0) {
		return KumValueStackElem(dummy, second.Error());
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			double FT = first.asFloat();
			double ST = second.asFloat();
			if (ST < 0) {
				*err = RUN_INTPOWER_LESS_ZERO;
				return KumValueStackElem(0);
			}
			if ((FT < 0.000001) && (FT > -0.000001) && (ST < 0.000001) && (ST > -0.000001)) {
				*err = RUN_NO_VALUE;
				return KumValueStackElem(0);
			};
			double test = pow(FT, ST);
			int result;

			if (!IntegerOverflowChecker::checkPower(first.asInt(), second.asInt())) {
				*err = CALC_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				result = (int)test;
			}
			return KumValueStackElem(result);
			//              return KumValueStackElem ( ( double ) pow ( first.asInt(),second.asInt() ) );
		}
		if (second.type == real) {
			double FT = first.asFloat();
			double ST = second.asFloat();
			double c = ceil(ST); // целая часть ST
			if (fabs(ST - c) > 0.) {
				// если показатель дробный
				if (FT < 0.) {
					*err = RUN_FRACTIONAL_POWER;
					return KumValueStackElem(0.);
				}
			}

			if ((FT < 0.000001) && (FT > -0.000001) && (ST < 0.000001) && (ST > -0.000001)) {
				*err = RUN_NO_VALUE;
				return KumValueStackElem(0);
			};

			double result = pow(FT, ST);
			//              if ( (result>1E30) || (result<(-1E30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			//              return KumValueStackElem(result);
			if (!DoubleOverflowChecker::checkPower(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			} else {
				return KumValueStackElem(result);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		if ((second.type == integer) || (second.type == real)) {
			double FT = first.asFloat();
			double ST = second.asFloat();
			double c = ceil(ST); // целая часть ST
			if (fabs(ST - c) > 0.) {
				// если показатель дробный
				if (FT < 0.) {
					*err = RUN_FRACTIONAL_POWER;
					return KumValueStackElem(0.);
				}
			}
			if ((FT < 0.000001) && (FT > -0.000001) && (ST < 0.000001) && (ST > -0.000001)) {
				*err = RUN_NO_VALUE;
				return KumValueStackElem(0);
			};
			double result = pow(FT, ST);
			//              if ( (result>10e30) || (result<(-10e30)) )
			//              {
			//                  *err=CALC_FLOAT_OVERFLOW;
			//                  result = 0;
			//              }
			if (!DoubleOverflowChecker::checkPower(first.asFloat(), second.asFloat())) {
				*err = CALC_FLOAT_OVERFLOW;
				return KumValueStackElem(0);
			}
			return KumValueStackElem(result);
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if ((second.type == kumString)) {
			return KumValueStackElem(first.asString() + second.asString());
		}
		return KumValueStackElem(false);
	};
	break;


	case kumBoolean:
		return KumValueStackElem(false);
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};



KumValueStackElem KumValueStackElem::NotEq(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	if (!second.ready) {
		return KumValueStackElem("INTERNAL ERROR:KumValueStackElem not ready");
	}
	switch (first.type) {
	case dummy: {
		return KumValueStackElem(dummy, first.Error());
	};
	case integer: {
		if (second.type == integer) {
			return KumValueStackElem(first.asInt() != second.asInt(), second.asInt());
		}
		if (second.type == real) {
			double fst = (double)(first.asInt());
			double snd = second.asFloat();
			return KumValueStackElem(fabs(fst - snd) > REAL_COMPARE_PRECISION, second.asFloat());
		}
		if (second.type == bool_or_num) {
			int fst = first.asInt();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fst != sndv;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toInt();
				double fstv = ((double)fst);
				double diff = fabs(fstv - sndv);
				bool result = diff > REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;

	case real: {
		double fst = first.asFloat();

		if (second.type == integer) {
			double snd = (double)(second.asInt());
			return KumValueStackElem(fabs(fst - snd) > REAL_COMPARE_PRECISION, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			double diff = fabs(fst - snd);
			return KumValueStackElem(diff > REAL_COMPARE_PRECISION, snd);
		}
		if (second.type == bool_or_num) {
			double fst = first.asFloat();

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = fabs(fst - ((double)sndv)) > REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				double fstv = fst;
				Q_UNUSED(fstv);
				double diff = fabs(fst - sndv);
				bool result = diff > REAL_COMPARE_PRECISION;
				result = result && second.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		if (second.type == kumString) {
			QString firsts = first.asString();
			QString seconfs = second.asString();
			bool result = firsts != seconfs;
			return KumValueStackElem(result);
		}
		if (second.type == charect) {
			QString firsts = first.asString();
			QString seconfs = QString(second.asCharect());
			bool result = firsts != seconfs;
			return KumValueStackElem(result);
		}
		return KumValueStackElem(false);
	};
	break;
	case charect: {
		if (second.type == kumString) {
			QString firsts = QString(first.asCharect());
			QString seconfs = second.asString();
			bool result = firsts != seconfs;
			return KumValueStackElem(result);
		}
		if (second.type == charect) {
			QChar firsts = first.asCharect();
			QChar seconfs = second.asCharect();
			bool result = firsts != seconfs;
			return KumValueStackElem(result);
		}
		return KumValueStackElem(false);
	};
	break;
	case bool_or_num: {


		if (second.type == integer) {
			int snd = second.asInt();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = first.m_value.toInt() != snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = fabs(first.m_value.toDouble() - (static_cast<double>(snd))) > REAL_COMPARE_PRECISION;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == real) {
			double snd = second.asFloat();
			bool result = false;
			if (first.m_value.type() == QVariant::Int) {
				result = (static_cast<double>(first.m_value.toInt())) != snd;
			}
			if (first.m_value.type() == QVariant::Double) {
				result = fabs(first.m_value.toDouble() - snd) > REAL_COMPARE_PRECISION;
			}
			result = result && first.m_boolValue;
			return KumValueStackElem(result, snd);
		}
		if (second.type == bool_or_num) {

			if (second.m_value.type() == QVariant::Int) {
				int sndv = second.m_value.toInt();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = first.m_value.toInt() != sndv;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = fabs(first.m_value.toDouble() - ((double)sndv)) > REAL_COMPARE_PRECISION;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
			if (second.m_value.type() == QVariant::Double) {
				double sndv = second.m_value.toDouble();
				bool result = false;
				if (first.m_value.type() == QVariant::Int) {
					result = fabs((static_cast<double>(first.m_value.toInt())) - sndv) > REAL_COMPARE_PRECISION;
				}
				if (first.m_value.type() == QVariant::Double) {
					result = fabs(first.m_value.toDouble() - sndv) > REAL_COMPARE_PRECISION;
				}
				result = result && second.m_boolValue;
				result = result && first.m_boolValue;
				return KumValueStackElem(result, sndv);
			}
		}
		return KumValueStackElem(false);
	}
	break;

	case kumBoolean:
		if (second.type == kumBoolean) {
			return KumValueStackElem(first.asBool() != second.asBool());
		} else {
			return KumValueStackElem("INTERNAL ERROR:KumValueStackElem bad oper");
		}
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};


bool KumValueStackElem::And(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return false;
	}
	if (!second.ready) {
		return false;
	}
	switch (first.type) {
	case dummy: {
		return false;
	};
	case integer: {

		return false;
	};
	break;

	case real: {

		return false;
	};
	break;
	case kumString: {
		return false;
	};
	break;


	case kumBoolean: {
		bool first_b = first.asBool();
		bool sec_b = second.asBool();
		Q_UNUSED(sec_b);
		if (!first_b) {
			return false;
		}
		return first.asBool() && second.asBool();
	}
	break;

	case bool_or_num:
		return first.asBool() && second.asBool();
		break;

	case bool_or_lit:
		return first.asBool() && second.asBool();
		break;

	default:
		return false;
		break;
	};



	return false;

};


KumValueStackElem KumValueStackElem::Or(KumValueStackElem first, KumValueStackElem second)
{
	if (!first.ready) {
		return KumValueStackElem(dummy, 2);
	}
	if (!second.ready) {
		return KumValueStackElem(dummy, 2);
	}
	switch (first.type) {
	case dummy: {
		if (second.asBool()) {
			return KumValueStackElem(true);
		} else {
			return KumValueStackElem(dummy, first.Error());
		}
	};
	case integer: {

		return KumValueStackElem(false);
	};
	break;

	case real: {

		return KumValueStackElem(false);
	};
	break;
	case kumString: {
		return KumValueStackElem(false);
	};
	break;


	case kumBoolean:
		if (first.asBool()) {
			return KumValueStackElem(true);
		}
		return KumValueStackElem(first.asBool() || second.asBool());
		break;

	case bool_or_num:
		return KumValueStackElem(first.asBool() || second.asBool());
		break;

	case bool_or_lit:
		return  KumValueStackElem(first.asBool() || second.asBool());
		break;

	default:
		return KumValueStackElem(false);
		break;
	};

	return KumValueStackElem(false);

};

// RUN SETUP

/**
 * Основной конструктор
 * @param Proga_Value_i � збор программы
 * @param functions_i Таблица функций
 * @param symbols_i Таблица величин
 * @param ispolniteli_i Таблица исполнителей
 * @param output_i Окно ввода/вывода
 * @param debugW_i Окно сообщений
 * @param main_i Основное окно
 */
RunSetup::RunSetup(KumModules *modules,

	QTextEdit *output_i,
	QTextEdit *debugW_i,
	MainWindow *main_i)
{
	Modules = modules;
	output = output_i;
	debugW = debugW_i;
	main = main_i;


	mode = Null;
	ready = false;
};

/**
 * Копирующий конструктор
 * @param Setup Откуда копируем
 * @param Mode � ежим работы
 */
RunSetup::RunSetup(RunSetup *Setup, uint Mode)
{
	Modules = Setup->modules();

	output = Setup->Output();
	debugW = Setup->DebugW();
	main = Setup->Main();


	mode = Mode;
	func_mode = Setup->func_mode;
	ready = Setup->isReady();
	start = Setup->start;
	ISP_BASE = Setup->ISP_BASE;
	initMain = Setup->initMainNeeded();
};

RunSetup RunSetup::copy()
{
	RunSetup  rs(this, mode);

	return rs;
}


