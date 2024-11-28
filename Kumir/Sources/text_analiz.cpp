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

#include "text_analiz.h"
#include "error.h"
#include "tools.h"
#include "kassert.h"
#include "config.h"

#include <cmath>
#include "integeroverflowchecker.h"
#include "doubleoverflowchecker.h"
#include "kumsinglemodule.h"

#include "config.h"

#include <QStack>


int find_viraj_Func(QStringList virajList, QList<PeremType> *virajTypes, function_table *table, KumModules *modules, QString func, int *err)
{
	//TODO ne FUNCT
	QString func_name = func.left(func.indexOf('&'));
	if (func.indexOf('&') < 0) {
		func_name = func;
	}
	int func_id = -1;
	int module_id = -1;
	modules->FuncByName(func_name, &module_id, &func_id);
	if (func_id < 0) {
		*err = UNKN_ALG;
		return -1;
	};

	QString args_id = func.mid(func.indexOf('&') + 1);
	QString args = virajList[args_id.toInt()];
	QStringList arg_List = args.split(",", QString::SkipEmptyParts);
	int otl = arg_List.count();
	// int dbg=table->func_table[func_id].nperem;
	if (func_id == -1) {
		return 2;
	}
	if (otl != table->argCountById(func_id)) {
		*err = PARAM_COUNT_ERR;
		return -1;
	};

	for (int i = 0; i < otl; i++) {
		int id;
		QString tmp = arg_List[i].mid(1);
		id = tmp.toInt();
		PeremType perType;
		QList<PeremType> types = *virajTypes;
		if (otl == 1) {
			perType = types[args_id.toInt()];
		} else {
			perType = types[id];
		}
		//        PeremType log=table->func_table[func_id].perems[i];
		if (table->argTypeById(func_id, i) != perType) {
			*err = PARAM_TYPE_ERR;
			return -1;
		};

	};

	return func_id;
};

/**
 * Проверка константы
 * @param name текст предполагаемой константы
 * @return Тип константы (none если не удалось определить)
 */
PeremType testConst(QString name, int *err)
{
	name = name.trimmed();
	if (name.startsWith("-") || name.startsWith("+")) {
		name = name.mid(1);
	}
	QString debug = name;
	bool ok;
	int base = 10;
	*err = 0;
	int iResult = 0;
	if (KumTools::instance()->isBooleanConstant(name)) {
		return kumBoolean;
	}
	bool isLit = name.startsWith("\"") && name.endsWith("\"");
	if (!isLit) {
		if (!name.startsWith("$")) {
			name.replace(QString::fromUtf8("Е"), "E");
			name.replace(QString::fromUtf8("е"), "E");
		}
		name.replace("e", "E");


		if (name.count("E") && !name.startsWith("$")) {
			if ((name.count("E+") == 0) && (name.count("E-") == 0)) {
				name.replace("E", "E+");
			}
		}



		if (name.startsWith("$")) {
			QRegExp rxHex = QRegExp("\\$[0-9A-F]+");
			if (!rxHex.exactMatch(name.simplified())) {
				*err = GNVR_BAD_HEX_CONST;
				return none;
			}
			base = 16;
			name = name.mid(1, name.length() - 1);
			if (name.count(".") || name.count("e+") || name.count("E+")) {
				*err = TN_BAD_NAME_1;
				return none;
			}
			if (!IntegerOverflowChecker::checkFromString("$" + name)) {
				*err = GNVR_BIG_INT_CONST;
				return none;
			}
			iResult = name.toInt(&ok, base);

			//          if ( ok &&
			//                          ( ( iResult < -1073741824 ) || ( iResult > 1073741824 ) )
			//               )
			//          {
			//              *err = GNVR_BIG_INT_CONST;
			//          }
			//
			if (ok) {
				return integer;
			} else {
				*err = GNVR_BIG_INT_CONST;
				return none;
			}
		};
		if (name.startsWith("0x")) {
			base = 16;
			if (name.count(".") || name.count("e+") || name.count("E+")) {
				*err = TN_BAD_NAME_1;
				return none;
			}
			if (!IntegerOverflowChecker::checkFromString(name.simplified())) {
				*err = GNVR_BIG_INT_CONST;
				return none;
			}
		}
		QString tmp = name.simplified();
		ok = true;
		for (int i = 0; i < tmp.length(); i++) {
			if (!tmp[i].isDigit()) {
				if (tmp[i] == '-' && i == 0) {
					continue;
				} else {
					ok = false;
					break;
				}
			}
		}

		if (ok) {
			if (!IntegerOverflowChecker::checkFromString(tmp)) {
				//                *err = GNVR_BIG_INT_CONST;
				//                return none;
			} else {
				return integer;
			}
		}
		//      iResult = name.toInt ( &ok,base );
		//
		//      if ( ok &&
		//                  ( ( iResult < -1073741824 ) || ( iResult > 1073741824 ) )
		//          )
		//      {
		//          *err = GNVR_BIG_INT_CONST;
		//      }


		bool possible_double = true;
		bool dot_found = false;
		bool e_found = false;
		for (int i = 0; i < name.length(); i++) {
			if (name[i].isDigit()) {
				continue;
			} else if (name[i] == '-' || name[i] == '+') {
				if (i > 0) {
					if (e_found) {
						possible_double = false;
						break;
					}
					if (name[i - 1] == 'E') {
						e_found = true;
						continue;
					}
				} else {
					possible_double = false;
					break;
				}
			} else if (name[i] == 'E') {
				if (e_found) {
					possible_double = false;
					break;
				} else {
					continue;
				}
			} else if (name[i] == '.') {
				if (dot_found) {
					possible_double = false;
					break;
				} else {
					dot_found = true;
				}
			} else {
				possible_double = false;
				break;
			}
		}

		if (ok || possible_double) {
			if (!DoubleOverflowChecker::checkFromString(tmp)) {
				*err = GNVR_BIG_FLOAT_CONST;
				return none;
			} else {
				return real;
			}
			//          if (
			//                  ( name.toLower() != "inf" )
			//                  &&
			//                  ( name.toLower() != "nan" )
			//          )
			//          {
			//              if ( ok &&
			//                              ( ( fResult < -1e+30 ) || ( fResult > 1e+30 ) )
			//                  )
			//              {
			//                  *err = GNVR_BIG_FLOAT_CONST;
			//              }
			//              return real;
			//          }
		}
	}

	// NEW V.Y. #4.3 02.08.2006
	if ((name.startsWith("\"") && (name.endsWith("\"")))) {
		if (name.length() == 3) {
			return charect;
		} else {
			return kumString;
		}
	}
	// end NEW #4.3
	if ((name.startsWith("\"") && (!name.endsWith("\"")))) {
		return none;
	}

	if (name == QChar(KS_DA)) {
		return kumBoolean;
	}
	if (name == QChar(KS_NET)) {
		return kumBoolean;
	}

	return none;
};

PeremType text_analiz::resType(
	int *err,
	PeremType type1,
	PeremType type2,
	QChar oper
)
{
	*err = 0;
	if (oper == QChar(KS_PRISV)) {
		*err = RESTP_ASSIGN_INSTEAD_EQ;
		return none;
	}
	if (type1 == none) {
		if (type2 == integer) {
			if (oper == QChar(UNARY_MINUS)) {
				return integer;
			}
			if (oper == QChar(UNARY_PLUS)) {
				return integer;
			}
			*err = RESTP_UNARY_INT;
			return none;
		}

		if (type2 == real) {
			if (oper == QChar(UNARY_MINUS)) {
				return real;
			}
			if (oper == QChar(UNARY_PLUS)) {
				return real;
			}
			*err = RESTP_UNARY_REAL;
			return none;
		}

		if (type2 == kumBoolean) {
			if (oper == QChar(KS_NOT)) {
				return kumBoolean;
			}
			*err = RESTP_UNARY_BOOL;
			return none;
		}



		if (type2 == bool_or_num) {
			if (oper == QChar(KS_NOT)) {
				return kumBoolean;
			}
			*err = RESTP_UNARY_BOOL;
			return none;
		}

		if (type2 == bool_or_lit) {
			if (oper == QChar(KS_NOT)) {
				return kumBoolean;
			}
			*err = RESTP_UNARY_BOOL;
			return none;
		}

		*err = RESTP_UNARY;
		return none;
	}

	if (type1 == integer) {
		if (type2 == integer) {
			if (oper == '+') {
				return integer;
			}
			if (oper == '-') {
				return integer;
			}
			if (oper == '*') {
				return integer;
			}
			if (oper == '/') {
				return real;
			}
			if (oper.unicode() == KS_STEPEN) {
				return integer;
			}

			if (oper == '>')

			{
				return bool_or_num;
			}
			if (oper == '<')

			{
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}


			if (oper.unicode() == KS_NE_RAVNO)

			{
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO)

			{
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO)

			{
				return bool_or_num;
			}

			if (oper.unicode() == KS_OR) {
				*err = RESTP_INT_INT_OR;
				return none;
			};
			if (oper.unicode() == KS_AND) {
				*err = RESTP_INT_INT_AND;
				return none;
			};
			if (oper.unicode() == KS_XOR) {
				*err = RESTP_INT_INT_XOR;
				return none;
			}
		}

		if (type2 == real) {
			if (oper == '+') {
				return real;
			}
			if (oper == '-') {
				return real;
			}
			if (oper == '*') {
				return real;
			}
			if (oper == '/') {
				return real;
			}
			if (oper == QChar(KS_STEPEN)) {
				return real;
			}

			if (oper.unicode() == KS_NE_RAVNO)

			{
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO)

			{
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO)

			{
				return bool_or_num;
			}

			if (oper == '>')

			{
				return bool_or_num;
			}
			if (oper == '<')

			{
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_OR) {
				*err = RESTP_INT_REAL_OR;
				return none;
			};
			if (oper.unicode() == KS_AND) {
				*err = RESTP_INT_REAL_AND;
				return none;
			};
			if (oper.unicode() == KS_XOR) {
				*err = RESTP_INT_REAL_XOR;
				return none;
			}

		}

		if (type2 == kumBoolean) {
			if (oper == '+') {
				*err = RESTP_INT_BOOL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_INT_BOOL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_INT_BOOL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_INT_BOOL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_INT_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_INT_BOOL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_INT_BOOL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_INT_BOOL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_INT_BOOL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_INT_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_INT_BOOL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_INT_BOOL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_INT_BOOL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_INT_BOOL_AND;
			}
			return none;
		}

		if (type2 == charect) {
			if (oper == '+') {
				*err = RESTP_INT_CHAR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_INT_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_INT_CHAR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_INT_CHAR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_INT_CHAR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_INT_CHAR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_INT_CHAR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_INT_CHAR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_INT_CHAR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_INT_CHAR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_INT_CHAR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_INT_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_INT_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_INT_CHAR_AND;
			}
			return none;
		}

		if (type2 == kumString) {
			if (oper == '+') {
				*err = RESTP_INT_STR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_INT_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_INT_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_INT_STR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_INT_STR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_INT_STR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_INT_STR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_INT_STR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_INT_STR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_INT_STR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_INT_STR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_INT_STR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_INT_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_INT_STR_AND;
			}
			return none;
		}

		if (type2 == bool_or_lit) {
			*err = INT_TA_14;
			if (oper == '>') {
				*err = RESTP_INT_BL_MORE;
			}
			if (oper == '<') {
				*err = RESTP_INT_BL_LESS;
			}
			if (oper == '=') {
				*err = RESTP_INT_BL_EQ;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				*err = RESTP_INT_BL_NEQ;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				*err = RESTP_INT_BL_GEQ;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				*err = RESTP_INT_BL_LEQ;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_INT_BL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_INT_BL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_INT_BL_AND;
			}

			return none;
		}

		if (type2 == bool_or_num) {
			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_num;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_INT_BN_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_INT_BN_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_INT_BN_AND;
			}

			return none;
		}

		*err = RESTP_INT;
		return none;

	} // end if (type1==integer)


	if (type1 == kumBoolean) {
		if (type2 == kumBoolean) {
			if (oper.unicode() == KS_OR) {
				return kumBoolean;
			};
			if (oper.unicode() == KS_AND) {
				return kumBoolean;
			};
			if (oper.unicode() == KS_XOR) {
				return kumBoolean;
			}
			if (oper.unicode() == KS_NE_RAVNO) {
				return kumBoolean;
			}
			if (oper == '=') {
				return kumBoolean;
			}
			if (oper == '+') {
				*err = RESTP_BOOL_BOOL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BOOL_BOOL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BOOL_BOOL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BOOL_BOOL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BOOL_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BOOL_BOOL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BOOL_BOOL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BOOL_BOOL_POW;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BOOL_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BOOL_BOOL_GEQ;
			}

			return none;

		}
		if (type2 == integer) {
			if (oper == '+') {
				*err = RESTP_BOOL_INT_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BOOL_INT_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BOOL_INT_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BOOL_INT_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BOOL_INT_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BOOL_INT_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BOOL_INT_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BOOL_INT_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BOOL_INT_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BOOL_INT_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BOOL_INT_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BOOL_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BOOL_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BOOL_INT_AND;
			}
			return none;
		}
		if (type2 == real) {
			if (oper == '+') {
				*err = RESTP_BOOL_REAL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BOOL_REAL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BOOL_REAL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BOOL_REAL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BOOL_REAL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BOOL_REAL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BOOL_REAL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BOOL_REAL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BOOL_REAL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BOOL_REAL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BOOL_REAL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BOOL_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BOOL_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BOOL_REAL_AND;
			}
			return none;
		}
		if (type2 == charect) {
			if (oper == '+') {
				*err = RESTP_BOOL_CHAR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BOOL_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BOOL_CHAR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BOOL_CHAR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BOOL_CHAR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BOOL_CHAR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BOOL_CHAR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BOOL_CHAR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BOOL_CHAR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BOOL_CHAR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BOOL_CHAR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BOOL_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BOOL_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BOOL_CHAR_AND;
			}
			return none;
		}
		if (type2 == kumString) {
			if (oper == '+') {
				*err = RESTP_BOOL_STR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BOOL_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BOOL_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BOOL_STR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BOOL_STR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BOOL_STR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BOOL_STR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BOOL_STR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BOOL_STR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BOOL_STR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BOOL_STR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BOOL_STR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BOOL_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BOOL_STR_AND;
			}
			return none;
		}
		if (type2 == bool_or_lit) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			if (oper == QChar('=')) {
				return kumBoolean;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				return kumBoolean;
			}
			*err = RESTP_BOOL;
			return none;
		}

		if (type2 == bool_or_num) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			if (oper == QChar('=')) {
				return kumBoolean;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				return kumBoolean;
			}
			*err = RESTP_BOOL;
			return none;
		}
		*err = RESTP_BOOL;
		return none;
	} // end if (type1==kumBoolean)

	if (type1 == real) {
		if (type2 == integer) {
			if (oper == '+') {
				return real;
			}
			if (oper == '-') {
				return real;
			}
			if (oper == '*') {
				return real;
			}
			if (oper == '/') {
				return real;
			}
			if (oper.unicode() == KS_STEPEN) {
				return real;
			}

			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return kumBoolean;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return kumBoolean;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return kumBoolean;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_INT_AND;
			}
			return none;

		}

		if (type2 == real) {
			if (oper == '+') {
				return real;
			}
			if (oper == '-') {
				return real;
			}
			if (oper == '*') {
				return real;
			}
			if (oper == '/') {
				return real;
			}
			if (oper.unicode() == KS_STEPEN) {
				return real;
			}

			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_num;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_REAL_AND;
			}
			return none;

		}

		if (type2 == kumString) {
			if (oper == '+') {
				*err = RESTP_REAL_STR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_REAL_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_REAL_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_REAL_STR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_REAL_STR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_REAL_STR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_REAL_STR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_REAL_STR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_REAL_STR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_REAL_STR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_REAL_STR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_STR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_STR_AND;
			}
			return none;
		}

		if (type2 == charect) {
			if (oper == '+') {
				*err = RESTP_REAL_CHAR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_REAL_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_REAL_CHAR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_REAL_CHAR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_REAL_CHAR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_REAL_CHAR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_REAL_CHAR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_REAL_CHAR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_REAL_CHAR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_REAL_CHAR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_REAL_CHAR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_CHAR_AND;
			}
			return none;
		}

		if (type2 == kumBoolean) {
			if (oper == '+') {
				*err = RESTP_REAL_BOOL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_REAL_BOOL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_REAL_BOOL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_REAL_BOOL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_REAL_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_REAL_BOOL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_REAL_BOOL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_REAL_BOOL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_REAL_BOOL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_REAL_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_REAL_BOOL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_BOOL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_BOOL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_BOOL_AND;
			}
			return none;
		}

		if (type2 == bool_or_lit) {
			*err = INT_TA_14;
			if (oper == '>') {
				*err = RESTP_REAL_BL_MORE;
			}
			if (oper == '<') {
				*err = RESTP_REAL_BL_LESS;
			}
			if (oper == '=') {
				*err = RESTP_REAL_BL_EQ;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				*err = RESTP_REAL_BL_NEQ;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				*err = RESTP_REAL_BL_GEQ;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				*err = RESTP_REAL_BL_LEQ;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_BL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_BL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_BL_AND;
			}

			return none;
		}

		if (type2 == bool_or_num) {
			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_num;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_REAL_BN_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_REAL_BN_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_REAL_BN_AND;
			}

			return none;
		}

		*err = RESTP_REAL;
		return none;

	} // end if (type1==real)

	if (type1 == kumString) {
		if (type2 == kumString) {
			if (oper == '+') {
				return kumString;
			}
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}

			if (oper == '-') {
				*err = RESTP_STR_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_STR_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_STR_STR_DIV;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_STR_OR;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_STR_STR_POW;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_STR_AND;
			}
			return none;
		}
		if (type2 == charect) {
			if (oper == '+') {
				return kumString;
			}
			if (oper == '>') {
				return kumBoolean;
			}
			if (oper == '<') {
				return kumBoolean;
			}
			if (oper == '=') {
				return kumBoolean;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return kumBoolean;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return kumBoolean;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return kumBoolean;
			}

			if (oper == '-') {
				*err = RESTP_STR_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_STR_CHAR_MUL;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_STR_CHAR_POW;
			}
			if (oper == '/') {
				*err = RESTP_STR_CHAR_DIV;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_CHAR_AND;
			}
			return none;
		}
		if (type2 == kumBoolean) {
			if (oper == '+') {
				*err = RESTP_STR_BOOL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_STR_BOOL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_STR_BOOL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_STR_BOOL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_STR_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_STR_BOOL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_STR_BOOL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_STR_BOOL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_STR_BOOL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_STR_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_STR_BOOL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_BOOL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_BOOL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_BOOL_AND;
			}
			return none;
		}
		if (type2 == real) {
			if (oper == '+') {
				*err = RESTP_STR_REAL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_STR_REAL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_STR_REAL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_STR_REAL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_STR_REAL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_STR_REAL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_STR_REAL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_STR_REAL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_STR_REAL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_STR_REAL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_STR_REAL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_REAL_AND;
			}
			return none;
		}
		if (type2 == integer) {
			if (oper == '+') {
				*err = RESTP_STR_INT_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_STR_INT_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_STR_INT_MUL;
			}
			if (oper == '/') {
				*err = RESTP_STR_INT_DIV;
			}
			if (oper == '<') {
				*err = RESTP_STR_INT_LESS;
			}
			if (oper == '>') {
				*err = RESTP_STR_INT_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_STR_INT_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_STR_INT_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_STR_INT_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_STR_INT_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_STR_INT_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_INT_AND;
			}
			return none;
		}

		if (type2 == bool_or_num) {
			*err = INT_TA_14;
			if (oper == '>') {
				*err = RESTP_STR_BL_MORE;
			}
			if (oper == '<') {
				*err = RESTP_STR_BL_LESS;
			}
			if (oper == '=') {
				*err = RESTP_STR_BL_EQ;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				*err = RESTP_STR_BL_NEQ;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				*err = RESTP_STR_BL_GEQ;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				*err = RESTP_STR_BL_LEQ;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_BL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_BL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_BL_AND;
			}

			return none;
		}

		if (type2 == bool_or_lit) {
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_STR_BN_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_STR_BN_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_STR_BN_AND;
			}

			return none;
		}

		*err = RESTP_STRING;
		return none;
	} //string


	if (type1 == charect) {
		if (type2 == kumString) {
			if (oper == '+') {
				return kumString;
			}
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper == '-') {
				*err = RESTP_CHAR_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_CHAR_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_CHAR_STR_DIV;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_STR_OR;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_CHAR_STR_POW;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_STR_AND;
			}
			return none;
		}
		if (type2 == charect) {
			if (oper == '+') {
				return kumString;
			}
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper == '-') {
				*err = RESTP_CHAR_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_CHAR_CHAR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_CHAR_CHAR_DIV;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_CHAR_OR;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_CHAR_CHAR_POW;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_CHAR_AND;
			}
			return none;
		}
		if (type2 == kumBoolean) {
			if (oper == '+') {
				*err = RESTP_CHAR_BOOL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_CHAR_BOOL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_CHAR_BOOL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_CHAR_BOOL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_CHAR_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_CHAR_BOOL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_CHAR_BOOL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_CHAR_BOOL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_CHAR_BOOL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_CHAR_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_CHAR_BOOL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_BOOL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_BOOL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_BOOL_AND;
			}
			return none;
		}
		if (type2 == real) {
			if (oper == '+') {
				*err = RESTP_CHAR_REAL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_CHAR_REAL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_CHAR_REAL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_CHAR_REAL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_CHAR_REAL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_CHAR_REAL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_CHAR_REAL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_CHAR_REAL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_CHAR_REAL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_CHAR_REAL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_CHAR_REAL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_REAL_AND;
			}
			return none;
		}
		if (type2 == integer) {
			if (oper == '+') {
				*err = RESTP_CHAR_INT_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_CHAR_INT_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_CHAR_INT_MUL;
			}
			if (oper == '/') {
				*err = RESTP_CHAR_INT_DIV;
			}
			if (oper == '<') {
				*err = RESTP_CHAR_INT_LESS;
			}
			if (oper == '>') {
				*err = RESTP_CHAR_INT_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_CHAR_INT_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_CHAR_INT_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_CHAR_INT_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_CHAR_INT_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_CHAR_INT_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_INT_AND;
			}
			return none;
		}
		if (type2 == bool_or_num) {
			*err = INT_TA_14;
			if (oper == '>') {
				*err = RESTP_CHAR_BL_MORE;
			}
			if (oper == '<') {
				*err = RESTP_CHAR_BL_LESS;
			}
			if (oper == '=') {
				*err = RESTP_CHAR_BL_EQ;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				*err = RESTP_CHAR_BL_NEQ;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				*err = RESTP_CHAR_BL_GEQ;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				*err = RESTP_CHAR_BL_LEQ;
			}

			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_BL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_BL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_BL_AND;
			}

			return none;
		}

		if (type2 == bool_or_lit) {
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_CHAR_BN_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_CHAR_BN_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_CHAR_BN_AND;
			}

			return none;
		}
		*err = RESTP_CHAR;
		return none;
	} //string

	if (type1 == bool_or_num) {
		if (type2 == integer) {
			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}


			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_num;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BN_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BN_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BN_INT_AND;
			}

			return none;
		} else if (type2 == real) {
			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_num;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_num;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BN_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BN_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BN_REAL_AND;
			}
			return none;
		} else if (type2 == kumString) {
			*err = INT_TA_14;
			if (oper == '+') {
				*err = RESTP_BN_STR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BN_STR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BN_STR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BN_STR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BN_STR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BN_STR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BN_STR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BN_STR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BN_STR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BN_STR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BN_STR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BN_STR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BN_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BN_STR_AND;
			}
			return none;
		} else if (type2 == charect) {
			*err = INT_TA_14;
			if (oper == '+') {
				*err = RESTP_BN_CHAR_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BN_CHAR_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BN_CHAR_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BN_CHAR_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BN_CHAR_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BN_CHAR_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BN_CHAR_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BN_CHAR_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BN_CHAR_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BN_CHAR_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BN_CHAR_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BN_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BN_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BN_CHAR_AND;
			}
			return none;
		} else if (type2 == kumBoolean) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			if (oper == QChar('=')) {
				return kumBoolean;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				return kumBoolean;
			}
			*err = INT_TA_14;
			if (oper == '<') {
				*err = RESTP_BN_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BN_BOOL_GREAT;
			}
			//          if ( oper == '=' )
			//              *err = RESTP_BN_BOOL_EQ;


			//          if ( oper == QChar(KS_NE_RAVNO) )
			//              *err = RESTP_BN_BOOL_NEQ;
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BN_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BN_BOOL_GEQ;
			}
			return none;
		} else if (type2 == bool_or_num) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			if (oper == '<') {
				return bool_or_num;
			}
			if (oper == '>') {
				return bool_or_num;
			}
			if (oper == '=') {
				return bool_or_num;
			}


			if (oper == QChar(KS_NE_RAVNO)) {
				return bool_or_num;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				return bool_or_num;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				return bool_or_num;
			}
			*err = INT_TA_14;
			return none;
		} else if (type2 == bool_or_lit) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			*err = INT_TA_14;
			if (oper == '<') {
				*err = RESTP_BN_BL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BN_BL_MORE;
			}
			if (oper == '=') {
				*err = RESTP_BN_BL_EQ;
			}

			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BN_BL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BN_BL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BN_BL_GEQ;
			}
			return none;
		}
	} // bool_or_num

	if (type1 == bool_or_lit) {
		if (type2 == charect) {
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}


			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BL_CHAR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BL_CHAR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BL_CHAR_AND;
			}

			return none;
		} else if (type2 == kumString) {
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper.unicode() == KS_NE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_BOLSHE_RAVNO) {
				return bool_or_lit;
			}
			if (oper.unicode() == KS_MENSHE_RAVNO) {
				return bool_or_lit;
			}
			*err = INT_TA_14;
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BL_STR_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BL_STR_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BL_STR_AND;
			}
			return none;
		} else if (type2 == real) {
			*err = INT_TA_14;
			if (oper == '+') {
				*err = RESTP_BL_REAL_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BL_REAL_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BL_REAL_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BL_REAL_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BL_REAL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BL_REAL_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BL_REAL_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BL_REAL_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BL_REAL_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BL_REAL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BL_REAL_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BL_REAL_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BL_REAL_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BL_REAL_AND;
			}
			return none;
		} else if (type2 == integer) {
			*err = INT_TA_14;
			if (oper == '+') {
				*err = RESTP_BL_INT_PLUS;
			}
			if (oper == '-') {
				*err = RESTP_BL_INT_MINUS;
			}
			if (oper == '*') {
				*err = RESTP_BL_INT_MUL;
			}
			if (oper == '/') {
				*err = RESTP_BL_INT_DIV;
			}
			if (oper == '<') {
				*err = RESTP_BL_INT_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BL_INT_GREAT;
			}
			if (oper == '=') {
				*err = RESTP_BL_INT_EQ;
			}
			if (oper == QChar(KS_STEPEN)) {
				*err = RESTP_BL_INT_POW;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BL_INT_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BL_INT_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BL_INT_GEQ;
			}
			if (oper == QChar(KS_OR)) {
				*err = RESTP_BL_INT_OR;
			}
			if (oper == QChar(KS_XOR)) {
				*err = RESTP_BL_INT_XOR;
			}
			if (oper == QChar(KS_AND)) {
				*err = RESTP_BL_INT_AND;
			}
			return none;
		} else if (type2 == kumBoolean) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}
			if (oper == QChar('=')) {
				return kumBoolean;
			}
			if (oper == QChar(KS_NE_RAVNO)) {
				return kumBoolean;
			}
			*err = INT_TA_14;
			if (oper == '<') {
				*err = RESTP_BL_BOOL_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BL_BOOL_GREAT;
			}
			//          if ( oper == '=' )
			//              *err = RESTP_BL_BOOL_EQ;

			//          if ( oper == QChar(KS_NE_RAVNO) )
			//              *err = RESTP_BL_BOOL_NEQ;
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BL_BOOL_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BL_BOOL_GEQ;
			}
			return none;
		} else if (type2 == bool_or_num) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}

			*err = INT_TA_14;
			if (oper == '<') {
				*err = RESTP_BL_BN_LESS;
			}
			if (oper == '>') {
				*err = RESTP_BL_BN_MORE;
			}
			if (oper == '=') {
				*err = RESTP_BL_BN_EQ;
			}

			if (oper == QChar(KS_NE_RAVNO)) {
				*err = RESTP_BL_BN_NEQ;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				*err = RESTP_BL_BN_LEQ;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				*err = RESTP_BL_BN_GEQ;
			}
			*err = INT_TA_14;
			return none;
		} else if (type2 == bool_or_lit) {
			if (oper == QChar(KS_OR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_XOR)) {
				return kumBoolean;
			}
			if (oper == QChar(KS_AND)) {
				return kumBoolean;
			}

			if (oper == '<') {
				return bool_or_lit;
			}
			if (oper == '>') {
				return bool_or_lit;
			}
			if (oper == '=') {
				return bool_or_lit;
			}

			if (oper == QChar(KS_NE_RAVNO)) {
				return bool_or_lit;
			}
			if (oper == QChar(KS_MENSHE_RAVNO)) {
				return bool_or_lit;
			}
			if (oper == QChar(KS_BOLSHE_RAVNO)) {
				return bool_or_lit;
			}
			*err = INT_TA_14;
			return none;
		}
	} // bool_or_lit

	*err = RESTP_FINAL;
	return none;
}


QString text_analiz::dropSpace(QString viraj, QList<int> &P, QList<int> &L)//,QList<int> *skobka,QList<int> *sk_type)
{
	QList<int> rP, rL;
	QString to_ret;
	bool inLit = false;
	bool space = false;
	bool first = true;
	for (int i = 0; i < viraj.length(); i++) {
		QChar next;
		if (i < (viraj.length() - 1)) {
			next = viraj[i + 1];
		} else {
			next = ' ';
		}
		if (viraj[i] == ' ' || viraj[i] == '\t') {
			if (
				(((next.unicode() < 1000) && (next.unicode() > 63)) || ((next.unicode() > 1006) && (next.unicode() < 3500)) || (next.isNumber()))
				&& !first
			) {
				to_ret = to_ret + viraj[i];
				rP << P[i];
				rL << L[i];
			}
			//                  if (inLit||!space) {
			//                      to_ret=to_ret+viraj[i];
			//                      rP << P[i];
			//                      rL << L[i];
			//                  }
			//                  space = true;
		} else {
			to_ret = to_ret + viraj[i];
			rP << P[i];
			rL << L[i];
			space = false;
			first = false;
		}
		if (viraj[i] == '"') {
			inLit = !inLit;
		}
	}
	P = rP;
	L = rL;
	return to_ret;
}

int ipow(int base, int power)
{
	int result = 1;
	for (int i = 0; i < power; i++) {
		result *= base;
	}
	return result;
}

int hexValue(QString strValue)
{
	strValue = strValue.toLower();
	int power = 0;
	int result = 0;
	while (!strValue.isEmpty()) {
		QChar c = strValue[strValue.length() - 1];
		int base = 0;
		if (c == '0') {
			base = 0;
		} else if (c == '1') {
			base = 1;
		} else if (c == '2') {
			base = 2;
		} else if (c == '3') {
			base = 3;
		} else if (c == '4') {
			base = 4;
		} else if (c == '5') {
			base = 5;
		} else if (c == '6') {
			base = 6;
		} else if (c == '7') {
			base = 7;
		} else if (c == '8') {
			base = 8;
		} else if (c == '9') {
			base = 9;
		} else if (c == 'a') {
			base = 10;
		} else if (c == 'b') {
			base = 11;
		} else if (c == 'c') {
			base = 12;
		} else if (c == 'd') {
			base = 13;
		} else if (c == 'e') {
			base = 14;
		} else if (c == 'f') {
			base = 15;
		}
		result += base * ipow(16, power);
		power++;
		strValue.truncate(strValue.length() - 1);
	}
	return result;
}

QVariant text_analiz::createConstValue(const QString &str, const PeremType pt)
{
	Q_ASSERT(!str.isEmpty());
	QVariant result;
	if (pt == integer) {
		int base = 10;
		bool ok;
		QString const_value = str;
		if (const_value.startsWith("$")) {
			base = 16;
			const_value = const_value.mid(1, const_value.length() - 1);
		}
		if (const_value.startsWith("0x")) {
			base = 16;
		}
		int int_value = const_value.toInt(&ok, base);
		if (base == 16) {
			int_value = hexValue(const_value);
		}
		result = QVariant(int_value);
	} else if (pt == real) {
		QString v = str;
		v.replace(QString::fromUtf8("Е"), "E");
		v.replace(QString::fromUtf8("е"), "e");
		v.replace("e", "E");
		bool ok;
		double d = v.toDouble(&ok);
		result = QVariant(d);
	} else if (pt == kumString) {
		QString normStr = str.trimmed();
		QString ready_const;
		for (int i = 1; i < normStr.length() - 1; i++) {
			ready_const += QChar(normStr[i].unicode() - STR_HIDE_OFFSET);
		}
		result = QVariant(ready_const);
	} else if (pt == charect) {
		QString ready_const;
		for (int i = 1; i < str.length() - 1; i++) {
			ready_const += QChar(str[i].unicode() - STR_HIDE_OFFSET);
		}
		Q_ASSERT(ready_const.length() == 1);
		result = QVariant(ready_const[0]);
	} else if (pt == kumBoolean) {
		QString yes = QString(QChar(KS_DA));
		QString no = QString(QChar(KS_NET));
		Q_ASSERT(str == yes || str == no);
		if (str == no) {
			result = QVariant(false);
		}
		if (str == yes) {
			result = QVariant(true);
		}
	} else {
		qFatal("Trying to create constant of non-regular type");
	}
	return result;
}

