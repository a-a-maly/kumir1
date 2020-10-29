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

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include "ui_DebugDialog.h"
#include "int_proga.h"
#include <QWidget>

class DebugDialog : public QWidget, Ui::DebugDialog
{
	Q_OBJECT

public:
	explicit DebugDialog(QWidget *parent = 0);
	~DebugDialog();
	void init(KumModules *modules);

public slots:
	void refreshVariables();
	void reset();
signals:
	void sync();
protected slots:
	void handleItemDoubleClicked(QTreeWidgetItem *item, int column);
	void handleTableViewerClosed();
	void handleItemHovered(QTreeWidgetItem *item, int column);

protected:
	static QString unserializeArray(const QVariant &data);
	static QString unserialize1D(const QList<QVariant> l, int size, int &index);

	void createSeparateTableViewer(int moduleId, int variableId);

	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);
	void setHeaders();
	bool isFunction(const QString &name);
	QVariant getValue(int mod, int id) ;
	KumModules *modules;
	QMap<QString, class TableViewer *> l_tables;
	QMap<QString, class SecondaryWindow *> l_childWindows;
};

#endif
