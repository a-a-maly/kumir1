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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include "ui_tooltip.h"


class ToolTip : public QWidget, public Ui::ToolTip
{
	Q_OBJECT

public:
	ToolTip(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~ToolTip();

	int cursorPosition;
	inline bool isClosed() const
	{
		return m_closed;
	}

public slots:
	void hide();
	void init();

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual void hideEvent(QHideEvent *event);
	bool m_closed;
	bool m_activatedByMouse;

protected slots:
	void setButtonsEnabled();
	void enter();
	void mouseClick(QListWidgetItem *item);
	void activated(QListWidgetItem *item);
	void help();

signals:
	void itemSelected(QListWidgetItem *, const QString &reason);
	void closed();

};

#endif