int set_const_value(int perem_id, const QString &val, symbol_table *symbols)
{
	QString const_value = val;

	if (const_value.isEmpty()) {
		symbols->symb_table[perem_id].value.setIntegerValue(0);
		symbols->setUsed(perem_id, true);
		return 0;
	}
	if (symbols->getTypeByID(perem_id) == integer) {
		int int_value = text_analiz::createConstValue(val, integer).toInt();
		symbols->symb_table[perem_id].value.setIntegerValue(int_value);
		symbols->setUsed(perem_id, true);
	}


	if (symbols->getTypeByID(perem_id) == real) {
		double d = text_analiz::createConstValue(val, real).toDouble();
		symbols->symb_table[perem_id].value.setFloatValue(d);
		symbols->setUsed(perem_id, true);
	}

	if (symbols->getTypeByID(perem_id) == kumString) {
		QString s = text_analiz::createConstValue(val, kumString).toString();
		symbols->symb_table[perem_id].value.setStringValue(s);
		symbols->setUsed(perem_id, true);
	}

	// NEW 28.08.2006
	if (symbols->getTypeByID(perem_id) == kumBoolean) {
		bool value = text_analiz::createConstValue(val, kumBoolean).toBool();
		symbols->symb_table[perem_id].value.setBoolValue(value);
		symbols->setUsed(perem_id, true);
	}
	if (symbols->getTypeByID(perem_id) == charect) {
		QChar c = text_analiz::createConstValue(val, charect).toChar();
		symbols->symb_table[perem_id].value.setCharectValue(c);
		symbols->setUsed(perem_id, true);
	}
	//
	/// END NEW

	return 0;
}


PeremType text_analiz::parceExpression2(QString viraj,
	function_table *table,
	symbol_table *symbols,
	QString algorithm,
	int *err,
	QStringList *parsedViraj,
	ErrorPosition *err_pos,
	QStringList &extraSubExpressions,
	bool isLeft)
{
	StructType structType;
	return parceExpression(viraj, table, symbols, algorithm, err, parsedViraj, &structType, err_pos, extraSubExpressions, isLeft);
}

/**
 * Определяет внутренний тип выражения. Является основной для исползования.
 * @param viraj выражение
 * @param table таблица алгоритмов
 * @param symbols таблица переменных
 * @param algorithm имя алгоритма в котором происходит разбор
 * @param err Вывод ошибки
 * @param parsedViraj Spisok podvyrazhenii v D-zapisi (see below)
 * @param err_block  Вывод местополоджения ошибки
 * @return Тип выражения
 */
/*
PeremType text_analiz::parceExpression(
                QString         viraj,          // In: Исходное выражение (текст)
                function_table  *table,         // In: Таблица имен функций
                symbol_table    *symbols,       // In: Таблица имен переменных
                QString         algorithm,      // In: Имя алгоритма (global, если вне?)
                int             *err,           // Out:Код ошибки (0 - ОК)
                QStringList     *parsedViraj,   // Out:список глпаных подвыражений (см. ниже)
                                                                        //     каждое подвыражение представлено
                                                                        //     в Д-форме (см. ниже)
                int             *err_block,     // Out:номер ошиб. блока, устарело (см. ниже)
//MR-New
        StructType      *virStruct
//RM
)

        Производит аналтз корректности записи выражения в соответствии с Описанием
         (ч.1, раздел 6; ч.2, раздел 2, пп.2.3-2.5);
         строит внутреннее предсталение выражения - список главных подвыражений;
         возвращает тип выражения

        Терминология.
        1. В соответствии с определениями пп.2.3-2.5 (ч.2) каждое выражение можно представить
в виде корневого дерева (дерева разбора), каждая вершина дерева помечена выражением
и правилом вывода, в соответствии с которым это дерево порождено.
        Главное подвыражение вырадения Е - это подвыражение, которое встречается в
дереве разбора Е.
        Замечание. Если Е1 - главное подвыражение Е, то поддерево дерева разбора Е с
корнем в Е1 - это дерево разбора Е1.
        2. На множестве главных подвыражений определен



Razobraanoe vyrazhenie - eto vyrazhenie v D-zapisi, t.e. v vide
[Oper0] Obj0 {Oper Obj}

zdes' Oper0 - eto znak unarnoi operatsii;
Obj0, Obj - eto object;
Oper - znak binarnoi operatsii
(vse probely udaleny).
Znak unarnoi operatsii - eto UNARY_PLUS, UNARY_MINUS, NOT
(??? Kogda '+' i '-' zameniautsia na  UNARY_PLUS, UNARY_MINUS ??)
Znak binarnoi operatsii - eto
arifm.oper ILI boolen oper ILI concaten (+) ILI sravnenie
(see Delimiters)
Obj - eto:
^<nomer> - Var ili  Const (v tom chisle - sdtroka)
??? NewGet ne ispolzuetsya pri obrabotke imen massivov, a ne ih elementiv ??
???    e.g. pri vyzove functsiy ????
@<nomer>{&<nom_vyr>} - vyzov Func, zatem - ssylka na vyrazh-argum
%<nomer>{{&<nom_vyr>}} - element massiva, t.e.
imya massiva, zatem - nepustoi spisok znachenii indexov
|<nomer>&<nom_vyr> - element stroki (charact)
|<nomer>&<nom_vyr>&<nom_vyr> - vyrezka iz stroki
<nomer> - ssylka v tablitsu perem ili algoritmov;
<nom_vyr> - ssylka na predshestv element parsedViraj

        Vozvrashchaet:
tip v sootvetstvii s imenem (NO MASS !!!!) - krome "%<nomer>"
esli %<nomer> - mass_.
*/

/**
* Возвращает начальную позицию и длину ошибки.
* @param start IN: позиция начала ошибки
* @param end IN: позиция конца ошибки
* @param P IN: вектор P (см. описание)
* @param L IN: вектор L (см. описание)
* @return позиция начала ошибки (с учетом преобразований по таблице) и ее длина
*/
QPoint translateErrorPosition(int start, int end, const QList<int> &P, const QList<int> &L)
{
	if (start == -1) {
		// Ошибка внутри сгенерированного кода
		return QPoint(-1, -1);
	}
	int min = 0;
	int max = 0;
	bool minSet = false;
	bool maxSet = false;
	for (int i = start; i < end; i++) {
		int p = P[i];
		int l = L[i];
		if (minSet) {
			min = qMin(min, p);
		} else {
			min = p;
			minSet = true;
		}
		if (maxSet) {
			max = qMax(max, p + l);
		} else {
			max = p + l;
			maxSet = true;
		}
	}
	return QPoint(min, max - min);
}

PeremType text_analiz::parceExpression(
	QString viraj,
	function_table *table,
	symbol_table *symbols,
	QString algorithm,
	int *err,
	QStringList *parsedViraj,
	//MR-New
	StructType  *VirStruct,
	//RM
	ErrorPosition *err_pos,
	QStringList &extraSubExpressions,
	bool isLeft
)
{
	Q_UNUSED(isLeft);
	QString dbg = viraj;

	enum  BlockType { // Block = fragment between delimiters
		None,           // empty or error
		Sim_perem,      // SimpleVar or Const or 0-ary function
		//  (vozmozhno, - imya massiva bez [] ili imya stroki
		Mass,           // Array or String, posle kotoryh idut []
		Sim_funct       // Function with ()
	};

	BlockType cur_block_type = None;
	QString out_put_string = "";
	QString work_block;
	//  viraj = viraj.trimmed();



	//              A. Prolog
	int cur_pos = 0; // Position in viraj
	int cur_block_id = 0; // Block number

	QChar oper = ' ', last_oper = ' '; // Current (oper) and previous (last_oper) delimiters

	// Pozitsii i tipy skobok (according skobka? See G.1 ??).
	//  QList<int> skobk_pos,skobk_type;
	//  QList<bool> op_skobka_is_funct,close_skobka_is_funct;   //  ????

	//      Remove Spaces and
	//    Set Skobka positions and types:

	QList<int> P, L, OP, OL;

	for (int i = 0; i < viraj.length(); i++) {
		P << i;
		L << 1;
	}

	viraj = dropSpace(viraj, P, L); //, &skobk_pos, &skobk_type );

	//  qDebug() << "P: " << P;
	//  qDebug() << "L: " << L;

	//              Main loop  (B-F)
	//      One prohod - one block.
	//      Exclusion: Exponential double

	// NEW V.Y. 21.08.2006
	cur_block_id = -2;

	int virLength = viraj.length();
	while (cur_pos < virLength) {
		//          B. Prolog of Main loop
		cur_block_id++;
		cur_block_type = None;

		//      B1. Search next delimiter
		int new_cur_pos = operators.indexIn(viraj, cur_pos);
		if (new_cur_pos == -1 || new_cur_pos - cur_pos >= 1) {
			bool allowEmptyOper = false;
			if (new_cur_pos != -1) {
				allowEmptyOper = viraj[new_cur_pos] == ')' || viraj[new_cur_pos] == ']';
			}
			if (!allowEmptyOper && (oper == ')' || oper == ']')) {
				//              *err_pos = ErrorPosition(cur_block_id,cur_block_id+1,1,-1);
				*err_pos = ErrorPosition(cur_pos, new_cur_pos == -1 ? viraj.length() - cur_pos : new_cur_pos - cur_pos);
				*err = GNVR_MISSED_OPERATOR;
				return none;
			}
		}
		if (new_cur_pos == -1) {
			new_cur_pos = viraj.length();
			oper = ' ';
		} else {
			oper = viraj[new_cur_pos];
		}

		//      B2. Get current block
		work_block = viraj.mid(cur_pos, new_cur_pos - cur_pos);


		//          C. Work exponential double
		//      C1. Check current block for Mantissa:
		//   1) starts with digit or point;
		//   2) ends with E (latin or cyrillic)
		//   3) next delimiter '+' or '-'
		bool expf;
		int test;
		if (new_cur_pos <= cur_pos && work_block.trimmed().length() > 0) {
			expf = false;
			test = 0;
		} else {
			QString wbt = work_block.trimmed();
			test = wbt[0].digitValue();
			if (wbt[0] == '.') {
				test = 10;
			}
			expf = false;
			if (!wbt.isEmpty())
				expf = (wbt[wbt.length() - 1] == 'E') || (wbt[wbt.length() - 1] == 'e') ||
					(wbt[wbt.length() - 1] == QChar(1045)) || (wbt[wbt.length() - 1] == QChar(0x0435)) ||
					(wbt.count('E') == 1) || (wbt.count('e') == 1) || (wbt.count(QChar(1045)) == 1) || (wbt.count(QChar(0x0435)) == 1) ;
		}

		bool check_expf = (expf) && (test >= 0) && ((oper == '-') || (oper == '+'));
		if (check_expf) {
			//          !!! Candidate to exp double !!!
			//      C2. Check it carefully
			//      this (=left) and next (=right) are candidates to exp double
			cur_pos = new_cur_pos + 1;
			new_cur_pos = operators.indexIn(viraj, cur_pos);
			if (new_cur_pos == -1) {
				new_cur_pos = viraj.length();
			}

			//      left: kill last 'E'
			QString work_block1;
			work_block1 = work_block.left(work_block.length() - 1);

			//      left must be double
			bool const_t;
			double left = work_block1.toFloat(&const_t);
			Q_UNUSED(left);

			if (!const_t) {

				*err = GNVR_BAD_EXP_LEFT;
				//              *err_block=cur_block_id;
				//              *err_pos = ErrorPosition ( cur_block_id,
				//                                         cur_block_id + 2,
				//                                         -1,-1 );
				*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);

				return none;
			}
			//      set right
			work_block = viraj.mid(cur_pos, new_cur_pos - cur_pos);
			//      right must be integer
			double right = work_block.toInt(&const_t);
			Q_UNUSED(right);
			if (!const_t) {

				*err = GNVR_BAD_EXP_RIGHT;
				//              *err_block=cur_block_id;
				//              *err_pos = ErrorPosition ( cur_block_id,
				//                                         cur_block_id + 2,
				//                                         -1,-1 );
				*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
				return none;
			}

			//      At last: reright block into decimal
			//  ??? Skolko tam budet znakov  ???
			//          if ( oper=='-' )
			//              work_block=work_block.setNum ( left*pow ( 10.0,-right ) );
			//          if ( oper=='+' )
			//              work_block=work_block.setNum ( left*pow ( 10.0,right ) );
			work_block = work_block1 + "E" + oper + work_block;

			int virLength = viraj.length();
			if (new_cur_pos < virLength) {
				oper = viraj[new_cur_pos];
			} else {
				oper = ' ';
			}
			// increment cur_block_id - for error mess
			cur_block_id++;       //
		} // if(check_expf)


		//          D. !!! Set Block Type - check next delimiter
		//  LEMMA.
		//    1. oper = '(' && NonEmpty => Function name
		//    2. oper = '[' ==>  NonEmpty && Array (or string) name
		//    3. Otherwise: NonEmpty => SimpleVar or Const or 0-ary Function
		//          or golyi massive (v vyzove funkcii) !!!         //MR-New


		if (oper == '(') {
			if ((work_block.simplified().length() > 0)  && (work_block.simplified() != QString(QChar(KS_NOT)))) {
				cur_block_type = Sim_funct;
			} else {
				cur_block_type = None;
			}
		} else if (oper == '[') {
			if (work_block.simplified().length() == 0) {
				//    Empty ==> Error
				//                  *err_block=cur_block_id;  // Pustoi block! => vydeliat' '[' !!!!!
				*err = GNVR_NO_ARRAY_NAME; // Empty before '['
				QPoint pnt = translateErrorPosition(new_cur_pos, new_cur_pos + 1, P, L);
				*err_pos = ErrorPosition(pnt.x(), 1);
				return none;
			}
			cur_block_type = Mass; // !!!!  Empty => Error ??
		}

		else {
			if (work_block.trimmed().length() > 0) {
				/*
				                        // begin NEW V.Y.
				                                        if ( (last_oper==')') || (last_oper==']') )
				                                        {
				                                        *err = SYNTAX_ERR;
				                                        *err_block = cur_block_id;
				                                        return none;
				                        }
				                        */
				// end NEW V.Y.
				cur_block_type = Sim_perem;
			} else {
				// NEW V.Y. 22.08.2006
				if (last_oper == ',') {
					if (oper == '-') {
						oper = QChar(UNARY_MINUS);
					} else if (oper == '+') {
						oper = QChar(UNARY_PLUS);
					} else {
						//                         Err_start = Err_end = cur_block_id;
						//                         Err_start_incl = Err_end_incl = 1;
						//                      *err_pos = ErrorPosition ( cur_block_id,
						//                                                 cur_block_id,
						//                                                 1,1 );
						*err_pos = ErrorPosition(cur_pos - 1, 1);
						*err = GNVR_EXTRA_COMMA;
						return none;
					}
				}
				cur_block_type = None; // Empty block has type None
			}
		}




		//          E1. Work NOT
		bool not_flag = false;
		int not_pos = 0;

		//      work_block = work_block.trimmed();
		//      qDebug("zzz: %s, %s",viraj.utf8().data(),work_block.utf8().data());
		//      no NOT v konce!!!!!
		if ((work_block.trimmed().length() > 1) && (work_block.trimmed()[work_block.trimmed().length() - 1] == QChar(KS_NOT))) {
			*err = GNVR_NOT_END;
			//          *err_block=cur_block_id;
			int t = viraj.indexOf(QChar(KS_NOT), cur_pos);
			K_ASSERT(t >= 0);
			if (t >= 0) {
				not_pos = t;
			}
			QPoint pnt = translateErrorPosition(not_pos, not_pos + 1, P, L);
			*err_pos = ErrorPosition(pnt.x(), pnt.y());
			return none;
		}

		//      no more than one NOT
		if (work_block.count(QChar(KS_NOT)) > 1) {
			*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
			int t = viraj.indexOf(QChar(KS_NOT), cur_pos);
			K_ASSERT(t >= 0);
			t = viraj.indexOf(QChar(KS_NOT), t + 1);
			if (t >= 0) {
				not_pos = t;
			}
			QPoint pnt = translateErrorPosition(not_pos, not_pos + 1, P, L);
			*err_pos = ErrorPosition(pnt.x(), pnt.y());

			*err = GNVR_MANY_NOT;
			return none;
		}

		//      one NOT is NOT
		if (work_block.count(QChar(KS_NOT)) == 1) {
			int t = viraj.indexOf(QChar(KS_NOT), cur_pos);
			K_ASSERT(t >= 0);
			if (t >= 0) {
				not_pos = t;
			}
			work_block.replace(QChar(KS_NOT), "");
			not_flag = true;
		}

		//          E2. Replace unary '-' and '+' with UNARY_MINUS and UNARY_PLUS
		if ((cur_block_type == None) &&
			((last_oper == ' ') ||
				(last_oper == '(') ||
				(last_oper == '[') ||
				// NEW 2007-07-24
				(last_oper == '<') ||
				(last_oper == '>') ||
				(last_oper == '=') ||
				(last_oper == QChar(KS_BOLSHE_RAVNO)) ||
				(last_oper == QChar(KS_MENSHE_RAVNO)) ||
				(last_oper == QChar(KS_NE_RAVNO))
				// END NEW
			)
		) {
			QChar local_oper;

			if (oper == '-') {
				local_oper = QChar(UNARY_MINUS);
			} else if (oper == '+') {
				local_oper = QChar(UNARY_PLUS);
			} else {
				local_oper = oper;
			}


			oper = local_oper;
		} // end if (....)

		//          E3. Remove spaces
		//      work_block=work_block.simplified();

		//              F. !!! Append out_put_string - the inner representation-1
		//    On this stage we replace every name (including consts)
		//  with the group <char><integer>  where
		//     <char> denotes the Block Type;
		//     <integer> is the id  in name table
		//  Notation for Block type:
		//          ^ - Sim_perem
		//          @ - Sim funct
		//          % - Mass
		//          & - podvyrazhenie (poiavitsya pozzhe)
		//
		//   NB:  esli <char> = '@', to id - ssylka v tabl functiy;
		//      esli <char> = '%', to id - ssylka v tabl imen;
		//        esli <char> = '^', to my id mozhet byt'
		//          ssylkoi v tabl functiy )0-arnaia function)
		//          ili v tabl imen (const ili simple var);
		//
		//  ================================================================

		//          F0. Check for wrong ksimbols within the name
		/*MR@@-99   Dopisat' pozzhe ??
		        bool  test = CheckWrongKsi(work_block);
		        if (!test){
		        *err_block=cur_block_id;
		        *err= WRONG_SYMB_IN_NAME;
		        return none;
		        };
		        */

		switch (cur_block_type) {
		//          F1. Functions
		case Sim_funct: {
			int tn_err = 0, tn_start = 0, tn_len = 0;
			tn_err = KumTools::instance()->test_name(work_block, tn_start, tn_len);
			if (tn_err > 0) {
				QPoint pnt = translateErrorPosition(cur_pos + tn_start, cur_pos + tn_start + tn_len, P, L);
				*err_pos = ErrorPosition(pnt.x(), pnt.y());
				*err = tn_err;
				return none;
			}
			int func_id = -1;
			int module_id = -1;
			modules->FuncByName(work_block.simplified(), &module_id, &func_id);

			if (work_block.simplified().startsWith("@")) {
				// Special function ...
				if (!algorithm.simplified().startsWith("@")) {
					// .... can't be called from ordinary function
					*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
					*err = GNVR_NO_IMPL;
					return none;
				}
			}

			if (func_id == -1) {
				*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
				*err = GNVR_NO_IMPL;
				return none;
			};
			if (modules->module(module_id)->Functions()->isBroken(func_id)) {
				*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
				*err = GNVR_USE_BAD_ALG;
				return none;
			}

			if (module_id != moduleId) {
				if (work_block.simplified().startsWith("_")) {
					*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
					*err = TA_PRIVATE_METHOD_CALL_2;
					return none;
				}
			}

			if (modules->module(module_id)->Functions()->typeById(func_id) == none) {
				*err_pos = ErrorPosition(cur_pos, new_cur_pos - cur_pos);
				*err = GNVR_ALG_INSTEAD_OF_FUNCTION;
				return none;
			}

			if (not_flag) {
				if (modules->module(module_id)->Functions()->typeById(func_id) != kumBoolean) {
					QPoint pnt = translateErrorPosition(not_pos, not_pos + 1, P, L);
					*err_pos = ErrorPosition(pnt.x(), pnt.y());
					*err = GNVR_NOT_LOG;
					return none;
				};
				out_put_string = out_put_string + QChar(KS_NOT);
				//                    OP << -1;
				//                    OL << 1;
				OP << not_pos;
				OL << 1;
			};
			QString out_append = "@" + QString::number(func_id) + "|" + QString::number(module_id) + oper;
			for (int x = 0; x < out_append.length() - 1; x++) {
				OP << cur_pos;
				OL << new_cur_pos - cur_pos;
			}
			OP << new_cur_pos;
			OL << 1;
			out_put_string = out_put_string + out_append;
		};
		break;

		//          F2. SimpeVar or Const or 0-ary Function
		case Sim_perem: {
			int func_id = -1;
			int module_id = -1;
			modules->FuncByName(work_block.simplified(), &module_id, &func_id);
			if (func_id == -1) {
				//      F2.2. Not funct => check for Var
				//   local?
				// NEW 28.08
				int perem_id = symbols->inTable(work_block.simplified(), algorithm);
				// END NEW
				//   global?
				if (perem_id < 0) {
					perem_id = symbols->inTable(work_block.simplified(), "global");
				}

				if (perem_id < 0 && work_block.trimmed() == QObject::trUtf8("знач")) {
					perem_id = symbols->inTable(algorithm, "global");

				}

				PeremType b_type;
				if (perem_id > -1) {
					b_type = symbols->getTypeByID(perem_id);
				} else {
					b_type = none;
					int le;
					if (work_block.count("\"") > 2) {
						int bqp = work_block.lastIndexOf("\"");
						*err = GNVR_MANY_QOUTES_IN_BLOCK;
						QPoint pnt = translateErrorPosition(bqp, bqp + 1, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());
						return none;
					}
					b_type = testConst(work_block, &le);
					if (le > 0) {
						*err = le;
						if (expf) {
							cur_pos = viraj.lastIndexOf(viraj, cur_pos);
							cur_pos = cur_pos == -1 ? 0 : cur_pos;
						}
						QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());
						return none;
					}

					if ((perem_id == -1) && (b_type == none) && (!work_block.isEmpty())) {
						if (work_block.trimmed() == QObject::trUtf8("знач")) {
							QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
							*err_pos = ErrorPosition(pnt.x(), pnt.y());
							*err = GNVR_PROCEDURE_RETVAL_ACCESS;
							return none;
						}
						int tn_start = 0, tn_len = 0;
						int tn = KumTools::instance()->test_name(work_block, tn_start, tn_len);
						if (tn > 0) {
							QPoint pnt = translateErrorPosition(cur_pos + tn_start, cur_pos + tn_start + tn_len, P, L);
							*err_pos = ErrorPosition(pnt.x(), pnt.y());
							*err = tn;

							return none;
						}
						QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());
						if (work_block.trimmed() == QObject::trUtf8("знач")) {
							*err = GNVR_PROCEDURE_RETVAL_ACCESS;
						} else {
							*err = GNVR_NO_IMPL_2;
						}
						return none;
					} else {
						//      F2.4. CONST !!!!!
						//   add line to the symbol table (including the value)
						symbols->add_symbol
						("!KUM_CONST", -1, b_type, algorithm, true);
						symbols->new_symbol(symbols->symb_table.size() - 1);
						perem_id = symbols->symb_table.size() - 1;
						QString const_value = work_block.simplified();
						set_const_value(perem_id, const_value, symbols);
						symbols->setUsed(perem_id, true);
					}
				}
				//      F2.5. Hurra! Correct Var or Const
				//  (perem_id - ref to symbol table)

				//  a) work NOT (check if kumBoolean later)
				//  b) append to out_put_string
				if (not_flag) {
					if (b_type != kumBoolean) {
						QPoint pnt = translateErrorPosition(not_pos, not_pos + 1, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());

						*err = GNVR_NOT_LOG_2;
						return none;
					} else {
						out_put_string = out_put_string +  QChar(KS_NOT);
						//                            $;
						//                            OL << 1;
						OP << not_pos;
						OL << 1;
					}
				}
				if (perem_id > -1) { // Zachrm etot if?
					QString out_append = "^" + QString::number(perem_id) + oper;
					for (int x = 0; x < out_append.length() - 1; x++) {
						OP << cur_pos;
						OL << new_cur_pos - cur_pos;
					}
					OP << new_cur_pos;
					OL << 1;
					out_put_string = out_put_string + out_append;
				}

			} // end if(func_id==-1)
			else {
				//      F2.6. 0-ary function
				if (module_id != moduleId) {
					if (work_block.simplified().startsWith("_")) {
						QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());
						*err = TA_PRIVATE_METHOD_CALL_2;
						return none;
					}
				}
				if (modules->module(module_id)->Functions()->argCountById(func_id) > 0) {
					//                         Err_start = cur_block_id;
					//                         Err_end = cur_block_id + 1;
					//                         Err_start_incl = -1;
					//                         Err_end_incl = -1;
					//                      *err_pos = ErrorPosition ( cur_block_id,
					//                                                 cur_block_id + 1,
					//                                                 -1,-1 );
					QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
					*err_pos = ErrorPosition(pnt.x(), pnt.y());

					*err = GNVR_NO_ARG;
					//                      *err_block = cur_block_id;
					return none;
				};
				if (modules->module(module_id)->Functions()->typeById(func_id) == none) {
					//                      *err_pos = ErrorPosition ( cur_block_id,
					//                                                                           cur_block_id + 1,
					//                                      -1,-1 );
					QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
					*err_pos = ErrorPosition(pnt.x(), pnt.y());
					*err = GNVR_ALG_INSTEAD_OF_FUNCTION;
					//                      *err_block = cur_block_id;
					return none;
				}
				if (not_flag) {
					if (modules->module(module_id)->Functions()->typeById(func_id) != kumBoolean) {
						QPoint pnt = translateErrorPosition(not_pos, not_pos + 1, P, L);
						*err_pos = ErrorPosition(pnt.x(), pnt.y());
						*err = GNVR_NOT_LOG_3;
						//                          *err_block = cur_block_id;
						return none;
					};

					out_put_string = out_put_string + QChar(KS_NOT);
					//                        OP << -1;
					//                        OL << 1;
					OP << not_pos;
					OL << 1;
				};
				QString out_append = "@" + QString::number(func_id) + "|" + QString::number(module_id) + oper;
				for (int x = 0; x < out_append.length() - 1; x++) {
					OP << cur_pos;
					OL << new_cur_pos - cur_pos;
				}
				OP << new_cur_pos;
				OL << 1;
				out_put_string = out_put_string + out_append;

			} // end else for if(func_id==-1)
		}
		break;
		//                  F.3. Array or String
		case Mass: {
			//  is it a name?
			int perem_id = symbols->inTable(work_block.simplified(), algorithm);
			if (perem_id < 0) {
				perem_id = symbols->inTable(work_block.simplified(), "global");
			}
			// NEW 28.08
			if (perem_id < 0 && work_block.simplified() == QObject::trUtf8("знач")) {
				perem_id = symbols->inTable(algorithm, "global");

			}

			// maybe function?

			int func_id = -1;
			int module_id = -1;
			modules->FuncByName(work_block.simplified(), &module_id, &func_id);
			if (func_id != -1) {
				QPoint pnt = translateErrorPosition(new_cur_pos, new_cur_pos + 1, P, L);
				*err_pos = ErrorPosition(pnt.x(), pnt.y());
				*err = GNVR_BRACKET_MISMATCH;
				return none;
			}


			if ((perem_id == -1)) {
				QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
				*err_pos = ErrorPosition(pnt.x(), pnt.y());
				//                  *err_block=cur_block_id;
				*err = GNVR_NO_IMPL_3;
				return none;
			};

			// @@@-B2. For strings
			// Predlagaetsia sdelat' raznye priznaki dlia stroki i massiva
			// % - dlia massiva;
			// | - dlia stroki
			// Eto potom ispolzovat' pri kontrole indeksov

			//   Etot kusok ne otlazhivalsia

			if (symbols->getTypeByID(perem_id) == kumString) {
				if (not_flag) {
					out_put_string = out_put_string + QChar(KS_NOT);
					OP << not_pos;
					OL << 1;
				}
				QString out_append = QString(QChar(TYPE_SKOBKA)) +
					QString::number(perem_id) +
					oper;
				out_put_string = out_put_string + out_append;
				for (int x = 0; x < out_append.length() - 1; x++) {
					OP << cur_pos;
					OL << new_cur_pos - cur_pos;
				}
				OP << new_cur_pos;
				OL << 1;
			} else {
				if ((symbols->getTypeByID(perem_id) == mass_integer) ||
					(symbols->getTypeByID(perem_id) == mass_charect) ||
					(symbols->getTypeByID(perem_id) == mass_string) ||
					(symbols->getTypeByID(perem_id) == mass_bool) ||
					(symbols->getTypeByID(perem_id) == mass_real)) {
					if (not_flag) {
						out_put_string = out_put_string + QChar(KS_NOT);
						//                            OP << -1;
						//                            OL << 1;
						OP << not_pos;
						OL << 1;
					}
					QString out_append = "%" + QString::number(perem_id) + oper;
					for (int x = 0; x < out_append.length() - 1; x++) {
						OP << cur_pos;
						OL << new_cur_pos - cur_pos;
					}
					OP << new_cur_pos;
					OL << 1;
					out_put_string = out_put_string + out_append;
				} else {
					//                         Err_start = cur_block_id;
					//                         Err_end = cur_block_id + 1;
					//                         Err_start_incl = -1;
					//                         Err_end_incl = -1;
					//                      *err_pos = ErrorPosition ( cur_block_id,
					//                                                 cur_block_id + 1,
					//                                                 -1,-1 );
					QPoint pnt = translateErrorPosition(cur_pos, new_cur_pos, P, L);
					*err_pos = ErrorPosition(pnt.x(), pnt.y());
					//                      *err_block=cur_block_id;
					*err = GNVR_CANT_INDEX;
					return none;
				}
			}
		} // end case Mass
		break;

		//              F.5. Empty block
		case None: {
			//      NOT pered skobkoi
			if (not_flag) {
				out_put_string = out_put_string + QChar(KS_NOT);
				//                    OP << -1;
				//                    OL << 1;
				OP << not_pos;
				OL << 1;
			}
			OP << new_cur_pos;
			OL << 1;
			out_put_string = out_put_string + oper;
		}
		break;

		} // end switch (cur_block_type)

		cur_pos = new_cur_pos + 1;
		last_oper = oper;
	} // end while (cur_pos<viraj.length())

	// ==== NEW 2007-07-10 ====
	// Проверка на мусор в конце выражения.
	// Гипотеза: последним оператором после разбора может быть
	//      а) " " -- ничего;
	//      б) ")" -- закр. скобка функции;
	//      в) "]" -- закр. скобка массива.
	// Всё остальное -- лишнее, и является ошибкой "Знак в конце выражения"
	if (
		(last_oper == ' ')
		||
		(last_oper == ')')
		||
		(last_oper == ']')
	) {
		// OK.
	} else {
		// Ошибка.
		QString compound_sign = QString(QChar(KS_NOT)) +
			QString(QChar(KS_OR)) +
			QString(QChar(KS_XOR)) +
			QString(QChar(KS_AND));
		if (compound_sign.indexOf(last_oper) != -1) {
			*err = GSV_ZNAK_V_KONCE_2;
		} else {
			*err = GSV_ZNAK_V_KONCE;
		}
		// Выделяем ошибку
		//      int voidParam;
		//      int a;
		//      a = FindNomerNorm ( out_put_string,out_put_string.length()-1,1,&voidParam );
		//      *err_pos = ErrorPosition ( a,a,1,1 );
		QPoint pnt = translateErrorPosition(cur_pos - 1, cur_pos, P, L);
		*err_pos = ErrorPosition(pnt.x(), pnt.y());
		return none;
	}
	// ==== END 2007-07-10 ====

	//========================================================================
	//                          G.PART 2
	//      Vydelenie podvyrazjenii (ili ih spiskov) v skobkah.
	//  Vhod: DN-representation (vse imena zameneny na ssylki <char><nomer>
	//  Vyhod: D-predstavlenie (spisok podvyrazhenii)
	//========================================================================
	// Vydelenie podvyrazjenii (ili ih spiskov) v skobkah.
	// Formirovanie (a) QStringList *parsedViraj (podvyrazheniya) i
	//              (b) QList<PeremType> int_types; - ih tipy
	// Zamena podvyrazhenii na &<integer>,
	//      gde <integer> - nomer v parsedViraj
	//
	//--------------------------------------------------------------------
	//              G.1. Data initialization
	//              -------------------------
	// dobit' probely - mogli poiavitsya posle NOT

	//  out_put_string=out_put_string.trimmed();


	//  qDebug() << "OP: " << OP;
	//  qDebug() << "OL: " << OL;

	KumTools::instance()->trim(out_put_string, OP, OL);

	//  qDebug() << "OP: " << OP;
	//  qDebug() << "OL: " << OL;


	K_ASSERT(out_put_string.length() == OP.count()) ;
	K_ASSERT(OL.count() == OP.count()) ;
	// vnutr tipy i struct tipy podvyrazhenii iz ParseVyraj //MR-New
	// stroiatsya parallelno s ParseVyraj
	QList<PeremType>  int_types;
	QList<StructType> virStruct;    //MR-New

	int_types.clear();
	virStruct.clear();                  //MR-New

	// rabochaia stroka - see dropSpace
	QString string_cpy = dropSpace(out_put_string, OP, OL);

	//  qDebug() << "OP: " << OP;
	//  qDebug() << "OL: " << OL;

	K_ASSERT(string_cpy.length() == OP.count()) ;
	K_ASSERT(OL.count() == OP.count()) ;

	// vremennye perem to replace podvyrazheniya
	QString work_str;
	// ili spiski podvyrazhenii pri vyzove functii, v mnogom. massivah i vyrezkah
	QStringList arg_viraj_List;

	// pozitsii skobok i ih tipy
	//      KRUG-KVAD:  0,2 - 1,3;
	//      OPEN-CLOSE: 0,1 - 2,3
	//   QList<int> skobk_pos,skobk_type; - see A.Prolog
	//  skobk_pos.clear();
	//  skobk_type.clear();


	//      Priznaki '[' and ']' ?? Co-numbered with skobk_pos ??
	//  QList<bool> open_sk_is_mass,close_sk_is_mass;
	// nu tak...
	*err = 0;

	//-------------------------------------------------------------------------------
	//              G.2. Cykl po podvyrazheniyam - iznutri naruzhu
	//              ----------------------------------------------
	//  Na ocherednom shage:
	//    1) berem ocherednuuiu paru sootvetsvennyh sosednih skobok;
	//    2) vydeliaem mezhdu nimi:
	//          2.1) SimpleVyraj ili
	//          2.2) gruppu SimpleVyraj, razdelennyh:
	//              a) zapiatymi (esli skobki - eto vyzov mnigo-param func
	//                            ili ssylka na element mnogomernogo massiva).
	//                  ili
	//              b) dvoetochiem (esli vyrezka)
	//    3) formiruem text vida {&<integer>};
	//       zdes'  <integer> - eto id v ParseVyraj
	//       Etot text - eto work_str (esli 1 vyr) ili arg_viraj_List (esli > 1)
	//
	//    4) zamenyaem nashi skobki i to, chto bylo mezhdu nimi
	//       na work_str ili arg_viraj_List (para skobok ushla !!!) i
	//       correctiroem skobk_pos, skobk_type
	//   ---------
	//      SimpleVyraj - eto Vyrazh, vnutri kotorogo net skobok
	//          string_cpy - eto stroka, s kotoroi my rabotaem
	//=================================================================================
	//  // NEW 25.08
	//  int block_start;
	//  int block_end;
	//  // END
	//
	//  // NEW 2006-11-20
	//  QRegExp rxEmptySkobk ( "\\(\\s*\\)" );
	//  bool dbg = rxEmptySkobk.isValid();
	//  block_start = rxEmptySkobk.indexIn ( out_put_string );
	//  if ( block_start!= -1 )
	//  {
	//      int a,b,c,d;
	//      block_end = block_start + rxEmptySkobk.matchedLength();
	//      a = FindNomerNorm ( out_put_string,block_start,1,&c );
	//      b = FindNomerNorm ( out_put_string,block_end-1,1,&d );
	//      *err_pos = ErrorPosition ( a,b,c,d );
	//      *err = GSV_NO_IMPL_2;
	//      return none;
	//  }
	// END NEW

	// NEW == � азбиение выражения на подвыражения без скобок

	// список подвыражений
	QStringList exprList = *parsedViraj;

	int local_error = 0;
	int local_err_pos = -1;
	int local_err_pos_end = -1;
	int local_err_len = -1;

	QList< QList<int> > FP, FL;

	for (int i = 0; i < exprList.count(); i++) {
		FP << QList<int>();
		FL << QList<int>();
	}

	//  qDebug("=== expr: %s",viraj.toUtf8().data());

	// разбиение на подвыражения
	F(string_cpy, &exprList, &local_error, &local_err_pos, &local_err_pos_end, FP, FL);



	if (local_error > 0) {
		// Вычисляем позицию ошибки по таблице перевода out_put_string -> viraj
		QPoint stage1 = translateErrorPosition(local_err_pos, local_err_pos_end + 1, OP, OL);
		// Вычисляем позицию ошибки по таблице перевода viraj -> in.viraj [после dropSpace(viraj)]
		QPoint stage2 = translateErrorPosition(stage1.x(), stage1.x() + stage1.y(), P, L);
		*err_pos = ErrorPosition(stage2.x(), stage2.y());
		//          *err_pos = ErrorPosition ( a,b,c,d );
		*err = local_error;
		return none;
	}

	// список типов подвыражений
	QList<PeremType> exprTypeList;
	// список видов подвыражений
	QList<StructType> exprStructList;

	for (int i = 0; i < exprList.count(); i++) {
		StructType st;
		PeremType pt;
		bool include_bounds_in_error; // включение граничных символов (\b) в область ошибки
		// Определяем тип каждого выражения
		QString expr;

		pt = G(exprList[i], &local_error, table, symbols, exprTypeList, exprStructList, exprList, &st, &local_err_pos, &local_err_len, include_bounds_in_error);

		if (local_error > 0) {
			// Вычисляем позицию ошибки по таблице перевода exprList[i] -> out_put_string
			QPoint stage0 = translateErrorPosition(local_err_pos, local_err_pos + local_err_len, FP[i], FL[i]);
			if (!include_bounds_in_error) {
				QChar start = out_put_string[stage0.x()];
				QChar end = out_put_string[stage0.x() + stage0.y() - 1];
				if (start == '(' || start == '[' || start == ',') {
					stage0.setX(stage0.x() + 1);
					stage0.setY(stage0.y() - 1);
				}
				if (end == ')' || end == ']' || end == ',') {
					stage0.setY(stage0.y() - 1);
				}
			}
			// Вычисляем позицию ошибки по таблице перевода out_put_string -> viraj
			QPoint stage1 = translateErrorPosition(stage0.x(), stage0.x() + stage0.y(), OP, OL);
			// Вычисляем позицию ошибки по таблице перевода viraj -> in.viraj [после dropSpace(viraj)]
			QPoint stage2 = translateErrorPosition(stage1.x(), stage1.x() + stage1.y(), P, L);
			*err_pos = ErrorPosition(stage2.x(), stage2.y());
			*err = local_error;
			return none;
		}

		exprTypeList.append(pt);
		exprStructList.append(st);
	}

	// Если в подвыражениях ошибок не обнаружено,
	// то удаляем из них пробелы

	for (int i = 0; i < exprList.count(); i++) {
		exprList[i].remove(' ');
	}

	// Выделяем подвыражения между логическими операторами в отдельные блоки
	int exprCount = exprList.count();

	for (int i = 0; i < exprCount; i++) {
		exprList[i] = splitLogicalExpressions(exprList[i], extraSubExpressions);
	}

	*parsedViraj = exprList;
	*VirStruct = exprStructList.last();
	PeremType result = exprTypeList.last();
	if ((result == bool_or_num) || (result == bool_or_lit)) {
		result = kumBoolean;
	}

	return result;

}


