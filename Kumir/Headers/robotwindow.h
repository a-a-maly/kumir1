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
#ifndef ROBOTWINDOW_H
#define ROBOTWINDOW_H

#include <QWidget>
#include "ui_RoboWindow.h"


class RobotWindow :
	public QWidget,
	public Ui::RobotWindow
{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссыка на объект-владелец
	 * @param fl флаги окна
	 */
	RobotWindow(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~RobotWindow();


};
#endif
