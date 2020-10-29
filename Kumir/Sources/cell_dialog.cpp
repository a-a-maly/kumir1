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

#include "cell_dialog.h"
#include <QDebug>


CellDialog::CellDialog(QWidget *parent, Qt::WindowFlags fl) :
	QDialog(parent, fl), Ui::CellDialog()
{
	setupUi(this);
	connect(RadSlider, SIGNAL(sliderMoved(int)), this, SLOT(radSliderChanged(int)));
	connect(TempSlider, SIGNAL(sliderMoved(int)), this, SLOT(tempSliderChanged(int)));
	connect(RadSpinBox, SIGNAL(valueChanged(double)), this, SLOT(refresh(void)));
	connect(TempSpinBox, SIGNAL(valueChanged(double)), this, SLOT(refresh(void)));
}

CellDialog::~CellDialog()
{
}


void CellDialog::radSliderChanged(int pos)
{
	RadSpinBox->setValue(pos);
}

void CellDialog::tempSliderChanged(int pos)
{
	TempSpinBox->setValue(pos);
}

void CellDialog::refresh()
{
	RadSlider->setValue(RadSpinBox->value());
	TempSlider->setValue(TempSpinBox->value());
}

CellDialog *CellDialog::m_instance = 0;

CellDialog *CellDialog::instance()
{
	if (m_instance == 0) {
		qDebug() << "CELL DIALOG RECREATE!!!!!!!";
		m_instance = new CellDialog();
	}
	return m_instance;
}

