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
#include "kum_tables.h"
#include "kum_instrument.h"
#include "programtab.h"
#include "int_proga.h"



QString function_table::nameById(int id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	return l_table[id].name;
}

bool function_table::isRes(int id, int arg_id) const
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < l_table[id].l_arguments.size());
	return l_table[id].l_arguments[arg_id].res;
}

void function_table::setArgType(int id, int arg_id, PeremType argType)
{
	Q_ASSERT(id >= 0);
	Q_ASSERT(id < l_table.size());
	Q_ASSERT(arg_id >= 0);
	Q_ASSERT(arg_id < l_table[id].l_arguments.size());
	l_table[id].l_arguments[arg_id].type = argType;

}

int function_table::append(const QString &name, PeremType returnType, int declPos)
{
	f_table f;
	f.name = name;
	f.proga_str = f.str = declPos;
	f.type = returnType;
	f.ends = 32000;
	l_table << f;
	return l_table.size() - 1;
}

int function_table::appendDummy(const QString &name)
{
	f_table f;
	f.name = name;
	f.type = none;
	f.ends = -1;
	f.str = -1;
	f.proga_str = -1;
	f.doc = -1;
	f.is_dummy = true;
	l_table << f;
	return l_table.size() - 1;
}

bool function_table::isDummy(int id) const
{
	if (id < 0 || id >= l_table.size()) {
		return true;
	} else {
		return l_table[id].is_dummy;
	}
}

void symbol_table::setConstantValue(int id, const QString &cval)
{
	symb_table[id].constantValue = cval;
	symb_table[id].used = true;
}

QList <QList<QVariant> > extract2DArray(const QString &data)
{
	QList< QList <QVariant> > result;
	QList< QVariant > row;
	QStringList rows = data.split(":");
	for (int j = 0; j < rows.count(); j++) {
		row.clear();
		QStringList elems = rows[j].split(".");
		for (int i = 0; i < elems.count(); i++) {
			QByteArray ba = QByteArray::fromBase64(elems[i].toLatin1());
			row << QVariant(QString::fromUtf8(ba));
		}
		result << row;
	}
	return result;
}

QList< QList <QList<QVariant> > > extract3DArray(const QString &data)
{
	QList< QList< QList <QVariant> > > result;
	QList< QVariant > row;
	QList< QList< QVariant > > page;
	QStringList pages = data.split("|");
	for (int k = 0; k < pages.count(); k++) {
		page.clear();
		QStringList rows = pages[k].split(":");
		for (int j = 0; j < rows.count(); j++) {
			row.clear();
			QStringList elems = rows[j].split(".");
			for (int i = 0; i < elems.count(); i++) {
				QByteArray ba = QByteArray::fromBase64(elems[i].toLatin1());
				row << QVariant(QString::fromUtf8(ba));
			}
			page << row;
		}
		result << page;
	}
	return result;
}

QList<QVariant> extract1DArray(const QString &data)
{
	QList<QVariant> result;
	QStringList elems = data.split(".");
	for (int i = 0; i < elems.count(); i++) {
		QByteArray ba = QByteArray::fromBase64(elems[i].toLatin1());
		result << QVariant(QString::fromUtf8(ba));
	}
	return result;
}

