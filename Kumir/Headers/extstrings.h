#ifndef EXTSTRINGS_H
#define EXTSTRINGS_H
#include "kum_instrument.h"
#include "int_proga.h"

class ExtStrings : public KumInstrument
{
	Q_OBJECT
public:
	ExtStrings(QWidget *parent = 0);
	int getFunctionList(function_table *Functions, symbol_table *Symbols);
public slots:
	void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err);
	void close() {};

private:
	function_table *functions;
	symbol_table *symbols;
	bool ready;
};

#endif // EXTSTRINGS_H
