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

#include "ui_pult.h"
#include <QWidget>
#include <QLabel>
class QFrame;

#define SCROLL_STEP 10
#define RESP_PANEL 50
#define LOGGER_BUTTONS 140
#define TEXT_STEP 14

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define TEXT 5


class  linkLight : public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     *
	     */
	linkLight(QWidget *parent = 0);
	/**
	     * Деструктор
	     */
	~linkLight() {};
	void setLink(bool b)
	{
		onLine = b;
	}
	QString text;
signals:
	//void pressed();
protected:
	void paintEvent(QPaintEvent *event);
	// void mousePressEvent ( QMouseEvent * event );
	//void mouseReleaseEvent ( QMouseEvent * event );
private:
	int posX, posY;
	bool onLine;
};


class  MainButton : public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     *
	     */
	MainButton(QWidget *parent = 0);
	/**
	     * Деструктор
	     */
	~MainButton() {};
	void setDirection(int d)
	{
		direction = d;
	};
	void setText(QString t)
	{
		text = t;
		direction = TEXT;
	};
	bool isChecked()
	{
		return checked;
	};
	void setCheckable(bool flag)
	{
		Q_UNUSED(flag);
		checkable = true;
	};
	void setChecked(bool flag)
	{
		checked = flag;
		downFlag = flag;
		repaint();
	};
	bool loadIcon(QString icon);

signals:
	void pressed();
	void clicked();
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:
	void drawAddons(QPainter *painter);
	int posX, posY;
	uint direction;
	QImage buttonImageUp, buttonImageDown, buttonIcon;
	bool downFlag, checked, checkable, mouseOver, icon;
	QWidget *Parent;
	QVector<QLine> upArrow, downArrow, leftArrow, rightArrow;
	QString text;
};



class  loggerButton : public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     *
	     */
	loggerButton(QWidget *parent = 0);
	/**
	     * Деструктор
	     */
	~loggerButton() {};
	void upArrowType(bool b)
	{
		isUpArrow = b;
	};
signals:
	void pressed();
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
private:
	int posX, posY;
	bool isUpArrow;
	QImage buttonImageUp, buttonImageDown;
	bool downFlag;
	QWidget *Parent;
	QVector<QLine> upArrow, downArrow;
};


class logLine
{
public:
	logLine(QString KumCommand,
		QString LogCommand,
		QString React, QFrame *frame, QFrame *respFrame, uint pos)
	{
		kumCommand = KumCommand;
		logCommand = LogCommand;
		react = React;
		textLabel = new QLabel(frame);
		textLabel->setText(logCommand);
		textLabel->move(4, pos);
		textLabel->resize(120, 20);
		textLabel->show();

		respLabel = new QLabel(respFrame);
		respLabel->setText(React);
		respLabel->move(4, pos);
		respLabel->resize(RESP_PANEL, 20);
		respLabel->show();
	};
	void moveUp()
	{
		textLabel->move(textLabel->x(), textLabel->y() - SCROLL_STEP);
		respLabel->move(respLabel->x(), respLabel->y() - SCROLL_STEP);
	};
	void moveDown()
	{
		textLabel->move(textLabel->x(), textLabel->y() + SCROLL_STEP);
		respLabel->move(respLabel->x(), respLabel->y() + SCROLL_STEP);
	};
	int pos()
	{
		return textLabel->y();
	};
	int removeLabels()
	{
		if (textLabel) {
			delete textLabel;
		}
		if (respLabel) {
			delete respLabel;
		}
		return 0;
	};
	QString KumCommand()
	{
		return kumCommand;
	};
private:
	QString kumCommand;
	QString logCommand;
	QString react;
	QLabel *textLabel;
	QLabel *respLabel;
};
class pultLogger : public QWidget
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     * @param fl флаги окна
	     */
	pultLogger(QWidget *parent = 0);
	/**
	     * Деструктор
	     */
	~pultLogger();
	void setSizes(uint w, uint h);
	void Move(uint x, uint y);




	void Show()
	{
		//mainFrame->show();
		//downButton->show();
		//upButton->show();
		//show();
	}
	void appendText(QString kumCommand, QString text, QString replay);

public slots:
	void upBtnPressed();
	void downBtnPressed();
	void ClearLog();
	void CopyLog();
private:
	QFrame *mainFrame;
	QFrame *dummyFrame;
	QFrame *respFrame;
	int W, H;
	int pos;
	//QLabel * testLabel;
	//QFrame * mainFrame;
	QList<logLine> lines;
	int buttonSize;
	loggerButton *downButton;
	loggerButton *upButton;
};

class RoboPult : public QWidget, public Ui::RoboPult
{
	Q_OBJECT
public:
	/**
	     * Конструктор
	     * @param parent ссыка на объект-владелец
	     * @param fl флаги окна
	     */
	RoboPult(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	     * Деструктор
	     */
	~RoboPult() {};

	bool Link()
	{
		return link;
	}

	pultLogger *Logger;
	pultLogger *pltLogger()
	{
		return Logger;
	};
	public
slots:
	void noLink();
	void LinkOK();

	void Up();
	void Down();
	void Left();
	void Right();

	void SwStena();
	void SwSvobodno();
	void TempS();
	void RadS();


	void CenterButton();

signals:
	void goUp();
	void goDown();
	void goLeft();
	void goRight();

	void hasUpWall();
	void hasDownWall();
	void hasLeftWall();
	void hasRightWall();

	void noUpWall();
	void noDownWall();
	void noLeftWall();
	void noRightWall();

	void Color();

	void Clean();
	void Colored();

	void Rad();
	void Temp();
	void PultCmd(QString text);

private:
	bool link;
	void paintEvent(QPaintEvent *);
	linkLight *greenLight;
	MainButton *buttUp, *buttDown, *buttLeft, *buttRight;
	MainButton *askStena, *askFree, *buttRad, *buttTemp;
};
