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
#ifndef CELLDIALOG_H
#define CELLDIALOG_H

#include <QDialog>
#include "ui_cell_dialog.h"

class CellDialog : public QDialog, public Ui::CellDialog
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссыка на объект-владелец
	 * @param fl флаги окна
	 */
	CellDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	static CellDialog *instance();
	/**
	 * Деструктор
	 */
	~CellDialog();

	void setRowCols(int rows, int columns) //Set text
	{
		pole->setText("[" + QString::number(rows) + "," + QString::number(columns) + "]");
	}

	void setPos(int row, int column)
	{
		row_pos = row - 1;
		col_pos = column - 1;
		stolbec->setText(QString::number(column));
		ryad->setText(QString::number(row));
	}

public:
	int row_pos, col_pos;
public slots:
	void radSliderChanged(int pos);
	void tempSliderChanged(int pos);
	void refresh();

private:
	int rows, columns;
	static CellDialog *m_instance;
};
#endif
