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


#include "newmacrodialog.h"
#include "application.h"
#include "macro.h"
#include <QMessageBox>

NewMacroDialog::NewMacroDialog(QWidget *parent, Qt::WindowFlags fl)
	: QDialog(parent, fl), Ui::NewMacroDialog()
{
	setupUi(this);
#ifdef Q_OS_MAC
	prefix = "Meta+";
#else
	prefix = "Esc, ";
#endif
	connect(key, SIGNAL(textEdited(const QString &)), this, SLOT(keyChanged(const QString &)));
	editMode = false;
	macro = NULL;
}

NewMacroDialog::NewMacroDialog(Macro *m, QWidget *parent, Qt::WindowFlags fl)
	: QDialog(parent, fl), Ui::NewMacroDialog()
{
	setupUi(this);
#ifdef Q_OS_MAC
	prefix = "Meta+";
#else
	prefix = "Esc,";
#endif
	connect(key, SIGNAL(textEdited(const QString &)), this, SLOT(keyChanged(const QString &)));

	name->setText(m->name());
	QString shortCut = m->keySequence();

	shortCut = shortCut.remove(prefix).trimmed();
	key->setText(shortCut);
	name->setFocus();
	editMode = true;
#ifdef Q_OS_MAC
	label_3->setText("<b>Control+</b>");
#endif

	macro = m;
}

NewMacroDialog::~NewMacroDialog()
{
}

QString NewMacroDialog::macroName()
{
	return name->text();
}

QString NewMacroDialog::macroSequence()
{
	QString text = key->text().toUpper().trimmed();
	return QString(prefix) + text;
}

void NewMacroDialog::keyChanged(const QString &text)
{
	if (text.length() == 0) {
		return;
	}
	QChar c = text[text.length() - 1];
	QString txt = QString(c);
	if (txt.toLower() == QString::fromUtf8("й")) {
		txt = "Q";
	}
	if (txt.toLower() == QString::fromUtf8("ц")) {
		txt = "W";
	}
	if (txt.toLower() == QString::fromUtf8("у")) {
		txt = "E";
	}
	if (txt.toLower() == QString::fromUtf8("к")) {
		txt = "R";
	}
	if (txt.toLower() == QString::fromUtf8("е")) {
		txt = "T";
	}
	if (txt.toLower() == QString::fromUtf8("н")) {
		txt = "Y";
	}
	if (txt.toLower() == QString::fromUtf8("г")) {
		txt = "U";
	}
	if (txt.toLower() == QString::fromUtf8("ш")) {
		txt = "I";
	}
	if (txt.toLower() == QString::fromUtf8("щ")) {
		txt = "O";
	}
	if (txt.toLower() == QString::fromUtf8("з")) {
		txt = "P";
	}
	if (txt.toLower() == QString::fromUtf8("х")) {
		txt = "[";
	}
	if (txt.toLower() == QString::fromUtf8("ъ")) {
		txt = "]";
	}
	if (txt.toLower() == QString::fromUtf8("ф")) {
		txt = "A";
	}
	if (txt.toLower() == QString::fromUtf8("ы")) {
		txt = "S";
	}
	if (txt.toLower() == QString::fromUtf8("в")) {
		txt = "D";
	}
	if (txt.toLower() == QString::fromUtf8("а")) {
		txt = "F";
	}
	if (txt.toLower() == QString::fromUtf8("п")) {
		txt = "G";
	}
	if (txt.toLower() == QString::fromUtf8("р")) {
		txt = "H";
	}
	if (txt.toLower() == QString::fromUtf8("о")) {
		txt = "J";
	}
	if (txt.toLower() == QString::fromUtf8("л")) {
		txt = "K";
	}
	if (txt.toLower() == QString::fromUtf8("д")) {
		txt = "L";
	}
	if (txt.toLower() == QString::fromUtf8("ж")) {
		txt = ";";
	}
	if (txt.toLower() == QString::fromUtf8("э")) {
		txt = "'";
	}
	if (txt.toLower() == QString::fromUtf8("я")) {
		txt = "Z";
	}
	if (txt.toLower() == QString::fromUtf8("ч")) {
		txt = "X";
	}
	if (txt.toLower() == QString::fromUtf8("с")) {
		txt = "C";
	}
	if (txt.toLower() == QString::fromUtf8("м")) {
		txt = "V";
	}
	if (txt.toLower() == QString::fromUtf8("и")) {
		txt = "B";
	}
	if (txt.toLower() == QString::fromUtf8("т")) {
		txt = "N";
	}
	if (txt.toLower() == QString::fromUtf8("ь")) {
		txt = "M";
	}
	if (txt.toLower() == QString::fromUtf8("б")) {
		txt = ",";
	}
	if (txt.toLower() == QString::fromUtf8("ю")) {
		txt = ".";
	}
	key->setText(txt);
}


void NewMacroDialog::accept()
{
	QString title = tr("Error creating new macro");
	QString errorText;
	if (name->text().isEmpty()) {
		errorText = tr("Macro name cannot be empty. Please enter macro name.");
		QMessageBox::critical(this, title, errorText);
		name->setFocus();
		return;
	}
	if (key->text().isEmpty()) {
		errorText = tr("You forgot to assign macro hotkey.");
		QMessageBox::critical(this, title, errorText);
		key->setFocus();
		return;
	}
	QList<Macro *> macros = app()->macros + app()->standardMacros;
	foreach (Macro *m, macros) {
		QString newText = name->text().trimmed();
		QString mText = m->name();
		QString newKey = macroSequence();
		newKey.remove(" ");
		QString mKey = m->keySequence();
		mKey.remove(" ");
		bool sameMacro = macro != NULL && m == macro;
		if (newText == mText && !sameMacro) {
			errorText = tr("Macro with such name already exists. Please enter another macro name.");
			QMessageBox::critical(this, title, errorText);
			name->setFocus();
			return;
		}
		if (newKey == mKey && !sameMacro) {
			errorText = tr("This hotkey is already assigned to macro \"");
			errorText += mText;
			errorText += "\". ";
			errorText += tr("Please assign another key.");
			QMessageBox::critical(this, title, errorText);
			key->setFocus();
			return;
		}
	}

	QDialog::accept();
}