struct Bracket {
	// Пустой конструктор (требуется синтаксисом языка C++)
	Bracket()
	{
		t = ' ';
		p1 = -1;
		p2 = -1;
		ot = ' ';
	}
	// Конструктор открывающей скобки
	Bracket(QChar a, int b)
	{
		t = a;
		p1 = b;
		p2 = b;
		ot = a;
	}
	// Конструктор внутренней скобки ( "," или ":" )
	Bracket(QChar a, int b, QChar c, int d)
	{
		t = a;
		p1 = b;
		ot = c;
		p2 = d;
	}
	QChar t; // тип скобки
	QChar ot; // тип открывающей скобки (если t == ',' или ':' )
	int p1; // позиция скобки
	int p2; // позиция соотв. открыв. скобки
};

void F_processSubExpr(const QString &s, const int start, QList< QList<int> > &P, QList< QList<int> > &L)
{
	QList<int> p, l;
	int cur, prev;
	prev = -1;
	int shift = 0;
	forever {
		cur = s.indexOf("&", prev + 1);
		if (cur == -1) {
			for (int i = prev + 1; i < s.length(); i++) {
				p << i + start + shift;
				l << 1;
			}
			break;
		} else {
			for (int i = prev + 1; i < cur; i++) {
				p << i + start + shift;
				l << 1;
			}
			QString link;
			prev = s.length() - 1;
			for (int i = cur + 1; i < s.length(); i++) {
				if (s[i].isDigit()) {
					link += s[i];
				} else {
					prev = i - 1;
					break;
				}
			}
			int lnk = link.toInt();
			int left = P[lnk][0];
			int right = 0;
			for (int i = 0; i < P[lnk].count(); i++) {
				left = qMin(left, P[lnk][i]);
				right = qMax(right, P[lnk][i] + L[lnk][i]);
			}
			int len = right - left;
			bool isMass = false;
			if (cur >= 0) {
				for (int i = cur - 1; i >= 0; i--) {
					if (s[i] == '%' || s[i] == QChar(TYPE_SKOBKA)) {
						isMass = true;
					}
					if (!s[i].isDigit() && !(s[i] == '&')) {
						break;
					}
				}
			}
			bool isFirstArg = true;
			if (cur >= 0) {
				for (int i = cur - 1; i >= 0; i--) {
					if (s[i] == '&') {
						isFirstArg = false;
					}
					if (!s[i].isDigit()) {
						break;
					}
				}
			}
			if (isMass) {
				shift += isFirstArg ? 2 : 1;
			}
			for (int i = 0; i < link.length() + 1; i++) {
				p << left;
				l << len;
			}
			shift += len;

			shift -= 1 + link.length();
		}
	}
	P << p;
	L << l;
}

/**
 * � азбивает выражение со скобками на простые подвыражения
 * @param s Исходное выражение
 * @param *EL Ссылка на список подвыражений
 * @param *BL Ссылка на список границ подвыражений
 * @param *err Ссылка на код ошибки
 * @param *err_start Ссылка на позицию начала ошибочной области
 * @param *err_end Ссылка на позицию конца ошибочной области
 */
