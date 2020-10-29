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
#ifndef KUMTABBAR_H
#define KUMTABBAR_H

#include <QTabBar>
#include <QIcon>

/**
    @author Victor Yacovlev <V.Yacovlev@gmail.com>
*/
class KumTabBar : public QTabBar
{
	Q_OBJECT
public:
	KumTabBar(QWidget *parent = 0);
	~KumTabBar();
	void setIcon(const QPoint &position);
protected:
	void mousePressEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	int clickedNo;
	QIcon realIcon;
	QIcon closeIcon;
	int prevTab;

protected slots:
	void closeTab();

signals:
	void tabCloseClicked(int no);

};

#endif
