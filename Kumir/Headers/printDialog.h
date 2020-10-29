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


/* printDialog
Печать PDF файлов
*/
#ifndef KUM_PRINTPDF_H
#define KUM_PRINTPDF_H
#include "ui_printDialog.h"
#include "kum_instrument.h"

#include <QWidget>
class QGraphicsView;
class QGraphicsScene;
class QPainter;
class QPrinter;


class RoboField;
class PrintPreview : public QWidget
{
	PrintPreview(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~PrintPreview() {};
};

class PrintDialog : public QDialog, Ui::PrintDialog
{
	Q_OBJECT

public:
	PrintDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~PrintDialog() {};
	QString fileName()
	{
		return PrintFileName;
	}
	void setSizes(int szX, int szY)
	{
		sizeX = szX;
		sizeY = szY;
	}
	void setFieldSize(int szX, int szY)
	{
		Cols = szX;
		Rows = szY;
	}
	void setRoboMode(bool m)
	{
		roboMode = m;
		if (!m) {
			field = NULL;
		}
	}
	RoboField  *field;
	QString curDir;
	QGraphicsView *view;
	QGraphicsScene *prvScene;

public slots:
	void accept();
	void selectFileName();

private:
	void drawRobot(QPainter *painter, QPrinter *printer);
	void drawDraw(QPainter *painter, QPrinter *printer);

	int sizeX, sizeY, Cols, Rows;
	QString PrintFileName;
	bool roboMode;
};
#endif
