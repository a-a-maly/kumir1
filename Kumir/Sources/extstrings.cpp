#include "extstrings.h"
#include "kum_tables.h"
#include "error.h"

ExtStrings::ExtStrings(QWidget *parent) : KumInstrument(parent)
{
}


/**
 * Добавляет в таблицы функции и переменные,устанавливает таблицы класса
 *  function_table и symbol_table
 * @param Functions Таблица функций
 * @param Symbols Таблица переменных
 * @return количество функций
 */
int ExtStrings::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	ready = true;
	symbols = Symbols;

	functions = Functions;
	int count = 0;
	Functions->addInternalFunction(trUtf8("найти"), integer, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, kumString);
	Symbols->add_symbol(trUtf8("найти"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	Functions->addInternalFunction(trUtf8("найти после"), integer, 3);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, kumString);
	Functions->setArgType(Functions->size() - 1, 2, kumString);
	Symbols->add_symbol(trUtf8("найти после"), -1, integer, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	Functions->addInternalFunction(trUtf8("удалить"), kumString, 3);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setArgType(Functions->size() - 1, 2, integer);
	Symbols->add_symbol(trUtf8("удалить"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);

	count++;

	Functions->addInternalFunction(trUtf8("вставить"), kumString, 3);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, kumString);
	Functions->setArgType(Functions->size() - 1, 2, integer);
	Symbols->add_symbol(trUtf8("вставить"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	Functions->addInternalFunction(trUtf8("верхний регистр"), kumString, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Symbols->add_symbol(trUtf8("верхний регистр"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	Functions->addInternalFunction(trUtf8("нижний регистр"), kumString, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Symbols->add_symbol(trUtf8("нижний регистр"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	Functions->addInternalFunction(trUtf8("замена"), kumString, 4);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, kumString);
	Functions->setArgType(Functions->size() - 1, 2, kumString);
	Functions->setArgType(Functions->size() - 1, 3, kumBoolean);

	Symbols->add_symbol(trUtf8("замена"), -1, kumString, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	count++;

	return count;
};


void ExtStrings::runFunc(QString name, QList<KumValueStackElem> *args, int *err)
{
	*err = 0;
	QList<KumValueStackElem> arguments = *args;
	if (name == trUtf8("верхний регистр")) {
		QString string = arguments.first().asString();
		int perem_id = symbols->inTable(name, "global");
		symbols->symb_table[perem_id].value.setStringValue(string.toUpper());
		return;
	};

	if (name == trUtf8("нижний регистр")) {
		QString string = arguments.first().asString();
		int perem_id = symbols->inTable(name, "global");
		symbols->symb_table[perem_id].value.setStringValue(string.toLower());
		return;
	};

	if (name == trUtf8("найти")) {
		QString sub_string = arguments.first().asString();
		QString string = arguments[1].asString();
		int perem_id = symbols->inTable(name, "global");
		int res = string.indexOf(sub_string);
		if (res > -1) {
			res++;
		}
		symbols->symb_table[perem_id].value.setIntegerValue(res);
		return;
	};

	if (name == trUtf8("найти после")) {
		int from = arguments.first().asInt();
		QString sub_string = arguments[1].asString();
		QString string = arguments[2].asString();
		if (from < 0) {
			*err = RUN_STRING_BOUND_1;
			return;
		}
		int perem_id = symbols->inTable(name, "global");
		int res = string.indexOf(sub_string, from);
		if (res > -1) {
			res++;
		}
		symbols->symb_table[perem_id].value.setIntegerValue(res);
		return;
	};


	if (name == trUtf8("удалить")) {
		QString string = arguments.first().asString();
		int from = arguments[1].asInt();

		int numb = arguments[2].asInt();
		int perem_id = symbols->inTable(name, "global");
		symbols->symb_table[perem_id].value.setStringValue(string.remove(from - 1, numb));
		return;
	};

	if (name == trUtf8("вставить")) {
		QString sub_string = arguments.first().asString();
		QString  string = arguments[1].asString();
		int from = arguments[2].asInt();
		if (from < 1) {
			*err = RUN_STRING_BOUND_1;
			return;
		}
		int perem_id = symbols->inTable(name, "global");
		symbols->symb_table[perem_id].value.setStringValue(string.insert(from - 1, sub_string));
		return;
	};
	if (name == trUtf8("замена")) {
		QString string = arguments.first().asString();
		QString  from = arguments[1].asString();
		QString to = arguments[2].asString();
		bool multi = arguments[3].asBool();
		int perem_id = symbols->inTable(name, "global");
		if (multi) {
			symbols->symb_table[perem_id].value.setStringValue(string.replace(from, to));
		} else {
			int pos = string.indexOf(from);
			string.remove(pos, from.length());
			string.insert(pos, to);
			symbols->symb_table[perem_id].value.setStringValue(string);
		};
		return;
	};

	*err = -2;
	return;
};