void text_analiz::F(const QString &s,
	QStringList *EL,
	int *err,
	int *err_pos,
	int *err_pos_end,
	QList< QList<int> > &P,
	QList< QList<int> > &L
)
{
	QRegExp rxDelim = QRegExp("[,:\\[(]");
	QString mainExpr;
	QList<int> mainP, mainL;
	// Стек найденных открывающих скобок
	QStack<Bracket> stack;
	*err = 0;
	*err_pos = -1;
	*err_pos_end = -1;
	for (int i = 0; i < s.length(); i++) {
		// Открывающая круглая скобка
		if (s[i] == '(') {
			stack.push(Bracket('(', i));
		} else if (s[i] == '[') {
			stack.push(Bracket('[', i));
		} else if (s[i] == ')') {
			if (stack.isEmpty()) {
				*err = FIS_1; // нет "(" для ")"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			if (stack.last().t == '[') {
				*err = FIS_2;  // ")" вместо "]"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			Bracket last = stack.pop();
			int delPos = rxDelim.lastIndexIn(mainExpr);
			QString subExpr = mainExpr.mid(delPos + 1);
			if (subExpr.trimmed().isEmpty()) {
				*err = FIS_3; // пусто между скобками
				*err_pos = last.p1;
				*err_pos_end = i;
				return;
			}
			if (last.t == ',') {
				if (last.ot == '[') {
					*err = FIS_10;  // ")" вместо "]"
					*err_pos = i;
					*err_pos_end = i;
					return;
				}
			}
			if (last.t == ':') {
				*err = FIS_11; // здесь не нужно ":"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			while (mainExpr.length() > 0) {
				bool stop = mainExpr.endsWith(last.t);
				mainP.pop_back();
				mainL.pop_back();
				mainExpr.chop(1);
				if (stop) {
					break;
				}
			}
			QString repl = "&" + QString::number(EL->count());
			mainExpr += repl;

			subExpr = " " + subExpr + " ";

			for (int j = 0; j < repl.length(); j++) {
				mainP << last.p1;
				mainL << i - last.p1;
			}

			F_processSubExpr(subExpr, last.p1, P, L);
			EL->append(subExpr);

			continue;
		} else if (s[i] == ']') {
			if (stack.isEmpty()) {
				*err = FIS_4; // нет "[" для "]"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			if (stack.last().t == '(') {
				*err = FIS_5;  // "]" вместо ")"
				*err_pos = *err_pos_end = i;
				return;
			}
			Bracket last = stack.pop();

			if (last.t == ',') {
				if (last.ot == '(') {
					*err = FIS_21;  // "]" вместо ")"
					*err_pos = i;
					*err_pos_end = i;
					return;
				}
			}
			int delPos = rxDelim.lastIndexIn(mainExpr);
			QString subExpr = mainExpr.mid(delPos + 1);
			if (subExpr.trimmed().isEmpty()) {
				if (last.t == ':' || last.t == ',') {
					*err = FIS_14;
				} else {
					*err = FIS_6;    // пусто между скобками
				}
				*err_pos = last.p1;
				*err_pos_end = i;
				return;
			}
			while (mainExpr.length() > 0) {
				bool stop = mainExpr.endsWith(last.t);
				mainP.pop_back();
				mainL.pop_back();
				mainExpr.chop(1);
				if (stop) {
					break;
				}
			}
			QString repl = "&" + QString::number(EL->count());
			mainExpr += repl;




			for (int j = 0; j < repl.length(); j++) {
				mainP << last.p1 + 1;
				mainL << i - last.p1 - 1;
			}

			F_processSubExpr(subExpr, last.p1 + 1, P, L);
			EL->append(subExpr);
			continue;
		} else if (s[i] == ',') {
			if (stack.isEmpty()) {
				*err = FIS_7; // лишняя запятая
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			Bracket open = stack.last();
			if ((open.ot != '(') && (open.ot != '[')) {
				*err = FIS_8; // лишняя запятая
				*err_pos = i;
				*err_pos_end = i;
				return;
			}

			if (open.ot == '[') {
				// Проверка типа -- должен быть массив, иначе -- ошибка
				int block_start = open.p2;
				while ((s[block_start] != '^') && (s[block_start] != '%') && (s[block_start] != QChar(TYPE_SKOBKA))
					&& (block_start > 0)) {
					block_start--;
				}

				if (s[block_start] != '%') {
					*err = FIS_20; // "," вместо ":"
					*err_pos = i;
					*err_pos_end = i;
					return;
				}
			}

			Bracket last = stack.pop();
			int delPos = rxDelim.lastIndexIn(mainExpr);
			QString subExpr = mainExpr.mid(delPos + 1);
			if (subExpr.trimmed().isEmpty()) {
				if (s[i] == ',') {
					*err = FIS_8;
					*err_pos = i;
					*err_pos_end = i;
				} else {
					*err = FIS_9; // пусто между скобками
					*err_pos = last.p1;
					*err_pos_end = i;
				}
				return;
			}
			while (mainExpr.length() > 0) {
				bool stop = mainExpr.endsWith(last.t);
				mainP.pop_back();
				mainL.pop_back();
				mainExpr.chop(1);
				if (stop) {
					break;
				}
			}
			QString repl = "&" + QString::number(EL->count());
			mainExpr += repl;
			for (int j = 0; j < repl.length(); j++) {
				mainP << last.p1 + 1;
				mainL << i - last.p1 - 1;
			}

			F_processSubExpr(subExpr, last.p1 + 1, P, L);
			EL->append(subExpr);

			stack.push(Bracket(',', i, open.ot, open.p2));
		} else if (s[i] == ':') {
			if (stack.isEmpty()) {
				*err = FIS_12; // лишнее ":"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}
			Bracket open = stack.last();
			if ((open.ot != '(') && (open.ot != '[')) {
				*err = FIS_13; // лишнее ":"
				*err_pos = i;
				*err_pos_end = i;
				return;
			}

			// Проверка типа -- должен быть string, иначе -- ошибка
			int block_start = open.p2;
			while ((s[block_start] != '^') && (s[block_start] != '%') && (s[block_start] != QChar(TYPE_SKOBKA))
				&& (block_start > 0)) {
				block_start--;
			}

			if (s[block_start] != QChar(TYPE_SKOBKA)) {
				*err = FIS_19; // Вырезка не из строки
				*err_pos = i;
				*err_pos_end = i;
				return;
			}


			Bracket last = stack.pop();
			int delPos = rxDelim.lastIndexIn(mainExpr);
			QString subExpr = mainExpr.mid(delPos + 1);
			if (subExpr.trimmed().isEmpty()) {
				*err = FIS_14; // нет границы
				*err_pos = last.p1;
				*err_pos_end = i;
				return;
			}
			while (mainExpr.length() > 0) {
				bool stop = mainExpr.endsWith(last.t);
				mainP.pop_back();
				mainL.pop_back();
				mainExpr.chop(1);
				if (stop) {
					break;
				}
			}
			QString repl = "&" + QString::number(EL->count());
			mainExpr += repl;



			for (int j = 0; j < repl.length(); j++) {
				mainP << last.p1 + 1;
				mainL << i - last.p1;
			}

			F_processSubExpr(subExpr, last.p1 + 1, P, L);
			EL->append(subExpr);

			stack.push(Bracket(':', i, open.ot, open.p2));
		}
		mainExpr += s[i];
		mainP << i;
		mainL << 1;
	}
	//  s.remove(' ');
	if (!stack.isEmpty()) {
		if (stack.last().t == '[') {
			*err = FIS_15;
			*err_pos = stack.last().p1;
			*err_pos_end = stack.last().p1;
			return;
		}
		if (stack.last().t == '(') {
			*err = FIS_16;
			*err_pos = stack.last().p1;
			*err_pos_end = stack.last().p1;
			return;
		}
		if ((stack.last().t == ',') || (stack.last().t == ':')) {
			if (stack.last().ot == '[') {
				*err = FIS_17;
				*err_pos = stack.last().p2;
				*err_pos_end = stack.last().p2;
				return;
			}
			if (stack.last().ot == '(') {
				*err = FIS_18;
				*err_pos = stack.last().p2;
				*err_pos_end = stack.last().p2;
				return;
			}
		}

	}
	EL->append(mainExpr);
	//  P.append(mainP);
	//  L.append(mainL);
	F_processSubExpr(mainExpr, 0, P, L);
	//  qDebug() << "F: =====";
	//  for ( int i=0; i<EL->count(); i++ ) {
	//      qDebug() << EL->at(i) << "   |   " << P[i] << "   |   " << L[i];
	//  }
}



int text_analiz::parceAlgorhitmCall(proga &pv, function_table *table, symbol_table *symbols, int str)
{

	const QString curAlg = table->nameByPos(str);
	QString func = pv.line;
	pv.VirajTypes.clear();
	pv.VirajList.clear();

	//     func=func.trimmed();

	if (func.startsWith(QChar(KS_DLYA))) {
		Err_start = 0;
		Err_length = 1;
		return FUNC_STARTS_WITH_FOR;
	}

	if (func.startsWith(QChar(KS_POKA))) {
		Err_start = 0;
		Err_length = 1;
		return FUNC_STARTS_WITH_WHILE;
	}

	QString name, args;
	int func_id = -1;
	int skobk_o = func.indexOf("(");
	//int skobk_z;skobk_o=func.indexOf(

	//  if((skobk_z<func.length()-1)&&(skobk_z>-1))return 57;

	// проверка на ошибочное присваивание
	int equalSignPos = func.indexOf('=');
	if (equalSignPos != -1 && (equalSignPos < skobk_o || skobk_o == -1)) {
		Err_start = equalSignPos;
		Err_length = 1;
		return FUNC_EQUAL_SIGN_IN_NAME;
	}


	if (skobk_o == -1) {
		// Net '(' => net argumentov
		name = func;
		int func_id = -1;
		int module_id = -1;
		if (name.simplified().startsWith("@")) {
			if (!curAlg.startsWith("@")) {
				Err_start = 0;
				Err_length = name.length();
				return VYZ_NO_IMPL_2;
			}
		}
		if (name.simplified() == QString::fromUtf8("НАЗНАЧИТЬ")) {
			Err_start = 0;
			Err_length = name.length();
			return ASSIGN_NO_ARGUMENTS;
		}
		modules->FuncByName(name.simplified(), &module_id, &func_id);
		if (func_id == -1) {
			int tn_start = 0, tn_len = 0;
			int test = KumTools::instance()->test_name(name, tn_start, tn_len);
			if (test != 0) {
				Err_start = tn_start;
				Err_length = tn_len;
				return test;
			} else {
				Err_start = 0;
				Err_length = name.length();
				return VYZ_NO_IMPL_1;
			}
		}



		if (modules->module(module_id)->Functions()->isBroken(func_id)) {
			Err_start = 0;
			Err_length = name.length();
			return  VYZ_BAD_ALG;
		}
		if ((modules->module(module_id)->Functions()->typeById(func_id)) != none) {
			Err_start = 0;
			Err_length = name.length();
			return  VYZ_NO_PROC_1;
		}

		// NEW V.Y. 21.08.2006
		if (modules->module(module_id)->Functions()->argCountById(func_id) > 0) {
			Err_start = 0;
			Err_length = name.length();
			return VYZ_NO_ARGS;
		}

		pv.line = QString::number(func_id) + "|" + QString::number(module_id); //09-01-Mordol
		return 0;
	} else {
		args = func.mid(skobk_o);
		name = func.left(skobk_o);
		//      name = name.trimmed();
		//  outPutWindow->append("FUNC:"+name+"|");
		// outPutWindow->append("args:"+args+"|");
	};
	if (name.trimmed().isEmpty()) {
		Err_start = 0;
		Err_length = 1;
		return VYZ_NO_ALG_NAME;
	}
	func_id = -1;
	int module_id = -1;
	modules->FuncByName(name.simplified(), &module_id, &func_id);
	if (name.simplified().startsWith("@")) {
		if (!curAlg.startsWith("@")) {
			Err_start = 0;
			Err_length = skobk_o;
			return VYZ_NO_IMPL_2;
		}
	}
	if (module_id != moduleId) {
		if (name.simplified().startsWith("_")) {
			Err_start = 0;
			Err_length = name.length();
			return TA_PRIVATE_METHOD_CALL_1;
		}
	}
	if (func_id == -1 && name.simplified() != QString::fromUtf8("НАЗНАЧИТЬ")) {
		int test;
		int tn_start = 0, tn_len = 0;
		test = KumTools::instance()->test_name(name, tn_start, tn_len);
		if (test != 0) {
			Err_start = tn_start;
			Err_length = tn_len;
			return VYZ_BAD_SYM_2;
		} else {
			Err_start = 0;
			Err_length = skobk_o;
			return VYZ_NO_IMPL_2;
		}
	}
	if (func_id > -1 && modules->module(module_id)->Functions()->isBroken(func_id)) {
		Err_start = 0;
		Err_length = skobk_o;
		return  VYZ_BAD_ALG;
	}
	// NEW V.Y. -- check for ret type

	if (func_id > -1 && (modules->module(module_id)->Functions()->typeById(func_id)) != none) {
		Err_start = 0;
		Err_length = skobk_o;
		return  VYZ_NO_PROC_2;
	}




	QRegExp rxEmptyArgs = QRegExp("\\((\\s*)\\)");

	if (rxEmptyArgs.exactMatch(args)) {
		Err_start = 0;
		Err_length = skobk_o;
		return VYZ_EMPTY_BRAKCETS;
	}

	// moved here from below
	int s_deep = 0;
	int s_open_pos = 0;
	int s_close_pos = 0;
	QChar cChar;
	for (int i = 0; i < args.length(); i++) {
		cChar = args[i];
		if (cChar == '(') {
			s_deep++;
			s_open_pos = i;
		}
		if (cChar == ')') {
			s_deep--;
			s_close_pos = i;
		}
	}
	if (s_deep > 0) {
		Err_start = s_open_pos + skobk_o;
		Err_length = 1;
		return VYZ_BAD_LAST;
	}
	if ((s_close_pos + 1) < args.length()) {
		bool spacesOnly = true;
		QChar current;
		bool currentSpace;
		for (int i = s_close_pos + 1; i < args.length(); i++) {
			current = args[i];
			currentSpace = (current == ' ') || (current == '\t');
			spacesOnly = spacesOnly && currentSpace;
		}
		if (!spacesOnly) {
			Err_start = s_close_pos + skobk_o + 1;
			Err_length = args.length() - s_close_pos - 1;
			return VYZ_TRASH;
		}
	}



	QStringList args_viraj;

	QString viraj;
	int deep = 0;

	int open_pos = 0;
	int close_pos = 0;

	int func_length = func.length();

	while ((open_pos != -1) && (open_pos < func_length)) {
		open_pos = func.indexOf('(', open_pos);
		close_pos = func.indexOf(')', close_pos);
		if ((close_pos < open_pos) && (close_pos != -1)) {
			Err_start = close_pos;
			Err_length = 1;
			return VYZ_LISH_ZAKR;
		}
		if ((close_pos != -1) && (open_pos == -1)) {
			Err_start = close_pos;
			Err_length = 1;
			return VYZ_LISH_ZAKR;
		}
		if ((close_pos == -1) && (open_pos != -1)) {
			Err_start = open_pos;
			Err_length = 1;
			return VYZ_LISH_OTKR;
		}
		if (open_pos != -1) {
			open_pos++;
		}
		if (close_pos != -1) {
			close_pos++;
		}
	}

	if (func.count("(") > 1) {
		if (func.count(")") != func.count("(")) {
			int err_pos = func.indexOf(')');
			err_pos = func.indexOf('(', err_pos); // next ( after )

			Err_start = err_pos;
			Err_length = 1;
			return VYZ_LISH_OTKR;
		}
	}

	// END NEW

	args = args.trimmed();

	QList<int> leftBounds;

	for (int i = 1; i < args.length(); i++) { //� азделяем выражения
		// NEW V.Y. -- for no args
		// RegExp matches ( any_count_of_spaces )
		QRegExp rxNoArgs(QString("\\([\\s]*\\)"));
		if (rxNoArgs.exactMatch(args)) {
			break;
		}
		// END
		if ((args[i] == '[') || (args[i] == '(')) {
			deep++;
		}
		if ((args[i] == ']') || (args[i] == ')')) {
			deep--;
		}
		if (((deep == 0) && (args[i] == ',')) || (i == args.length() - 1)) {

			args_viraj.append(viraj);
			int shft = i + skobk_o - viraj.length();
			leftBounds << shft;
			//outPutWindow->append("viraj:"+viraj+"|");
			viraj = "";
			continue;
		};
		viraj += args[i];
		if (deep < 0) {
			int err_pos = func.indexOf('(');
			int last_pos = -1;
			bool isClosed = false;
			int func_length = func.length();
			while ((err_pos < func_length) && (err_pos != -1)) {
				int close_pos = func.indexOf(')', err_pos + 1);
				if (close_pos != -1) {
					isClosed = true;
				}
				last_pos = err_pos;
				err_pos = func.indexOf('(', err_pos + 1);
			}
			if ((last_pos != -1) && (!isClosed)) {
				Err_start = last_pos;
				Err_length = 1;
				return VYZ_LISH_OTKR;
			}
		}
	} // end for (i)

	if (name.simplified() == QString::fromUtf8("НАЗНАЧИТЬ") && func_id == -1) {
		if (args_viraj.count() != 2) {
			Err_start = skobk_o + 1;
			int Err_end = func.lastIndexOf(')');
			Err_length = Err_end - Err_start;
			return ASSIGN_IO_WONG_ARGS_COUNT;
		}
	} else if (args_viraj.count() != modules->module(module_id)->Functions()->argCountById(func_id)) {
		Err_start = skobk_o + 1;
		int Err_end = func.lastIndexOf(')');
		Err_length = Err_end - Err_start;
		return VYZ_PARAM_COUNT_MISMATCH;
	}

	if (deep > 0) {
		int err_pos = func.indexOf('(');
		int last_pos = -1;
		bool isClosed = false;
		int fl = func.length();
		while ((err_pos < fl) && (err_pos != -1)) {
			int close_pos = func.indexOf(')', err_pos + 1);
			if (close_pos != -1) {
				isClosed = true;
			}
			last_pos = err_pos;
			err_pos = func.indexOf('(', err_pos + 1);
		}
		if ((last_pos != -1) && (!isClosed)) {
			Err_start = last_pos;
			Err_length = 1;
			return VYZ_LISH_OTKR;
		}
	}

	int av_count = args_viraj.count();

	if (func_id > -1 && av_count != modules->module(module_id)->Functions()->argCountById(func_id)) {
		Err_start = func.indexOf('(');
		int end = func.lastIndexOf(')');
		Err_length = end - Err_start;
		return VYZ_BAD_ARG_NUM;
	}

	QStringList parsedViraj;
	QStringList extraSubExpressions;
	QString callExpr;
	callExpr = "@" + QString::number(func_id);

	foreach (QString arg, args_viraj) {
		if (arg.trimmed().isEmpty()) {
			Err_start = func.indexOf('(');
			int end = func.lastIndexOf(')');
			Err_length = end - Err_start;
			return VYZ_EMPTY_ARG;
		}
	}

	if (name == QObject::trUtf8("установить цвет")) {
		if (modules->module(module_id)->isInternal()) {
			if (args_viraj.count() > 0) {
				QString arg = args_viraj[0];
				int arglen = arg.length();
				if (arg.trimmed().startsWith("\"") && arg.trimmed().endsWith("\"") && arg.count("\"") == 2) {
					arg.remove("\"");
					for (int i = 0; i < arg.count(); i++) {
						arg[i] = QChar(arg[i].unicode() - STR_HIDE_OFFSET);
					}
					QRegExp validColorNames = QRegExp(QObject::trUtf8("черный|белый|красный|оранжевый|желтый|зеленый|голубой|синий|фиолетовый"));
					if (!validColorNames.exactMatch(arg.trimmed())) {
						Err_start = leftBounds[0];
						Err_length = arglen;
						return BAD_COLOR;
					}
				}
			}
		}
	}

	if (name.trimmed() == QString::fromUtf8("НАЗНАЧИТЬ")) {
		if (app()->isExamMode()) {
			return UNKN_ALG;
		};
		int assign_out_id = -1;
		int assign_in_id = -1;
		int assign_id = -1;
		modules->FuncByName(QString::fromUtf8("НАЗНАЧИТЬ ВВОД"),
			&module_id,
			&assign_in_id);
		modules->FuncByName(QString::fromUtf8("НАЗНАЧИТЬ ВЫВОД"),
			&module_id,
			&assign_out_id);
		const QString firstArg = args_viraj.at(0).trimmed();
		const QString secondArg = args_viraj.at(1);

		if (firstArg.size() != 1) {
			Err_start = func.indexOf("(") + 1;
			Err_length = func.indexOf(",") - Err_start;
			return ASSIGN_IO_WRONG_FIRST_ARGUMENT;
		}
		if (firstArg[0] == QChar(KS_VVOD)) {
			assign_id = assign_in_id;
		} else if (firstArg[0] == QChar(KS_VIVOD)) {
			assign_id = assign_out_id;
		} else {
			Err_start = func.indexOf("(") + 1;
			Err_length = func.indexOf(",") - Err_start;
			return ASSIGN_IO_WRONG_FIRST_ARGUMENT;
		}

		int err = 0;
		ErrorPosition local_err_pos;

		StructType VirStruct;
		PeremType intType =
			parceExpression(args_viraj[1],
				table,
				symbols,
				table->nameByPos(str),
				&err,
				&parsedViraj,
				&VirStruct,
				&local_err_pos,
				extraSubExpressions,
				false);

		if (err) {
			int sh = func.indexOf(",") + 1;
			Err_start = local_err_pos.start + sh;
			Err_length = local_err_pos.len;
			return err;
		}

		if (intType != kumString && intType != charect) {
			Err_start = func.indexOf(",") + 1;
			Err_length = args_viraj[1].length();
			return ASSIGN_IO_WRONG_SECOND_ARGUMENT;
		}

		callExpr = "@" + QString::number(assign_id) + "&0";
		pv.VirajTypes.append(intType);
		pv.line = "@" + QString::number(assign_id) + "|0";

	} else
		for (int i = 0; i < args_viraj.count(); i++) {
			Err_start = func.indexOf('(');
			Err_length = 1;
			int err = 0;

			ErrorPosition local_err_pos;

			StructType VirStruct;
			PeremType intType =
				parceExpression(args_viraj[i],
					table,
					symbols,
					table->nameByPos(str),
					&err,
					&parsedViraj,
					&VirStruct,
					&local_err_pos,
					extraSubExpressions,
					false);


			PeremType extType = modules->module(module_id)->Functions()->argTypeById(func_id, i);
			callExpr += "&" + QString::number(parsedViraj.count() - 1);

			bool allowTypeChange = !modules->module(module_id)->Functions()->isRes(func_id, i)
				&& !modules->module(module_id)->Functions()->isArgRes(func_id, i);

			if (err > 0) {
				int sh = leftBounds[i];
				Err_start = local_err_pos.start + sh;
				Err_length = local_err_pos.len;
				return err;
			}
			if (extType != intType) {
				if (!allowTypeChange || (!((intType == integer) && (extType == real)) &&
						!((intType == charect) && (extType == kumString)) /*&&
                                                                                               !((intType==mass_integer)&&(extType==mass_real))*/
					)
				) {
					Err_start = leftBounds[i];
					Err_length = args_viraj[i].length();
					return VYZ_TYPE_MISS_1;
				}
			}

			if (modules->module(module_id)->Functions()->isRes(func_id, i)) {
				if (
					(VirStruct != simple_var) &&
					(VirStruct != array)   /*&& (VirStruct != string_elem )*/
				) {
					Err_start = leftBounds[i];
					Err_length = args_viraj[i].length();
					return VYZ_RES_NOT_SIMPLE;
				}
			}
			if (modules->module(module_id)->Functions()->isArgRes(func_id, i)) {
				if (
					(VirStruct != simple_var) &&
					(VirStruct != array)
				) {
					Err_start = leftBounds[i];
					Err_length = args_viraj[i].length();
					return VYZ_ARGRES_NOT_SIMPLE;
				}
			}
			bool isRes = modules->module(module_id)->Functions()->isRes(func_id, i);
			bool isArgRes = modules->module(module_id)->Functions()->isArgRes(func_id, i);
			if (isRes || isArgRes) {
				// Имя алгоритма, из которого происходит вызов
				// (нужно для определения ID переменной)
				QString caller_f_name = table->nameByPos(str);
				args_viraj[i] = args_viraj[i].trimmed();
				int perem_id = symbols->inTable(args_viraj[i], caller_f_name);
				if (perem_id == -1) {
					perem_id = symbols->inTable(args_viraj[i], "global");
				}
				if (symbols->symb_table[perem_id].is_Arg && !(symbols->symb_table[perem_id].is_Res || symbols->symb_table[perem_id].is_ArgRes)) {
					Err_start = leftBounds[i];
					Err_length = args_viraj[i].length();
					return isRes ? VYZ_ARG_AS_RES : VYZ_ARG_AS_ARGRES;
				}
			}
			// Проверка на размерности массива
			if (((intType == mass_integer) || (intType == mass_real) || (intType == mass_bool) || (intType == mass_charect) || (intType == mass_string)) && (VirStruct == array)) {
				// Имя алгоритма, из которого происходит вызов
				// (нужно для определения ID переменной)
				QString caller_f_name = table->nameByPos(str);
				args_viraj[i] = args_viraj[i].trimmed();
				int perem_id = symbols->inTable(args_viraj[i], caller_f_name);
				if (perem_id == -1) {
					perem_id = symbols->inTable(args_viraj[i], "global");
				}
				// � азмерность передаваемого массива
				int intRazm = symbols->symb_table[perem_id].razm;
				// � азмерность аргумента функции
				int extRazm = modules->module(module_id)->Functions()->argDimById(func_id, i);
				if (intRazm != extRazm) {
					Err_start = leftBounds[i];
					Err_length = args_viraj[i].length();
					return VYZ_MASS_BAD_RAZM;

				}
			}
			// Конец проверки размерностей массива


			pv.VirajTypes.append(intType);

			pv.line = QString::number(func_id) + "|" + QString::number(module_id); // ??? 30.08
			//      qDebug() << *xml;
		}

	parsedViraj.append(callExpr);
	pv.VirajList.append(parsedViraj);
	pv.extraSubExpressions = extraSubExpressions;

	return 0;
}

QStringList text_analiz::splitMassArgs(QString mass_args, QList<int> &starts, int *err)
{
	QStringList temp;
	QString viraj;
	temp.clear();
	int last_zpt_pos = 0;
	int deep_sk = 0;
	int deep_mass = 0;
	for (int i = 0; i < mass_args.length(); i++) {
		if (mass_args[i] == '(') {
			deep_sk++;
		}
		if (mass_args[i] == ')') {
			deep_sk--;
		}

		if (mass_args[i] == '[') {
			deep_mass++;
		}
		if (mass_args[i] == ']') {
			deep_mass--;
		}

		if ((deep_mass < 0) || (deep_sk < 0)) {
			*err = SYNTAX_ERR;
			return temp;
		};
		if ((deep_mass == 0) && (deep_sk == 0) && (mass_args[i] == ',')) {
			viraj = mass_args.mid(last_zpt_pos, i - last_zpt_pos);
			starts << last_zpt_pos;
			last_zpt_pos = i + 1;
			temp.append(viraj);

		};

	};


	viraj = mass_args.mid(last_zpt_pos, mass_args.length() - last_zpt_pos);
	starts << last_zpt_pos;
	if (viraj[0] == ',') {
		viraj = viraj.mid(1);
	}
	temp.append(viraj);



	return temp;

};



/**
* Возвращет тип переменной в массиве. Если не массив возвращеет тип исходной переменной
* @param inType тип массива
* @return тип переменной
 */
PeremType text_analiz::retUnMassType(PeremType inType)
{
	if ((inType == integer) || (inType == charect) || (inType == real) || (inType == kumBoolean)) {
		return inType;
	};
	if ((inType == kumString)) {
		return charect;
	}
	if ((inType == mass_string)) {
		return kumString;
	}
	if ((inType == mass_integer)) {
		return integer;
	}
	if ((inType == mass_real)) {
		return real;
	}
	if ((inType == mass_bool)) {
		return kumBoolean;
	}
	if ((inType == mass_charect)) {
		return charect;
	}

	return none;

}



int text_analiz::parceAssignment(proga &pv, function_table *table, symbol_table *symbols, int str)
{
	QString prisv = pv.line;

	// левая и правая части присваивания
	QString left, right;

	// разделяем выражение на две части
	int assignPos = prisv.indexOf(QChar(KS_PRISV));
	K_ASSERT(assignPos > -1);
	left = prisv.left(assignPos);
	right = prisv.mid(assignPos + 1);

	// левая часть должна быть непустой
	if (left.simplified().isEmpty()) {
		Err_start = assignPos;
		Err_length = 1;
		return PRISV_NO_LEFT;
	}

	// правая часть должна быть непустой
	if (right.simplified().isEmpty()) {
		Err_start = assignPos;
		Err_length = 1;
		return PRISV_NO_RIGHT;
	}

	// список подвыражений, учавствующих в присваивании
	QStringList exprList;

	// типы выражений слева и справа
	PeremType leftType, rightType;
	// структура выражения слева
	StructType leftStructType;

	// выполняем разбор выражения слева
	int local_error = 0;

	ErrorPosition local_err_pos;
	QString prisvElem, leftElem, rightElem;

	QString currentAlgorhitm = table->nameByPos(str);

	leftType = parceExpression(left,
			table,
			symbols,
			currentAlgorhitm,
			&local_error,
			&exprList,
			&leftStructType,
			&local_err_pos,
			pv.extraSubExpressions,
			true);
	if (local_error > 0) {
		Err_start = local_err_pos.start;
		Err_length = local_err_pos.len;
		return local_error;
	}

	// выражение слева может быть простой переменной, элементом массива, элементом строки или вырезкой
	if (leftStructType != simple_var && leftStructType != array_elem && leftStructType != string_elem && leftStructType != substring) {
		Err_start = 0;
		Err_length = assignPos;
		switch (leftStructType) {
		case constant:
			return PRISV_CONST;
		case array:
			return PRISV_MASS;
		case complex:
			return PRISV_COMPLEX;
		default:
			return PRISV_NONE;
		}
	}

	if (leftType == mass_integer || leftType == mass_real || leftType == mass_charect || leftType == mass_string || leftType == mass_bool) {
		Err_start = assignPos;
		Err_length = 1;
		return PRISV_LEFT_ARRAY;
	}
	// Знач может быть только у функций, возвращающих значение
	if (left.simplified() == QObject::trUtf8("знач")) {
		int fID = table->idByPos(str);
		if (fID == -1 || table->typeById(fID) == none) {
			Err_start = 0;
			Err_length = assignPos;
			return PRISV_EXTRA_RETURN;
		}

	}

	// Аргумент алгоритма по умолчанию является const, поэтому ему нельзя присваивать
	QString leftCode = exprList.last();
	QRegExp rxNumber("[0-9]+");
	int p = rxNumber.indexIn(leftCode);
	int variable_id = leftCode.mid(p, rxNumber.matchedLength()).toInt();
	if (symbols->symb_table[variable_id].is_Arg && !(symbols->symb_table[variable_id].is_Res || symbols->symb_table[variable_id].is_ArgRes)) {
		Err_start = 0;
		Err_length = assignPos;
		return PRISV_LEFT_IS_ARG;
	}

	// результирующая строка
	// Имеет формат: &номер_выражения_слева_в_общем_списке{PRISV}&номер_выражения_справа_в_общем_списке
	// Пример: &2{PRISV}&7
	//   -- вычислить значение 7-го подвыражения в списке и записать его в переменную,
	//   -- которая вычисляется во 2-м подвыражении списка
	QString out;

	out = "&" + QString::number(exprList.count() - 1) + QString(QChar(KS_PRISV));

	// выполняем разбор выражения справа

	rightType = parceExpression2(right,
			table,
			symbols,
			currentAlgorhitm,
			&local_error,
			&exprList,
			&local_err_pos,
			pv.extraSubExpressions,
			false);

	if (local_error > 0) {
		Err_start = local_err_pos.start + assignPos + 1;
		Err_length = local_err_pos.len;
		return local_error;
	}

	if (rightType == mass_integer || rightType == mass_real || rightType == mass_charect || rightType == mass_string || rightType == mass_bool) {
		Err_start = assignPos;
		Err_length = 1;
		return PRISV_RIGHT_ARRAY;
	}

	// проверяем соответствие типов и выбираем подходящее ругательство

	if (leftType != rightType) {
		Err_start = assignPos;
		Err_length = 1;
		if (leftType == integer) {
			if (rightType == real) {
				return PRISV_INT_REAL;
			}
			if (rightType == charect) {
				return PRISV_INT_CHAR;
			}
			if (rightType == kumString) {
				return PRISV_INT_STRING;
			}
			if (rightType == kumBoolean) {
				return PRISV_INT_BOOL;
			}
			if (rightType == mass_real) {
				return PRISV_INT_MASSREAL;
			}
			if (rightType == mass_charect) {
				return PRISV_INT_MASSCHAR;
			}
			if (rightType == mass_string) {
				return PRISV_INT_MASSSTRING;
			}
			if (rightType == mass_bool) {
				return PRISV_INT_MASSBOOL;
			}
		}
		if (leftType == real) {
			if (rightType == charect) {
				return PRISV_REAL_CHAR;
			}
			if (rightType == kumString) {
				return PRISV_REAL_STRING;
			}
			if (rightType == kumBoolean) {
				return PRISV_REAL_BOOL;
			}
			if (rightType == mass_real) {
				return PRISV_REAL_MASSREAL;
			}
			if (rightType == mass_charect) {
				return PRISV_REAL_MASSCHAR;
			}
			if (rightType == mass_string) {
				return PRISV_REAL_MASSSTRING;
			}
			if (rightType == mass_bool) {
				return PRISV_REAL_MASSBOOL;
			}
		}
		if (leftType == charect) {
			if (rightType == real) {
				return PRISV_CHAR_REAL;
			}
			if (rightType == integer) {
				return PRISV_CHAR_INT;
			}
			if (rightType == kumString) {
				return PRISV_CHAR_STRING;
			}
			if (rightType == kumBoolean) {
				return PRISV_CHAR_BOOL;
			}
			if (rightType == mass_real) {
				return PRISV_CHAR_MASSREAL;
			}
			if (rightType == mass_charect) {
				return PRISV_CHAR_MASSCHAR;
			}
			if (rightType == mass_string) {
				return PRISV_CHAR_MASSSTRING;
			}
			if (rightType == mass_bool) {
				return PRISV_CHAR_MASSBOOL;
			}
		}
		if (leftType == kumBoolean) {
			if (rightType == real) {
				return PRISV_BOOL_REAL;
			}
			if (rightType == integer) {
				return PRISV_BOOL_INT;
			}
			if (rightType == kumString) {
				return PRISV_BOOL_STRING;
			}
			if (rightType == integer) {
				return PRISV_BOOL_INT;
			}
			if (rightType == mass_real) {
				return PRISV_BOOL_MASSREAL;
			}
			if (rightType == mass_charect) {
				return PRISV_BOOL_MASSCHAR;
			}
			if (rightType == mass_string) {
				return PRISV_BOOL_MASSSTRING;
			}
			if (rightType == mass_bool) {
				return PRISV_BOOL_MASSBOOL;
			}
		}
		if (leftType == kumString) {
			if (rightType == real) {
				return PRISV_STR_REAL;
			}
			if (rightType == integer) {
				return PRISV_STR_INT;
			}
			if (rightType == kumBoolean) {
				return PRISV_STR_BOOL;
			}
			if (rightType == integer) {
				return PRISV_STR_INT;
			}
			if (rightType == mass_real) {
				return PRISV_STR_MASSREAL;
			}
			if (rightType == mass_charect) {
				return PRISV_STR_MASSCHAR;
			}
			if (rightType == mass_string) {
				return PRISV_STR_MASSSTRING;
			}
			if (rightType == mass_bool) {
				return PRISV_STR_MASSBOOL;
			}
		}
	}

	// добавляем ссылку на правую часть в результирующую строку
	out += "&" + QString::number(exprList.count() - 1);

	// заносим все в STR_DATA
	pv.VirajList.clear();
	pv.VirajList << exprList;
	pv.line = out;

	return 0;
}



//==================================== CLASS text_analiz ========================================
PeremType getPeremTypeByKS(int p_KS)
{
	const PeremType l_PeremTypes[] = {
		integer,
		mass_integer,
		real,
		mass_real,
		none,
		none,
		none,
		kumString,
		mass_string,
		charect,
		mass_charect,
		none,
		kumBoolean,
		mass_bool
	};
	if (p_KS < KS_CEL || p_KS > KS_LOG_TAB) {
		return none;
	}
	return l_PeremTypes[p_KS - KS_CEL];
}

int text_analiz::parceVariablesDeclaration(proga &pv, int str, QString alg, symbol_table *symbols, function_table *table)
{
	if (m_denyVariableDeclaration > 0) {
		Err_start = 0;
		Err_length = pv.line.length();
		return SYMB_DECL_IN_LOOP;
	}
	QString stroka = pv.line;
	int ispId = moduleId;
	QList<int> ids;
	ErrorPosition local_err_pos;
	pv.VirajList.clear();
	if (stroka.trimmed().endsWith(",")) {
		Err_start = stroka.lastIndexOf(",");
		Err_length = 1;
		return SYMB_EXTRA_COMMA;
	}
	QList<proga> extraProgram;
	int local_err = parceVariables(stroka, ispId, alg, arg, str, symbols, table, local_err_pos, ids, pv.VirajList, pv.extraSubExpressions, false, extraProgram);
	Err_start = local_err_pos.start;
	Err_length = local_err_pos.len;
	int counter = 0;
	pv.line = "";
	for (int i = 0; i < extraProgram.count(); i++) {
		extraProgram[i].real_line = pv.real_line;
	}
	pv.generatedLinesAfter = extraProgram;

	if (local_err == 0) {
		for (int i = 0; i < ids.count(); i++) {
			PeremType type = symbols->symb_table[ids[i]].type;

			if ((type == integer) || (type == real) || (type == kumBoolean) || (type == kumString) || (type == charect)) {
				pv.line += "^" + QString::number(ids[i]);
			} else {
				pv.line += "^" + QString::number(ids[i]);
				int dim = symbols->symb_table[ids[i]].razm;
				for (int j = 0; j < dim * 2; j++) {
					pv.line += "#" + QString::number(counter);
					counter++;
				}
			}

			if (i != (ids.count() - 1)) {
				pv.line += ",";
			}
		}
	}

	return local_err;
}


/**
* Парсировка "если ...". Использует глоб. переменные. Набор аргументов стандартный.
* @param instr
* @param functions
* @param symbols
* @param str
* @return Код ошибки
 */
int text_analiz::parce_if(proga &pv, symbol_table *symbols, function_table *functions, int str)
{
	QString instr = pv.line;
	int to_pos = instr.indexOf(QChar(KS_TO));
	QString uslovie = "";
	if (to_pos > -1) {
		uslovie = instr.mid(1, to_pos - 2);
		to_pos = instr.length();
	} else {
		uslovie = instr.mid(1);
	}
	if (uslovie.isNull()) {

		return IF_NO_CONDITION_1;
	}
	if (uslovie == "\n") {

		return IF_NO_CONDITION_2;
	}
	if (uslovie.isEmpty()) {

		return IF_NO_CONDITION_3;
	}

	if (KumTools::instance()->test_skobk(uslovie) != 0) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return IF_BAD_CONDITION;
	}

	int err = 0;
	//QStringList virajList=KumTools::instance()->split_prisv(uslovie);
	QStringList parsedViraj;
	QStringList resViraj;

	ErrorPosition local_err_pos;


	PeremType intType = parceExpression2(uslovie,
			functions,
			symbols,
			functions->nameByPos(str),
			&err,
			&parsedViraj,
			&local_err_pos,
			pv.extraSubExpressions,
			false);

	if (err > 0) {
		// NEW V.Y. 21.08.2006-22.08.2006

		Err_start = 1 + local_err_pos.start;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 2;
		Err_length = instr.length() - Err_start;
		return IF_TYPE_MISS;
	};
	pv.VirajList.clear();

	pv.VirajList.append(parsedViraj);

	pv.line = instr;

	return 0;
};

/**
*  Парсировка строки цикла. Использует глоб. переменные.
*
* @param instr Строка
* @param symbols Таблица функций
* @param functions Таблица переменных
* @param str Номер строки в тексте
* @return Код ошибки.
 */
int text_analiz::parceLoopBegin(proga &pv, symbol_table *symbols, function_table *functions, int str)
{
	QString instr = pv.line;
	int iz, iz1;
	int circle_type = 0;
	QString out_line = instr;
	int ras_pos = instr.indexOf(QChar(KS_RAZ));
	pv.VirajList.clear();

	circle_type = -1;

	if (instr[2] == QChar(KS_DLYA)) {
		circle_type = FOR_CIRCLE;
	} else if (instr[2] == QChar(KS_POKA)) {
		circle_type = WHILE_CIRCLE;
	}

	else if (instr.length() == 1) {
		circle_type = FREE_CIRCLE;
	}

	else if (ras_pos > -1) {
		circle_type = RAZ_CIRCLE;
	}

	else {
		Err_start = 0;
		Err_length = instr.length();
		return CIKL_NO_KEYWORD;
	}


	//if ((ras_pos>-1)&&(ras_pos<=2))
	//  return NO_EXP;

	if (circle_type == FOR_CIRCLE) {
		int ot_pos = instr.indexOf(QChar(KS_OT));
		out_line = QChar(KS_NC) + ' ' + QChar(KS_DLYA) + ' ';
		if (ot_pos < 5 && ot_pos > -1) {
			Err_start = 2;
			Err_length = 1;
			return CIKL_NO_LOOP_VAR;
		} else if (ot_pos < 5) {
			Err_start = 2;
			Err_length = 1;
			return CICK_NO_FROM;
		}

		// V.Y. -- moved check for 'do' from below
		int do_pos = instr.indexOf(QChar(KS_DO));
		if (do_pos == -1) {
			Err_start = 0;
			Err_length = instr.length();
			return CIKL_NO_DO;
		}

		if (do_pos < ot_pos + 1) {
			//MR-8
			// Mark ot OT do DO
			iz = do_pos;
			iz1 = ot_pos;
			if (ot_pos < do_pos) {
				iz = ot_pos;
				iz1 = do_pos;
			}
			Err_start = iz;
			Err_length = iz1 - iz;
			return CIKL_DO_NE_TAM; // 2607
			//RM
		}


		QString circle_perem = instr.mid(4, ot_pos - 5);
		QString alg = functions->nameByPos(str);
		int perem_id = symbols->inTable(circle_perem, functions->nameByPos(str));

		if (perem_id == -1) {
			perem_id = symbols->inTable(circle_perem, "global");
		}



		// test for constant -- NEW V.Y.
		bool isLiteralConst = (circle_perem.startsWith("\"") && circle_perem.endsWith("\""))
			|| (circle_perem.startsWith("'") && circle_perem.endsWith("'"));

		QRegExp rxNumericConst(QObject::trUtf8("^[0-9]*\\.?[0-9]+[EЕ]?[0-9]*\\.?[0-9]*"));

		if (
			(rxNumericConst.indexIn(circle_perem) != -1)
			|| isLiteralConst
		) {
			Err_start = 4;
			Err_length = circle_perem.length();
			return CIKL_CONST;
		}
		if (perem_id < 0) {
			Err_start = 4;
			Err_length = circle_perem.length();
			return CIKL_NO_IMPL;
		}

		if (symbols->getTypeByID(perem_id) != integer) {
			Err_start = 4;
			Err_length = circle_perem.length();
			return CIKL_PARAM_TYPE_ERR;
		}
		out_line = out_line + '^' + QString::number(perem_id) + ' ';



		//от проверка
		QString viraj = instr.mid(ot_pos + 2, do_pos - ot_pos - 2);
		//QStringList virajList=KumTools::instance()->split_prisv(viraj);
		int err = 0;


		QStringList parsedViraj;

		ErrorPosition local_err_pos;

		if (viraj.trimmed().isEmpty()) {
			Err_start = ot_pos;
			Err_length = do_pos - ot_pos;
			return CICK_EMPTY_FROM;
		}


		PeremType intType = parceExpression2(viraj, // source
				functions, // function table
				symbols, // symbol table
				functions->nameByPos(str), // current algorhitm
				&err, // error code
				&parsedViraj, // expr list

				&local_err_pos, // error position
				// moduleId, // module id
				pv.extraSubExpressions,
				false // id left part
			); // xml out

		if (err > 0) {
			Err_start = local_err_pos.start + ot_pos + 2;
			Err_length = local_err_pos.len;
			return err;
		}
		if (intType != integer) {
			Err_start = ot_pos + 2;
			Err_length = viraj.length();
			return CIKL_TYPE_MISS_1;
		}
		out_line = out_line + QChar(KS_OT) + " #0 ";
		pv.VirajList.append(parsedViraj);

		parsedViraj.clear();
		//до проверка
		int shag = instr.indexOf(QChar(KS_SHAG));
		if (shag > -1) {
			viraj = instr.mid(do_pos + 1, instr.indexOf(QChar(KS_SHAG)) - do_pos - 1);
		} else {
			viraj = instr.mid(do_pos + 1);
		}

		if (viraj.simplified().isEmpty()) {
			Err_start = do_pos;
			Err_length = 1;
			return CICK_EMPTY_TO;
		}

		iz = shag;

		if (iz == -1) {
			iz = instr.length();
		}
		//virajList=KumTools::instance()->split_prisv(viraj);

		intType = parceExpression2(viraj, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

		if (err > 0) {
			Err_start = local_err_pos.start + do_pos + 1;
			Err_length = local_err_pos.len;
			return err;
		}
		if (intType != integer) {
			Err_start = do_pos + 1;
			Err_length = viraj.trimmed().length() + 1;
			return CIKL_TYPE_MISS_2;
		}

		QStringList resViraj;
		//KumTools::instance()->SplitPrisvBlock(parsedViraj,&resViraj,-1,false);
		pv.VirajList.append(parsedViraj);
		out_line = out_line + QChar(KS_DO) + " #1 ";

		if (shag > -1) { //для циклов с шагом
			viraj = instr.mid(shag + 1);
			//virajList=KumTools::instance()->split_prisv(viraj);

			if (viraj.simplified().isEmpty()) {
				Err_start = shag;
				Err_length = 1;
				return CICK_EMPTY_STEP;
			}

			intType = parceExpression2(viraj, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);


			if (err > 0) {
				Err_start = local_err_pos.start + shag + 1;
				Err_length = local_err_pos.len;
				return err;
			}
			if (intType != integer) {
				Err_start = shag + 1;
				Err_length = viraj.trimmed().length() + 1;
				return CIKL_TYPE_MISS_3;
			}

			// KumTools::instance()->SplitPrisvBlock(parsedViraj,&resViraj,-1,false);
			pv.VirajList.append(parsedViraj);
			out_line = out_line + QChar(KS_SHAG) + " #2 ";

		};

	};

	if (circle_type == RAZ_CIRCLE) {
		if (ras_pos <= 2) {
			Err_start = ras_pos;
			Err_length = 1;
			return CIKL_SKOLKO_RAZ;  // 2611
		}
		int instr_len = instr.length();
		if (ras_pos != (instr_len - 1)) {
			Err_start = ras_pos + 1;
			Err_length = instr.length() - Err_start;
			return CIKL_TRASH_RAZ;
		}
		out_line = QChar(KS_NC) + ' ';
		QString viraj = instr.mid(2, ras_pos - 2);

		int err = 0;

		QStringList parsedViraj;
		ErrorPosition local_err_pos;

		PeremType intType = parceExpression2(viraj, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

		if (err > 0) {
			Err_start = local_err_pos.start + 2;
			Err_length = local_err_pos.len;
			return err;
		}
		if (intType != integer) {
			Err_start = 2;
			Err_length = viraj.length();
			return CIKL_TYPE_MISS_4;    // 2613
		}
		out_line = out_line + "#0 " + QChar(KS_RAZ);
		//    QStringList resViraj;
		//     KumTools::instance()->SplitPrisvBlock(parsedViraj,&resViraj,-1,false);
		pv.VirajList.append(parsedViraj);

	};

	if (circle_type == WHILE_CIRCLE) {

		out_line = QChar(KS_NC) + ' ' + QChar(KS_POKA);
		QString viraj = instr.mid(3);
		if ((viraj.isEmpty()) || (viraj.isNull()) || (viraj == "\n")) {
			Err_start = 2;
			Err_length = 1;
			return CIKL_NO_CONDITION;
		}
		//        QStringList virajList=KumTools::instance()->split_prisv(viraj);
		int err = 0;
		QStringList parsedViraj;
		ErrorPosition local_err_pos;

		PeremType intType = parceExpression2(viraj,
				functions, symbols, functions->nameByPos(str),
				&err,
				&parsedViraj,

				&local_err_pos, pv.extraSubExpressions, false);

		if (err > 0) {
			Err_start = local_err_pos.start + 3;
			Err_length = local_err_pos.len;
			return err;
		}
		if (intType != kumBoolean) {
			Err_start = 4;
			Err_length = instr.length() - 4;
			return CIKL_TYPE_MISS_5;
		}
		out_line = out_line + " #0";
		pv.VirajList.append(parsedViraj);

	};

	if (circle_type == FREE_CIRCLE) {
		out_line = QChar(KS_NC);
	};

	pv.line = out_line;

	//if (circle_type>0) return 0;else return SYNTAX_ERR;
	return 0;

}


//++++++++++++++++++++++ANALIZ
/**
* Проверка правильности расстановки опер. скобок. и расстановка строк прыжка
* @param nagr Нагрузка текста
* @param Proga_Value Внутреннее представление текста см form1.ui.h
 */

struct StackElem {
	StackElem(int a, int b, int c)
	{
		str_type = a;
		open_str = b;
		elem_str = c;
	}
	StackElem()
	{
		str_type = 0;
		open_str = 0;
		elem_str = 0;
	}
	int str_type;
	int open_str;
	int elem_str;
};





/**
 * Парсировка команды "вывод". Использует глоб. переменные. Набор аргументов стандартный.
 * @param prisv
 * @param functions
 * @param symbols
 * @param str
 * @return Код ошибки
 */
int text_analiz::parceOutput(
	proga &pv,
	function_table *functions,
	symbol_table *symbols,
	int str
)
{
	QString prisv = pv.line;
	if (prisv.trimmed().length() == 1) {
		Err_start = 0;
		Err_length = 1;
		return VYVOD_EMPTY;
	}
	if (prisv.simplified().length() > 0 && !prisv.simplified()[1].isSpace()) {

		return VYVOD_NO_SPACE;
	}
	QString viraj = prisv.mid(2);
	QStringList virajs;
	QList<int> start_pos;
	QList<int> end_pos;
	// NEW 24.08
	if (viraj.trimmed().length() == 0) {

		return VYVOD_EMPTY;
	}
	// END NEW

	//                      A. Prolog
	int skobka_op = 0, skobka_mass = 0, last_zpt = 1;
	//              A1. Form virajs - spisok termov vyvoda
	for (int i = 0; i < prisv.length(); i++) {
		if (prisv[i] == '(') {
			skobka_op++;
		}
		if (prisv[i] == ')') {
			skobka_op--;
		}

		if (prisv[i] == '[') {
			skobka_mass++;
		}
		if (prisv[i] == ']') {
			skobka_mass--;
		}

		//if(skobka_op==0)skobka_mass=0;

		if ((skobka_op < 0) || (skobka_mass < 0)) {
			Err_start = 2 + viraj.indexOf(')');
			Err_length = 1;
			return VYVOD_BAD_SKOBKA_1;
		}
		if ((prisv[i] == ',') && (skobka_op == 0) && (skobka_mass == 0)) {
			QString test = prisv.mid(last_zpt + 1, i - last_zpt - 1);
			virajs.append(prisv.mid(last_zpt + 1, i - last_zpt - 1));
			start_pos.append(last_zpt + 1);
			end_pos.append(i);
			last_zpt = i;
		}
	} // end for ( i )
	if ((skobka_op > 0) || (skobka_mass > 0)) {
		Err_start = 2 + viraj.lastIndexOf('(');
		Err_length = 1;
		return VYVOD_BAD_SKOBKA_2;
	}
	//MR1-NEW 24.08
	int lastSymb = prisv.length() - 1;
	if (last_zpt == lastSymb) {
		Err_start = 2 + viraj.lastIndexOf(',');
		Err_length = 1;
		return VYVOD_END_ZAP; // 2503-VYVOD_END_ZAP; // Запятая в конце
	} else {
		virajs.append(prisv.mid(last_zpt + 1));
		start_pos.append(last_zpt + 1);
		end_pos.append(prisv.length());
	}
	//RM
	// Bylo: virajs.append(prisv.mid(last_zpt+1));
	//MR-2 Zapomnit' polozheniyz zapiatyh (esli neslozhno - see last_zpt = i)

	//              A2, Obnulenie VirajTypes, VirajList
	pv.VirajTypes.clear();
	pv.VirajList.clear();
	//                      B. Main Loop po termam
	for (int i = 0; i < virajs.count(); i++) {
		QString vir = virajs[i];
		QStringList virajList;
		int err = 0;

		if (vir.trimmed().isEmpty()) {
			Err_start = start_pos[i];
			Err_length = 1;
			return 2504;
		}

		QStringList parsedViraj;
		//MR-3  DENIS - PROVER' POZH
		if (virajs[i].trimmed() == QChar(KS_NS)) {

			QStringList sList;
			sList.append(QString(QChar(KS_NS)));
			pv.VirajTypes.append(none);
			pv.VirajList.append(sList);

			continue;
		}
		//MR-3 End
		int spaces = 0;
		while (virajs[i].startsWith(" ")) {
			spaces++;
			virajs[i].remove(0, 1);
		}
		ErrorPosition local_err_pos;


		PeremType intType =
			parceExpression2(
				virajs[i],
				functions, symbols,
				functions->nameByPos(str),
				&err,
				&virajList,
				&local_err_pos,
				pv.extraSubExpressions,
				false);


		if (err > 0) {
			Err_start = start_pos[i] + local_err_pos.start + spaces;
			Err_length = local_err_pos.len;
			return err;
		}
		if (
			(intType == mass_bool)
			||
			(intType == mass_integer)
			||
			(intType == mass_real)
			||
			(intType == mass_charect)
			||
			(intType == mass_string)
		) {
			Err_start = start_pos[i] + spaces;
			Err_length = virajs[i].length();

			return VYVOD_MASS;
		}
		intType = retUnMassType(intType);
		pv.VirajTypes.append(intType);
		pv.VirajList.append(virajList);
	}

	return 0;
}





/**
 * Парсировка команды "вывод". Использует глоб. переменные. Набор аргументов стандартный.
 * @param prisv
 * @param functions
 * @param symbols
 * @param str
 * @return Код ошибки
 */
int text_analiz::parceFileOutput(
	proga &pv,
	function_table *functions,
	symbol_table *symbols,
	int str
)
{
	QString prisv = pv.line;
	int f_id = modules->idByName("FilesP");
	if (f_id == -1 || !modules->module(f_id)->isEnabled()) {
		Err_start = 0;
		Err_length = 1;
		return RT_ERR_NO_FILE_ISP;
	};

	QString viraj = prisv.mid(2);
	QStringList virajs;
	QList<int> start_pos;
	QList<int> end_pos;
	// NEW 24.08
	if (viraj.trimmed().length() == 0) {

		return VYVOD_EMPTY;
	}
	// END NEW

	//                      A. Prolog
	int skobka_op = 0, skobka_mass = 0, last_zpt = 1;
	//              A1. Form virajs - spisok termov vyvoda

	for (int i = 0; i < prisv.length(); i++) {
		if (prisv[i] == '(') {
			skobka_op++;
		}
		if (prisv[i] == ')') {
			skobka_op--;
		}

		if (prisv[i] == '[') {
			skobka_mass++;
		}
		if (prisv[i] == ']') {
			skobka_mass--;
		}

		//if(skobka_op==0)skobka_mass=0;

		if ((skobka_op < 0) || (skobka_mass < 0)) {

			Err_start = 2 + viraj.indexOf(')');
			Err_length = 1;
			return VYVOD_BAD_SKOBKA_1;
		}
		if ((prisv[i] == ',') && (skobka_op == 0) && (skobka_mass == 0)) {
			QString test = prisv.mid(last_zpt + 1, i - last_zpt - 1);
			virajs.append(prisv.mid(last_zpt + 1, i - last_zpt - 1));
			start_pos.append(last_zpt + 1);
			end_pos.append((i - last_zpt - 1) + (last_zpt));
			last_zpt = i;
		}
	} // end for ( i )
	if ((skobka_op > 0) || (skobka_mass > 0)) {
		Err_start = 2 + viraj.lastIndexOf('(');
		Err_length = 1;
		return VYVOD_BAD_SKOBKA_2;
	}
	//MR1-NEW 24.08
	int lastSymb = prisv.length() - 1;
	if (last_zpt == lastSymb) {
		Err_start = 2 + viraj.lastIndexOf(',');
		Err_length = 1;
		return VYVOD_END_ZAP; // 2503-VYVOD_END_ZAP; // Запятая в конце
	} else {
		virajs.append(prisv.mid(last_zpt + 1));
		start_pos.append(last_zpt);
		end_pos.append(prisv.length() - 1);
	}

	//              A2, Obnulenie VirajTypes, VirajList
	pv.VirajTypes.clear();
	pv.VirajList.clear();

	//                      B. Main Loop po termam
	if (virajs.count() < 2) {
		Err_start = 1;
		Err_length = prisv.length() - 1;
		return VYVOD_EMPTY;

	};
	for (int i = 0; i < virajs.count(); i++) {
		QString vir = virajs[i];
		QStringList virajList;
		int err = 0;
		if (vir.trimmed().isEmpty()) {
			Err_start = start_pos[i];
			Err_length = 1;
			return 2504;
		}

		QStringList parsedViraj;
		//MR-3  DENIS - PROVER' POZH
		if (virajs[i].trimmed() == QChar(KS_NS)) {
			QStringList sList;
			sList.append(QString(QChar(KS_NS)));
			pv.VirajTypes.append(none);
			pv.VirajList.append(sList);
			if (i == 0) {
				Err_start = start_pos[i];
				if (i < virajs.count() - 1) {

					Err_length = start_pos[i + 1] - start_pos[i];
				} else {

					Err_length = prisv.length() - start_pos[i];
				}
				return F_KEY_NOT_INT;
			}

			continue;
		}
		//MR-3 End
		ErrorPosition local_err_pos;

		PeremType intType =
			parceExpression2(
				virajs[i],
				functions, symbols,
				functions->nameByPos(str),
				&err,
				&virajList,
				&local_err_pos,
				pv.extraSubExpressions,
				false);

		if (err > 0) {
			Err_start = start_pos[i] + local_err_pos.start;
			Err_length = local_err_pos.len;

			return err;
		}
		if ((i == 0) && (intType != integer)) {
			QString dbg = prisv;
			Err_start = start_pos[i];
			if (i < virajs.count() - 1) {


				Err_length = start_pos[i + 1] - start_pos[i];
			} else {

				Err_length = prisv.length() - start_pos[i];
			}

			return F_KEY_NOT_INT;
		}
		if (
			(intType == mass_bool)
			||
			(intType == mass_integer)
			||
			(intType == mass_real)
			||
			(intType == mass_charect)
			||
			(intType == mass_string)
		) {
			Err_start = start_pos[i] + local_err_pos.start;
			Err_length = virajs[i].length();
			return VYVOD_MASS;
		}
		intType = retUnMassType(intType);
		pv.VirajTypes.append(intType);
		pv.VirajList.append(virajList);
	}

	return 0;
}



int text_analiz::parce_dano(proga &pv, function_table *functions, symbol_table *symbols, int str)
{
	QString instr = pv.line;

	if (functions->nameByPos(str) == "global") {
		return DANO_NOFUNCT;
	}
	QString uslovie = "";
	uslovie = instr.mid(1);
	if (uslovie.isNull()) {
		pv.VirajList.clear();
		return 0;
	};
	if (uslovie == "\n") {
		pv.VirajList.clear();
		return 0;
	};
	if (uslovie == " ") {
		pv.VirajList.clear();
		return 0;
	};
	if (uslovie.isEmpty()) {
		pv.VirajList.clear();
		return 0;
	};



	int err;
	//QStringList virajList=KumTools::instance()->split_prisv(uslovie);
	// QStringList parcedViraj;
	QStringList resViraj;

	QStringList parsedViraj;
	//int * err_start;

	ErrorPosition local_err_pos;


	PeremType     intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

	if (err != 0) {
		Err_start = local_err_pos.start + 1;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return DANO_TYPE_MISS;
	}

	pv.VirajList.clear();
	pv.VirajList.append(parsedViraj);


	pv.line = instr;

	return 0;
};




int text_analiz::parce_nado(proga &pv, function_table *functions, symbol_table *symbols, int str)
{
	QString instr = pv.line;
	pv.VirajList.clear();
	if (functions->nameByPos(str) == "global") {

		return NADO_NOFUNCT;
	}
	QString uslovie = "";
	uslovie = instr.mid(1);
	if (uslovie.isNull()) {
		return 0;
	}
	if (uslovie == "\n") {
		return 0;
	}
	if (uslovie == " ") {
		pv.VirajList.clear();
		QStringList empty;
		empty << "";
		pv.VirajList.append(empty);
		return 0;
	};
	if (uslovie.isEmpty()) {
		return 0;
	}



	int err;
	// QStringList virajList=KumTools::instance()->split_prisv(uslovie);

	QStringList parsedViraj;
	ErrorPosition local_err_pos;


	PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

	if (err != 0) {
		Err_start = local_err_pos.start + 1;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return NADO_TYPE_MISS;
	}
	pv.VirajList.append(parsedViraj);

	pv.line = instr;
	return 0;
};


/**
* Парсировк строки ввод.  Использует глоб. переменные.
* @param instr
* @param functions
* @param symbols
* @param str
* @param err_start
* @param err_end
* @return
 */

struct Term {
	QString text;
	int start;
	int end;
};

int text_analiz::parceInput(proga &pv, function_table *functions, symbol_table *symbols, int str)
//16-08-Mordol
{
	QString instr = pv.line;
	if (instr.trimmed().length() == 1) {
		Err_start = 0;
		Err_length = 1;
		return VVOD_EMPTY;
	}

	if (instr.simplified().length() > 0 && !instr.simplified()[1].isSpace()) {
		Err_start = 0;
		Err_length = 1;
		return VVOD_NO_SPACE;
	}

	pv.VirajList.clear();

	instr.remove(0, 2);
	// NEW V.Y. 24.08
	if ((instr.isNull())
		||
		(instr == "\n")
		||
		(instr == " ")
		||
		(instr.isEmpty())
	)

	{
		Err_start = 0;
		Err_length = 1;
		return VVOD_EMPTY;
	}
	// END NEW

	// NEW 25.08
	if (instr.trimmed().endsWith(",")) {
		Err_start = 2 + instr.lastIndexOf(",");
		Err_length = 1;
		return VVOD_END_ZAP;
	}

	// END NEW
	int err = 0;

	instr = instr.trimmed();

	QList<Term> terms;
	Term first;
	first.text = "";
	first.start = 0;
	first.end = 0;
	terms << first;
	int deep1 = 0; // уровень вложенности ()
	int deep2 = 0; // уровень вложенности []

	for (int i = 0; i < instr.length(); i++) {
		if (instr[i] == '[') {
			deep2++;
		}
		if (instr[i] == ']') {
			deep2--;
		}
		if (instr[i] == '(') {
			deep1++;
		}
		if (instr[i] == ')') {
			deep1--;
		}

		if (instr[i] == ','  &&  deep1 == 0 && deep2 == 0) {
			terms.last().end = i;
			Term next;
			next.text = "";
			next.start = i + 1;
			terms << next;
		} else {
			terms.last().text += instr[i];
		}
	}

	terms.last().end = instr.length();
	bool hasSim = false;

	if ((terms.count() > 1) && (app()->isExamMode())) {
		return VVOD_MANY;

	}
	for (int t = 0; t < terms.count(); t++) {
//        terms[t].start += 2;
		while (terms[t].text.startsWith(" ")) {
			terms[t].text.remove(0, 1);
			terms[t].start ++;
		}
		QString uslovie = terms[t].text.trimmed();
		int op_sk = uslovie.indexOf('[');
		int isp_id = moduleId;
		QList<int> mass_arg_starts;
		if (op_sk < 1) {

			int p_id = symbols->inTable(uslovie, functions->nameByPos(str));
			if (p_id < 0) {
				p_id = symbols->inTable(uslovie, "global");
			}


			QStringList parsedViraj;
			ErrorPosition local_err_pos;

			StructType st;

			PeremType perem_type = parceExpression(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &st, &local_err_pos, pv.extraSubExpressions, false);

			if (
				(perem_type == mass_bool)
				||
				(perem_type == mass_integer)
				||
				(perem_type == mass_real)
				||
				(perem_type == mass_charect)
				||
				(perem_type == mass_string)
			) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return VVOD_MASS;
			}
			if (err > 0) {
				int shift = terms[t].start + 2;
				Err_start = local_err_pos.start + shift;
				Err_length = local_err_pos.len;
				return err;
			}
			if (st != simple_var) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				if (st == constant /*&& perem_type != kumString*/) {

					return VVOD_NON_STRING_CONST;
				} else if (st != constant) {
					return VVOD_EXPRESSION;
				}
			}
			if (!pv.line.isEmpty()) {
				pv.line += ",";
			}
			pv.VirajList.append(parsedViraj);
			pv.VirajTypes.append(perem_type);
			pv.line += "^" + QString::number(p_id);

			if (perem_type == charect) {
				hasSim = true;
			}

			if (symbols->isConst(p_id)) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return VVOD_CONST;
			}
			if (symbols->symb_table[p_id].is_Arg && !(symbols->symb_table[p_id].is_Res || symbols->symb_table[p_id].is_ArgRes)) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return VVOD_ARG;
			}

		};
		if (op_sk >= 1) {
			int close_skobk = uslovie.indexOf("]");
			int lastSymb = uslovie.length() - 1;
			if (close_skobk != lastSymb) {
				if (close_skobk == -1) {
					Err_start = instr.indexOf('[') + 2;
				} else {
					Err_start = instr.indexOf(']') + 2;
				}
				Err_length = 1;
				return VVOD_BAD_KVAD_ZAKR;
			};
			QString viraj = uslovie.mid(op_sk + 1, close_skobk - op_sk - 1); //Для отладки
			//outPutWindow->append("INDEX:"+viraj+"|");
			QString name = uslovie.left(op_sk);
			QStringList args = splitMassArgs(viraj, mass_arg_starts, &err);
			if (err > 0) {
				Err_start = 3;
				Err_length = instr.length() - 3;
				return err;
			}

			//outPutWindow->append("NAME:"+name+"|");

			int p_id = symbols->inTable(name, functions->nameByPos(str));
			if (p_id < 0) {
				p_id = symbols->inTable(name, "global");
			}
			if (p_id < 0) {
				int tn_start = 0, tn_len = 0;
				if (KumTools::instance()->test_name(name, tn_start, tn_len) > 0) {
					Err_start = 3;
					Err_length = 1;
					return VVOD_BAD_NAME_2; // VVOD_BAD_NAME_2;
				};
				Err_start = 3;
				Err_length = 1;
				return VVOD_NO_IMPL_2; // VVOD_NO_IMPL_2;
			};
			if (symbols->getTypeByID(p_id) == integer
				|| symbols->getTypeByID(p_id) == kumBoolean
				|| symbols->getTypeByID(p_id) == real
				|| symbols->getTypeByID(p_id) == charect
				|| symbols->getTypeByID(p_id) == kumString) {
				Err_start = 3;
				Err_length = 1;
				return VVOD_TYPE_MISS_1;
			}

			//outPutWindow->append("arg_count:"+QString::number(args.count()));
			int args_count = args.count();
			if (symbols->getRazmById(p_id) != args_count) {
				Err_start = op_sk + 2;
				Err_length = instr.indexOf(']') - op_sk;
				return VVOD_CANT_INDEX; // VVOD_CANT_INDEX;
			}
			//


			QStringList parsedViraj;
			ErrorPosition local_err_pos;

			PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, isp_id);

			if (err > 0) {
				Err_start = instr.indexOf('[') + 2;
				Err_length = instr.indexOf(']') - Err_start + 2;
				return err;
			}
			/*
			                    if ( hasSim )
			                    {

			                            return VVOD_MANY_SYMBOLS;
			                    }*/
			if (intType == charect) {
				hasSim = true;
			}

			QString line = "^" + QString::number(p_id);

			for (int i = 0; i < args.count(); i++) {
				line = line + "&" + QString::number(i);

			};


			if (!pv.line.isEmpty()) {
				pv.line += ",";
			}
			pv.VirajList.append(parsedViraj);
			pv.VirajTypes.append(intType);
			pv.line += line;

		};
	}

	return 0;
}



