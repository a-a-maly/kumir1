
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
#ifndef INT_PROGA_H
#define INT_PROGA_H

#include "application.h"
#include "network.h"
#include "kum_instrument.h"

class QTextEdit;

#define MAX_ISP 64

class ProgramTab;
class MainWindow;


enum RunMode {
	Null = 0,
	po_shagam = 1,
	neprerivno = 2,
	bezPauz = 3,
	debug = 4,
	funct = 5
};


/**
 * Представление элемента стека для подсчета выражений
 */

struct BoolOrNum {
	BoolOrNum()
	{
		b = false;
		i = 0;
		d = 0.0;
		t = none;
	}
	BoolOrNum(bool _b, int _i)
	{
		b = _b;
		i = _i;
		d = 0.0;
		t = integer;
	}
	BoolOrNum(bool _b, double _d)
	{
		b = _b;
		i = 0;
		d = _d;
		t = real;
	}
	bool b;
	int i;
	double d;
	PeremType t;
};

struct BoolOrLit {
	BoolOrLit()
	{
		b = false;
		s = "";
		c = ' ';
		t = none;
	}
	BoolOrLit(bool _b, QString _s)
	{
		b = _b;
		s = _s;
		c = ' ';
		t = kumString;
	}
	BoolOrLit(bool _b, QChar _c)
	{
		b = _b;
		s = "";
		c = _c;
		t = charect;
	}
	bool b;
	QString s;
	QChar c;
	PeremType t;
};



/**
 * Таблица исполнителей
 */
