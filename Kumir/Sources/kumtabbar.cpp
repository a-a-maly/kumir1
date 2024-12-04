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
#include "kumtabbar.h"
#include <QMenu>
#include <QMouseEvent>

KumTabBar::KumTabBar(QWidget *parent) : QTabBar(parent)
{
	closeIcon = QIcon(QString::fromUtf8(":/icons/window-close.png"));
	setMouseTracking(true);
	prevTab = -1;
}

void KumTabBar::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		clickedNo = tabAt(event->pos());
		if (clickedNo > 0) {
			QMenu *menu = new QMenu(this);
			menu->addAction(tr("Close"), this, SLOT(closeTab()));
			menu->exec(event->globalPos());
		}
	} else {
		clickedNo = tabAt(event->pos());
		QRect r = tabRect(clickedNo);
		int w = iconSize().width();
		int x = event->pos().x();
		if (x >= r.x() && x <= r.x() + w * 1.5  && clickedNo > 0) {
			emit tabCloseClicked(clickedNo);
		} else {
			QTabBar::mousePressEvent(event);
		}
	}
}

void KumTabBar::enterEvent(QEvent *event)
{
	QWidget::enterEvent(event);
}

void KumTabBar::leaveEvent(QEvent *event)
{
	setIcon(QPoint(0, 0));
	QWidget::leaveEvent(event);
}

void KumTabBar::mouseMoveEvent(QMouseEvent *event)
{
	setIcon(event->pos());
	QTabBar::mouseMoveEvent(event);
}

void KumTabBar::setIcon(const QPoint &position)
{
	int tabNo = tabAt(position);
	if (tabNo != prevTab) {
		if (prevTab > -1) {
			setTabIcon(prevTab, realIcon);
		}
		realIcon = tabIcon(tabNo);
		if (tabNo > 0) {
			setTabIcon(tabNo, closeIcon);
		}
		prevTab = tabNo;
	}
}

KumTabBar::~KumTabBar()
{
}

void KumTabBar::closeTab()
{
	qDebug("Close tab: %i", clickedNo);
	emit tabCloseClicked(clickedNo);
}
