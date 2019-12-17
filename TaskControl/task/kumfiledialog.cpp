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
#include "kumfiledialog.h"


KumFileDialog::KumFileDialog(
	QWidget *parent,
	const QString &caption,
	const QString &directory,
	const QString &filter,
	bool showEncodings
)
	: QFileDialog(parent, caption, directory, filter)
{
	encodings = NULL;
	l = NULL;

	if (showEncodings) {
		l = new QLabel(tr("Text encoding:"), this);
		layout()->addWidget(l);
		encodings = new QComboBox(this);
		encodings->addItems(QStringList() << "UTF-8" << "UTF-16" << "Windows-1251" << "IBM-866" << "KOI8-R");
		layout()->addWidget(encodings);
	}
}

KumFileDialog::~KumFileDialog()
{
}

QString KumFileDialog::encoding()
{
	if (encodings != NULL) {
		return encodings->currentText();
	} else {
		return "";
	}
}

void KumFileDialog::setEncoding(const QString &e)
{
	if (encodings == NULL) {
		return;
	}

	int no = -1;
	for (int i = 0; i < encodings->count(); i++) {
		if (encodings->itemText(i) == e) {
			no = i;
			break;
		}
	}
	if (no > -1) {
		encodings->setCurrentIndex(no);
	}
}

TextFileInfo KumFileDialog::getOpenFileName(
	QWidget *parent,
	const QString &caption,
	const QString &dir,
	const QString &filter,
	QString encoding,
	bool showEncodings
) {
	KumFileDialog *dlg = new KumFileDialog(parent, caption, dir, filter, showEncodings);
	dlg->setFileMode(QFileDialog::ExistingFile);
	dlg->setAcceptMode(QFileDialog::AcceptOpen);
	dlg->setModal(true);
	dlg->setEncoding(encoding);
	TextFileInfo result;
	result.accepted = false;
	if (dlg->exec() == QDialog::Accepted) {
		result.path = dlg->selectedFiles().first();
		result.encoding = dlg->encoding();
		result.accepted = true;
	}
	delete dlg;
	return result;
}

TextFileInfo KumFileDialog::getSaveFileName(
	QWidget *parent,
	const QString &caption,
	const QString &dir,
	const QString &filter,
	QString encoding,
	bool showEncodings
) {
	KumFileDialog *dlg = new KumFileDialog(parent, caption, dir, filter, showEncodings);
	dlg->setFileMode(QFileDialog::AnyFile);
	dlg->setAcceptMode(QFileDialog::AcceptSave);
	dlg->setModal(true);
	dlg->setEncoding(encoding);
	TextFileInfo result;
	result.accepted = false;
	if (dlg->exec() == QDialog::Accepted) {
		result.path = dlg->selectedFiles().first();
		result.encoding = dlg->encoding();
		result.accepted = true;
	}
	delete dlg;
	return result;
}