//class KumInstrument;
class KumValueStackElem;
class symbol_table;
class isp_table
{
	struct i_table {
		QString name;
		QString file;
		IspType type;
		int isp_begins;
		int isp_ends;
// bool isInternal;//DUMMY
// int firstPeremId;
		KNPConnection *NetworkServer; //ССылка на сетевое соединение
		//KumInstrument* Instrument; //Сылка на обьект
		bool graphics;
	};

public:
	isp_table();
public:
	int addIsp(QString isp_name, int isp_impl_str, IspType type, QString file);
public:
	int setIspEnd(int id, int str)
	{
		isp_Table[id].isp_ends = str;
		return 0;
	};
public:
	int getIspIdbyStr(int str);
public:
	QString getIspNamebyStr(int str);
public:
	QString getIspNamebyId(int id)
	{
		if ((id > count) || (id < 0)) {
			return "Isp Internal Error";
		}
		return isp_Table[id].name;
	};
public:
	QString getFileNamebyId(int id)
	{
		if ((id > count) || (id < 0)) {
			return "Isp Internal Error";
		}
		return isp_Table[id].file;
	};
public:
	int getIspStrbyId(int id)
	{
		if ((id > count) || (id < 0)) {
			return -1;
		}
		return isp_Table[id].isp_begins;
	};
public:
	int getIspEndbyId(int id)
	{
		if ((id > count) || (id < 0)) {
			return -1;
		}
		return isp_Table[id].isp_ends;
	};


public:
	int inTable(QString name);
public:
	void clean(int last);

public:
	void resetInstruments();
public:
	void closeInstruments();
public:
	void lockInstrumentsGUI();
public:
	void unlockInstrumentsGUI();


public:
	int lastId(void)
	{
		return count;
	};

public:
	bool isInternal(int id)
	{
		if (isp_Table[id].type == internal) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isKumIsp(int id)
	{
		if (isp_Table[id].type == kumIsp) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isIntsrument(int id)
	{
		if (isp_Table[id].type == instrument) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isNetwork(int id)
	{
		if (isp_Table[id].type == network) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isSystem(int id)
	{
		if (isp_Table[id].type == KumSystem) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isGraphics(int id)
	{
		return isp_Table[id].graphics;
	};
public:
	void setGraphics(int id, bool flag)
	{
		isp_Table[id].graphics = flag;
	};



public:
	QString removeFromFile(QString File)
	{
		for (int i = 0; i <= count; i++) {
			if (File == isp_Table[i].file) {
				count = i - 1;
				return isp_Table[i].name;
			};
		};
		return "!!!!No ISP";
	};

public:
	i_table get(int id)
	{
		return isp_Table[id];
	}

	/**
	* Номера исполнителей описанные в вфайле
	*/
public:
	QString ispsInFile(QString fileName)
	{
		QString toret = "";
		for (int i = 0; i <= count; i++) {
			if (fileName == isp_Table[i].file) {
				toret += QString::number(i) + ",";
			};
		};
		return toret;
	}
public:
	int FirstIspInFile(QString fileName)
	{
		int toret = -1;
		for (int i = 0; i <= count; i++) {
			//bool debug=isp_Table[i].file.endsWith(fileName,Qt::CaseInsensitive);
			if (isp_Table[i].file.endsWith(fileName, Qt::CaseInsensitive)) {
				return i;
			};
		};
		return toret;
	}

public:
	int count;

private:
	i_table isp_Table[MAX_ISP];
};





enum FileMode {
	ReadOnly,
	WriteOnly
};


class KumValueStackElem
{



public:

//Constructors and destructor
	explicit KumValueStackElem();
	explicit KumValueStackElem(int val);
	explicit KumValueStackElem(double val);
	explicit KumValueStackElem(PeremType typ, int err);
	explicit KumValueStackElem(bool val);
	explicit KumValueStackElem(QString val);
	explicit KumValueStackElem(QChar val);
	explicit KumValueStackElem(symbol_table *symbols, int id);
	explicit KumValueStackElem(bool b, double d);
	explicit KumValueStackElem(bool b, int i);
	explicit KumValueStackElem(bool b, QString s);
	explicit KumValueStackElem(bool b, QChar c);
	~KumValueStackElem();
//end of constraction destruction


//Возврат значений
	int  asInt();
	double asFloat();
	bool asBool();
	QString asString();
	QChar asCharect();
	QVariant asVariant();
	int Error()
	{
		return error;
	};
	inline PeremType getType()
	{
		return type;
	}


//Установка значений
	void fromInt(int val);
	void fromBool(bool val);
	void fromFloat(double val);
	void fromString(QString val);


	static KumValueStackElem compare(KumValueStackElem first, KumValueStackElem second);   //Сравнение элементов // <<<


	static KumValueStackElem Summ(KumValueStackElem first, KumValueStackElem second, int *err);  //Сумма элементов
	static KumValueStackElem Minus(KumValueStackElem first, KumValueStackElem second, int *err);  //-
	static KumValueStackElem Multi(KumValueStackElem first, KumValueStackElem second, int *err);  //-
	static KumValueStackElem Div(KumValueStackElem first, KumValueStackElem second, int *err);  //-
	static KumValueStackElem Pow(KumValueStackElem first, KumValueStackElem second, int *err);   //stepen
	static KumValueStackElem NotEq(KumValueStackElem first, KumValueStackElem second);   // !=

	static KumValueStackElem More(KumValueStackElem first, KumValueStackElem second);   //> // <<<
	static KumValueStackElem Less(KumValueStackElem first, KumValueStackElem second);   //< // <<<
	static KumValueStackElem GEQ(KumValueStackElem first, KumValueStackElem second);   //>= // <<<
	static KumValueStackElem LEQ(KumValueStackElem first, KumValueStackElem second);   //<= // <<<
	static KumValueStackElem Or(KumValueStackElem first, KumValueStackElem second);   //or
	static bool And(KumValueStackElem first, KumValueStackElem second);   //and



	bool isReady()
	{
		return ready;
	};

	bool isValid()
	{
		return valid;
	};


private:
	QVariant m_value;
	bool m_boolValue;
	PeremType type;
	bool ready;
	bool valid;
	int error;
};





/**
 * Таблица файлов
 */
class file_table
{

	struct file {
		QFile *data;
		QString name;
		FileMode mode;
		PeremType p_type;
	};

	typedef  QMap<QString, file> FileMap;

public:
	file_table();

	void openFile(QString fileName, FileMode fileMode, PeremType fileType, int *err);
	void closeFile(QString fileName, int *err);

	void setToBegin(QString fileName, int *err);
	bool hasData(QString fileName, int *err);

	int readIntegerElem(QString fileName, int *err);
	double readFloatElem(QString fileName, int *err);
	QChar readCharectElem(QString fileName, int *err);
	QString readStringElem(QString fileName, int *err);
	bool readBooleanElem(QString fileName, int *err);

	void clearFile(QString fileName, int *err);

	void writeIntegerElem(QString fileName, int value, int *err);
	void writeFloatElem(QString fileName, double value, int *err);
	void writeCharectElem(QString fileName, QChar value, int *err);
	void writeStringElem(QString fileName, QString value, int *err);
	void writeBooleanElem(QString fileName, bool value, int *err);

	QString getCurrentDir();
	void setCurrentDir(QString path, int *err);

	void clear();
	int count();
	~file_table();
private:
	QString currentDir;
	bool currentDirInitialized;
	FileMap fileMap;

	QString getFullName(QString name) const;
};


/**
 * Внутреннее представление строки программы
 */
class KumirEdit;
class KumModules;



class KumOutBuffElem
{
public:
	KumOutBuffElem();
	KumOutBuffElem(int typ, int str, QString text);
	int TYPE;
	int str;
	int size;
	QString text;
};

class KumOutBuff
{


public:
	KumOutBuff();

	int count()
	{
		return Buffer.count();
	};


	void clean(int pos)
	{
		Q_UNUSED(pos);
		Buffer.clear();
		updated = false;
	};
	void setUpdated(bool state)
	{
		updated = state;
		updated = true;
	};

	bool isUpdated()
	{
		return updated;
	};


	bool isEmpty()
	{
		return Buffer.empty();
	};

	void append(int type, QString text, int str)
	{
		Buffer.append(KumOutBuffElem(type, str, text));
	};
	void appendMainMsg(QString text);
	void appendDebugMsg(QString text, int str);
	void appendErrMsg(QString text, int str);
	void appendCounterMsg(int increment);
	KumOutBuffElem GetFirst();
	KumOutBuffElem first();
	void removeUnactualMessages();



private:
	QList<KumOutBuffElem> Buffer;
	bool updated;
	bool locked;

};


class RunSetup
{

public:
	RunSetup(KumModules *modules,

		QTextEdit *output_i,
		QTextEdit *debugW_i,
		MainWindow *main_i);

	RunSetup(RunSetup *Setup, uint Mode);
	RunSetup copy();
	RunSetup()
	{
		ready = false;
		mode = neprerivno;
		initMain = false;
	};

	bool setMode(RunMode Mode)
	{
		mode = Mode;
		ready = true;
		if (main) {
			return true;
		} else {
			return false;
		}
	};
	bool initMainNeeded()
	{
		return initMain;
	}
	void setInitMain(bool flag) //Нужно ли отдeльно инициализировать main
	{
		initMain = flag;
	}

	void setFuncMode(RunMode Mode)
	{
		func_mode = Mode;
	};

	bool setStart(int Start)
	{
		start = Start;
		if (mode != Null) {
			ready = true;
			return true;
		} else {
			return false;
		}
	};

	bool isReady()
	{
		return ready;
	};

	bool isDebug()
	{
		if (mode == debug) {
			return true;
		} else {
			return false;
		}
	};
	bool isNeprerivno()
	{
		if (mode == neprerivno) {
			return true;
		} else {
			return false;
		}
	};
	bool isPoShagam()
	{
		if (mode == po_shagam) {
			return true;
		} else {
			return false;
		}
	};
	bool isFunct()
	{
		if (mode == funct) {
			return true;
		} else {
			return false;
		}
	};
	bool isBezPauz()
	{
		if (mode == bezPauz) {
			return true;
		} else {
			return false;
		}
	};




	KumModules *modules()
	{
		return Modules;
	};


	isp_table *Ispolniteli()
	{
		return ispolniteli;
	};

	QTextEdit *Output()
	{
		return output;
	};

	QTextEdit *DebugW()
	{
		return debugW;
	};
	uint         Mode()
	{
		return mode;
	};

	uint         Start()
	{
		return start;
	};
	MainWindow *Main()
	{
		return main;
	};

	int ISP_BASE;
	RunMode func_mode;

private:
	bool ready;

	KumModules *Modules;
	QTextEdit *output;
	QTextEdit *debugW;
	uint mode;
	MainWindow *main;
	isp_table *ispolniteli;
	int start;
	bool initMain;

};

#endif
