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
#ifndef KUMTABWIDGET_H
#define KUMTABWIDGET_H

#include <QTabWidget>

class KumTabBar;

/**
    @author Victor Yacovlev <V.Yacovlev@gmail.com>
*/
class KumTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	KumTabWidget(QWidget *parent = 0);
	~KumTabWidget();
	void hideTabBar();
	void showTabBar();

protected:
	KumTabBar *tabBar;

protected slots:
	void closeTab(int no);

signals:
	void closeTab1(int no);

};

#endif
