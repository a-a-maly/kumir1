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

#include "macrosmanagementdialog.h"
#include "application.h"
#include "macro.h"
#include "newmacrodialog.h"

#include <QMessageBox>
#include <QDomDocument>
#include <QDir>
#include <QTextStream>

MacrosManagementDialog::MacrosManagementDialog ( QWidget* parent, Qt::WindowFlags fl )
		: QDialog ( parent, fl ), Ui::Dialog()
{
	setupUi ( this );
	connect ( list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectMacro(QListWidgetItem*)) );
	connect ( list, SIGNAL(itemSelectionChanged()), this, SLOT(selectMacro()) );
//	connect ( list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(editMacro(QListWidgetItem*)) );
	connect ( btnEdit, SIGNAL(clicked()), this, SLOT(editMacro()) );
	connect ( btnRemove, SIGNAL(clicked()), this, SLOT(deleteMacro()) );
	foreach ( Macro *m, app()->macros )
	{
		if ( m->isEditable() )
			list->addItem(new QListWidgetItem(m->name(),list));
		//else
		//	list->addItem(new QListWidgetItem(QIcon(":/icons/16x16_protected.png"),m->name(),list));
	}
}

MacrosManagementDialog::~MacrosManagementDialog()
{
}

void MacrosManagementDialog::selectMacro(QListWidgetItem *item)
{
}

void MacrosManagementDialog::deleteMacro(QListWidgetItem * item)
{
	qDebug("deleteMacro");
	if ( item==NULL )
	{
		item = list->currentItem();
		if (item)
		{
			Macro *m;
			foreach ( Macro *mm, app()->macros )
			{
				if ( mm->name() == item->text() )
					m = mm;
			}
			QString macroDir;
		#ifdef Q_OS_MAC
			macroDir = QDir::cleanPath(QApplication::applicationDirPath()+"/../Resources");
		#else
			macroDir = app()->getMacroDir();
		#endif
			QFile f(macroDir + QDir::separator() + m->fileName());
			if ( f.remove() )
			{
				app()->macros.removeOne(m);
				list->clear();
				foreach ( Macro *mm, app()->macros )
					list->addItem(new QListWidgetItem(mm->name(),list));
			}
			else
				QMessageBox::warning(this, tr("Macro"), tr("Unable to delete file ")+m->fileName());
		}
	}
}

void MacrosManagementDialog::editMacro(QListWidgetItem * item)
{
	if ( item == NULL )
	{
		item = list->currentItem();
		if ( item == NULL )
			return;
	}
	Macro *m = 0;
	foreach ( Macro *mm, app()->macros )
	{
		if ( mm->name() == item->text() )
		{
			m = mm;
		}
	}
	//if ( !m->isEditable() )
		//return;
	NewMacroDialog dialog(m,this);
	if ( dialog.exec() == QDialog::Accepted )
	{
		m->setName(dialog.macroName());
		m->setKeySequence(dialog.macroSequence());
		QString macroDir;
#ifdef Q_OS_MAC
		macroDir = QDir::cleanPath(QApplication::applicationDirPath()+"/../Resources");
#else
		macroDir = app()->getMacroDir();
#endif
		QFile f ( macroDir+"/"+m->fileName() );
		if ( f.open(QIODevice::WriteOnly) )
		{
			QTextStream ts(&f);
			ts.setCodec("UTF-8");
			QDomDocument document("KumirMacro-1.5");
			m->save(document);
			document.save(ts, 1);
			f.close();
			item->setText(m->name());
		}
	}
}