int text_analiz::parceFileInput(proga &pv, function_table *functions, symbol_table *symbols, int str)
//16-08-Mordol
{
	QString instr = pv.line;
	QList<int> mass_arg_starts;

	int f_id = modules->idByName("FilesP");
	if (f_id == -1 || !modules->module(f_id)->isEnabled()) {
		Err_start = 0;
		Err_length = 1;
		return RT_ERR_NO_FILE_ISP;
	};
	pv.VirajList.clear();

	instr.remove(0, 2);
	// NEW V.Y. 24.08
	if ((instr.isNull())
		||
		(instr == "\n")
		||
		(instr == " ")
		||
		(instr.isEmpty())
	)

	{
		Err_start = 0;
		Err_length = 1;
		return VVOD_EMPTY;
	}
	// END NEW

	// NEW 25.08
	if (instr.trimmed().endsWith(",")) {
		Err_start = instr.lastIndexOf(',');
		Err_length = 1;
		return VVOD_END_ZAP;
	}

	// END NEW
	int err = 0;

	//     instr = instr.trimmed();

	QList<Term> terms;
	Term first;
	first.text = "";
	first.start = 0;
	first.end = 0;
	terms << first;
	int deep1 = 0; // уровень вложенности ()
	int deep2 = 0; // уровень вложенности []

	for (int i = 0; i < instr.length(); i++) {
		if (instr[i] == '[') {
			deep2++;
		}
		if (instr[i] == ']') {
			deep2--;
		}
		if (instr[i] == '(') {
			deep1++;
		}
		if (instr[i] == ')') {
			deep1--;
		}

		if (instr[i] == ','  &&  deep1 == 0 && deep2 == 0) {
			terms.last().end = i;
			Term next;
			next.text = "";
			next.start = i + 1;
			terms << next;
		} else {
			terms.last().text += instr[i];
		}
	}

	terms.last().end = instr.length();
	bool hasSim = false;
	if (terms.count() < 2) {
		Err_start = 0;
		Err_length = 1;
		return F_VVOD_NO_PEREM;
	};
	for (int t = 0; t < terms.count(); t++) {
		terms[t].start += 2;
		while (terms[t].text.startsWith(" ")) {
			terms[t].text.remove(0, 1);
			terms[t].start += 1;
		}
		QString uslovie = terms[t].text.trimmed();
		int op_sk = uslovie.indexOf('[');
		int isp_id = moduleId;
		if (op_sk < 1) {

			int p_id = symbols->inTable(uslovie, functions->nameByPos(str));
			if (p_id < 0) {
				p_id = symbols->inTable(uslovie, "global");
			}

			if ((t == 0) && (p_id < 0)) {

				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return VVOD_NO_IMPL_2;
			};

			if (symbols->isConst(p_id) && t > 0) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return VVOD_CONST;
			}

			if ((t == 0) && (symbols->getTypeByID(p_id) != integer)) {
				Err_start = terms[t].start + 2;
				Err_length = terms[t].text.length();
				return F_VVOD_KEY_ERROR;
			};


			QStringList parsedViraj;
			ErrorPosition local_err_pos;
			StructType st;

			PeremType perem_type = parceExpression(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &st, &local_err_pos, pv.extraSubExpressions, false);

			if (
				(perem_type == mass_bool)
				||
				(perem_type == mass_integer)
				||
				(perem_type == mass_real)
				||
				(perem_type == mass_charect)
				||
				(perem_type == mass_string)
			) {
				Err_start = terms[t].start;
				Err_length = terms[t].text.length();
				return VVOD_MASS;
			}
			if (err > 0) {
				Err_start = terms[t].start + local_err_pos.start;
				Err_length = local_err_pos.len;
				return err;
			}
			if (st != simple_var && t > 0) {
				Err_start = terms[t].start + local_err_pos.start;
				Err_length = terms[t].text.length();
				if (st == constant /*&& perem_type != kumString*/) {

					return VVOD_NON_STRING_CONST;
				} else if (st != constant) {
					return VVOD_EXPRESSION;
				}
			}
			if (!pv.line.isEmpty()) {
				pv.line += ",";
			}
			pv.VirajList.append(parsedViraj);
			pv.VirajTypes.append(perem_type);
			pv.line += "^" + QString::number(p_id);

			if (perem_type == charect) {
				hasSim = true;
			}
		};
		if (op_sk >= 1) {
			int close_skobk = uslovie.indexOf("]");
			int lastSymb = uslovie.length() - 1;
			if (close_skobk != lastSymb) {
				if (close_skobk == -1) {
					Err_start = instr.indexOf('[');
				} else {
					Err_start = instr.indexOf(']');
				}
				Err_length = instr.length() - Err_start;
				return VVOD_BAD_KVAD_ZAKR;
			};
			QString viraj = uslovie.mid(op_sk + 1, close_skobk - op_sk - 1); //Для отладки
			//outPutWindow->append("INDEX:"+viraj+"|");
			QString name = uslovie.left(op_sk);
			QStringList args = splitMassArgs(viraj, mass_arg_starts, &err);
			if (err > 0) {
				Err_start = 1;
				Err_length = instr.length() - 1;
				return err;
			}

			//outPutWindow->append("NAME:"+name+"|");

			int p_id = symbols->inTable(name, functions->nameByPos(str));
			if (p_id < 0) {
				p_id = symbols->inTable(name, "global");
			}
			if (p_id < 0) {
				int tn_start = 0, tn_len = 0;
				if (KumTools::instance()->test_name(name, tn_start, tn_len) > 0) {
					Err_start = 1;
					Err_length = instr.length() - 1;
					return VVOD_BAD_NAME_2; // VVOD_BAD_NAME_2;
				};
				Err_start = 1;
				Err_length = instr.length() - 1;
				return VVOD_NO_IMPL_2; // VVOD_NO_IMPL_2;
			};
			if (symbols->getTypeByID(p_id) == integer
				|| symbols->getTypeByID(p_id) == kumBoolean
				|| symbols->getTypeByID(p_id) == real
				|| symbols->getTypeByID(p_id) == charect) {
				Err_start = 1;
				Err_length = instr.length() - 1;
				return VVOD_TYPE_MISS_1;
			}

			//outPutWindow->append("arg_count:"+QString::number(args.count()));
			int args_count = args.count();
			if (symbols->getRazmById(p_id) != args_count) {
				Err_start = 1;
				Err_length = instr.length() - 1;
				return VVOD_CANT_INDEX; // VVOD_CANT_INDEX;
			}
			//


			QStringList parsedViraj;
			ErrorPosition local_err_pos;
			PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, isp_id);
			if (err > 0) {
				Err_start = 1;
				Err_length = instr.length() - 1;
				return err;
			}

			if (hasSim) {
				Err_start = 1;
				Err_length = instr.length() - 1;
				return VVOD_MANY_SYMBOLS;
			}
			if (intType == charect) {
				hasSim = true;
			}

			QString line = "^" + QString::number(p_id);

			for (int i = 0; i < args.count(); i++) {
				line = line + "&" + QString::number(i);
			};


			if (!pv.line.isEmpty()) {
				pv.line += ",";
			}
			pv.VirajList.append(parsedViraj);
			pv.VirajTypes.append(intType);
			pv.line += line;

		};
	}

	return 0;
};




/**
* Парсровка строки выбор. Использует глоб. переменные.
* @param instr Строка
* @param functions Таблица алгоритмов
* @param symbols Таблица переменных
* @param str Номер строки
* @param err_start Для вывода ошибок
* @param err_end  Для вывода ошибок
* @return Код ошибки
 */
int text_analiz::parce_vibor(QString instr, function_table *functions, symbol_table *symbols, int str)
{
	Q_UNUSED(functions);
	Q_UNUSED(symbols);
	Q_UNUSED(str);
	//if(functions->nameByPos(str)=="global")return SYNTAX_ERR;
	QString uslovie = "";

	uslovie = instr.mid(1);


	if (!uslovie.isEmpty()) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return  VYBOR_TRASH;
	}
	return 0;
};



/**
* Парсировка оперетора при
* @param instr Строка
* @param functions
* @param symbols
* @param str номер строки
* @param err_start  для вывода ошибок
* @param err_end для вывода ошибок
* @return Код ошибки
 */
int text_analiz::parce_pri(proga &pv, function_table *functions, symbol_table *symbols, int str)
{
	QString instr = pv.line;
	//if(functions->nameByPos(str)=="global")return SYNTAX_ERR;
	QString uslovie = "";
	uslovie = instr.mid(1);

	if (uslovie.isNull()) {
		return PRI_NO_CONDITION_1;
	}
	if (uslovie == "\n") {
		return PRI_NO_CONDITION_2;
	}
	if (uslovie == " ") {
		pv.VirajList.clear();
		QStringList empty;
		empty << "";
		pv.VirajList.append(empty);
		return PRI_NO_CONDITION_2;
	};
	if (uslovie.isEmpty()) {
		return PRI_NO_CONDITION_3;
	}

	if (uslovie[uslovie.length() - 1] != ':') {
		Err_start = 0;
		Err_length = 1;
		return PRI_NO_DVOET;
	}
	//uslovie=uslovie.mid(0,-1);
	uslovie = uslovie.left(uslovie.length() - 1);


	pv.VirajList.clear();


	int err = 0;

	uslovie = uslovie.trimmed();

	QStringList parsedViraj;
	ErrorPosition local_err_pos;


	PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

	if (err != 0) {
		Err_start = 2 + local_err_pos.start;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 2 + local_err_pos.start;
		Err_length = instr.length() - 2;
		return PRI_TYPE_MISS;
	}

	pv.VirajList.append(parsedViraj);
	pv.line = "PRI";
	return 0;
};



/**
* Парсировка команды "Утв". Использует глоб. переменные. Набор аргументов стандартный.
* @param instr
* @param functions
* @param symbols
* @param str
* @return Код ошибки
 */


int text_analiz::parce_utv(proga &pv, function_table *functions, symbol_table *symbols, int str)
{
	QString instr = pv.line;

	QString uslovie = "";
	uslovie = instr.mid(1);
	if (uslovie.isNull() || uslovie.isEmpty()) {
		uslovie = QString(QChar(KS_DA));
	}
	if (uslovie == "\n") {
		return UTV_NO_CONDITION_2;
	}
	int err = 0;

	uslovie = uslovie.trimmed();

	QStringList parsedViraj;
	ErrorPosition local_err_pos;


	PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

	if (err != 0) {
		Err_start = 2 + local_err_pos.start;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 2;
		Err_length = instr.length() - 2;
		return UTV_TYPE_MISS;
	}
	pv.VirajList.clear();
	pv.VirajTypes.clear();
	pv.VirajList.append(parsedViraj);
	pv.VirajTypes.append(kumBoolean);
	pv.line = instr;

	return 0;
};



/**
* Парсировка команды "кц_при". Использует глоб. переменные. Набор аргументов стандартный.
* @param instr
* @param functions
* @param symbols
* @param str
* @return Код ошибки
 */


int text_analiz::parce_kc_pri(proga &pv, function_table *functions, symbol_table *symbols, int str)
{
	QString instr = pv.line;
	//    int to_pos=instr.indexOf(QChar(KS_TO));
	QString uslovie = instr;;
	uslovie = instr.mid(2);
	Err_start = 0;
	Err_length = 1;
	if (uslovie.isNull()) {

		return KCPRI_NO_CONDITION_1;
	}
	if (uslovie == "\n") {

		return KCPRI_NO_CONDITION_2;
	}
	if (uslovie.isEmpty()) {

		return KCPRI_NO_CONDITION_3;
	}

	int err = 0;

	uslovie = uslovie.trimmed();
	pv.VirajList.clear();
	QStringList parsedViraj;//09-01-Mordol
	ErrorPosition local_err_pos;



	PeremType       intType = parceExpression2(uslovie, functions, symbols, functions->nameByPos(str), &err, &parsedViraj, &local_err_pos, pv.extraSubExpressions, false);

	if (err != 0) {
		Err_start = 2 + local_err_pos.start;
		Err_length = local_err_pos.len;
		return err;
	}
	if (intType != kumBoolean) {
		Err_start = 2;
		Err_length = instr.length() - 2;
		return KCPRI_TYPE_MISS;
	}

	pv.VirajList.append(parsedViraj);

	pv.line = instr;

	return 0;
};


text_analiz::text_analiz()
{
	nestedAlg = 0;
	firstAlg = false;
	// Множество Ω. � азбито на классы по приоритетности операторов.
	Omega.append(QRegExp(QString(QChar(KS_OR)) + "|" + QString(QChar(KS_AND))));
	Omega.append(QRegExp(QString(QChar(KS_NOT))));
	Omega.append(QRegExp(QString(QChar(KS_BOLSHE_RAVNO)) + "|" + QString(QChar(KS_MENSHE_RAVNO))
			+ "|" + QString(QChar(KS_NE_RAVNO)) + "|" + QString(QChar(KS_PRISV)) + "|<|>|="));
	Omega.append(QRegExp("\\+|-|" + QString(QChar(UNARY_PLUS)) + "|" + QString(QChar(UNARY_MINUS))));
	Omega.append(QRegExp("\\*|/"));
	Omega.append(QRegExp(QString(QChar(KS_STEPEN))));


	//    Delim= "[+|=|*|\\-|/|<|>|\\(|\\)|\\[|\\]|\\,|\\:|"+QString(QChar(UNARY_MINUS))+"|"+
	//           QString(QChar(KS_STEPEN))+"|"+QString(QChar(KS_NE_RAVNO))+"|"
	//           +QString(QChar(KS_OR))+"|"+QString(QChar(KS_AND))+"|"+QString(QChar(KS_BOLSHE_RAVNO))+"|"
	//           +QString(QChar(KS_MENSHE_RAVNO))+"|"+QString(QChar(UNARY_PLUS))+"|"+QString(QChar(KS_PRISV))+"]";

	//    DelimNOT= "[+|=|*|\\-|/|<|>|\\(|\\)|\\[|\\]|\\,|\\:|"+QString(QChar(KS_NOT))+QString(QChar(UNARY_MINUS))+"|"+
	//              QString(QChar(KS_STEPEN))+"|"+QString(QChar(KS_NE_RAVNO))+"|"
	//              +QString(QChar(KS_OR))+"|"+QString(QChar(KS_AND))+"|"+QString(QChar(KS_BOLSHE_RAVNO))+"|"
	//              +QString(QChar(KS_MENSHE_RAVNO))+"|"+QString(QChar(UNARY_PLUS))+"]";

	Delim = "[+=*\\-/<>\\(\\)\\[\\]\\,\\:" + QString(QChar(UNARY_MINUS)) +
		QString(QChar(KS_STEPEN)) + QString(QChar(KS_NE_RAVNO)) +
		QString(QChar(KS_OR)) + QString(QChar(KS_AND)) + QString(QChar(KS_BOLSHE_RAVNO)) +
		QString(QChar(KS_MENSHE_RAVNO)) + QString(QChar(UNARY_PLUS)) + QString(QChar(KS_PRISV)) + "]";

	DelimNOT = "[+=*\\-/<>\\(\\)\\[\\]\\,\\:" + QString(QChar(KS_NOT)) + QString(QChar(UNARY_MINUS)) +
		QString(QChar(KS_STEPEN)) + QString(QChar(KS_NE_RAVNO)) +
		QString(QChar(KS_OR)) + QString(QChar(KS_AND)) + QString(QChar(KS_BOLSHE_RAVNO)) +
		QString(QChar(KS_MENSHE_RAVNO)) + QString(QChar(UNARY_PLUS)) + "]";

	Breaks = QObject::trUtf8("[+№$;=\\?*~`\\-\\\\/<>\\(\\)\\[\\]\\,\\:") + QString(QChar(KS_DOC)) + "" + //QString(QChar(KS_NOT))+
		QString(QChar(UNARY_MINUS)) + "" +
		QString(QChar(KS_STEPEN)) + "" + QString(QChar(KS_NE_RAVNO)) + ""
		+ QString(QChar(KS_OR)) + "" + QString(QChar(KS_AND)) + ""
		+ QString(QChar(KS_BOLSHE_RAVNO)) + "" + QString(QChar(KS_MENSHE_RAVNO)) + ""
		+ QString(QChar(UNARY_PLUS)) + "" + QString(QChar(FOR_CIRCLE)) + ""
		+ QString(QChar(RAZ_CIRCLE)) + "" + QString(QChar(WHILE_CIRCLE)) + ""
		+ QString(QChar(KS_STEPEN)) + "" + QString(QChar(KS_BOLSHE_RAVNO)) + ""
		+ QString(QChar(KS_MENSHE_RAVNO)) + "" + QString(QChar(KS_NE_RAVNO)) + ""
		+ "" + QString(QChar(KS_OR)) + "" + QString(QChar(KS_AND)) + ""
		+ QString(QChar(KS_PRISV)) + "" + QString(QChar(KS_CEL)) + ""
		+ QString(QChar(KS_CEL_TAB)) + "" + QString(QChar(KS_VES)) + ""
		+ QString(QChar(KS_VES_TAB)) + "" + QString(QChar(KS_LIT)) + ""
		+ QString(QChar(KS_LIT_TAB)) + "" + QString(QChar(KS_SIM)) + ""
		+ QString(QChar(KS_SIM_TAB)) + "" + QString(QChar(KS_LOG)) + ""
		+ QString(QChar(KS_LOG_TAB)) + "" + QString(QChar(KS_VSE)) + ""
		+ QString(QChar(KS_ESLI)) + "" + QString(QChar(KS_TO)) + ""
		+ QString(QChar(KS_INACHE)) + "" + QString(QChar(KS_NACH)) + ""
		+ QString(QChar(KS_KON)) + ""
		+ QString(QChar(KS_SHAG)) + ""
		+ QString(QChar(KS_NC)) + "" + QString(QChar(KS_KC)) + ""
		+ QString(QChar(KS_DLYA)) + "" + QString(QChar(KS_RAZ)) + ""
		+ QString(QChar(KS_POKA)) + "" + QString(QChar(KS_MALG)) + ""
		+ QString(QChar(KS_ALG)) + "" + QString(QChar(KS_DO)) + ""
		+ QString(QChar(KS_OT)) + "" + QString(QChar(KS_PRI)) + ""
		+ QString(QChar(KS_ARG)) + "" + QString(QChar(KS_REZ)) + "" + QString(QChar(KS_ARGREZ)) + ""
		+ QString(QChar(KS_VVOD)) + "" + QString(QChar(KS_VIVOD)) + ""
		+ QString(QChar(KS_UTV)) + "" + QString(QChar(KS_DANO)) + ""
		+ QString(QChar(KS_NADO)) + "" + QString(QChar(KS_KC_PRI)) + ""
		+ QString(QChar(KS_EXP_PLUS)) + "" + QString(QChar(KS_EXP_MINUS)) + ""
		+ QString(QChar(KS_NS)) + "" + QString(QChar(KS_DOC))
		+ "" + QString(QChar(KS_AT)) + "" + QString(QChar(KS_TILDA)) + ""
		+ "" + QString(QChar(KS_PERCENT)) + "" + QString(QChar(KS_DOLLAR)) + ""
		+ "" + QString(QChar(KS_PAR)) + "" + QString(QChar(KS_DOC)) + ""
		+ "" + QString(QChar(KS_VIBOR))/*+"\\^"*/ + "]";

	operators = QRegExp(Delim);
	operatorsNOT = QRegExp(DelimNOT);
	brks = QRegExp(Breaks);
}

#include <QtXml>

QString text_analiz::splitLogicalExpressions(const QString &s, QStringList &exprList)
{
	QString head, tail;
	int operPos;
	operPos = s.indexOf(QChar(KS_OR));
	if (operPos == -1) {
		operPos = s.indexOf(QChar(KS_AND));
	}
	if (operPos == -1) {
		// Нет операторов "или" или "и"
		return s;
	} else {
		head = s.left(operPos);
		tail = s.mid(operPos + 1);
		QString result;
		QString subExprId;
		if (head.contains(operatorsNOT)) {
			exprList << splitLogicalExpressions(head, exprList);
			subExprId = QString::number(exprList.count() - 1);
			result += "#" + subExprId;
		} else {
			result += head;
		}
		result += s[operPos];
		if (tail.contains(operatorsNOT)) {
			exprList << splitLogicalExpressions(tail, exprList);
			subExprId = QString::number(exprList.count() - 1);
			result += "#" + subExprId;
		} else {
			result += tail;
		}
		return result;
	}
}

int text_analiz::L(QString s)
{
	// Поиск оператора из множества Ω_i в строке s
	int result = -1;
	for (int i = 0; i < Omega.count(); i++) {
		result = Omega[i].lastIndexIn(s);
		// Если оператор найден в подмножестве Ω_i, то возвращаем его позицию,
		// иначе ищем в следующем подмножестве Ω_{i+1}
		if (result != -1) {
			break;
		}
	}
	return result;
};

PeremType text_analiz::G(QString s,
	int *err,
	function_table *table,
	symbol_table *symbols,
	const QList<PeremType> &type_list,
	const QList<StructType> &struct_list,
	const QStringList &expr_list,
	StructType *strNew,
	int *err_start,
	int *err_len,
	bool &include_bounds_in_error
)

