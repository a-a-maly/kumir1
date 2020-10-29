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



/* Kum_Instrument
Класс встроенных функций кумир



*/

#include "tools.h"
#include "pult_webkit.h"

#include "kum_instrument.h"
#include "kumsinglemodule.h"
//#include "kum_draw.cpp"
#include <math.h>
#ifdef Q_CC_SUN
#define isinf(x) (x>=MAXFLOAT || x<=(-MAXFLOAT))
#endif
#ifdef Q_CC_MSVC
#include <float.h>
#include <xmath.h>
#define isnan(x) _isnan(x)
#define isinf(x) (x==INF)
#endif
#include <QGraphicsItem>
#include <QFileDialog>
#include <QFile>
#include "application.h"
#include "compiler.h"
#include "error.h"

#include "integeroverflowchecker.h"
#include "doubleoverflowchecker.h"
#define EPSILON 0.0001
#define STYLESHEET "QMainWindow {background-color: #808080} QMenuBar {background-color: #808080}  QMenuBar::item {background-color: #808080} QMenuBar::item:selected {background-color: #C0C000}"
#define BACKGROUND_COLOR "lightgreen"

#include "run.h"

#include "secondarywindow.h"


int KumInstrument::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	Q_UNUSED(Functions);
	Q_UNUSED(Symbols);
	return 0;
}


KumStandartFunctions::KumStandartFunctions(KumModules *modules, QWidget *parent)
	: KumInstrument(parent)
{
	codec = QTextCodec::codecForName("KOI8-R");
	m_modules = modules;
};





/**
 * Добавляет в таблицы функции и переменные,устанавливает таблицы класса
 *  function_table и symbol_table
 * @param Functions Таблица функций
 * @param Symbols Таблица переменных
 * @return количество функций
 */
