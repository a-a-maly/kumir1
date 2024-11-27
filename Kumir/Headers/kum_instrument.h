//****************************************************************************
//**
//** Copyright (C) 2004-2008 IMPB RAS. All rights reserved.
//**
//** This file is part of the KuMir.
//**
//** This file may be used under the terms of the GNU General Public
//** License version 2.0 as published by the Free Software Foundation
//** and appearing in the file LICENSE.GPL included in the packaging of
//** this file.
//**
//** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
//** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//**
//****************************************************************************/


/* Kum_Instrument
Класс встроенных функций кумир
*/

#ifndef KUM_INSTR_H
#define KUM_INSTR_H

#include <QObject>
#include <QRect>
#include <QDebug>

class symbol_table;

#define LEFT_WALL   0x1
#define RIGHT_WALL  0x2
#define DOWN_WALL   0x4
#define UP_WALL     0x8
#define MAX_CLICK_DELTA 7

#define NO_CRASH 0
#define UP_CRASH 1
#define DOWN_CRASH 2
#define LEFT_CRASH 3
#define RIGHT_CRASH 4

class function_table;
//class symbol_table;
class KumValueStackElem;
// class RobotWindow;

class PrintDialog;


class KumInstrument : public QObject

{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent сслыка на объект-владелец
	     */
	KumInstrument(QWidget *parent = 0)
	{
		Q_UNUSED(parent);
	};

	virtual void setMode(bool EditMode)
	{
		Q_UNUSED(EditMode);
	};

	virtual void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err)
	{
		Q_UNUSED(name);
		Q_UNUSED(aruments);
		Q_UNUSED(err);
		qDebug() << "BUG:Use Of prototype!!!!";
	};//Преопределяется классом выполнения  функцийp
public slots:
	virtual void lockControls(void)
	{
	};//Преопределяется классом выполнения  функций
	virtual void unLockControls(void)
	{
	};//Преопределяется классом выполнения  функций
	virtual void CloseInst(void)
	{
	};//Преопределяется классом выполнения  функций
	virtual void reset(void)
	{
	};//Преопределяется классом выполнения  функций
	virtual void showHideWindow(bool)
	{
	};
	virtual void setWindowPos(int x, int y)
	{
		Q_UNUSED(x);
		Q_UNUSED(y);
	};
	virtual void getWindowPos(int *x, int *y)
	{
		Q_UNUSED(x);
		Q_UNUSED(y);
	};
	//Преопределяется классом выполнения  функций
	virtual void SwitchMode(int x)
	{
		Q_UNUSED(x);
		emit sync(); //!После переключения режима небходима синхронизация
		qDebug("Sync from virual KumInstrument");
	};
	virtual void close() = 0;
	virtual QRect getWindowGeometry()
	{
		return QRect();
	};

	//ZAPLATKI
	virtual void setRobotStartField(class RoboField *startField)
	{
		Q_UNUSED(startField);
	};
	virtual void restoreRobotStartField()
	{};
signals:
	void sync();
	void FieldEdit();
	void pultCommand(QString text);
public:
	virtual int getFunctionList(function_table *Functions, symbol_table *Symbols);

};


class KumModules;

//Стандартные функции
class KumStandartFunctions : public KumInstrument

{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent сслыка на объект-владелец
	     */
	KumStandartFunctions(KumModules *modules, QWidget *parent = 0);
public:
	int getFunctionList(function_table *Functions, symbol_table *Symbols);
public slots:
	void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err);
	void close() {}
signals:
	void sync();
private:
	function_table *functions;
	symbol_table *symbols;
	QTextCodec *codec;
	KumModules *m_modules;
};


//Robot
//#define FIELD_SIZE 20
#define FIELD_SIZE_SMALL 30
#define BUTTON_SIZE 20

#define NOT_LEFT_WALL   14
#define NOT_RIGHT_WALL   13
#define NOT_DOWN_WALL   11
#define NOT_UP_WALL   7
#define DEFAULT_X 16
#define DEFAULT_Y 9
#define BORT 5
#define MAX_SIZE 256
#define MIN_SIZE 3
// NEW V.K. 05-10
#define MIN_TEMP -273
#define DPANEL_HIGTH 0
//#define PANEL_HIGTH 0
#define BTNXSDVIG 0
#define SEE_MODE 0
#define RUN_MODE 1
#define EDIT_MODE 2
#define ANALYZE_MODE 3
#define PAUSE_MODE 4
#define DEFAULT_SIZEX 400
#define DEFAULT_SIZEY 400



#endif