{

	QString H, T; // Head, Tail. См. описание в getSimpleViraj.pdf
	int l; // Значение функции L(s). См. описание в getSimpleViraj.pdf
	include_bounds_in_error = true;
	// Есть ли выражение
	if (s.trimmed().isEmpty()) {
		*err = 0;
		*strNew = s_none;
		return none;
	}

	// Определяем, если ли оператор
	l = L(s);

	if (l == -1) {
		// Нет оператора. Возвращаем P(s) (getObjType)
		int local_err;
		int local_err_start;
		int local_err_len;
		PeremType p = getObjType(s, &local_err, table, symbols, type_list, struct_list, expr_list, strNew, local_err_start, local_err_len, include_bounds_in_error);
		if (local_err > 0) {
			*err = local_err;
			*err_start = local_err_start;
			*err_len = local_err_len;
			*strNew = s_none;
			return none;
		} else {
			*err = 0;
			return p;
		}
	} else {
		QString leftXml, rightXml;
		// Есть оператор. Возвращаем R(G(H(s,L(s))),L(s),G(T(s,L(s))))
		H = s.left(l);
		T = s.right(s.length() - l - 1);

		// Если левая часть выражения пустая, а оператор +, - или "не",
		// то подразумевается унарный оператор. Если другой оператор -- ошибка
		if (H.isEmpty()) {
			if (s[l] == '+') {
				s[l] = QChar(UNARY_PLUS);
			} else if (s[l] == '-') {
				s[l] = QChar(UNARY_MINUS);
			} else if (s[l] == QChar(UNARY_PLUS))
				;
			else if (s[l] == QChar(UNARY_MINUS))
				;
			else if (s[l] == QChar(KS_NOT))
				;
			else {
				*err = GSV_NOT_UNARY;
				*err_start = l;
				*err_len = 1;
				*strNew = s_none;
				return none;
			}
		}

		// Если обе части выражения пустые -- ошибка
		if (H.isEmpty() && T.isEmpty()) {
			*err_start = l;
			*err_len = 1;
			*err = GSV_ONLY_OPER;
			*strNew = s_none;
			return none;
		}

		PeremType leftType, rightType;
		StructType leftStruct, rightStruct;
		int local_err, local_err_start, local_err_len;



		// Определяем типы H и T
		if (H.isEmpty()) {
			leftType = none;
			leftStruct = s_none;
		} else {
			leftType = G(H, &local_err, table, symbols, type_list, struct_list, expr_list, &leftStruct, &local_err_start, &local_err_len, include_bounds_in_error);

			// Если в левой части ошибка -- пересчитываем позицию ошибки
			if (local_err > 0) {
				*err = local_err;
				*err_start = local_err_start;
				*err_len = local_err_len;
				*strNew = s_none;
				return none;
			}
		}

		// Правая часть не может быть пустой,
		// т.к. в КУМИ� е не предусмотрено постфиксных операторов
		if (T.isEmpty()) {
			*err = GSV_EXTRA_OPER; // "Лишний оператор"
			*err_start = l;
			*err_len = 1;
			*strNew = s_none;
			return none;
		} else {
			rightType = G(T, &local_err, table, symbols, type_list, struct_list, expr_list, &rightStruct, &local_err_start, &local_err_len, include_bounds_in_error);
			// Если в правой части ошибка -- пересчитываем позицию ошибки
			if (local_err > 0) {
				*err = local_err;
				*err_start = (l + 1) + local_err_start;
				*err_len = local_err_len;
				*strNew = s_none;
				return none;
			}
		}

		// Определяем тип тройки {p_1,λ,p_2}
		PeremType r = resType(&local_err, leftType, rightType, s[l]);
		if (local_err > 0) {
			*err = local_err;
			*err_start = l;
			*err_len = 1;
			*strNew = s_none;
			return none;
		}

		*strNew = complex;
		*err = 0;
		*err_start = 0;
		*err_len = 0;

		return r;

	}

}


/*MR-New
        1) Proveriaet semantiku opisatelia ob'ekta i
        2) opredeliaet ego tip

         Format opisatelya:
        <DS_VAR><SYM_adr>                                   - simple var or const or array;
                <DS_ARR><SYM_adr> {<DS_VYR><VYR_adr>}**n            - elem of array (n >0)
        <DS_STR><SYM_adr><DS_VYR><VYR_adr>                  - elem of string
        <DS_STR><SYM_adr><DS_VYR><VYR_adr><DS_VYR><VYR_adr> - fragm of string
        <DS_FUN><FUN_adr> {<DS_VYR><VYR_adr>}**f            - function call (f >=0)

        Input-output patameters - see below
*/

void getObjType_findArgErrPos(const QString &s, const int argNo, int &err_start, int &err_end, const int spaces = 0)
{
	QRegExp rxArg("&\\d+");
	err_start = -1;
	int curArg = -1;
	forever {
		err_start = rxArg.indexIn(s, err_start + 1);
		if (err_start != -1) {
			err_end = err_start + rxArg.matchedLength() - 1 ;
			curArg++;
			if (curArg == argNo) {
				break;
			}
		} else {
			break;
		}
	};
	for (int i = err_end + 1; i < s.length(); i++) {
		if (s[i] == ' ') {
			err_end++;
		} else {
			break;
		}
	}
	err_start += spaces;
	err_end += spaces;
}

QList<int> getObjType_extractExprIds(const QString &s)
{
	QRegExp rxId("&(\\d+)");
	int start_pos = -1;
	QList<int> result;
	bool OK;
	forever {
		start_pos = rxId.indexIn(s, start_pos + 1);
		if (start_pos != -1) {
			result << rxId.cap(1).toInt(&OK);
			K_ASSERT(OK);
		} else {
			break;
		}
	}
	return result;
}

bool isTableType(PeremType t)
{
	return t == mass_bool || t == mass_charect || t == mass_integer || t == mass_real || t == mass_string;
}

bool text_analiz::isRecursive(QString block, QStringList exprList, int moduleNo, int funcNo, bool firstCall, int &exprNo)
{
	QString dbg = block;
	bool result = false;

	if (!firstCall) {
		result = block.indexOf("@" + QString::number(funcNo) + "|" + QString::number(moduleNo)) > -1;
	}
	QRegExp rxArgs = QRegExp("&\\d+");
	if (result) {
		return result;
	}
	int p = -1;
	forever {
		p = rxArgs.indexIn(block, p + 1);
		if (p > -1) {
			QString part = block.mid(p + 1, rxArgs.matchedLength() - 1);
			int no = part.toInt();
			result = result || isRecursive(exprList[no], exprList, moduleNo, funcNo, false, exprNo);
			if (result) {
				exprNo = no;
				return result;
			}
			p++;
		} else {
			break;
		}
	}
	return result;
}

bool getObjType_isSimpleVar(int id, QStringList exprList, symbol_table *symbols)
{
	QString expr = exprList[id].trimmed();
	bool isVarOrConst = QRegExp("\\^\\d+").exactMatch(expr);
	if (isVarOrConst) {
		expr.remove(0, 1);
		bool OK;
		int sId = expr.toInt(&OK);
		K_ASSERT(OK);
		return !symbols->symb_table[sId].is_const;
	} else {
		return false;
	}
}


PeremType text_analiz::getObjType(
	QString                 work_block, // In: Obj descriptor
	int                     *err,       // Out:Error code (0 - OK)
	function_table          *table,     // In: Function table
	symbol_table            *symbols,   // In: Symbol table
	QList<PeremType>    type_List,  // In: Types of subexpressions
	// MR-New
	QList<StructType> struct_List,
	//QStringList virazh_List,          // In: Descriptors of subexpressions
	QStringList exprList, // IN: Subexpressions (for BUGFIX)
	StructType              *StrNew,
	int &err_start,
	int &err_len, bool &include_bounds_in_error

)
{
	Q_UNUSED(table);
	QString debug = work_block;
	int spacesAfter = 0;
	for (int i = work_block.length() - 1; i >= 0; i--) {
		if (work_block[i] == ' ') {
			spacesAfter++;
		} else {
			break;
		}
	}
	int spaces = 0;
	while (work_block.startsWith(" ")) {
		spaces++;
		work_block.remove(0, 1);
	}
	err_start = spaces;
	err_len = work_block.length();
	work_block = work_block.trimmed();

	PeremType viraj_type;

	viraj_type = none;
	int p_id;
	*err = 0;

	//  1. Var or Const or 0-ary function or Array (as a whole)
	if (work_block[0] == '^') {
		bool OK;
		p_id = work_block.mid(1).toInt(&OK);
		if (OK && (p_id >= 0)) {
			viraj_type = symbols->getTypeByID(p_id);
			//MR-New
			*StrNew = simple_var;
			if (symbols->isConst(p_id)) {
				*StrNew = constant;
			}
			if ((viraj_type == mass_integer) ||
				(viraj_type == mass_charect) ||
				(viraj_type == mass_string) ||
				(viraj_type == mass_real) ||
				(viraj_type == mass_bool)
			) {
				*StrNew = array;
			}

			//RM
		} else {
			*err = INT_TA_01;
			return none;
		}
	}

	//     2. Array's element
	//  2.1 Razbiraemsia s imenem
	if (work_block[0] == '%') {
		*StrNew = array_elem;  //MR-New
		int diez_pos = work_block.indexOf('&');
		if (diez_pos == -1) {
			// No arguments
			*err = GOT_ARRAY_1;
			return none;
		}
		bool OK;            // Uneseno vverh
		p_id = work_block.mid(1, diez_pos - 1).toInt(&OK);
		if ((!OK) || (p_id < 0)) {
			*err = INT_TA_02;
			return none;
		}
		//  2.2  Razbiraemsia s argumentami
		//     kolichestvo
		int diez_count = work_block.count('&');
		int mustBe_count = symbols->symb_table[p_id].razm;
		if (diez_count != mustBe_count) {
			err_start = diez_pos + spaces;
			err_len = work_block.length() - spaces - diez_pos;
			*err = GOT_ARRAY_2;
			return none;
		}
		int cur_diez_pos, old_diez_pos;
		old_diez_pos = diez_pos;
		int viraj_id = -1;


		//          tip kazhdogo argumenta - integer
		QList<int> refs;
		for (int i = 0; i < diez_count; i++) {
			QString cap;
			cur_diez_pos = work_block.indexOf('&', old_diez_pos + 1);
			if (cur_diez_pos == -1) {
				cap = work_block.mid(old_diez_pos + 1);
			} else {
				cap = work_block.mid(old_diez_pos + 1, cur_diez_pos - old_diez_pos - 1);
			}
			viraj_id = cap.toInt(&OK);
			refs << viraj_id;
			err_start = old_diez_pos + spaces;
			err_len = cur_diez_pos == -1 ? work_block.length() - old_diez_pos : cur_diez_pos - old_diez_pos;
			old_diez_pos = cur_diez_pos;
			if ((!OK) || (viraj_id < 0)) {
				//                *err = INT_TA_03;
				*err = GOT_NO_COMA;
				return none;
			}
			if (type_List[viraj_id] != integer) {
				*err = GOT_ARRAY_3;
				return none;
			}

		}

		int mass_type = symbols->getTypeByID(p_id);
		if (mass_type == mass_integer) {
			viraj_type = integer;
		}
		if (mass_type == mass_bool) {
			viraj_type = kumBoolean;
		}
		if (mass_type == mass_real) {
			viraj_type = real;
		}
		if (mass_type == mass_string) {
			viraj_type = kumString;
		}
		if (mass_type == mass_charect) {
			viraj_type = charect;
		}
		//MR-New eto rudiment. Vstav' pozh zacommentarennym :)
		//  SKIP_ARGS:;                                 //MR-New
		//RM

	} // end if (work_block[0]=='%')

	// NEW V.Y.

	//      3. STROKA s [] - t.e. obrashchenie k symbolu ili vyrezka
	//  3.1 Razbiraemsia s imenem

	if (work_block[0] == QChar(TYPE_SKOBKA)) {
		int diez_pos = work_block.indexOf('&');
		if (diez_pos == -1) {
			*err = GOT_STRING_1;
			return none;
		}
		bool OK;

		p_id = work_block.mid(1, diez_pos - 1).toInt(&OK);
		if ((!OK) || (p_id < 0)) {
			*err = INT_TA_04;
			return none;
		}

		//  3.2  Razbiraemsia s argumentami
		//     kolichestvo
		int diez_count = work_block.count('&');

		if (diez_count > 2) {
			err_start = diez_pos + spaces;
			err_len = work_block.length() - err_start;
			*err = GOT_STRING_2;
			return none;
		} else {
			//MR-New - Izmeneniya !!!
			if (diez_count == 1) {  // charect s[a]
				viraj_type = charect;
				*StrNew = string_elem;      //MR-New
			} else { // string s[a:b]
				viraj_type = kumString;
				*StrNew = substring;        //MR-New
			}
			//RM
		}
		int cur_diez_pos, old_diez_pos = diez_pos;
		int viraj_id = -1;
		QList<int> refs;
		QString dbg = work_block;
		for (int i = 0; i < diez_count; i++) {
			cur_diez_pos = work_block.indexOf('&', old_diez_pos + 1);
			if (cur_diez_pos == -1) {
				viraj_id = work_block.mid(old_diez_pos + 1).toInt(&OK);
			} else {
				viraj_id = work_block.mid(old_diez_pos + 1, cur_diez_pos - old_diez_pos - 1).toInt(&OK);
			}
			refs << viraj_id;
			if ((!OK) || (viraj_id < 0)) {
				*err = INT_TA_05;
				return none;
			}
			if (type_List[viraj_id] != integer) {
				err_start = old_diez_pos + spaces;
				err_len = cur_diez_pos == -1 ? work_block.length() - err_start : cur_diez_pos - old_diez_pos;
				*err = GOT_STRING_3;
				return none;
			}
			old_diez_pos = cur_diez_pos;
		} // end for ( int i=0; i<diez_count; i++ )

	} // end if ( work_block[0]==QChar(TYPE_SKOBKA) )

	// end NEW V.Y.

	//      4. Vyrazhenie
	if (work_block[0] == '&') {
		*StrNew = complex;      //MR-New
		bool OK;
		p_id = work_block.mid(1).toInt(&OK);
		if ((OK) && (p_id >= 0)) {
			viraj_type = type_List[p_id];
		} else {
			*err = GOT_BAD_SKOBK;
			return none;
		}
		if (viraj_type == bool_or_lit || viraj_type == bool_or_num) {
			viraj_type = kumBoolean;
		}

	};

	//      5. Vyzov funccii
	if (work_block[0] == '@') {
		*StrNew = func;     //MR-New
		QRegExp rxFunctionCall("@(\\d+)\\|(\\d+)\\s*");
		rxFunctionCall.indexIn(work_block);
		bool OK;
		int funcId = rxFunctionCall.cap(1).toInt(&OK);
		K_ASSERT(OK);
		int moduleId = rxFunctionCall.cap(2).toInt(&OK);
		K_ASSERT(OK);
		QList<int> factParameters = getObjType_extractExprIds(work_block);
		int factParametersCount = factParameters.count();
		K_ASSERT(modules->module(moduleId) != NULL);
		K_ASSERT(funcId < modules->module(moduleId)->Functions()->size());
		int formalParametersCount = modules->module(moduleId)->Functions()->argCountById(funcId);

		if (factParametersCount != formalParametersCount) {
			*err = GOT_FUNC_1;
			err_start = work_block.indexOf("&") + spaces;
			err_len = work_block.length() - err_start;
			include_bounds_in_error = false;
			return none;
		}

		int recExprNo = -1;
		bool recursive = isRecursive(work_block, exprList, moduleId, funcId, true, recExprNo);
		bool tableArgs = false;
		for (int i = 0; i < factParametersCount; i++) {
			int exprId = factParameters[i];
			K_ASSERT(OK);
			K_ASSERT(exprId >= 0);
			K_ASSERT(exprId < type_List.count());
			K_ASSERT(exprId < exprList.count());
			K_ASSERT(exprId < struct_List.count());
			PeremType factType = type_List[exprId];
			PeremType formalType = modules->module(moduleId)->Functions()->argTypeById(funcId, i);
			err_start = -1;
			for (int j = -1; j < i; j++) {
				err_start = work_block.indexOf("&", err_start + 1);
			}
			err_start += spaces;
			err_len = QString::number(exprId).length() + 1;
			if (factType != formalType) {
				if (formalType == real && factType == integer) {
					bool res = modules->module(moduleId)->Functions()->isRes(funcId, i);
					bool argRes = modules->module(moduleId)->Functions()->isArgRes(funcId, i);
					if (res || argRes) {
						*err = GOT_TYPE_MISS;
						include_bounds_in_error = false;
						return none;
					}
				} else if (formalType == kumString && factType == charect) {
					bool res = modules->module(moduleId)->Functions()->isRes(funcId, i);
					bool argRes = modules->module(moduleId)->Functions()->isArgRes(funcId, i);
					if (res || argRes) {
						*err = GOT_TYPE_MISS;
						include_bounds_in_error = false;
						return none;
					}
				} else {
					*err = GOT_TYPE_MISS;
					include_bounds_in_error = false;
					return none;
				}
			}
			if (isTableType(formalType)) {
				tableArgs = true;
				int symbId = exprList[exprId].trimmed().remove("^").toInt(&OK);
				K_ASSERT(OK);
				int factDim = symbols->getRazmById(symbId);
				int formalDim = modules->module(moduleId)->Functions()->argDimById(funcId, i);
				if (factDim != formalDim) {
					*err = GOT_BAD_DIM;
					return none;
				}
			}
			bool res = modules->module(moduleId)->Functions()->isRes(funcId, i);
			bool argRes = modules->module(moduleId)->Functions()->isArgRes(funcId, i);
			if (res || argRes) {
				bool isSimple = getObjType_isSimpleVar(exprId, exprList, symbols);
				if (!isSimple) {
					*err = GOT_RES_PARAM_NOT_SIMPLE;
					return none;
				}
				int symbId = exprList[exprId].trimmed().remove("^").toInt(&OK);
				K_ASSERT(OK);
				bool isArg = symbols->symb_table[symbId].is_Arg;
				if (isArg && !symbols->symb_table[symbId].is_Res && !symbols->symb_table[symbId].is_ArgRes) {
					*err = res ? GOT_ARG_PARAM_AS_RES : GOT_ARG_PARAM_AS_ARGRES;
					return none;
				}
			}
			if (recursive && tableArgs) {
				*err = GOT_RECURSIVE_TABLE;
				QRegExp rxSubExpr = QRegExp("&" + QString::number(recExprNo) + "\\s*");
				// TODO выделение ошибки
				//                              err_start = spaces + rxSubExpr.indexIn(work_block);
				//                              err_end = err_start + rxSubExpr.matchedLength();
				return none;

			}
			if (modules->module(moduleId)->Functions()->isRes(funcId, i) ||
				modules->module(moduleId)->Functions()->isArgRes(funcId, i)) {
				StructType factStructType = struct_List[exprId];
				if (factStructType != simple_var &&
					factStructType != array_elem &&
					factStructType != array) {
					*err = GOT_BAD_RES;

					return none;
				}
			}
		}
		viraj_type = modules->FuncType(funcId, moduleId);

	}
	return viraj_type;
}



/**
 * Находит порядковый номер первого вхождения разделителя,
 * не правее (левее) позиции pos в нормализованной строке.
 * � азделители нумеруются с 0.
 * @param normString нормализованная строка
 * @param breaks список разделителей для RegExp
 * @param pos позиция в строке, правее (левее) которой ищем разделитель
 * @param priznak 1, если ищем слева или -1, если справа
 * @param *incl 1, если сам разделитель входит в область или -1, если не входит
 * @return порядковый номер разделителя в строке, если он найден, или -1, если не найден
 */
/*
int text_analiz::FindNomerNorm(QString normString, int pos, int priznak, int *incl, symbol_table *symbols, bool bugfix)
{
                int result;
                int curPos;
                int lastPos;
                QRegExp rxVariable = QRegExp("\\^\\d+");
                if ( priznak == 1 )
                                // search from left
                                {
                                                result = -1;
                                                int ns_len = normString.length();
                                                if ( ( pos >= ns_len ) || ( pos < 0 ) )
                                                                return -1;
                                                curPos = -1;
                                                while  ( curPos < pos )
                                                {
                                                                lastPos = curPos;
                                                                curPos = normString.indexOf(brks, curPos+1);
                                                                if ( ( curPos == -1 ) || ( curPos > pos ) )
                                                                {
                                                                        if ( curPos - pos == 1 && !bugfix)
                                                                        {
                                                                                result++;
                                                                                *incl = -1;
                                                                                return result;
                                                                        }
                                                                                curPos = lastPos;
                                                                                break;
                                                                }

                                                                QString found = brks.cap(0);
//                              if ( rxVariable.exactMatch(found) )
//                              {
//                                  if ( symbols != NULL ) {
//                                      found.remove(0,1);
//                                      int id = found.toInt();
//                                      if ( id <= symbols->count )
//                                      {
//                                          if ( symbols->symb_table[id].is_const && symbols->symb_table[id].type == kumBoolean )
//                                          {
//                                              result++;
//                                          }
//                                      }
//                                  }
//                              }
//                              else
                                                                        result++;
                                                }


                                } // end case (search_from_left)
                else
                                // search from right
                                {
                                                result = normString.count(brks);
                                                curPos = normString.length();
                                                int ns_len = normString.length();
                                                if ( ( pos >= ns_len ) || ( pos < 0 ) )
                                                                return -1;
                                                while  ( curPos > pos )
                                                {
                                                                lastPos = curPos;
                                                                curPos = normString.lastIndexOf(brks, curPos-1);
                                                                if ( curPos == -1 )
                                                                {
                                                                                curPos = lastPos;
                                                                                break;
                                                                }
                                                                result--;
                                                }

                                } // end case (search_from_left)

                // check if delimeter in region
                if ( curPos == pos )
                                *incl =  1;
                else
                                *incl = -1;

                return result;
}

*/
int text_analiz::parce_to(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return TO_TRASH;
	}


	return 0;
}

int text_analiz::parce_inache(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return INACHE_TRASH;
	}


	return 0;
}

int text_analiz::parce_vse(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return VSE_TRASH;
	}


	return 0;
}

int text_analiz::parceLoopEnd(const QString &instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return KC_TRASH;
	}
	return 0;
}

int text_analiz::parce_kon(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return KON_TRASH;
	}


	return 0;
}

int text_analiz::parceAlgorhitmBegin(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return NACH_TRASH;
	}
	return 0;
}

int text_analiz::parce_exit(QString instr)
{
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return EXIT_TRASH;
	}
	return 0;
}



int text_analiz::parce_isp(QString instr,
	function_table *table,
	symbol_table *symbols,
	int str,
	int *err_start,
	int *err_len,
	QString file,
	QString *returnName)
{
	if (app()->isExamMode()) {
		*err_start = 0;
		*err_len = instr.size();
		return 65534;
	}
	Q_UNUSED(table);
	Q_UNUSED(symbols);
	Q_UNUSED(str);
	Q_UNUSED(file);
	Q_UNUSED(err_start);
	instr.remove(0, 1);
	instr = instr.trimmed();

	if (instr.isEmpty()) {
		*err_start = 0;
		*err_len = 1;
		return ISP_NO_NAME;
	}
	KumTools t;
	int tn_start = 0, tn_len = 0;
	int err = t.test_name(instr, tn_start, tn_len);
	if (err > 0) {
		*err_start = 1 + tn_start;
		*err_len = tn_len;
		return err;
	}
	ispName = QObject::trUtf8(instr.toUtf8().data());

	//ispolniteli->addIsp( ispName,str,kumIsp,file);
	//  qDebug("ISPNAME = %s",instr.utf8().data());
	ispName = QObject::trUtf8(instr.toUtf8().data());
	if (returnName != NULL) {
		*returnName = ispName;
	}
	int modId = modules->idByName(ispName);
	if (modId > -1) {
		KumSingleModule *module = modules->module(modId);
		if (module->isEnabled()) {
			*err_start = 2;
			*err_len = ispName.length();
			return ISP_RELOAD;
		}
	}
	return 0;

}


int text_analiz::parce_endisp(QString instr)
{
	if (app()->isExamMode()) {
		Err_start = 0;
		Err_length = instr.size();
		return 65534;
	}
	if (instr.trimmed().length() > 1) {
		Err_start = 1;
		Err_length = instr.length() - 1;
		return ENDISP_THRASH;
	}
	//  ispolniteli->setIspEnd(ispolniteli->lastId(),str);
	//ispName = "main"; // TODO !!!
	return 0;

}


int text_analiz::parce_use_isp(QString instr, int str, QString *fileName, int *err_start, int *err_end)
{
	//TODO переделать по людски
	Q_UNUSED(err_start);
	Q_UNUSED(err_end);
	Q_UNUSED(str);
	Kumir::InstructionType strtype = KumTools::instance()->getStringType(instr);
	*fileName = "";
	if ((strtype != Kumir::UseModule) && (strtype != Kumir::Empty)) {
		return -10;
	}
	if (instr[2] != '\"') {
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Робот")) {
			return -2;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Робот Строитель")) {
			return -3;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Файлы")) {
			return -4;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Чертежник")) {
			return -5;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Двуног")) {
			return -6;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Файлы П")) {
			return -7;
		};
		*fileName = instr.mid(2, instr.length());
		return 0;

	} else {
		for (int i = 3; i < instr.length() - 1; i++) {
			*fileName += QChar(instr[i].unicode() - STR_HIDE_OFFSET);
		};
	};
	return 0;
};

int text_analiz::parce_use_isp(proga pv, QString *fileName, int *err_start, int *err_end)
{
	//TODO переделать по людски
	Kumir::InstructionType strtype = pv.str_type;
	QString instr = pv.line;
	Q_UNUSED(err_start);
	Q_UNUSED(err_end);
	*fileName = "";
	if ((strtype != Kumir::UseModule) && (strtype != Kumir::Empty)) {
		return -10;
	}
	if (instr[2] != '\"') {
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Робот")) {
			return -2;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Робот Строитель")) {
			return -3;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Строки")) {
			return -4;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Чертежник")) {
			return -5;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Двуног")) {
			return -6;
		};
		if (instr.mid(2, instr.length()) == QObject::trUtf8("Файлы П")) {
			return -7;
		};
		*fileName = instr.mid(2, instr.length());
		return 0;

	} else {
		for (int i = 3; i < instr.length() - 1; i++) {
			*fileName += QChar(instr[i].unicode() - STR_HIDE_OFFSET);
		};
	};
	return 0;
};

/**
 * Выполняет разбор описание группы величин.
 * @param source нормализованная строка описания группы величин
 * @param ispName имя исполнителя, к которому относится описание
 * @param algName имя алгоритма, к которому относится описание
 * @param mod модификатор доступа группы величин
 * @param str номер строки, где встретилось описание
 * @param symbols ссылка на таблицу символов
 * @param functions ссылка на таблицу функций
 * @param err_pos позиция ошибки при разборе
 * @param ids список id успешно разобранных величин
 * @param exprList список выражений для вычисления границ массивов
 * @return код ошибки или 0, если успешно
 */