int KumStandartFunctions::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	symbols = Symbols;
	functions = Functions;
	int count = 0;
	Symbols->add_symbol("div", -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction("div", integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	count++;


	Symbols->add_symbol("mod", -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction("mod", integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	count++;


	//

	Symbols->add_symbol(trUtf8("юникод"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("юникод"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, charect);
	count++;


	Symbols->add_symbol(trUtf8("код"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("код"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, charect);
	count++;


	Symbols->add_symbol(trUtf8("символ"), -1, charect, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("символ"), charect, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;
	//

	Symbols->add_symbol(trUtf8("символ2"), -1, charect, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("символ2"), charect, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;


	Symbols->add_symbol(trUtf8("вещ_в_лит"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("вещ_в_лит"), kumString, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;


	Symbols->add_symbol(trUtf8("цел_в_лит"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("цел_в_лит"), kumString, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;



	Symbols->add_symbol(QString::fromLatin1("sqrt"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("sqrt"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;


	Symbols->add_symbol(QString::fromLatin1("abs"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("abs"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;



	Symbols->add_symbol(QString::fromLatin1("sign"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("sign"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;


	Symbols->add_symbol(QString::fromLatin1("min"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("min"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, real);
	Functions->setArgType(Functions->size() - 1, 1, real);
	count++;

	Symbols->add_symbol(QString::fromLatin1("imin"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("imin"), integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	count++;


	Symbols->add_symbol(QString::fromLatin1("max"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("max"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, real);
	Functions->setArgType(Functions->size() - 1, 1, real);
	count++;

	Symbols->add_symbol(QString::fromLatin1("imax"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("imax"), integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	count++;


	Symbols->add_symbol(QString::fromLatin1("rnd"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("rnd"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;

	Symbols->add_symbol(QString::fromLatin1("irnd"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("irnd"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;

	Symbols->add_symbol(QString::fromLatin1("irand"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("irand"), integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	count++;

	Symbols->add_symbol(QString::fromLatin1("rand"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("rand"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, real);
	Functions->setArgType(Functions->size() - 1, 1, real);
	count++;



	Symbols->add_symbol(QString::fromLatin1("sin"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("sin"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("cos"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("cos"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("tg"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("tg"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("ctg"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("ctg"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("arcsin"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("arcsin"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("arccos"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("arccos"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("arctg"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("arctg"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("arcctg"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("arcctg"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("ln"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("ln"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("lg"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("lg"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("exp"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("exp"), real, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(QString::fromLatin1("iabs"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("iabs"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);


	Symbols->add_symbol(QString::fromLatin1("int"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(QString::fromLatin1("int"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, real);


	Symbols->add_symbol(trUtf8("длин"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("длин"), integer, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);


	Symbols->add_symbol(trUtf8("время"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("время"), integer, 0);

	if (!app()->isExamMode()) {
		Symbols->add_symbol(trUtf8("клав"), -1, integer, "global", true);
		Symbols->new_symbol(Symbols->symb_table.size() - 1);
		Functions->addInternalFunction(trUtf8("клав"), integer, 0);
	};

	Symbols->add_symbol(trUtf8("лит_в_цел"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("лит_в_цел"), integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, kumBoolean);
	Functions->setRes(Functions->size() - 1, 1, true);


	Symbols->add_symbol(trUtf8("лит_в_вещ"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("лит_в_вещ"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, kumBoolean);
	Functions->setRes(Functions->size() - 1, 1, true);


	Symbols->add_symbol(trUtf8("МАКСВЕЩ"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("МАКСВЕЩ"), real, 0);

	Symbols->add_symbol(trUtf8("МАКСЦЕЛ"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("МАКСЦЕЛ"), integer, 0);

	Functions->addInternalFunction(trUtf8("delay"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);

	QString fills[] = { "i1", "i2", "i3",
			"d1", "d2", "d3",
			"b1", "b2", "b3",
			"c1", "c2", "c3",
			"s1", "s2", "s3"
		};

	for (int f = 0; f < 15; f++) {
		QString fname = "!memfill_" + fills[f];
		PeremType pt = none;
		if (fills[f].startsWith("i")) {
			pt = mass_integer;
		} else if (fills[f].startsWith("d")) {
			pt = mass_real;
		} else if (fills[f].startsWith("b")) {
			pt = mass_bool;
		} else if (fills[f].startsWith("c")) {
			pt = mass_charect;
		} else if (fills[f].startsWith("s")) {
			pt = mass_string;
		}

		int dim = fills[f].mid(1, 1).toInt();

		Functions->addInternalFunction(fname, none, 2);
		Functions->setArgType(Functions->size() - 1, 0, pt);
		Functions->setArgType(Functions->size() - 1, 1, kumString);
		Functions->setRes(Functions->size() - 1, 0, true);
		Functions->setArgDim(Functions->size() - 1, 0, dim);
	}

	return count;
}


/**
 * Выполнение функций
 * @param name Имя функции
 * @param aruments список аргументов
 */
void KumStandartFunctions::runFunc(QString name, QList<KumValueStackElem> *arg, int *err)
{
	QList<KumValueStackElem> arguments = *arg;
	*err = 0;
	if (name == trUtf8("юникод")) {
		QChar chr = arguments[0].asString()[0];
		int res = chr.unicode();
		int perem_id = symbols->inTable(trUtf8("юникод"), "global");
		symbols->symb_table[perem_id].value.setIntegerValue(res);
	} else


		if (name == trUtf8("код")) {
			//qDebug("Vizov \"kod\":========================");
			QString str = arguments[0].asString();
			if (str.length() != 1) {
				*err = INTFUN_NOT_KOI8_RANGE;
				return;
			}


			if ((! codec->canEncode(str)) && str[0] != '\0') {
				*err = INTFUN_NOT_KOI8_RANGE;
				return;
			};
			QByteArray KOI8string = codec->fromUnicode(str);
			char code;
			if (str.isEmpty()) {
				code = '\0';
			} else {
				code = KOI8string[0];
			}
			//qDebug("code = %i",code);
			int res = code;
			if (res < 0) {
				res += 256;
			}
			//qDebug("result = %i",res);
			int perem_id = symbols->inTable(trUtf8("код"), "global");
			symbols->symb_table[perem_id].value.setIntegerValue(res);
		} else if (name == trUtf8("символ")) {
			int x = arguments[0].asInt();

			if ((x < 0) || (x > 255)) {
				*err = INTFUN_NOT_KOI8_RANGE;
				return;
			};
			QChar simbol;
			switch (x) { //Таблица кодировки


			case 32:
				simbol = ' ';
				break;
			case 33:
				simbol = '!';
				break;
			case 34:
				simbol = '"';
				break;
			case 35:
				simbol = '#';
				break;
			case 36:
				simbol = '$';
				break;
			case 37:
				simbol = '%';
				break;
			case 38:
				simbol = '&';
				break;

			case 39:
				simbol = QChar(0x0027);
				break;
			case 40:
				simbol = QChar(0x0028);
				break;
			case 41:
				simbol = QChar(0x0029);
				break;
			case 42:
				simbol = QChar(0x002A);
				break;
			case 43:
				simbol = QChar(0x002B);
				break;
			case 44:
				simbol = QChar(0x002C);
				break;
			case 45:
				simbol = QChar(0x002D);
				break;
			case 46:
				simbol = QChar(0x002E);
				break;
			case 47:
				simbol = QChar(0x002F);
				break;
			case 48:
				simbol = QChar(0x0030);
				break;
			case 49:
				simbol = QChar(0x0031);
				break;
			case 50:
				simbol = QChar(0x0032);
				break;
			case 51:
				simbol = QChar(0x0033);
				break;
			case 52:
				simbol = QChar(0x0034);
				break;
			case 53:
				simbol = QChar(0x0035);
				break;
			case 54:
				simbol = QChar(0x0036);
				break;
			case 55:
				simbol = QChar(0x0037);
				break;
			case 56:
				simbol = QChar(0x0038);
				break;
			case 57:
				simbol = QChar(0x0039);
				break;
			case 58:
				simbol = QChar(0x003A);
				break;
			case 59:
				simbol = QChar(0x003B);
				break;
			case 60:
				simbol = QChar(0x003C);
				break;
			case 61:
				simbol = QChar(0x003D);
				break;
			case 62:
				simbol = QChar(0x003E);
				break;
			case 63:
				simbol = QChar(0x003F);
				break;
			case 64:
				simbol = QChar(0x0040);
				break;
			case 65:
				simbol = QChar(0x0041);
				break;
			case 66:
				simbol = QChar(0x0042);
				break;
			case 67:
				simbol = QChar(0x0043);
				break;
			case 68:
				simbol = QChar(0x0044);
				break;
			case 69:
				simbol = QChar(0x0045);
				break;
			case 70:
				simbol = QChar(0x0046);
				break;
			case 71:
				simbol = QChar(0x0047);
				break;
			case 72:
				simbol = QChar(0x0048);
				break;
			case 73:
				simbol = QChar(0x0049);
				break;
			case 74:
				simbol = QChar(0x004A);
				break;
			case 75:
				simbol = QChar(0x004B);
				break;
			case 76:
				simbol = QChar(0x004C);
				break;
			case 77:
				simbol = QChar(0x004D);
				break;
			case 78:
				simbol = QChar(0x004E);
				break;
			case 79:
				simbol = QChar(0x004F);
				break;
			case 80:
				simbol = QChar(0x0050);
				break;
			case 81:
				simbol = QChar(0x0051);
				break;
			case 82:
				simbol = QChar(0x0052);
				break;
			case 83:
				simbol = QChar(0x0053);
				break;
			case 84:
				simbol = QChar(0x0054);
				break;
			case 85:
				simbol = QChar(0x0055);
				break;
			case 86:
				simbol = QChar(0x0056);
				break;
			case 87:
				simbol = QChar(0x0057);
				break;
			case 88:
				simbol = QChar(0x0058);
				break;
			case 89:
				simbol = QChar(0x0059);
				break;
			case 90:
				simbol = QChar(0x005A);
				break;
			case 91:
				simbol = QChar(0x005B);
				break;
			case 92:
				simbol = QChar(0x005C);
				break;
			case 93:
				simbol = QChar(0x005D);
				break;
			case 94:
				simbol = QChar(0x005E);
				break;
			case 95:
				simbol = QChar(0x005F);
				break;
			case 96:
				simbol = QChar(0x0060);
				break;
			case 97:
				simbol = QChar(0x0061);
				break;
			case 98:
				simbol = QChar(0x0062);
				break;
			case 99:
				simbol = QChar(0x0063);
				break;
			case 100:
				simbol = QChar(0x0064);
				break;
			case 101:
				simbol = QChar(0x0065);
				break;
			case 102:
				simbol = QChar(0x0066);
				break;
			case 103:
				simbol = QChar(0x0067);
				break;
			case 104:
				simbol = QChar(0x0068);
				break;
			case 105:
				simbol = QChar(0x0069);
				break;
			case 106:
				simbol = QChar(0x006A);
				break;
			case 107:
				simbol = QChar(0x006B);
				break;
			case 108:
				simbol = QChar(0x006C);
				break;
			case 109:
				simbol = QChar(0x006D);
				break;
			case 110:
				simbol = QChar(0x006E);
				break;
			case 111:
				simbol = QChar(0x006F);
				break;
			case 112:
				simbol = QChar(0x0070);
				break;
			case 113:
				simbol = QChar(0x0071);
				break;
			case 114:
				simbol = QChar(0x0072);
				break;
			case 115:
				simbol = QChar(0x0073);
				break;
			case 116:
				simbol = QChar(0x0074);
				break;
			case 117:
				simbol = QChar(0x0075);
				break;
			case 118:
				simbol = QChar(0x0076);
				break;
			case 119:
				simbol = QChar(0x0077);
				break;
			case 120:
				simbol = QChar(0x0078);
				break;
			case 121:
				simbol = QChar(0x0079);
				break;
			case 122:
				simbol = QChar(0x007A);
				break;
			case 123:
				simbol = QChar(0x007B);
				break;
			case 124:
				simbol = QChar(0x007C);
				break;
			case 125:
				simbol = QChar(0x007D);
				break;
			case 126:
				simbol = QChar(0x007E);
				break;
			case 127:
				simbol = QChar(0x007F);
				break;
			case 128:
				simbol = QChar(0x2500);
				break;
			case 129:
				simbol = QChar(0x2502);
				break;
			case 130:
				simbol = QChar(0x250C);
				break;
			case 131:
				simbol = QChar(0x2510);
				break;
			case 132:
				simbol = QChar(0x2514);
				break;
			case 133:
				simbol = QChar(0x2518);
				break;
			case 134:
				simbol = QChar(0x251C);
				break;
			case 135:
				simbol = QChar(0x2524);
				break;
			case 136:
				simbol = QChar(0x252C);
				break;
			case 137:
				simbol = QChar(0x2534);
				break;
			case 138:
				simbol = QChar(0x253C);
				break;
			case 139:
				simbol = QChar(0x2580);
				break;
			case 140:
				simbol = QChar(0x2584);
				break;
			case 141:
				simbol = QChar(0x2588);
				break;
			case 142:
				simbol = QChar(0x258C);
				break;
			case 143:
				simbol = QChar(0x2590);
				break;
			case 144:
				simbol = QChar(0x2591);
				break;
			case 145:
				simbol = QChar(0x2592);
				break;
			case 146:
				simbol = QChar(0x2593);
				break;
			case 147:
				simbol = QChar(0x2320);
				break;
			case 148:
				simbol = QChar(0x25A0);
				break;
			case 149:
				simbol = QChar(0x2219);
				break;
			case 150:
				simbol = QChar(0x221A);
				break;
			case 151:
				simbol = QChar(0x2248);
				break;
			case 152:
				simbol = QChar(0x2264);
				break;
			case 153:
				simbol = QChar(0x2265);
				break;
			case 154:
				simbol = QChar(0x00A0);
				break;
			case 155:
				simbol = QChar(0x2321);
				break;
			case 156:
				simbol = QChar(0x00B0);
				break;
			case 157:
				simbol = QChar(0x00B2);
				break;
			case 158:
				simbol = QChar(0x00B7);
				break;
			case 159:
				simbol = QChar(0x00F7);
				break;
			case 160:
				simbol = QChar(0x2550);
				break;
			case 161:
				simbol = QChar(0x2551);
				break;
			case 162:
				simbol = QChar(0x2552);
				break;
			case 163:
				simbol = QChar(0x0451);
				break;
			case 164:
				simbol = QChar(0x2553);
				break;
			case 165:
				simbol = QChar(0x2554);
				break;
			case 166:
				simbol = QChar(0x2555);
				break;
			case 167:
				simbol = QChar(0x2556);
				break;
			case 168:
				simbol = QChar(0x2557);
				break;
			case 169:
				simbol = QChar(0x2558);
				break;
			case 170:
				simbol = QChar(0x2559);
				break;
			case 171:
				simbol = QChar(0x255A);
				break;
			case 172:
				simbol = QChar(0x255B);
				break;
			case 173:
				simbol = QChar(0x255C);
				break;
			case 174:
				simbol = QChar(0x255D);
				break;
			case 175:
				simbol = QChar(0x255E);
				break;
			case 176:
				simbol = QChar(0x255F);
				break;
			case 177:
				simbol = QChar(0x2560);
				break;
			case 178:
				simbol = QChar(0x2561);
				break;
			case 179:
				simbol = QChar(0x0401);
				break;
			case 180:
				simbol = QChar(0x2562);
				break;
			case 181:
				simbol = QChar(0x2563);
				break;
			case 182:
				simbol = QChar(0x2564);
				break;
			case 183:
				simbol = QChar(0x2565);
				break;
			case 184:
				simbol = QChar(0x2566);
				break;
			case 185:
				simbol = QChar(0x2567);
				break;
			case 186:
				simbol = QChar(0x2568);
				break;
			case 187:
				simbol = QChar(0x2569);
				break;
			case 188:
				simbol = QChar(0x256A);
				break;
			case 189:
				simbol = QChar(0x256B);
				break;
			case 190:
				simbol = QChar(0x256C);
				break;
			case 191:
				simbol = QChar(0x00A9);
				break;
			case 192:
				simbol = QChar(0x044E);
				break;
			case 193:
				simbol = QChar(0x0430);
				break;
			case 194:
				simbol = QChar(0x0431);
				break;
			case 195:
				simbol = QChar(0x0446);
				break;
			case 196:
				simbol = QChar(0x0434);
				break;
			case 197:
				simbol = QChar(0x0435);
				break;
			case 198:
				simbol = QChar(0x0444);
				break;
			case 199:
				simbol = QChar(0x0433);
				break;
			case 200:
				simbol = QChar(0x0445);
				break;
			case 201:
				simbol = QChar(0x0438);
				break;
			case 202:
				simbol = QChar(0x0439);
				break;
			case 203:
				simbol = QChar(0x043A);
				break;
			case 204:
				simbol = QChar(0x043B);
				break;
			case 205:
				simbol = QChar(0x043C);
				break;
			case 206:
				simbol = QChar(0x043D);
				break;
			case 207:
				simbol = QChar(0x043E);
				break;
			case 208:
				simbol = QChar(0x043F);
				break;
			case 209:
				simbol = QChar(0x044F);
				break;
			case 210:
				simbol = QChar(0x0440);
				break;
			case 211:
				simbol = QChar(0x0441);
				break;
			case 212:
				simbol = QChar(0x0442);
				break;
			case 213:
				simbol = QChar(0x0443);
				break;
			case 214:
				simbol = QChar(0x0436);
				break;
			case 215:
				simbol = QChar(0x0432);
				break;
			case 216:
				simbol = QChar(0x044C);
				break;
			case 217:
				simbol = QChar(0x044B);
				break;
			case 218:
				simbol = QChar(0x0437);
				break;
			case 219:
				simbol = QChar(0x0448);
				break;
			case 220:
				simbol = QChar(0x044D);
				break;
			case 221:
				simbol = QChar(0x0449);
				break;
			case 222:
				simbol = QChar(0x0447);
				break;
			case 223:
				simbol = QChar(0x044A);
				break;
			case 224:
				simbol = QChar(0x042E);
				break;
			case 225:
				simbol = QChar(0x0410);
				break;
			case 226:
				simbol = QChar(0x0411);
				break;
			case 227:
				simbol = QChar(0x0426);
				break;
			case 228:
				simbol = QChar(0x0414);
				break;
			case 229:
				simbol = QChar(0x0415);
				break;
			case 230:
				simbol = QChar(0x0424);
				break;
			case 231:
				simbol = QChar(0x0413);
				break;
			case 232:
				simbol = QChar(0x0425);
				break;
			case 233:
				simbol = QChar(0x0418);
				break;
			case 234:
				simbol = QChar(0x0419);
				break;
			case 235:
				simbol = QChar(0x041A);
				break;
			case 236:
				simbol = QChar(0x041B);
				break;
			case 237:
				simbol = QChar(0x041C);
				break;
			case 238:
				simbol = QChar(0x041D);
				break;
			case 239:
				simbol = QChar(0x041E);
				break;
			case 240:
				simbol = QChar(0x041F);
				break;
			case 241:
				simbol = QChar(0x042F);
				break;
			case 242:
				simbol = QChar(0x0420);
				break;
			case 243:
				simbol = QChar(0x0421);
				break;
			case 244:
				simbol = QChar(0x0422);
				break;
			case 245:
				simbol = QChar(0x0423);
				break;
			case 246:
				simbol = QChar(0x0416);
				break;
			case 247:
				simbol = QChar(0x0412);
				break;
			case 248:
				simbol = QChar(0x042C);
				break;
			case 249:
				simbol = QChar(0x042B);
				break;
			case 250:
				simbol = QChar(0x0417);
				break;
			case 251:
				simbol = QChar(0x0428);
				break;
			case 252:
				simbol = QChar(0x042D);
				break;
			case 253:
				simbol = QChar(0x0429);
				break;
			case 254:
				simbol = QChar(0x0427);
				break;
			case 255:
				simbol = QChar(0x042A);
				break;

			};
			if (x < 32) {
				simbol = QChar(x);
			}
			int perem_id = symbols->inTable(trUtf8("символ"), "global");
			symbols->symb_table[perem_id].value.setCharectValue(simbol);
			return;
		}
	if (name == trUtf8("символ2")) {
		int x = arguments[0].asInt();
		QChar simbol = QChar(x);
		if (!KumTools::instance()->isValidChar(simbol)) {
			*err = RUN_SYMBOL_NOT_ALLOWED;
			return;
		}
		int perem_id = symbols->inTable(trUtf8("символ2"), "global");
		symbols->symb_table[perem_id].value.setCharectValue(simbol);
		return;
	} else if (name == "sqrt") {
		//      double test=arguments[0].asFloat();
		int perem_id = symbols->inTable("sqrt", "global");
		if (arguments[0].asFloat() < 0) {
			*err = RUN_ROOT_OF_NEGATIVE;
			return;
		}
		double res = sqrtf(arguments[0].asFloat());
		if (isnan(res) != 0) {
			*err = RUN_NO_VALUE;
			return;
		};
		symbols->symb_table[perem_id].value.setFloatValue(res);
	};

	// NEW V.Y. 05.09.2006

	if (name == trUtf8("модуль")) {
		int x = arguments[0].asInt();
		int perem_id = symbols->inTable(trUtf8("модуль"), "global");
		symbols->symb_table[perem_id].value.setIntegerValue(abs(x));
	} else

		if (name == "abs") {
			double x = arguments[0].asFloat();
			int perem_id = symbols->inTable("abs", "global");
			symbols->symb_table[perem_id].value.setFloatValue(fabs(x));
		} else

			if (name == trUtf8("знак")) {
				int x = arguments[0].asInt();
				int perem_id = symbols->inTable(trUtf8("знак"), "global");
				if (x < 0) {
					symbols->symb_table[perem_id].value.setIntegerValue(-1);
				} else if (x > 0) {
					symbols->symb_table[perem_id].value.setIntegerValue(1);
				} else {
					symbols->symb_table[perem_id].value.setIntegerValue(0);
				}
			} else

				if (name == "sign") {
					double x = arguments[0].asFloat();
					int perem_id = symbols->inTable("sign", "global");
					if (x < -EPSILON) {
						symbols->symb_table[perem_id].value.setIntegerValue(-1);
					} else if (x > EPSILON) {
						symbols->symb_table[perem_id].value.setIntegerValue(1);
					} else {
						symbols->symb_table[perem_id].value.setIntegerValue(0);
					}
				} else

					if (name == trUtf8("imin")) {
						int x = arguments[0].asInt();
						int y = arguments[1].asInt();
						int perem_id = symbols->inTable(trUtf8("imin"), "global");
						if (x > y) {
							symbols->symb_table[perem_id].value.setIntegerValue(y);
						} else {
							symbols->symb_table[perem_id].value.setIntegerValue(x);
						}
					} else

						if (name == trUtf8("imax")) {
							int x = arguments[0].asInt();
							int y = arguments[1].asInt();
							int perem_id = symbols->inTable(trUtf8("imax"), "global");
							if (x < y) {
								symbols->symb_table[perem_id].value.setIntegerValue(y);
							} else {
								symbols->symb_table[perem_id].value.setIntegerValue(x);
							}
						} else

							if (name == "min") {
								double x = arguments[0].asFloat();
								double y = arguments[1].asFloat();
								int perem_id = symbols->inTable("min", "global");
								if (x > y) {
									symbols->symb_table[perem_id].value.setFloatValue(y);
								} else {
									symbols->symb_table[perem_id].value.setFloatValue(x);
								}
							} else

								if (name == "max") {
									double x = arguments[0].asFloat();
									double y = arguments[1].asFloat();
									int perem_id = symbols->inTable("max", "global");
									if (x < y) {
										symbols->symb_table[perem_id].value.setFloatValue(y);
									} else {
										symbols->symb_table[perem_id].value.setFloatValue(x);
									}
								} else if (name == trUtf8("irnd")) {
									int x = arguments[0].asInt();
									int rndValue = qrand(); // in range [ 0; 2^32 ]
									qreal scale = qreal(x) / qreal(RAND_MAX);
									int result = 1 + int(scale * rndValue); // as I(1;x)
									if (result > x) {
										result = x;
									}
									int perem_id = symbols->inTable(trUtf8("irnd"), "global");
									symbols->symb_table[perem_id].value.setIntegerValue(result);
								} else if (name == trUtf8("irand")) {
									int x = arguments[0].asInt();
									int y = arguments[1].asInt();
									if (x > y) {
										*err = INTFUN_RND_INVALID_RANGE;
										return;
									}
									int rndValue = qrand(); // in range [ 0; 2^32 ]
									qreal scale = qreal(y - x + 1) / qreal(RAND_MAX);
									int result = x + int(scale * rndValue); // as I(x;y)
									if (result > y) {
										result = y;
									}
									int perem_id = symbols->inTable(trUtf8("irand"), "global");
									symbols->symb_table[perem_id].value.setIntegerValue(result);
								} else if (name == trUtf8("rand")) {
									qreal x = arguments[0].asInt();
									qreal y = arguments[1].asInt();
									if (x > y) {
										*err = INTFUN_RND_INVALID_RANGE;
										return;
									}
									int rndValue = qrand(); // in range [ 0; 2^32 ]
									qreal scale = (y - x) / qreal(RAND_MAX);
									qreal result = x + scale * rndValue; // as R(x;y)
									int perem_id = symbols->inTable(trUtf8("rand"), "global");
									symbols->symb_table[perem_id].value.setFloatValue(result);
								} else if (name == "rnd") {
									double x = arguments[0].asFloat();
									long int rndValue = qrand();
									double stdValue = (double)rndValue / RAND_MAX;
									double result = (double)(x * stdValue);
									int perem_id = symbols->inTable("rnd", "global");
									symbols->symb_table[perem_id].value.setFloatValue(result);

								}



	if (name == "sin") {
		double x = arguments[0].asFloat();
		int perem_id = symbols->inTable("sin", "global");
		symbols->symb_table[perem_id].value.setFloatValue(sin(x));
	} else

		if (name == "cos") {
			double x = arguments[0].asFloat();
			int perem_id = symbols->inTable("cos", "global");
			symbols->symb_table[perem_id].value.setFloatValue(cos(x)); //TODO CHECK ME
		} else

			if (name == "tg") {
				double x = arguments[0].asFloat();
				int perem_id = symbols->inTable("tg", "global");
				symbols->symb_table[perem_id].value.setFloatValue(tanf(x));
			} else

				if (name == "ctg") {
					double x = arguments[0].asFloat();
					int perem_id = symbols->inTable("ctg", "global");
					double tg = tan(x);
					double ctg = 1.0 / tg;
					if (isinf(ctg) != 0) {
						*err = RUN_NO_VALUE;
						return;
					}
					symbols->symb_table[perem_id].value.setFloatValue(ctg);
				} else

					if (name == "arcsin") {
						double x = arguments[0].asFloat();
						if (fabs(x) > 1) {
							*err = RUN_BAG_INVERSE_TRIGONOMETRIC_ARGUMENT;
							return;
						}
						int perem_id = symbols->inTable("arcsin", "global");
						double res = sin(x);
						if (isnan(res) != 0) {
							*err = RUN_NO_VALUE;
							return;
						}
						if (isinf(res) != 0) {
							*err = RUN_NO_VALUE;
							return;
						}
						symbols->symb_table[perem_id].value.setFloatValue(res);
					} else

						if (name == "arccos") {
							double x = arguments[0].asFloat();
							if (fabs(x) > 1) {
								*err = RUN_BAG_INVERSE_TRIGONOMETRIC_ARGUMENT;
								return;
							}
							int perem_id = symbols->inTable("arccos", "global");
							double res = acos(x);
							if (isnan(res) != 0) {
								*err = RUN_NO_VALUE;
								return;
							}
							symbols->symb_table[perem_id].value.setFloatValue(res);
						} else

							if (name == "arctg") {
								double x = arguments[0].asFloat();
								int perem_id = symbols->inTable("arctg", "global");
								symbols->symb_table[perem_id].value.setFloatValue(atan(x));
							} else

								if (name == "arcctg") {
									double x = arguments[0].asFloat();
									double tg = 1.0 / x; // x:=ctg(alpha); result=alpha:=argctg(ctg(alpha))
									double result = atan(tg);
									int perem_id = symbols->inTable("arcctg", "global");
									symbols->symb_table[perem_id].value.setFloatValue(result);
								} else

									if (name == "ln") {
										double x = arguments[0].asFloat();
										if (x <= 0.) {
											*err = INTFUN_LN_ZERO;
											return;
										}
										int perem_id = symbols->inTable("ln", "global");
										symbols->symb_table[perem_id].value.setFloatValue(logf(x));
									} else

										if (name == "lg") {
											double x = arguments[0].asFloat();
											if (x <= 0.) {
												*err = INTFUN_LN_ZERO;
												return;
											}
											double ln = logf(x);
											double result = ln / logf(10.0); // log_a(b) = log_c(b) / log_c(a)
											qDebug() << "Lg Result: " << result;
											int perem_id = symbols->inTable("lg", "global");
											symbols->symb_table[perem_id].value.setFloatValue(result);
										} else

											if (name == "exp") {
												double x = arguments[0].asFloat();
												int perem_id = symbols->inTable("exp", "global");
												if (isnan(expf(x)) != 0) {
													*err = RUN_NO_VALUE;
													return;
												}
												if (isinf(expf(x)) != 0) {
													*err = CALC_FLOAT_OVERFLOW;
													return;
												}
												symbols->symb_table[perem_id].value.setFloatValue(expf(x));
											};

	// 06.09

	if (name == "iabs") {
		int x = arguments[0].asInt();
		int perem_id = symbols->inTable("iabs", "global");
		symbols->symb_table[perem_id].value.setIntegerValue(abs(x));
	} else

		if (name == "mod") {
			int x = arguments[0].asInt();
			int y = arguments[1].asInt();
			int perem_id = symbols->inTable("mod", "global");
			if (y <= 0) {
				*err = INTFUN_MOD_BAD_Y;
				return;
			}
			while (x < 0) {
				x += y;
			}
			symbols->symb_table[perem_id].value.setIntegerValue(x % y);
		} else

			if (name == "div") {
				int x = arguments[0].asInt();
				int y = arguments[1].asInt();
				int perem_id = symbols->inTable("div", "global");
				if (y <= 0) {
					*err = INTFUN_DIV_BAD_Y;
					return;
				}
				int xx = x;
				while (xx < 0) { //ZACHEM???
					xx += y;
				}
				int mod = xx % y;
				int result = (x - mod) / y;
				symbols->symb_table[perem_id].value.setIntegerValue(result);
				symbols->setUsed(perem_id, true);
			} else if (name == "int") {
				double x = arguments[0].asFloat();
				double y = floor(x);
				bool ok = IntegerOverflowChecker::checkFromReal(y);
				int result = (int)y;
				if (!ok) {
					*err = CALC_OVERFLOW;
					return;
				}
				int perem_id = symbols->inTable("int", "global");
				symbols->symb_table[perem_id].value.setIntegerValue(result);
			} else if (name == trUtf8("МАКСВЕЩ")) {
				double value = DoubleOverflowChecker::MAX_DOUBLE_VALUE;
				int perem_id = symbols->inTable(trUtf8("МАКСВЕЩ"), "global");
				symbols->symb_table[perem_id].value.setFloatValue(value);
			} else if (name == trUtf8("МАКСЦЕЛ")) {
				int perem_id = symbols->inTable(trUtf8("МАКСЦЕЛ"), "global");
				symbols->symb_table[perem_id].value.setFloatValue(2147483647);
			} else if (name == trUtf8("длин")) {
				QString s = arguments[0].asString();
				int perem_id = symbols->inTable(trUtf8("длин"), "global");
				symbols->symb_table[perem_id].value.setIntegerValue(s.length());
			} else

				if (name == trUtf8("время")) {
					QTime time = QTime::currentTime();
					int sec = time.hour() * 3600 + time.minute() * 60 + time.second();
					int csec = sec * 100;
					csec += time.msec() / 10;
					int result = csec;
					int perem_id = symbols->inTable(trUtf8("время"), "global");
					symbols->symb_table[perem_id].value.setIntegerValue(result);
				}

				else if (name == trUtf8("клав")) {
					qDebug() << "Klav call";
					uint code = app()->run->getKeyCode();
					int perem_id = symbols->inTable(trUtf8("клав"), "global");
					qDebug() << "Perem id" << perem_id << "Code" << code;
					symbols->symb_table[perem_id].value.setIntegerValue(code);
				}

				else

					if (name == trUtf8("лит_в_цел")) {
						if (arguments.count() < 3) {
							*err = 2;
							return;
						};
						QString literal = arguments[0].asString();
						PeremPrt link;
						link.perem_id = arguments[1].asInt();
						link.mod_id = arguments[2].asInt();
						if (link.perem_id > 1024) {
							*err = 2;
							return;
						};
						if (link.perem_id < 0) {
							*err = 2;
							return;
						};

						if (link.mod_id < 0) {
							*err = 2;
							return;
						};
						if (link.mod_id > 256) {
							*err = 2;
							return;
						};
						bool ok;
						int result = literal.toInt(&ok);
						app()->compiler->modules()->setBoolValue(link, ok);

						app()->compiler->modules()->setUsed(link, true);
						int perem_id = symbols->inTable(trUtf8("лит_в_цел"), "global");
						if (perem_id < 0) {
							*err = 2;
							return;
						};
						symbols->symb_table[perem_id].value.setIntegerValue(result);
						symbols->setUsed(perem_id, true);
					} else

						if (name == trUtf8("лит_в_вещ")) {
							if (arguments.count() < 3) {
								*err = 2;
								return;
							};
							QString literal = arguments[0].asString();
							PeremPrt link;
							link.perem_id = arguments[1].asInt();
							link.mod_id = arguments[2].asInt();
							if (link.perem_id > 1024) {
								*err = 2;
								return;
							};
							if (link.perem_id < 0) {
								*err = 2;
								return;
							};

							if (link.mod_id < 0) {
								*err = 2;
								return;
							};
							if (link.mod_id > 256) {
								*err = 2;
								return;
							};

							bool ok;
							double result = literal.toDouble(&ok);
							if ((literal.toLower() == "inf") || (literal.toLower() == "-inf") || (literal.toLower() == "nan")) {
								ok = false;
							}
							app()->compiler->modules()->setBoolValue(link, ok);
							app()->compiler->modules()->setUsed(link, true);
							int perem_id = symbols->inTable(trUtf8("лит_в_вещ"), "global");
							if (perem_id < 0) {
								*err = 2;
								return;
							};
							symbols->symb_table[perem_id].value.setFloatValue(result);
							symbols->setUsed(perem_id, true);
						};

	if (name == trUtf8("вещ_в_лит")) {
		double value = arguments[0].asFloat();
		QString result = QString("%1").arg(value, 0, 'f');
		QStringList pair = result.split(".", QString::KeepEmptyParts);
		if (pair.count() == 2) {
			while (pair[1].endsWith("0")) {
				pair[1].remove(pair[1].length() - 1, 1);
			}
			result = pair.join(".");
		}
		if (result.endsWith(".")) {
			result.remove(result.length() - 1, 1);
		}
		int perem_id = symbols->inTable(trUtf8("вещ_в_лит"), "global");
		symbols->symb_table[perem_id].value.setStringValue(result);
		symbols->setUsed(perem_id, true);
	}

	if (name == trUtf8("цел_в_лит")) {
		int value = arguments[0].asInt();
		QString result = QString("%1").arg(value);
		int perem_id = symbols->inTable(trUtf8("цел_в_лит"), "global");
		symbols->symb_table[perem_id].value.setStringValue(result);
		symbols->setUsed(perem_id, true);
	}

	if (name == trUtf8("лит")) {
		int perem_id = symbols->inTable(trUtf8("лит"), "global");
		symbols->symb_table[perem_id].value.setStringValue(QString::number(arguments[0].asFloat()));
		//     delete (int *)virajList[0];

	};

	if (name == "delay") {
		int msec = arguments[0].asInt();
		qDebug() << "delay(" << msec << ")";
		app()->run->evaluateDelay(msec);
	}

	//  if (name.startsWith("!memfill_"))
	//  {
	//      int mod_id = arguments[1].asInt();
	//      int var_id = arguments[0].asInt();
	//      symbol_table *st = m_modules->module(mod_id)->Values();
	//      QString s_data = arguments[2].asString();
	//      for (int i=0; i<s_data.length(); i++) {
	//          int code = s_data[i].unicode();
	//          code += STR_HIDE_OFFSET;
	//          s_data[i] = QChar(code);
	//      }

	//      int dim = name.mid(name.length()-1,1).toInt();
	//      PeremType baseType;
	//      if (name[name.length()-2]=='i')
	//          baseType = integer;
	//      else if (name[name.length()-2]=='d')
	//          baseType = real;
	//      else if (name[name.length()-2]=='b')
	//          baseType = kumBoolean;
	//      else if (name[name.length()-2]=='c')
	//          baseType = charect;
	//      else if (name[name.length()-2]=='s')
	//          baseType = kumString;

	//      int x_max = st->symb_table[var_id].size0_end-st->symb_table[var_id].size0_start + 1;
	//      int y_max = st->symb_table[var_id].size1_end-st->symb_table[var_id].size1_start + 1;
	//      int z_max = st->symb_table[var_id].size2_end-st->symb_table[var_id].size2_start + 1;
	//      if ( dim==1 ) {
	//          QList<QVariant> data = extract1DArray(s_data);
	//          QVariant value;
	//          int y = 0;
	//          int z = 0;
	//          for ( int x=0; x<data.count(); x++ ) {
	//              if (x>=x_max) {
	//                  *err = RUN_CONST_ARRAY_OUT_OF_BOUNDS;
	//                  return;
	//              }
	//              value = data[x];
	//              if (baseType==integer) {
	//                  st->symb_table[var_id].value.setIntMassValue(value.toInt(),x,y,z);
	//              }
	//              else if (baseType==real) {
	//                  st->symb_table[var_id].value.setFloatMassValue(value.toDouble(),x,y,z);
	//              }
	//              else if (baseType==kumBoolean) {
	//                  st->symb_table[var_id].value.setBoolMassValue(value.toBool(),x,y,z);
	//              }
	//              else if (baseType==charect) {
	//                  st->symb_table[var_id].value.setCharMassValue(value.toChar(),x,y,z);
	//              }
	//              else if (baseType==kumString) {
	//                  st->symb_table[var_id].value.setStringMassValue(value.toString(),x,y,z);
	//              }
	//          }
	//      }
	//      if ( dim==2 ) {
	//          QList< QList<QVariant> > data = extract2DArray(s_data);
	//          QVariant value;
	//          int z = 0;
	//          for ( int y=0; y<data.count(); y++ ) {
	//              for ( int x=0; x<data[y].count(); x++ ) {
	//                  if (x>=x_max || y>=y_max) {
	//                      *err = RUN_CONST_ARRAY_OUT_OF_BOUNDS;
	//                      return;
	//                  }
	//                  value = data[y][x];
	//                  if (baseType==integer) {
	//                      st->symb_table[var_id].value.setIntMassValue(value.toInt(),x,y,z);
	//                  }
	//                  else if (baseType==real) {
	//                      st->symb_table[var_id].value.setFloatMassValue(value.toDouble(),x,y,z);
	//                  }
	//                  else if (baseType==kumBoolean) {
	//                      st->symb_table[var_id].value.setBoolMassValue(value.toBool(),x,y,z);
	//                  }
	//                  else if (baseType==charect) {
	//                      st->symb_table[var_id].value.setCharMassValue(value.toChar(),x,y,z);
	//                  }
	//                  else if (baseType==kumString) {
	//                      st->symb_table[var_id].value.setStringMassValue(value.toString(),x,y,z);
	//                  }
	//              }
	//          }
	//      }
	//      if ( dim==3 ) {
	//          QList< QList< QList<QVariant> > > data = extract3DArray(s_data);
	//          QVariant value;
	//          for ( int z=0; z<data.size(); z++ ) {
	//              for ( int y=0; y<data[z].count(); y++ ) {
	//                  for ( int x=0; x<data[z][y].count(); x++ ) {
	//                      if (x>=x_max || y>=y_max || z>=z_max) {
	//                          *err = RUN_CONST_ARRAY_OUT_OF_BOUNDS;
	//                          return;
	//                      }
	//                      value = data[z][y][x].toInt();
	//                      if (baseType==integer) {
	//                          st->symb_table[var_id].value.setIntMassValue(value.toInt(),x,y,0);
	//                      }
	//                      else if (baseType==real) {
	//                          st->symb_table[var_id].value.setFloatMassValue(value.toDouble(),x,y,0);
	//                      }
	//                      else if (baseType==kumBoolean) {
	//                          st->symb_table[var_id].value.setBoolMassValue(value.toBool(),x,y,0);
	//                      }
	//                      else if (baseType==charect) {
	//                          st->symb_table[var_id].value.setCharMassValue(value.toChar(),x,y,0);
	//                      }
	//                      else if (baseType==kumString) {
	//                          st->symb_table[var_id].value.setStringMassValue(value.toString(),x,y,0);
	//                      }
	//                  }
	//              }
	//          }
	//      }
	//  }

}