int symbol_table::prepareConstValue(int var_id)
{
	int dim = symb_table[var_id].razm;
	QString s_data = symb_table[var_id].constantValue;
	if (s_data.isEmpty()) {
		return 0;
	}
	symbol_table *st = this;
	PeremType baseType = none;
	switch (symb_table[var_id].type) {
	case mass_integer:
		baseType = integer;
		break;
	case mass_real:
		baseType = real;
		break;
	case mass_bool:
		baseType = kumBoolean;
		break;
	case mass_charect:
		baseType = charect;
		break;
	case mass_string:
		baseType = kumString;
		break;
	default:
		return RUN_CONST_ARRAY_OUT_OF_BOUNDS;
		break;
	}

	int x_max = st->symb_table[var_id].size0_end - st->symb_table[var_id].size0_start + 1;
	int y_max = st->symb_table[var_id].size1_end - st->symb_table[var_id].size1_start + 1;
	int z_max = st->symb_table[var_id].size2_end - st->symb_table[var_id].size2_start + 1;
	st->symb_table[var_id].value.init(0,
		st->symb_table[var_id].type,
		st->symb_table[var_id].size0_start,
		st->symb_table[var_id].size0_end,
		st->symb_table[var_id].size1_start,
		st->symb_table[var_id].size1_end,
		st->symb_table[var_id].size2_start,
		st->symb_table[var_id].size2_end,
		dim
	);
	st->symb_table[var_id].ready = true;
	if (dim == 1) {
		QList<QVariant> data = extract1DArray(s_data);
		QVariant value;
		int y = 0;
		int z = 0;
		for (int x = 0; x < data.count(); x++) {
			if (x >= x_max) {
				return RUN_CONST_ARRAY_OUT_OF_BOUNDS;

			}
			value = data[x];
			if (baseType == integer) {
				st->symb_table[var_id].value.setIntMassValue(value.toInt(), x, y, z);
			} else if (baseType == real) {
				st->symb_table[var_id].value.setFloatMassValue(value.toDouble(), x, y, z);
			} else if (baseType == kumBoolean) {
				st->symb_table[var_id].value.setBoolMassValue(value.toBool(), x, y, z);
			} else if (baseType == charect) {
				st->symb_table[var_id].value.setCharMassValue(value.toChar(), x, y, z);
			} else if (baseType == kumString) {
				st->symb_table[var_id].value.setStringMassValue(value.toString(), x, y, z);
			}
		}
	}
	if (dim == 2) {
		QList< QList<QVariant> > data = extract2DArray(s_data);
		QVariant value;
		int z = 0;
		for (int y = 0; y < data.count(); y++) {
			for (int x = 0; x < data[y].count(); x++) {
				if (x >= x_max || y >= y_max) {
					return RUN_CONST_ARRAY_OUT_OF_BOUNDS;
				}
				value = data[y][x];
				if (baseType == integer) {
					st->symb_table[var_id].value.setIntMassValue(value.toInt(), x, y, z);
				} else if (baseType == real) {
					st->symb_table[var_id].value.setFloatMassValue(value.toDouble(), x, y, z);
				} else if (baseType == kumBoolean) {
					st->symb_table[var_id].value.setBoolMassValue(value.toBool(), x, y, z);
				} else if (baseType == charect) {
					st->symb_table[var_id].value.setCharMassValue(value.toChar(), x, y, z);
				} else if (baseType == kumString) {
					st->symb_table[var_id].value.setStringMassValue(value.toString(), x, y, z);
				}
			}
		}
	}
	if (dim == 3) {
		QList< QList< QList<QVariant> > > data = extract3DArray(s_data);
		QVariant value;
		for (int z = 0; z < data.size(); z++) {
			for (int y = 0; y < data[z].count(); y++) {
				for (int x = 0; x < data[z][y].count(); x++) {
					if (x >= x_max || y >= y_max || z >= z_max) {
						return RUN_CONST_ARRAY_OUT_OF_BOUNDS;
					}
					value = data[z][y][x].toInt();
					if (baseType == integer) {
						st->symb_table[var_id].value.setIntMassValue(value.toInt(), x, y, 0);
					} else if (baseType == real) {
						st->symb_table[var_id].value.setFloatMassValue(value.toDouble(), x, y, 0);
					} else if (baseType == kumBoolean) {
						st->symb_table[var_id].value.setBoolMassValue(value.toBool(), x, y, 0);
					} else if (baseType == charect) {
						st->symb_table[var_id].value.setCharMassValue(value.toChar(), x, y, 0);
					} else if (baseType == kumString) {
						st->symb_table[var_id].value.setStringMassValue(value.toString(), x, y, 0);
					}
				}
			}
		}
	}
	return 0;
}


//---KumSingleModule

//---KumModules


bool symbol_table::isUsed(uint id)
{
	if ((uint) symb_table.size() <= id) {
		return false;
	}
	if (
		(symb_table[id].type == mass_bool) ||
		(symb_table[id].type == mass_integer) ||
		(symb_table[id].type == mass_real) ||
		(symb_table[id].type == mass_string) ||
		(symb_table[id].type == mass_charect)) {
		return true;
	}
	return symb_table[id].used;
}

bool symbol_table::isReady(uint id, int x, int y, int z)
{
	bool valueReady = symb_table[id].ready;
	if (!valueReady) {
		return false;
	}
	bool valueElementReady = symb_table[id].value.isReady(x, y, z);
	return valueElementReady;
}