int text_analiz::parceVariables(QString source,
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
	QList<proga> &extraProgram
)
{
	Q_UNUSED(extraProgram);
	Q_UNUSED(ispId);
	qDebug() << "Perm src" << source;
	while (source.endsWith(" ")) {
		source.remove(source.length() - 1, 1);
	}
	source += ";";

	//  � ежим работы автомата-парсера.
	//  type -- разбор типа
	//  name -- разбор имени переменной
	//  tn -- разбор типа или имени
	//  tab -- разбор массива
	//  lbN -- разбор левой границы
	//  lbN -- разбор правой границы
	//  value -- разбор константного значения

	enum Parcing { type, name, tn, lb, rb, value };

	//  Состояния автомата
	Parcing par = type; // режим разбора
	PeremType cType = none; // тип текущей величины
	QString cName = ""; // имя текущей величины
	QString cBound = ""; // текущая граница массива
	QString initValue = ""; // значение инициализации
	int deepV = 0; // глубина вложенности {}
	int deep1 = 0; // глубина вложенности [] внутри правой границы
	int deep2 = 0; // глубина вложенности () внутри правой границы
	int dim = 0;
	bool massDeclared = false;
	int currentID = -1;

	// Переменные цикла:
	// i - номер символа в анализируемой строке
	// p - начало имени переменной
	// q - начало границы массива (символ [)
	// s - начало правой граници массива (символ :)
	// t - позиция ксимвола типа данных
	// z - позиция начала константы
	int z = 0;

	for (int curPos = 0, nameStart = 0, arrayStart = 0, arrayBoundStart = 0, typePos = 0; curPos < source.length(); curPos++) {

		//      Предполагается, что модификатор группы уже задан, поэтому
		//      если встречается арг, рез или аргрез, то это ошибка

		if (
			(source[curPos] == QChar(KS_ARG))
			||
			(source[curPos] == QChar(KS_REZ))
			||
			(source[curPos] == QChar(KS_ARGREZ))
		) {
			err_pos.start = curPos;
			err_pos.len = 1;
			if (source[curPos] == QChar(KS_ARG)) {
				return PV_EXTRA_ARG;
			}
			if (source[curPos] == QChar(KS_REZ)) {
				return PV_EXTRA_RES;
			}
			if (source[curPos] == QChar(KS_ARGREZ)) {
				return PV_EXTRA_ARGRES;
			}
		} else if ((par == type) || (par == tn)) {
			if (source[curPos] == QChar(' ')) {
				nameStart++;
				continue;
			}
			//          � азбираемся с типом переменных
			if (
				(source[curPos] == QChar(KS_CEL))
				||
				(source[curPos] == QChar(KS_VES))
				||
				(source[curPos] == QChar(KS_LOG))
				||
				(source[curPos] == QChar(KS_LIT))
				||
				(source[curPos] == QChar(KS_SIM))
				||
				(source[curPos] == QChar(KS_CEL_TAB))
				||
				(source[curPos] == QChar(KS_VES_TAB))
				||
				(source[curPos] == QChar(KS_LOG_TAB))
				||
				(source[curPos] == QChar(KS_LIT_TAB))
				||
				(source[curPos] == QChar(KS_SIM_TAB))
			) {
				if (source[curPos] == QChar(KS_CEL)) {
					cType = integer;
				} else if (source[curPos] == QChar(KS_VES)) {
					cType = real;
				} else if (source[curPos] == QChar(KS_LOG)) {
					cType = kumBoolean;
				} else if (source[curPos] == QChar(KS_LIT)) {
					cType = kumString;
				} else if (source[curPos] == QChar(KS_SIM)) {
					cType = charect;
				} else if (source[curPos] == QChar(KS_CEL_TAB)) {
					cType = mass_integer;
				} else if (source[curPos] == QChar(KS_VES_TAB)) {
					cType = mass_real;
				} else if (source[curPos] == QChar(KS_LOG_TAB)) {
					cType = mass_bool;
				} else if (source[curPos] == QChar(KS_LIT_TAB)) {
					cType = mass_string;
				} else {
					cType = mass_charect;
				}
				//              Переходим в режим поиска имени
				par = name;
				cName = "";
				nameStart = curPos + 1;
				typePos = curPos;
				continue;
			} else if (par == tn) {
				par = name;
			} else {
				if (typePos != 0) {
					err_pos.start = typePos;
					err_pos.len = 1;
				} else {
					err_pos.start = 0;
					err_pos.len = curPos;
				}
				return PV_NO_TYPE; // не указан тип переменной
			}
		} // endif ( par == type || par == tn )
		if (par == name) {
			if ((source[curPos] == ' ') && cName.isEmpty()) {
				nameStart++;
				continue;
			}
			if (source[curPos] == ']') {
				err_pos.start = curPos;
				err_pos.len = 1;
				return PV_NO_OPEN_QB;
			}
			if (source[curPos] == '[') {
				if (massDeclared) {
					err_pos.start = curPos;
					err_pos.len = 1;
					return PV_BOUND_REDECL;
				}
				if (cName.trimmed().isEmpty()) {
					err_pos.start = typePos;
					err_pos.len = 1;
					return PV_NO_NAME; // нет имени переменной
				} else {
					if ((cType == integer) || (cType == real) || (cType == kumBoolean) || (cType == kumString) || (cType == charect)) {
						err_pos.start = curPos;
						int e = source.indexOf("]", curPos);
						if (e == -1) {
							err_pos.len = source.length() - curPos - 1;
						} else {
							err_pos.len = e - curPos + 1;
						}
						return PV_EXTRA_BOUNDS; // границы не нужны
					} else {
						cBound = "";
						par = lb;
						arrayStart = curPos;
						arrayBoundStart = curPos;
					}
				}
			} // if ( '[' )
			else if (source[curPos] == QChar(KS_PRISV)) {
				err_pos.start = curPos;
				err_pos.len = 1;
				return PV_ASSIGNMENT;
			} else if ((source[curPos] == ',') || (source[curPos] == ';') || (source[curPos] == '=')) {

				if (cName.trimmed().isEmpty()) {
					err_pos.start = typePos;
					err_pos.len = 1;
					return PV_NO_NAME;
				}



				int loc_err, err_start = 0, err_len = 0;
				loc_err = KumTools::instance()->test_name(cName, err_start, err_len);
				if (loc_err > 0) {
					err_pos.start = nameStart + err_start;
					err_pos.len = err_len;
					return loc_err; // ошибка в имени
				}
				if (cName.count("\"") || cName.count("'")) {
					QRegExp rxQuot = QRegExp("[\"']");
					int st = rxQuot.indexIn(cName);
					int en = rxQuot.lastIndexIn(cName);
					err_pos.start = nameStart + st;
					err_pos.len = st == en ? 1 : en - st + 1;
					return PV_LITCONST;
				}
				cName = cName.trimmed();
				err_pos.start = nameStart;
				for (int j = nameStart; j < curPos; j++) {
					if (source[j] == ' ') {
						err_pos.start++;
					} else {
						break;
					}
				}
				err_pos.len = curPos - nameStart;
				for (int j = curPos - 1; j > nameStart; j--) {
					if (source[j] == ' ') {
						err_pos.len--;
					} else {
						break;
					}
				}


				if (cName == QObject::trUtf8("знач")) {
					return PV_ZNACH_NAME;
				}
				if (symbols->inTable(cName, algName) != -1) {
					return PV_REDECLARATION;    // переменная уже объявлена
				}
				//              if ( symbols->inTable(cName,"global",ispId) != -1 )

				int func_id = -1;
				int module_id = -1;
				modules->FuncByName(cName, &module_id, &func_id);
				if (func_id != -1) {
					return PV_REDECLARATION_ALG;
				}
				if (
					!((cType == integer) || (cType == real) || (cType == kumBoolean) || (cType == kumString) || (cType == charect))
					&&
					! massDeclared
				) {
					err_pos.start = nameStart;
					err_pos.len = curPos - nameStart;
					return PV_NO_BOUNDS; // нет границ
				}
				loc_err = symbols->add_symbol(cName, // имя переменной
						str, // номер строки
						cType, // тип переменной
						algName,
						isArg && (mod == arg) // константа, если "арг"
					);
				if (loc_err) {
					return INT_TA_13; // TODO error code
				}
				int id = symbols->symb_table.size() - 1;
				symbols->symb_table[id].is_Res = mod == res;
				symbols->symb_table[id].is_ArgRes = mod == argres;
				symbols->symb_table[id].used = false;
				symbols->symb_table[id].is_const = false;
				symbols->symb_table[id].is_Arg = isArg;


				if ((cType == integer) || (cType == real) || (cType == kumBoolean) || (cType == kumString) || (cType == charect)) {
					symbols->new_symbol(id);
					symbols->symb_table[id].razm = 0;
				} else {
					symbols->symb_table[id].razm = dim;
				}
				currentID = id;
				ids << id;
				cName = "";
				par = tn;
				dim = 0;
				massDeclared = false;
				nameStart = curPos + 1;
				if (source[curPos] == '=') {
					par = value;
					z = curPos + 1;
				}
			} else {
				cName += source[curPos];
			}
		} // endif ( par == name || par == tn )
		else if (par == lb) {
			if (source[curPos] == '[') {
				deep1 ++;
				cBound += '[';
			} else if (source[curPos] == '(') {
				deep2 ++;
				cBound += '(';
			} else if ((source[curPos] == ']') && (deep1 > 0)) {
				deep1 --;
				cBound += ']';

			} else if (source[curPos] == ')') {
				deep2 --;
				cBound += ')';
			} else if (source[curPos] == ',' && deep2 == 0 && deep1 == 0) {
				if (cBound.trimmed().isEmpty()) {
					err_pos.start = arrayStart;
					err_pos.len = curPos - arrayStart;
					return PV_NO_LEFT_BOUND;
				} else {
					err_pos.start = curPos;
					err_pos.len = 1;
					return PV_COMMA_INSTEAD_SEMICOLON;
				}
			}
			//          � азбор левой границы массива
			else if (source[curPos] == ';') {
				err_pos.start = arrayStart;
				err_pos.len = 1;
				return PV_NO_CLOSED_QB;
			} else if (source[curPos] == ':') {
				QStringList el;
				int locErr = 0;

				ErrorPosition locErrPos;
				if (cBound.trimmed().isEmpty()) {
					err_pos.start = arrayStart;
					err_pos.len = curPos - arrayStart;
					return PV_NO_LEFT_BOUND;
				}

				PeremType leftType = parceExpression2(cBound.trimmed(), functions, symbols, algName, &locErr, &el, &locErrPos, extraSubExpressions, false);
				if (locErr > 0) {
					err_pos.start = arrayBoundStart + 1 + locErrPos.start;
					err_pos.len = locErrPos.len;
					return locErr; // ошибка разбора левой границы
				}
				if (leftType != integer) {
					err_pos.start = arrayBoundStart;
					err_pos.len = curPos - arrayBoundStart;
					return PV_BAD_LEFT; // левая граница -- не целое число
				}

				QString pattern = "\\b" + cName + "\\b";
				if (!algName.isEmpty()) {
					pattern += "|\\b" + algName + "\\b";
				}
				QRegExp rxRecursion = QRegExp(pattern);
				int rxPos = rxRecursion.indexIn(cBound, 0);
				if (rxPos != -1) {
					int rxLen = rxRecursion.matchedLength();
					err_pos.start = arrayBoundStart + rxPos;
					err_pos.len = rxLen;
					return PV_RECURSION_L; // рекурсивное определение массива
				}
				exprList << el;
				cBound = "";
				par = rb;
				arrayBoundStart = curPos;
			} else if (source[curPos] == ']' && deep1 == 0) {
				err_pos.start = curPos;
				err_pos.len = 1;
				return PV_NO_LEFT_BOUND;
			} else {
				cBound += source[curPos];
			}
		} // endif ( par == lb )
		else if (par == rb) {
			if (source[curPos] == '[') {
				deep1 ++;
				cBound += '[';
			} else if (source[curPos] == '(') {
				deep2 ++;
				cBound += '(';
			} else if ((source[curPos] == ']') && (deep1 > 0)) {
				deep1 --;
				cBound += ']';

			} else if (source[curPos] == ')') {
				deep2 --;
				cBound += ')';
			} else if ((source[curPos] == ',') && (deep1 == 0) && (deep2 == 0)) {
				if (dim > 2) {
					err_pos.start = arrayStart;
					int e = source.indexOf("]", curPos);
					if (e == -1) {
						err_pos.len = curPos - arrayStart;
					} else {
						err_pos.len = e - arrayStart + 1;
					}
					return PV_BIG_RAZM; // размерность > 3
				}
				dim++;

				QStringList el;
				int locErr = 0;

				ErrorPosition locErrPos;

				PeremType rightType = parceExpression2(cBound, functions, symbols, algName, &locErr, &el, &locErrPos, extraSubExpressions, false);
				if (locErr > 0) {
					err_pos.start = arrayBoundStart + locErrPos.start;
					err_pos.len = locErrPos.len;
					return locErr; // ошибка разбора левой границы
				}
				if (rightType != integer) {
					err_pos.start = arrayBoundStart;
					err_pos.len = curPos - arrayBoundStart;
					return PV_BAD_RIGHT; // правая граница -- не целое число
				}

				QString pattern = "\\b" + cName + "\\b";
				if (!algName.isEmpty()) {
					pattern += "|\\b" + algName + "\\b";
				}
				QRegExp rxRecursion = QRegExp(pattern);
				int rxPos = rxRecursion.indexIn(cBound, 0);
				if (rxPos != -1) {
					int rxLen = rxRecursion.matchedLength();
					err_pos.start = arrayBoundStart + rxPos;
					err_pos.len = rxLen;
					return PV_RECURSION_R; // рекурсивное определение массива
				}
				exprList << el;
				cBound = "";
				par = lb;
				arrayBoundStart = curPos;
			} else if ((source[curPos] == ']') && (deep1 == 0)) {
				if (dim > 2) {
					err_pos.start = arrayStart;
					int e = source.indexOf("]", curPos);
					if (e == -1) {
						err_pos.len = curPos - arrayStart;
					} else {
						err_pos.len = e - arrayStart;
					}
					return PV_BIG_RAZM; // размерность > 3
				}

				dim++;

				QStringList el;
				int locErr = 0;

				ErrorPosition locErrPos;
				if (cBound.trimmed().isEmpty()) {
					err_pos.start = arrayBoundStart;
					err_pos.len = curPos - arrayBoundStart;
					return PV_NO_RIGHT_BOUND;
				}

				PeremType rightType = parceExpression2(cBound, functions, symbols, algName, &locErr, &el, &locErrPos, extraSubExpressions, false);
				if (locErr > 0) {
					err_pos.start = arrayBoundStart + 1 + locErrPos.start;
					err_pos.len = locErrPos.len;
					return locErr; // ошибка разбора левой границы
				}
				if (rightType != integer) {
					err_pos.start = arrayBoundStart + 1 + locErrPos.start;
					err_pos.len = curPos - arrayBoundStart - 1;
					return PV_BAD_RIGHT; // правая граница -- не целое число
				}
				QString pattern = "\\b" + cName + "\\b";
				if (!algName.isEmpty()) {
					pattern += "|\\b" + algName + "\\b";
				}
				QRegExp rxRecursion = QRegExp(pattern);
				int rxPos = rxRecursion.indexIn(cBound, 0);
				if (rxPos != -1) {
					int rxLen = rxRecursion.matchedLength();
					err_pos.start = arrayBoundStart + rxPos;
					err_pos.len = rxLen;
					return PV_RECURSION_R; // рекурсивное определение массива
				}
				exprList << el;
				par = tn;
				massDeclared = true;

			} else if (source[curPos] == ';') {
				err_pos.start = arrayBoundStart;
				err_pos.len = 1;
				return PV_NO_CLOSE_BRACKET;
			} else {
				cBound += source[curPos];
			}
		} // endif ( par == rb )
		else if (par == value) {
			if (isArg) {
				err_pos.start = z - 1;
				err_pos.len = 1;
				return PV_CONSTANT_IN_RES;
			}
			if (source[curPos] == '{') {
				deepV ++;
			} else if (source[curPos] == '}') {
				deepV --;
			}
			if (deepV == 0 && (source[curPos] == ',' || source[curPos] == ';')) {

//                qDebug() << "Extracted constant value: " << initValue;
				if (initValue.trimmed().isEmpty()) {
					err_pos.start = z - 1;
					err_pos.len = 1;
					return PV_CONST_IS_EMPTY;
				}
				QString cValue;
				int localErrPos;
				int localErrLen;

				int err = parceConstant(initValue, cType, dim, cValue, localErrPos, localErrLen);
				if (err != 0) {
					err_pos.start = z + localErrPos;
					err_pos.len = localErrLen;
					return err;
				}

				int i_value;
				double d_value;
				QChar c_value;
				bool b_value;
				QString s_value;

				if (cType == integer || cType == real || cType == kumBoolean || cType == charect || cType == kumString) {
					QByteArray ba = QByteArray::fromBase64(cValue.toLatin1());
					QString sval = QString::fromUtf8(ba);
					if (cType == kumString) {
						s_value = sval;
					} else if (cType == charect) {
						Q_ASSERT(sval.length() == 1);
						c_value = sval[0];
					} else if (cType == kumBoolean) {
						b_value = sval == "true";
					} else if (cType == integer) {
						bool ok;
						i_value = sval.toInt(&ok);
						Q_ASSERT(ok);
					} else if (cType == real) {
						bool ok;
						d_value = sval.toDouble(&ok);
						Q_ASSERT(ok);
					}
					symbols->setUsed(currentID, true);
				}

				if (cType == integer) {
					symbols->symb_table[currentID].value.setIntegerValue(i_value);
				} else if (cType == real) {
					symbols->symb_table[currentID].value.setFloatValue(d_value);
				} else if (cType == kumBoolean) {
					symbols->symb_table[currentID].value.setBoolValue(b_value);
				} else if (cType == charect) {
					symbols->symb_table[currentID].value.setCharectValue(c_value);
				} else if (cType == kumString) {
					symbols->symb_table[currentID].value.setStringValue(s_value);
				} else {

					// Формируем скрытую инструкцию - вызов одной из функций:
					// алг !memfill_*$(аргрез *таб$ массив, арг лит данные)
					// где массив - это заполняемый массив,
					//     данные - сериализованные данные в Base64


					symbols->symb_table[currentID].razm = dim;
					symbols->setConstantValue(currentID, cValue);


					// Формируем имя функции на основании типа данных массива
					// и его размерности

					//                    QString mft;
					//                    if ( cType==mass_integer )
					//                        mft = "i";
					//                    else if ( cType==mass_real )
					//                        mft = "d";
					//                    else if ( cType==mass_bool )
					//                        mft = "b";
					//                    else if ( cType==mass_charect )
					//                        mft = "c";
					//                    else if ( cType==mass_string )
					//                        mft = "s";
					//                    QString functName = QString("!memfill_%1%2").arg(mft).arg(dim);

					// Заводим константу в таблице символов

					//                    symbols->add_symbol("!KUM_CONST",-1,kumString,algName,true);
					//                    symbols->new_symbol(symbols->count);
					//                    int literal_id = symbols->count;
					//                    set_const_value(literal_id, "\""+cValue+"\"", symbols);
					//                    symbols->setUsed(literal_id, true);

					// Формируем инструкцию


					//                    proga e;
					//                    e.str_type = ALG_USE;
					//                    int module_id = -1;
					//                    int func_id = -1;

					//                    modules->FuncByName(functName, &module_id, &func_id);
					//                    e.line = QString("%1|%2").arg(func_id).arg(module_id);
					//                    QStringList vl;
					//                    vl << "^"+QString::number(currentID);
					//                    vl << "^"+QString::number(literal_id);
					//                    vl << QString("@%1&0&1").arg(func_id);
					//                    e.VirajList << vl;

					//                    e.VirajTypes << cType;
					//                    e.VirajTypes << kumString;

					//                    extraProgram << e;

				}
				par = tn;
				initValue = "";
			} else {
				initValue += source[curPos];
			}

		} // endif ( par==value )
	}
	return 0;
}

int text_analiz::parceConstant(const QString &constant, const PeremType pt, int &maxDim, QString &value, int &errPos, int &errLen)
{
	int localErr = 0;
	PeremType ct;

	if (constant.trimmed() == "...") {
		errPos = 0;
		errLen = constant.length();
		return PV_CONSTANT_NOT_TYPED;
	}
	if (pt == integer || pt == real || pt == kumBoolean || pt == charect || pt == kumString) {
		ct = testConst(constant, &localErr);
		if (ct == none) {
			errPos = 0;
			errLen = constant.length();
			return PV_NOT_A_CONSTANT;
		}
		if (localErr > 0) {
			errPos = 0;
			errLen = constant.length();
		} else if ((ct == pt) || (ct == integer || pt == real)) {
			QByteArray ba = createConstValue(constant, ct).toString().toUtf8().toBase64();
			value = QString::fromLatin1(ba);
		} else {
			errPos = 0;
			errLen = constant.length();
			return PV_REGULAR_TYPE_MISMATCH; // FIXME: type mismatch
		}
	} else {
		QVariant d0;
		QList<QVariant> d1;
		QList< QList<QVariant> > d2;
		QList< QList< QList<QVariant> > > d3;
		QList<QVariant> items;
		QString current = "";
		int deep = 0;
		PeremType t;
		if (pt == mass_integer) {
			t = integer;
		} else if (pt == mass_real) {
			t = real;
		} else if (pt == mass_bool) {
			t = kumBoolean;
		} else if (pt == mass_charect) {
			t = charect;
		} else if (pt == mass_string) {
			t = kumString;
		} else {
			qFatal("Trying to untable regular type");
		}
		int maxDeep = 0;
		int lexemStart = 0;
		PeremType it;
		for (int i = 0; i < constant.length(); i++) {
			if (constant[i] == '{') {
				deep++;
				lexemStart = i + 1;
				maxDeep = qMax(deep, maxDeep);
				if (deep > 3) {
					errPos = i;
					errLen = 1;
					return PV_CONST_EXTRA_OPEN_BRACKET; // FIXME: extra opening
				}
			} else if (constant[i] == '}') {
				if (deep == 0) {
					errPos = i;
					errLen = 1;
					return PV_CONST_EXTRA_CLOSE_BRACKET; // FIXME: extra closing }
				}
				deep--;
			}
		}
		for (int i = 0; i < constant.length(); i++) {
			if (constant[i] == '{') {
				deep++;
				lexemStart = i + 1;

				if (deep > 3) {
					errPos = i;
					errLen = 1;
					return PV_CONST_EXTRA_OPEN_BRACKET; // FIXME: extra opening
				}
			} else if (constant[i] == '}') {
				if (deep == 0) {
					errPos = i;
					errLen = 1;
					return PV_CONST_EXTRA_CLOSE_BRACKET; // FIXME: extra closing }
				}
				if (deep == maxDeep) {
					if (current.trimmed().isEmpty()) {
						d0 = QVariant();
						d1 << d0;
					} else {
						it = testConst(current, &localErr);
						if (localErr > 0) {
							errPos = lexemStart;
							errLen = i - lexemStart;
							return localErr;
						}
						if (it != t && !(it == integer && t == real) && !(it == charect && t == kumString)) {
							errPos = lexemStart;
							errLen = i - lexemStart;
							return PV_CONST_ELEM_TYPE_MISMATCH; // FIXME: type mismatch
						}
						d0 = createConstValue(current, t);
						d1 << d0;
						lexemStart = i + 1;
						current = "";
					}
				} else if (deep == maxDeep - 1) {
					d2 << d1;
					d1.clear();
				} else if (deep == maxDeep - 2) {
					d3 << d2;
					d2.clear();
				}
				deep--;
			} else if (constant[i] == ',') {
				if (deep == maxDeep) {
					if (current.trimmed().isEmpty()) {
						d0 = QVariant();
						d1 << d0;
					} else {
						it = testConst(current, &localErr);
						if (localErr > 0) {
							errPos = lexemStart;
							errLen = i - lexemStart;
							return localErr;
						}
						if (it != t && !(it == integer && t == real) && !(it == charect && t == kumString)) {
							errPos = lexemStart;
							errLen = i - lexemStart;
							return PV_CONST_ELEM_TYPE_MISMATCH; // FIXME: type mismatch
						}
						d0 = createConstValue(current, t);
						d1 << d0;
						lexemStart = i + 1;
						current = "";
					}
				} else if (deep == maxDeep - 1) {
					d2 << d1;
					d1.clear();
				} else if (deep == maxDeep - 2) {
					d3 << d2;
					d2.clear();
				}
			} else {
				current += constant[i];
			}
		}

		int dim;
		if (maxDim == 0) {
			dim = maxDeep;
			maxDim = dim;
		} else {
			if (maxDeep != maxDim) {
				errPos = 0;
				errLen = constant.length();
				return PV_CONST_DIMENSION_MISMATCH; // FIXME: dimension mismatch
			}
			dim = maxDim;
		}
		Q_ASSERT(dim >= 1 && dim <= 3);
		Q_ASSERT(deep == 0);
		value = "";
		if (dim == 1) {
			for (int i = 0; i < d1.size(); i++) {
				QByteArray ba = d1.at(i).toString().toUtf8().toBase64();
				value += ba;
				if (i < d1.size() - 1) {
					value += ".";
				}
			}
		} else if (dim == 2) {
			for (int i = 0; i < d2.size(); i++) {
				for (int j = 0; j < d2.at(i).size(); j++) {
					QByteArray ba = d2.at(i).at(j).toString().toUtf8().toBase64();
					value += ba;
					if (j < d2.at(i).size() - 1) {
						value += ".";
					}
				}
				if (i < d2.size() - 1) {
					value += ":";
				}
			}
		} else if (dim == 3) {
			for (int i = 0; i < d3.size(); i++) {
				for (int j = 0; j < d3.at(i).size(); j++) {
					for (int k = 0; k < d3.at(i).at(j).size(); k++) {
						QByteArray ba = d3.at(i).at(j).at(k).toString().toUtf8().toBase64();
						value += ba;
						if (k < d3.at(i).at(j).size() - 1) {
							value += ".";
						}
					}
					if (j < d3.at(i).size() - 1) {
						value += ":";
					}
				}
				if (i < d3.size() - 1) {
					value += "|";
				}
			}
		}
	}

//    qDebug() << "VALUE: " << value;
	return localErr;
}

/**
 * � азбор строки описания алгоритма
 * @param stroka нормализованная разбираемая строка
 * @param str номер нормализованной(?) строки
 * @param *symbols ссылка на таблицу символов
 * @param *functions ссылка на таблицу функций
 * @param proga_str
 * @return код ошибки
 */
int text_analiz::parceAlgorhitmHeader(proga &pv, int str, symbol_table *symbols, function_table *functions)
{
	QString stroka = pv.line;
	// Проверка возможности добавления в таблицу функций


	// � азбираем строку на имя алгоритма, его тип и список аргументов

	QString name; // имя алгоритма
	PeremType returnType = none; // возвращаемый тип
	bool hasArgs = true; // флаг "Есть аргументы"
	QRegExp rxNonSpace = QRegExp("\\S+"); // непробелный символ

	// ============== ИМЯ АЛГО� ИТМА

	// ищем первый пробел в строке
	int p = stroka.indexOf(' ');
	int str_len = stroka.length();
	int tp = -1;
	if ((p < str_len) && (p != -1)) {
		p++;
		returnType = getPeremTypeByKS(stroka[p].unicode());
		if (returnType != none) {
			tp = p;
			p = stroka.indexOf(rxNonSpace, p + 1);
			if (p == -1) {
				p = stroka.length() - 1;
			}
		}
	}

	// ищем открывающую скобку
	int p2 = stroka.indexOf('(');
	if (p2 == -1) {
		p2 = stroka.length();
		hasArgs = false;
	}

	// выделяем имя алгоритма
	if (p != -1) {
		name = stroka.mid(p, p2 - p).trimmed();
	}
	if (name.startsWith(QString(QChar(KS_DUMMY_ALG_NAME)))) {
		QStringList pair = name.split(" ");
		name = "!DUMMY_" + pair[1];
	} else {
		// проверяем корректность имени алгоритма
		int tn_start = 0, tn_len = 0;
		int local_err = KumTools::instance()->test_name(name, tn_start, tn_len);
		if (local_err > 0) {
			Err_start = p + tn_start;
			Err_length = tn_len;
			return local_err;
		}
	}
	// qDebug()<<"modules in analizer"<< modules;
	// qDebug()<<"modules in compiler"<< app()->compiler->modules();
	//qDebug()<<"moduleId" <<moduleId;

	QString ispName = app()->compiler->modules()->module(moduleId)->Name();
	bool isFirst = firstAlg && (ispName == "@main");

	// флаг firstAlg проставляется в test_all, означает "первый алгоритм"

	if (isFirst) {
		firstAlg = false;
		// если алгоритм первый в тексте, то допускается пустое имя,
		// но при этом не должно быть аргументов и возвращаемого параметра
		if (name.isEmpty()) {
			if (hasArgs) {
				Err_start = p2;
				Err_length = stroka.length() - p2;
				return FUNC_EXTRA_ARGS;
			}
			if (returnType != none) {
				Err_start = tp;
				Err_length = 1;
				return FUNC_MUST_NONE;
			}
		}
	} else {
		// если алгоритм не первый, обязательно должно быть имя, иначе -- ошибка
		if (name.isEmpty()) {
			Err_start = 0;
			Err_length = stroka.length();
			return FUNC_NET_IMENI_2;
		}
	}

	// Проверяем на повторное описание алгоритма
	int func_id = -1;
	int module_Id = -1;
	modules->FuncByName(name, &module_Id, &func_id);
	if (func_id != -1) {
		Err_start = p;
		Err_length = p2 - p;
		return FUNC_ALG_EXIST;
	}

	// Проверяем на наличие переменной с таким же именем
	int isp_id = moduleId;

	if (symbols->inTable(name, "global") != -1) {
		Err_start = p;
		Err_length = p2 - p;
		return FUNC_NAME_USED;
	}

	// Если функция что-то возвращает, то добавляем в таблицу символов
	// одноименную переменную

	if (returnType != none) {
		int local_error = symbols->add_symbol(name, str, returnType, "global", false);
		if (local_error > 0) {
			//          Err_start = -1;
			//          Err_end = -1;
			// Если не получилось добавить имя в таблицу символов,
			// то это внутренняя ошибка КУМИ� а
			return INT_TA_11;
		}
		symbols->new_symbol(symbols->symb_table.size() - 1);
		symbols->setUsed(symbols->symb_table.size() - 1, false);
		symbols->symb_table[symbols->symb_table.size() - 1].is_FunctionReturnValue = true;
	}

	if (returnType == mass_integer || returnType == mass_real || returnType == mass_bool || returnType == mass_charect || returnType == mass_string) {
		Err_start = tp;
		Err_length = 1;
		return FUNC_RETURN_MASS;
	}

	// Заносим алгоритм в таблицу функций
	int id = functions->append(name, returnType, str);

	// Формируем строку выполнителя
	pv.VirajList.clear();
	pv.line = QString(QChar(KS_ALG)) + " ";
	if (returnType == integer) {
		pv.line += QString(QChar(KS_CEL)) + " ";
	}
	if (returnType == real) {
		pv.line += QString(QChar(KS_VES)) + " ";
	}
	if (returnType == kumBoolean) {
		pv.line += QString(QChar(KS_LOG)) + " ";
	}
	if (returnType == charect) {
		pv.line += QString(QChar(KS_SIM)) + " ";
	}
	if (returnType == kumString) {
		pv.line += QString(QChar(KS_LIT)) + " ";
	}
	if (returnType == mass_integer) {
		pv.line += QString(QChar(KS_CEL_TAB)) + " ";
	}
	if (returnType == mass_real) {
		pv.line += QString(QChar(KS_VES_TAB)) + " ";
	}
	if (returnType == mass_bool) {
		pv.line += QString(QChar(KS_LOG_TAB)) + " ";
	}
	if (returnType == mass_charect) {
		pv.line += QString(QChar(KS_SIM_TAB)) + " ";
	}
	if (returnType == mass_string) {
		pv.line += QString(QChar(KS_LIT_TAB)) + " ";
	}
	pv.line += name;

	functions->setRunStepInto(id, isFirst && hasArgs);

	// Если нет аргументов, то всё

	if (!hasArgs) {
		functions->setBroken(id, false);
		return 0;
	}


	// Если алгоритм первый, то у него не может быть аргументов
	//  if ( hasArgs && isFirst )
	//  {
	//      functions->func_table[id].badalg = true;
	//      Err_start = p2;
	//      Err_length = stroka.length()-p2;
	//      return FUNC_EXTRA_ARGS_FIRST;
	//  }

	// =============== � АЗБОР  АРГУМЕНТОВ


	QString argLine = stroka.mid(p2);
	if (argLine.startsWith("(") && argLine.endsWith(")")) {
		argLine.replace(0, 1, " ");
		argLine.replace(argLine.length() - 1, 1, " ");
		if (argLine.trimmed().isEmpty()) {
			functions->setBroken(id, true);
			Err_start = p2;
			Err_length = stroka.length() - p2;
			return FUNC_NOARG;
		}
		QStringList groups;
		QList<ArgModifer> mods;
		QList<int> shifts;
		ArgModifer mod = arg;
		bool hasArraysInArgs = false;
		QString group;
		int specifierPos = -1;
		for (int i = 0; i < argLine.length(); i++) {
			if (argLine[i] == QChar(KS_ARG) || argLine[i] == QChar(KS_REZ) || argLine[i] == QChar(KS_ARGREZ)) {
				specifierPos = i;
				if (group.trimmed().endsWith(",")) {
					mod = arg;
					int cp = group.lastIndexOf(",");
					group.truncate(cp);
				} else if (!group.trimmed().isEmpty()) {
					functions->setBroken(id, true);
					Err_start = p2 + i;
					Err_length = 1;
					return FUNC_NO_COMMA_BETWEEN_GROUPS;
				}
				if (argLine[i] == QChar(KS_ARG)) {
					mod = arg;
				}
				if (argLine[i] == QChar(KS_REZ)) {
					if (mod == res || mod == argres) {
						return 20110;
					};
					mod = res;

				}
				if (argLine[i] == QChar(KS_ARGREZ)) {
					if (mod == res || mod == argres) {
						return 20112;
					};
					mod = argres;

				};
				mods << mod;
				if (!group.trimmed().isEmpty()) {
					groups << group;
					group = "";
				}
				shifts << i + 1;

			} else if (argLine[i] == ',' && group.trimmed().isEmpty()) {
				functions->setBroken(id, true);;
				if (specifierPos == -1) {
					Err_start = p2 + i;
					Err_length = 1;
					return 1234000;
				} else {
					Err_start = p2 + specifierPos;
					Err_length = 1;
					if (argLine[specifierPos] == QChar(KS_REZ)) {
						return 7890000;
					} else if (argLine[specifierPos] == QChar(KS_ARGREZ)) {
						return 7890001;
					} else {
						return 7890002;
					}
				}
			} else if (argLine[i] == QChar(KS_ALG)) {
				functions->setBroken(id, true);;
				Err_start = p2 + i;
				Err_length = 1;
				return FUNC_ALG_INSTEAD_ARG;
			} else {
				group += argLine[i];
			}
		}
		groups << group;
		if (groups.count() != mods.count()) {
			mods.prepend(arg);
		}
		if (groups.count() != shifts.count()) {
			shifts.prepend(0);
		}
		QStringList exprList;
		QList<int> ids;

		for (int i = 0; i < groups.count(); i++) {
			int locErr = 0;
			ErrorPosition locErrPos;
			QList<proga> extraProgram;
			locErr = parceVariables(groups[i], isp_id, name, mods[i], str, symbols, functions, locErrPos, ids, pv.VirajList, pv.extraSubExpressions, true, extraProgram);
			if (locErr > 0) {
				Err_start = p2 + shifts[i] + locErrPos.start;
				Err_length = locErrPos.len;
				functions->setBroken(id, true);
				return locErr;
			}
		}
		QString argNormLine;
		int counter = 0;
		for (int i = 0; i < ids.count(); i++) {
			PeremType type = symbols->symb_table[ids[i]].type;
			if ((type == integer) || (type == real) || (type == kumBoolean) || (type == kumString) || (type == charect)) {
				argNormLine += "^" + QString::number(ids[i]);
			} else {
				hasArraysInArgs = true;
				argNormLine += "^" + QString::number(ids[i]);
				int dim = symbols->symb_table[ids[i]].razm;
				for (int j = 0; j < dim * 2; j++) {
					argNormLine += "#" + QString::number(counter);
					counter++;
				}
			}
			if (i != (ids.count() - 1)) {
				argNormLine += ",";
			}

			functions->appendArg(id,
				symbols->symb_table[ids[i]].type,
				symbols->symb_table[ids[i]].razm,
				symbols->symb_table[ids[i]].is_Res,
				symbols->symb_table[ids[i]].is_ArgRes,
				ids[i]);


		}
		pv.line += "(" + argNormLine + ")";
		if (isFirst && hasArgs) {
			//            // Заплатка: запрещаем аргументы-массивы в первом алгоритме с параметрами
			//            if (hasArraysInArgs) {
			//                functions->setBroken(id, true);
			//                Err_start = p2;
			//                Err_length = stroka.length()-p2;
			//                return FUNC_ARRAY_PARAM_IN_FIRST_ALG;

			//            }
			// Создание точки входа, которая
			// будет требовать ввод аргументов
			// и вывод результатов
			m_firstAlgorhitmIP = str;
			generateEntryPoint(groups, mods, symbols, functions, name, returnType, pv.real_line);
		}
	} else {
		functions->setBroken(id, true);
		if (! argLine.startsWith("(") && argLine.endsWith(")")) {
			Err_start = stroka.length() - 1;
			Err_length = 1;
			return 20201;
		}
		if (argLine.startsWith("(") && ! argLine.endsWith(")")) {
			Err_start = p2;
			Err_length = 1;
			return 20202;
		}
	}
	functions->setBroken(id, false);

	return 0;
}

QString screenString(const QString &txt)
{
	QString result;
	for (int i = 0; i < txt.length(); i++) {
		result += QChar(txt[i].unicode() + STR_HIDE_OFFSET);
	}
	return result;
}

struct KumVariable {
	PeremType type;
	int dimension;
	QString name;
	QString decl;
};

struct KumVariableGroup {
	text_analiz::ArgModifer mod;
	QList<KumVariable> vars;
};


QString typeToKsymbol(const PeremType pt)
{
	int ks = KS_CEL;
	if (pt == integer) {
		ks = KS_CEL;
	} else if (pt == real) {
		ks = KS_VES;
	} else if (pt == kumBoolean) {
		ks = KS_LOG;
	} else if (pt == kumString) {
		ks = KS_LIT;
	} else if (pt == charect) {
		ks = KS_SIM;
	} else if (pt == mass_integer) {
		ks = KS_CEL_TAB;
	} else if (pt == mass_real) {
		ks = KS_VES_TAB;
	} else if (pt == mass_bool) {
		ks = KS_LOG_TAB;
	} else if (pt == mass_charect) {
		ks = KS_SIM_TAB;
	} else if (pt == mass_string) {
		ks = KS_LIT_TAB;
	}
	return QString(QChar(ks));
}

