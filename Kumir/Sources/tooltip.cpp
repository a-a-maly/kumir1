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


#include "tooltip.h"
#include "assistant.h"
#include "application.h"

ToolTip::ToolTip(QWidget *parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), Ui::ToolTip()
{
	setupUi(this);
	setVisible(false);
	connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(setButtonsEnabled()));
	connect(list, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(mouseClick(QListWidgetItem *)));
	connect(list, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(activated(QListWidgetItem *)));
	connect(btnEnter, SIGNAL(clicked()), this, SLOT(enter()));
	connect(btnInfo, SIGNAL(clicked()), this, SLOT(help()));
	m_closed = true;
	setWindowFlags(Qt::Popup);
}

void ToolTip::closeEvent(QCloseEvent *event)
{
	QWidget::closeEvent(event);
	emit closed();
}

void ToolTip::hideEvent(QHideEvent *event)
{
	QWidget::hideEvent(event);
	emit closed();
}

ToolTip::~ToolTip()
{
}

void ToolTip::init()
{
	m_closed = false;
	m_activatedByMouse = false;
}

void ToolTip::hide()
{
	QWidget::hide();
	m_closed = true;
	emit closed();
}

void ToolTip::setButtonsEnabled()
{
	if (!list->currentItem()->text().isEmpty()) {
		//      qDebug("aaa: %s",list->currentItem()->text().toUtf8().data());
		btnInfo->setEnabled(true);
		btnEnter->setEnabled(true);
	} else {
		//      qDebug("bbb");
		btnInfo->setEnabled(false);
		btnEnter->setEnabled(false);
	}
}

void ToolTip::enter()
{
	emit itemSelected(list->currentItem(), "button");
}

void ToolTip::mouseClick(QListWidgetItem *item)
{
	Q_UNUSED(item);
	m_activatedByMouse = true;
}

void ToolTip::activated(QListWidgetItem *item)
{
	QString reason;
	if (m_activatedByMouse) {
		reason = "click";
	} else {
		reason = "key";
	}
	emit itemSelected(item, reason);
	m_activatedByMouse = false;
}

void ToolTip::help()
{
	//  app()->assistant->showTip(list->currentItem()->text());
}
