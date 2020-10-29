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


#include "gotolinedialog.h"

GoToLineDialog::GoToLineDialog ( QWidget* parent, Qt::WindowFlags fl )
		: QDialog ( parent, fl ), Ui::GoToLineDialog()
{
	setupUi ( this );
}

GoToLineDialog::~GoToLineDialog()
{
}

/*$SPECIALIZATION$*/
void GoToLineDialog::reject()
{
	QDialog::reject();
}

void GoToLineDialog::accept()
{
	QDialog::accept();
}