QList<KumVariableGroup> parceGroups(const QStringList &argGroups,
	const QList<text_analiz::ArgModifer> &argMods)
{
	QList<KumVariableGroup> groups;
	K_ASSERT(argGroups.count() == argMods.count());
	for (int i = 0; i < argGroups.count(); i++) {
		KumVariableGroup group;
		group.mod = argMods[i];
		int deep = 0;
		QString line = argGroups[i];
		KumVariable cur;
		cur.dimension = 0;
		for (int j = 0; j < line.length(); j++) {
			if (line[j] == '[') {
				deep++;
				cur.decl += "[";
				cur.dimension++;
			} else if (line[j] == ']') {
				deep--;
				cur.decl += "]";
			} else if (line[j] == ',' && deep == 0) {
				group.vars << cur;
				cur.name = "";
				cur.decl = "";
			} else if (line[j] == ',' && deep > 0) {
				cur.dimension++;
				cur.decl += ",";
			} else {
				cur.decl += line[j];
				if (line[j] == QChar(KS_CEL)) {
					cur.type = integer;
				} else if (line[j] == QChar(KS_VES)) {
					cur.type = real;
				} else if (line[j] == QChar(KS_LOG)) {
					cur.type = kumBoolean;
				} else if (line[j] == QChar(KS_SIM)) {
					cur.type = charect;
				} else if (line[j] == QChar(KS_LIT)) {
					cur.type = kumString;
				} else if (line[j] == QChar(KS_CEL_TAB)) {
					cur.type = mass_integer;
				} else if (line[j] == QChar(KS_VES_TAB)) {
					cur.type = mass_real;
				} else if (line[j] == QChar(KS_LOG_TAB)) {
					cur.type = mass_bool;
				} else if (line[j] == QChar(KS_SIM_TAB)) {
					cur.type = mass_charect;
				} else if (line[j] == QChar(KS_LIT_TAB)) {
					cur.type = mass_string;
				} else if (deep == 0) {
					cur.name += line[j];
				}
			}
		}
		if (!cur.decl.isEmpty()) {
			group.vars << cur;
		}
		for (int j = 0; j < group.vars.size(); j++) {
			group.vars[j].decl = group.vars[j].decl.simplified();
			group.vars[j].name = group.vars[j].name.simplified();
			int firstCode = group.vars[j].decl[0].unicode();
			if (firstCode < KS_CEL || firstCode > KS_LOG_TAB) {
				QString first = typeToKsymbol(group.vars[j].type);
				group.vars[j].decl = first + " " + group.vars[j].decl;
			}
		}
		groups << group;
	}
	return groups;
}

proga generateAlgHeader()
{
	proga p;
	p.line = QString(QChar(KS_ALG)) + " @hidden";
	p.error = 0;
	p.str_type = Kumir::AlgDecl;
	p.else_pos = 1;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

proga generateAlgBegin()
{
	proga p;
	p.line = QString(QChar(KS_NACH));
	p.error = 0;
	p.str_type = Kumir::AlgBegin;
	p.else_pos = 2;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

proga generateOutput(const QStringList &terms)
{
	proga p;
	p.str_type = Kumir::Output;
	p.VirajList.clear();
	p.VirajTypes.clear();
	p.line = QString(QChar(KS_VIVOD)) + " " + terms.join(",");
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

proga generateVariableDeclaration(const QString &decl)
{
	proga p;
	p.str_type = Kumir::VarDecl;
	p.VirajList.clear();
	p.VirajTypes.clear();
	p.line = decl;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

proga generateInput(const QString &inp)
{
	proga p;
	p.str_type = Kumir::Input;
	p.VirajList.clear();
	p.VirajTypes.clear();
	p.line = QString(QChar(KS_VVOD)) + " " + inp;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

#define IS_TABLE_TYPE(t) (t==mass_bool || t==mass_charect || t==mass_integer || t==mass_real || t==mass_string)

typedef QPair<QString, QString> StringPair;



QList<StringPair> extractTableBounds(const QString &decl)
{
	int op = decl.indexOf("[");
	int cl = decl.indexOf("]");
	Q_ASSERT(op > -1);
	Q_ASSERT(cl > -1);
	const QString boundsDecl = decl.mid(op + 1, cl - op - 1);
	const QStringList bounds = boundsDecl.split(",");
	QList<StringPair> result;
	for (int i = 0; i < bounds.size(); i++) {
		const QStringList a = bounds[i].split(":");
		Q_ASSERT(a.size() == 2);
		StringPair p;
		p.first = a[0].simplified();
		p.second = a[1].simplified();
		result.prepend(p);
	}
	return result;
}

proga generateLoopBegin(const QString &var, const StringPair &bounds, int elsePos)
{
	proga p;
	p.line = QString("%1 %2 %3 %4 %5 %6 %7")
		.arg(QChar(KS_NC))
		.arg(QChar(KS_DLYA))
		.arg(var)
		.arg(QChar(KS_OT))
		.arg(bounds.first)
		.arg(QChar(KS_DO))
		.arg(bounds.second);
	p.error = 0;
	p.str_type = Kumir::LoopBegin;
	p.else_pos = elsePos;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

proga generateLoopEnd(int elsePos)
{
	proga p;
	p.line = QString(QChar(KS_KC));
	p.error = 0;
	p.str_type = Kumir::LoopEnd;
	p.else_pos = elsePos;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

static proga generateAlgorhitmCall(
	const QString &callName,
	const QList<KumVariableGroup> &args,
	const PeremType &callReturnType
) {
	proga p;
	QStringList arguments;
	for (int i = 0; i < args.size(); i++) {
		KumVariableGroup group = args[i];
		for (int j = 0; j < group.vars.size(); j++) {
			KumVariable var = group.vars[j];
			arguments << var.name;
		}
	}
	if (callReturnType == none) {
		p.error = 0;
		p.else_pos = 0;
		p.real_line.tab = NULL;
		p.real_line.lineNo = -1;
		p.line = QString(QChar(KS_KC));
		p.str_type = Kumir::CallAlg;
		p.VirajList.clear();
		p.VirajTypes.clear();
		p.line = callName + "(" + arguments.join(",") + ")";
	} else {
		QStringList terms;
		terms << "\"" + screenString(QObject::tr("Return value = ")) + "\"";
		terms << callName + "(" + arguments.join(",") + ")";
		p = generateOutput(terms);
	}
	return p;
}



static proga generateAlgorhitmEnd()
{
	proga p;
	p.line = QString(QChar(KS_KON));
	p.error = 0;
	p.str_type = Kumir::AlgEnd;
//    p.else_pos = elsePos;
	p.else_pos = -1;
	p.real_line.tab = NULL;
	p.real_line.lineNo = -1;
	return p;
}

void text_analiz::generateEntryPoint(const QStringList &argGroups,
	const QList<ArgModifer> &argMods,
	symbol_table *symbols,
	function_table *functions,
	const QString &callName,
	const PeremType &callReturnType,
	RealLine realLine)
{
	KumTools::instance()->setAllowHiddenNames(true);
	QRegExp rxSymbolsToRemove("[\\x1000-\\x119D]");
	Q_ASSERT(rxSymbolsToRemove.isValid());
	m_hiddenAlgorhitm.clear();
	bool temp = m_denyVariableDeclaration;
	m_denyVariableDeclaration = false;
	proga cpv;
	cpv.real_line = realLine;

	QList<KumVariableGroup> groups = parceGroups(argGroups, argMods);


	int id = functions->appendDummy("@hidden");
	Q_UNUSED(id);

	m_hiddenAlgorhitm << generateAlgHeader();
	m_hiddenAlgorhitm << generateAlgBegin();

	proga p;
	p = generateVariableDeclaration(QString(QChar(KS_CEL)) + " !dim1");
	K_ASSERT(parceVariablesDeclaration(p, -1, "", symbols, functions) == 0);
	m_hiddenAlgorhitm << p;
	p = generateVariableDeclaration(QString(QChar(KS_CEL)) + " !dim2");
	K_ASSERT(parceVariablesDeclaration(p, -1, "", symbols, functions) == 0);
	m_hiddenAlgorhitm << p;
	p = generateVariableDeclaration(QString(QChar(KS_CEL)) + " !dim3");
	K_ASSERT(parceVariablesDeclaration(p, -1, "", symbols, functions) == 0);
	m_hiddenAlgorhitm << p;

	int error = 0;

	for (int i = 0; i < groups.count(); i++) {
		for (int j = 0; j < groups[i].vars.count(); j++) {
			KumVariable var = groups[i].vars[j];
			p = generateVariableDeclaration(var.decl);
			error = parceVariablesDeclaration(p, -1, "", symbols, functions);
			K_ASSERT(error == 0);
			m_hiddenAlgorhitm << p;
			if (groups[i].mod == arg || groups[i].mod == argres) {
				if (IS_TABLE_TYPE(var.type)) {
					QStack<int> loopStart;
					const QList<StringPair> bounds = extractTableBounds(var.decl);
					K_ASSERT(bounds.size() == var.dimension);
					if (var.dimension == 3) {
//                        qDebug() << "loop for !dim3 from "  << bounds[2].first << " to " << bounds[2].second;
						p = generateLoopBegin("!dim3", bounds[2], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					if (var.dimension >= 2) {
//                        qDebug() << "loop for !dim2 from "  << bounds[1].first << " to " << bounds[1].second;
						p = generateLoopBegin("!dim2", bounds[1], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					if (var.dimension >= 1) {
//                        qDebug() << "loop for !dim1 from "  << bounds[0].first << " to " << bounds[0].second;
						p = generateLoopBegin("!dim1", bounds[0], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					QStringList pleaseEnter;
					pleaseEnter << "\"" + screenString(QObject::tr("Please enter ") + var.name + "[") + "\"";
					QString inp = var.name + "[";
					if (var.dimension == 3) {
						pleaseEnter << "!dim3" << "\"" + screenString(",") + "\"";
						inp += "!dim3,";
					}
					if (var.dimension >= 2) {
						pleaseEnter << "!dim2" << "\"" + screenString(",") + "\"";
						inp += "!dim2,";
					}
					if (var.dimension >= 1) {
						pleaseEnter << "!dim1" << "\"" + screenString("] : ") + "\"";
						inp += "!dim1]";
					}
					p = generateOutput(pleaseEnter);
					error = parceOutput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
					p = generateInput(inp);
					error = parceInput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
					for (int k = 0; k < var.dimension; k++) {
						int start = loopStart.pop();
						p = generateLoopEnd(start);
						m_hiddenAlgorhitm[start].else_pos = m_hiddenAlgorhitm.size();
						m_hiddenAlgorhitm << p;
					}
				} else {
					p = generateOutput(QStringList() << "\"" + screenString(QObject::tr("Please enter ") + var.name + ": ") + "\"");
					error = parceOutput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
					p = generateInput(var.name);
					error = parceInput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
				}
			}
		}
	}

	p = generateAlgorhitmCall(callName, groups, callReturnType);
	if (callReturnType == none) {
		error = parceAlgorhitmCall(p, functions, symbols, -1);
		K_ASSERT(error == 0);
	} else {
		error = parceOutput(p, functions, symbols, -1);
		K_ASSERT(error == 0);
	}
	m_hiddenAlgorhitm << p;

	for (int i = 0; i < groups.count(); i++) {
		for (int j = 0; j < groups[i].vars.count(); j++) {
			KumVariable var = groups[i].vars[j];
			if (groups[i].mod == res || groups[i].mod == argres) {
				if (IS_TABLE_TYPE(var.type)) {
					QStack<int> loopStart;
					const QList<StringPair> bounds = extractTableBounds(var.decl);
					K_ASSERT(bounds.size() == var.dimension);
					if (var.dimension == 3) {
						p = generateLoopBegin("!dim3", bounds[2], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					if (var.dimension >= 2) {
						p = generateLoopBegin("!dim2", bounds[1], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					if (var.dimension >= 1) {
						p = generateLoopBegin("!dim1", bounds[0], 0);
						error = parceLoopBegin(p, symbols, functions, -1);
						K_ASSERT(error == 0);
						loopStart.push(m_hiddenAlgorhitm.size());
						m_hiddenAlgorhitm << p;
					}
					QStringList terms;
					QString lexem = var.name + "[";
					terms << "\"" + screenString(var.name + "[") + "\"";
					if (var.dimension == 3) {
						terms << "!dim3";
						lexem += "!dim3,";
						terms << "\"" + screenString(",") + "\"";
					}
					if (var.dimension >= 2) {
						terms << "!dim2";
						lexem += "!dim2,";
						terms << "\"" + screenString(",") + "\"";
					}
					if (var.dimension >= 1) {
						terms << "!dim1";
						lexem += "!dim1]";
						terms << "\"" + screenString("] = ") + "\"";
					}
					terms << lexem;
					terms << QString(QChar(KS_NS));
					p = generateOutput(terms);
					error = parceOutput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
					for (int k = 0; k < var.dimension; k++) {
						int start = loopStart.pop();
						p = generateLoopEnd(start);
						m_hiddenAlgorhitm[start].else_pos = m_hiddenAlgorhitm.size();
						m_hiddenAlgorhitm << p;
					}
				} else {
					QStringList terms;
					terms << "\"" + screenString(var.name + " = ") + "\"";
					terms << var.name;
					terms << QString(QChar(KS_NS));
					p = generateOutput(terms);
					error = parceOutput(p, functions, symbols, -1);
					K_ASSERT(error == 0);
					m_hiddenAlgorhitm << p;
				}
			}
		}
	}

	p = generateAlgorhitmEnd();
	m_hiddenAlgorhitm << p;

	m_denyVariableDeclaration = temp;
	KumTools::instance()->setAllowHiddenNames(false);

}

void text_analiz::generateEntryPoint2(const QStringList &argGroups,
	const QList<ArgModifer> &argMods,
	symbol_table *symbols,
	function_table *functions,
	const QString &callName,
	const PeremType &callReturnType,
	RealLine realLine)
{
	KumTools::instance()->setAllowHiddenNames(true);
	QRegExp rxSymbolsToRemove("[\\x1000-\\x119D]");
	K_ASSERT(rxSymbolsToRemove.isValid());
	m_hiddenAlgorhitm.clear();
	proga cpv;
	cpv.real_line = realLine;
	QStringList declarations;
	QStringList input;
	QStringList inputTypes;
	QList<int> inpDeclNo;
	QList<int> outDeclNo;
	QVector<bool> declared;
	QStringList output;
	QStringList outputTypes;
	QString varName;
	QStringList vars;

	QStringList arguments;
	QString currentType;
	QChar ct;

	firstAlgorhitmText = QString::fromUtf8("алг\nнач\n");
	K_ASSERT(argGroups.count() == argMods.count());
	for (int i = 0; i < argGroups.count(); i++) {
		varName = argGroups[i];
		ct = varName.trimmed()[0];
		if (ct == QChar(KS_CEL)) {
			currentType = QString::fromUtf8("цел");
		} else if (ct == QChar(KS_VES)) {
			currentType = QString::fromUtf8("вещ");
		} else if (ct == QChar(KS_LOG)) {
			currentType = QString::fromUtf8("лог");
		} else if (ct == QChar(KS_SIM)) {
			currentType = QString::fromUtf8("сим");
		} else if (ct == QChar(KS_LIT)) {
			currentType = QString::fromUtf8("лит");
		} else if (ct == QChar(KS_CEL_TAB)) {
			currentType = QString::fromUtf8("целтаб");
		} else if (ct == QChar(KS_VES_TAB)) {
			currentType = QString::fromUtf8("вещтаб");
		} else if (ct == QChar(KS_LOG_TAB)) {
			currentType = QString::fromUtf8("логтаб");
		} else if (ct == QChar(KS_SIM_TAB)) {
			currentType = QString::fromUtf8("симтаб");
		} else if (ct == QChar(KS_LIT_TAB)) {
			currentType = QString::fromUtf8("литтаб");
		}
		if (argMods[i] == arg || argMods[i] == argres) {
			varName.remove(rxSymbolsToRemove);
			vars = varName.split(",");
			foreach (const QString &var, vars) {
				input << var.simplified();
				firstAlgorhitmText += currentType + " " + var.simplified() + " = ...\n";
				arguments << var.simplified();
				inpDeclNo << i;
			}
		}
		if (argMods[i] == res || argMods[i] == argres) {
			varName = argGroups[i];
			varName.remove(rxSymbolsToRemove);
			vars = varName.split(",");
			foreach (const QString &var, vars) {
				output << var.simplified();
				if (!input.contains(var.simplified())) {
					firstAlgorhitmText += currentType + " " + var.simplified() + "\n";
				}
				arguments << var.simplified();
				outDeclNo << i;
			}
		}
		declarations << argGroups[i];
	}

	if (callReturnType != none) {
		if (callReturnType == integer) {
			firstAlgorhitmText += QString::fromUtf8("цел ");
		} else if (callReturnType == real) {
			firstAlgorhitmText += QString::fromUtf8("вещ ");
		} else if (callReturnType == kumBoolean) {
			firstAlgorhitmText += QString::fromUtf8("лог ");
		} else if (callReturnType == charect) {
			firstAlgorhitmText += QString::fromUtf8("сим ");
		} else if (callReturnType == kumString) {
			firstAlgorhitmText += QString::fromUtf8("лит ");
		}
		firstAlgorhitmText += QString::fromUtf8("результат_") + callName + "\n";
		firstAlgorhitmText += QString::fromUtf8("результат_") + callName + ":=" + callName;
	} else {
		firstAlgorhitmText += callName;
	}
	firstAlgorhitmText += "(" + arguments.join(", ") + ")\n";

	declared = QVector<bool>(declarations.count(), false);

	// Заносим алгоритм в таблицу функций

	int id = functions->appendDummy("@teacher");
	Q_UNUSED(id);

	// Формируем алг-нач
	cpv.line = QString(QChar(KS_ALG)) + " ";
	cpv.error = 0;
	cpv.str_type = Kumir::AlgDecl; // A_IMPL;
	cpv.else_pos = 1;
	m_hiddenAlgorhitm.append(cpv);
	cpv.real_line.tab = 0;
	cpv.real_line.lineNo = -1;
	cpv.line = QString(QChar(KS_NACH));
	cpv.str_type = Kumir::AlgBegin;  // A_OTKR;
	cpv.else_pos = 2;
	m_hiddenAlgorhitm.append(cpv);

	int error = 0;

	int declNo = -1;
	QString inp;
	QString decl;

	// Формируем строки "ввод"
	for (int i = 0; i < input.count(); i++) {
		inp = input[i];
		declNo = inpDeclNo[i];
		cpv.str_type = Kumir::Output; // VIVOD;
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		cpv.line = QString(QChar(KS_VIVOD)) + " \"" + screenString(QObject::tr("Please enter ") + inp + ": ") + "\"";
		error = parceOutput(cpv, functions, symbols, -1);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
		cpv.str_type =  Kumir::VarDecl; // P_VAR_IMPL;
		cpv.line = declarations[declNo];
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		error = parceVariablesDeclaration(cpv, -1, "", symbols, functions);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
		declared[declNo] = true;
		cpv.str_type = Kumir::Input; // VVOD;
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		cpv.line = QString(QChar(KS_VVOD)) + " " + inp;
		error = parceInput(cpv, functions, symbols, -1);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
	}

	bool notDecl;

	// Формируем строки объявления переменных
	for (int i = 0; i < declarations.count(); i++) {
		notDecl = ! declared[i];
		if (notDecl) {
			decl = declarations[i];
			cpv.str_type = Kumir::VarDecl; // P_VAR_IMPL;
			cpv.line = decl;
			cpv.VirajList.clear();
			cpv.VirajTypes.clear();
			error = parceVariablesDeclaration(cpv, -1, "", symbols, functions);
			K_ASSERT(error == 0);
			m_hiddenAlgorhitm.append(cpv);
		}
	}

	// Формируем вызов алгоритма
	if (callReturnType == none) {
		cpv.str_type = Kumir::CallAlg; // ALG_USE;
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		cpv.line = callName + "(" + arguments.join(",") + ")";
		error = parceAlgorhitmCall(cpv, functions, symbols, -1);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
	} else {
		cpv.str_type = Kumir::Output; // VIVOD;
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		cpv.line = QString(QChar(KS_VIVOD)) + " \"" + screenString(QObject::tr("Return value = ")) + "\"," + callName + "(" + arguments.join(",") + ")," + QString(QChar(KS_NS));
		error = parceOutput(cpv, functions, symbols, -1);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
	}

	// Формируем вывод результатов
	foreach (const QString &out, output) {
		cpv.str_type = Kumir::Input; // VIVOD;
		cpv.VirajList.clear();
		cpv.VirajTypes.clear();
		cpv.line = QString(QChar(KS_VIVOD)) + " \"" + screenString(out + " = ") + "\"," + out + "," + QString(QChar(KS_NS));
		error = parceOutput(cpv, functions, symbols, -1);
		K_ASSERT(error == 0);
		m_hiddenAlgorhitm.append(cpv);
	}

	// Формируем конец алгоритма
	cpv.str_type = Kumir::AlgEnd; // A_ZAKR;
	cpv.VirajList.clear();
	cpv.VirajTypes.clear();
	cpv.line = QString(QChar(KS_KON));
	cpv.else_pos = -1;
	m_hiddenAlgorhitm.append(cpv);
	firstAlgorhitmText += QString::fromUtf8("кон\n");
	KumTools::instance()->setAllowHiddenNames(false);
//    qDebug() << "First Algorhitm text: " << firstAlgorhitmText;
}


/**
 * Проверяет, находится ли позиция pos внутри литеральной константы
 * @param s исходная ненормализованная строка
 * @param pos позиция
 * @return true, если внутри литеральной константы или false, если нет
 */

int text_analiz::parce_doc(QString instr, function_table *functions, symbol_table *symbols, int str)
{
	Q_UNUSED(symbols);
	QString algName = functions->nameByPos(str);
	int id = functions->idByName(algName);
	int pos;

	// Заменяем синонимы тегов
	instr.replace(QObject::trUtf8("<прм>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QObject::trUtf8("<ARG>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("<ПРМ>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</прм>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</ПРМ>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</ARG>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\прм{"), "\\arg{", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\ARG{"), "\\arg{", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\ПРМ{"), "\\arg{", Qt::CaseInsensitive);

	instr.replace(QString::fromUtf8("<прм>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("<ARG>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("<ПРМ>"), "<arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</прм>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</ПРМ>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</ARG>"), "</arg>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\прм{"), "\\arg{", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\ARG{"), "\\arg{", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\ПРМ{"), "\\arg{", Qt::CaseInsensitive);

	instr.replace(QString::fromUtf8("<ж>"), "<b>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</ж>"), "</b>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("<к>"), "<i>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("</к>"), "</i>", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\ж{"), "\\bf{", Qt::CaseInsensitive);
	instr.replace(QString::fromUtf8("\\к{"), "\\it{", Qt::CaseInsensitive);

	// Заменяем ТеХ-теги на HTML-теги
	QRegExp rxAlgTex = QRegExp("\\\\arg\\{(.*)\\}", Qt::CaseInsensitive);
	rxAlgTex.setMinimal(true);
	while ((pos = rxAlgTex.indexIn(instr)) != -1) {
		int len = rxAlgTex.matchedLength();
		QString param = rxAlgTex.cap(1);
		instr.replace(pos, len, "<arg>" + param + "</arg>");
	}
	QRegExp rxBoldTex = QRegExp("\\\\textbf\\{(.*)\\}|\\\\bf\\{(.*)\\}", Qt::CaseInsensitive);
	rxBoldTex.setMinimal(true);
	while ((pos = rxBoldTex.indexIn(instr)) != -1) {
		int len = rxBoldTex.matchedLength();
		QString param = rxBoldTex.capturedTexts().last();
		if (param.isEmpty()) {
			param = rxBoldTex.cap(1);
		}
		instr.replace(pos, len, "<b>" + param + "</b>");
	}
	QRegExp rxItalicTex = QRegExp("\\\\textit\\{(.*)\\}|\\\\it\\{(.*)\\}", Qt::CaseInsensitive);
	rxItalicTex.setMinimal(true);
	while ((pos = rxItalicTex.indexIn(instr)) != -1) {
		int len = rxItalicTex.matchedLength();
		QString param = rxItalicTex.capturedTexts().last();
		if (param.isEmpty()) {
			param = rxItalicTex.cap(1);
		}
		instr.replace(pos, len, "<i>" + param + "</i>");
	}

	// Убиваем недопустимые теги
	QRegExp rxHTMLTag = QRegExp("</?(.*)>", Qt::CaseInsensitive);
	rxHTMLTag.setMinimal(true);
	pos = 0;
	while ((pos = rxHTMLTag.indexIn(instr, pos)) != -1) {
		int len = rxHTMLTag.matchedLength();
		QStringList caps = rxHTMLTag.capturedTexts();
		QString value;
		if (caps.count() > 1) {
			value = caps[1];
		}
		if (value.trimmed().toLower() != "b")
			if (value.trimmed().toLower() != "i")
				if (value.trimmed().toLower() != "arg")
					if (value.trimmed().toLower() != "br")
						if (value.trimmed().toLower() != "/b")
							if (value.trimmed().toLower() != "/i")
								if (value.trimmed().toLower() != "/arg") {
									instr.replace(pos, len, "&laquo;" + value + "&raquo;");
								}
		pos += len;

	}

	// Записываем строку документации
	if (id != -1) {
		instr.remove(QChar(KS_DOC));
		instr = instr.trimmed();
		instr = instr.simplified();
		QString docString = functions->docString(id);
		if (instr.isEmpty()) {
			docString += "<br>";
		} else {
			if (!docString.isEmpty()) {
				docString += " ";
			}
			docString += instr;
		}
		functions->setDocString(id, docString);
		return 0;
	} else {
		return TOS_OUT_OF_ALG;
	}
}
int text_analiz::parceFuncList(int module_id, QStringList funcList)
{
	//qDebug()<<"Modules"<<modules;

	moduleId = module_id;

	KumSingleModule *module = modules->module(module_id);
	function_table *functions = module->Functions();
	symbol_table *symbols = module->Values();
	QString normaString;
	int err, err_start, err_end;
	err = 0;
	// qDebug()<<"FuncList:"<<funcList;
	for (int i = 0; i < funcList.count(); i++) {
		normaString = KumTools::instance()->normalizeString(funcList[i], &err, &err_start, &err_end);
		qDebug() << "Norma string:" << normaString;
		proga pv;
		pv.line = normaString;
		pv.real_line.lineNo = -1;
		pv.real_line.tab = NULL;
		Kumir::InstructionType sttype = KumTools::instance()->getStringType(normaString);
		pv.str_type = sttype;
		if (err != 0) {
			qDebug() << "text_analiz::parceFuncList Error:" << err;
			return -1;
		};
		if (pv.str_type == Kumir::AlgDecl) {
			err = parceAlgorhitmHeader(pv, i, symbols, functions);
		}
	}
	if (err == 0) {
		return functions->size();
	} else {
		return -1;
	}
};


void text_analiz::buildTablesForModule(KumModules *modules, int module_id, QString fileName)
{
	m_hiddenAlgorhitm.clear();
	this->modules = modules;
	moduleId = module_id;

	KumSingleModule *module = modules->module(module_id);
	Q_CHECK_PTR(module);
	function_table *functions = module->Functions();
	Q_CHECK_PTR(functions);
	symbol_table *symbols = module->Values();
	Q_CHECK_PTR(symbols);
	QString moduleName;

	for (int i = 0; i < module->Proga_Value()->count(); i++) {

		Err_start = 0;

		proga pv = module->Proga_Value()->at(i);

		Err_length = pv.line.length();

		int error = 0;

		if (pv.str_type == Kumir::ModuleBegin && pv.error == 0) {
			error = parce_isp(
					pv.line,
					functions,
					symbols,
					i,
					&Err_start,
					&Err_length,
					"",
					&moduleName);
			if (error) {
				pv.error = error;
				pv.err_start = Err_start;
				pv.err_length = Err_length;
			} else {
				module->setModuleName(moduleName);
			}
		}

		if (pv.str_type == Kumir::AlgDecl) {
			error = parceAlgorhitmHeader(pv, i, symbols, functions);
			if (error > 0 && pv.error == 0) {
				pv.error = error;
				pv.err_start = Err_start;
				pv.err_length = Err_length;
			}
			module->setProgaValueAt(i, pv);
		}
		if (error) {
			module->setBroken(true);
		}
	}
	Q_UNUSED(fileName);
}

enum FI_closes { IF_THEN, IF_ELSE, SWITCH, SWITCH_CASE, SWITCH_ELSE };

void text_analiz::analizeModule(int module_id, QString fileName)
{

	m_denyVariableDeclaration = 0;
	Q_UNUSED(fileName);
	KumSingleModule *module = modules->module(module_id);
	int f_id = -1;


	function_table *functions = module->Functions();
	symbol_table *symbols = module->Values();

	QStack<FI_closes> fi_closes;
	for (int i = 0; i < module->Proga_Value()->count(); i++) {

		Err_start = 0;
		proga pv = module->Proga_Value()->at(i);
		pv.extraSubExpressions.clear();
		Err_length = pv.line.length();
		int error = 0;

		switch (pv.str_type) {

		case Kumir::UseModule: {
			error = ISP_USE_WRONG_POS;
			break;
		}

		case Kumir::AlgBegin: {
			error = parceAlgorhitmBegin(pv.line);
			break;
		}

		case Kumir::AlgEnd: {
			error = parce_kon(pv.line);
			break;
		}

		case Kumir::LoopBegin: {
			error = parceLoopBegin(pv, symbols, functions, i);
			m_denyVariableDeclaration ++;
			break;
		}

		case Kumir::LoopEnd: {
			m_denyVariableDeclaration --;
			error = parceLoopEnd(pv.line);
			break;
		}

		case Kumir::Else: {
			if (!fi_closes.isEmpty()) {
				FI_closes l = fi_closes.pop();
				if (l == IF_THEN) {
					fi_closes.push(IF_ELSE);
				}
				if (l == SWITCH_CASE || l == SWITCH) {
					fi_closes.push(SWITCH_ELSE);
				}
			}
			break;
		}

		case Kumir::Exit: {
			error = parce_exit(pv.line);
			break;
		}

		case Kumir::If: {
			fi_closes.push(IF_THEN);
			error = parce_if(pv, symbols, functions, i);
			break;
		}

		case Kumir::Switch: {
			fi_closes.push(SWITCH);
			m_denyVariableDeclaration ++;
			error = parce_vibor(pv.line, functions, symbols, i);
			break;
		}

		case Kumir::Case: {
			if (!fi_closes.isEmpty()) {
//                    FI_closes l = fi_closes.pop();
				fi_closes.push(SWITCH_CASE);
			}
			error = parce_pri(pv, functions, symbols, i);
			break;
		}

		case Kumir::Then: {
			m_denyVariableDeclaration ++;
			error = parce_to(pv.line);
			break;
		}

		case Kumir::Fin: {
			error = parce_vse(pv.line);
//                if (!fi_closes.isEmpty()) {
//                    FI_closes l = fi_closes.pop();
//                }
			m_denyVariableDeclaration --;
			break;
		}

		case Kumir::Assign: {
			error = parceAssignment(pv, functions, symbols, i);
			break;
		}

		//        case PRISV_ERR: {
		//            error = ANALIZ_MANY_PRISV;
		//            break;
		//        }

		case Kumir::Output: {
			error = parceOutput(pv, functions, symbols, i);
			break;
		}

		case Kumir::FileOutput: {
			error = parceFileOutput(pv, functions, symbols, i);
			break;
		}

		case Kumir::Assert: {
			error = parce_utv(pv, functions, symbols, i);
			break;
		}

		case Kumir::LoopEndCond: {
			error = parce_kc_pri(pv, functions, symbols, i);
			m_denyVariableDeclaration --;
			break;
		}

		case Kumir::Pre: {
			error = parce_dano(pv, functions, symbols, i);
			break;
		}

		case Kumir::Post: {
			error = parce_nado(pv, functions, symbols, i);
			break;
		}

		case Kumir::Input: {
			error = parceInput(pv, functions, symbols, i);
			break;
		}

		case Kumir::FileInput: {
			error = parceFileInput(pv, functions, symbols, i);
			break;
		}

		case Kumir::Doc: {
			error = parce_doc(pv.line, functions, symbols, i);
			break;
		}

		case Kumir::CallAlg: {
			error = parceAlgorhitmCall(pv, functions, symbols, i);
			break;
		}

		case Kumir::ModuleEnd: {
			error = parce_endisp(pv.line);
			break;
		}

		case Kumir::VarDecl: {
			error = parceVariablesDeclaration(pv, i, functions->nameByPos(i), symbols, functions);
			break;
		}

		default: {
			break;
		}

		} // end switch

		if (error > 0 && pv.error == 0) {
			pv.error = error;
			pv.err_start = Err_start;
			pv.err_length = Err_length;
		}
		if (error == 0) {
			f_id = functions->idByPos(i);
		}
		module->setProgaValueAt(i, pv);
		if (error) {
			module->setBroken(true);
		}

	} // end for ( int i=0; i<module->Proga_Value()->count(); i++ )
	if (!m_hiddenAlgorhitm.isEmpty()) {
		int hiddenIP = module->bytecodeSize();
		for (int i = 0; i < m_hiddenAlgorhitm.count(); i++) {
			if (m_hiddenAlgorhitm[i].else_pos >= 0) {
				m_hiddenAlgorhitm[i].else_pos += hiddenIP;
			}
//            qDebug() << "Instr: " << hiddenIP+i << " Type: " << m_hiddenAlgorhitm[i].str_type << " ElsePos: " << m_hiddenAlgorhitm[i].else_pos << " Line: " << m_hiddenAlgorhitm[i].line;
		}
		proga p = module->Proga_Value()->at(m_firstAlgorhitmIP);
		//p.forceJump = hiddenIP;
		//module->setProgaValueAt(m_firstAlgorhitmIP, p);
		module->appendBytecode(m_hiddenAlgorhitm);
		module->setInitialIP(hiddenIP);
	} else {
		//        module->setInitialIP(app()->compiler->usesListCount());
		module->setInitialIP(app()->compiler->usesListCount());
	}

}
