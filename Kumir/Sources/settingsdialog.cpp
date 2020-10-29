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

#include "settingsdialog.h"
#include "application.h"
#include "mainwindow.h"
#include "tab.h"
#include "textedit.h"
#include "config.h"
#include "httpdaemon.h"
#include "compiler.h"

#include <QMessageBox>
#include <QStyleFactory>
#include <QFileDialog>
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget *parent, Qt::WindowFlags fl)
	: QDialog(parent, fl), Ui::SettingsDialog()
{
	setupUi(this);
#ifdef ONLY_RUSSIAN_LANGUAGE
	QString Language = tr("Language");
	QList<QListWidgetItem *> langItems = listWidget->findItems(Language, Qt::MatchFixedString);
	foreach (QListWidgetItem *item, langItems) {
		item->setHidden(true);
	}
#endif

	connect(fontComboBox, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(fontFamilyChanged(const QFont &)));
	connect(useBoldFont, SIGNAL(stateChanged(int)), this, SLOT(fontBoldChanged(int)));
	connect(sizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)));
	connect(btnBrowseLib, SIGNAL(clicked()), this, SLOT(browseStdLib()));
	connect(btnBrowseIO, SIGNAL(clicked()), this, SLOT(browseIO()));
	connect(btnBrowseHtml, SIGNAL(clicked()), this, SLOT(browseHtml()));
	connect(btnGetFromEditor, SIGNAL(clicked()), this, SLOT(getTextFromEditor()));
	connect(kwColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(typeColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(valueColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(stringColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(ispColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(nameColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(docColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(commColor, SIGNAL(clicked()), this, SLOT(chooseColor()));

	connect(lineColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(wallColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(editColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(normalColor, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(textColor, SIGNAL(clicked()), this, SLOT(chooseColor()));

	connect(fixedWidth, SIGNAL(stateChanged(int)), this, SLOT(fixedSizeChanged(int)));
	connect(highlightInput, SIGNAL(stateChanged(int)), this, SLOT(highlightInputChanged(int)));
	connect(btnBackground, SIGNAL(clicked()), this, SLOT(chooseColor()));
	connect(btnBrowseEditor, SIGNAL(clicked()), this, SLOT(browseExternalEditor()));

	connect(btnDefault, SIGNAL(clicked()), this, SLOT(setDefaultSettings()));

	textEdit->disconnect(SIGNAL(requestCompilation(int)));
	listWidget->setCurrentRow(0);
	QStringList styles = QStyleFactory::keys();
	QStyle *currentStyle = qApp->style();
	QString currentStyleName = currentStyle->objectName();
	styleComboBox->addItems(styles);
	for (int i = 0; i < styles.count(); i++) {
		if (styles[i].toLower() == currentStyleName.toLower()) {
			styleComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (IGNORE_STYLES_OPTION) {
		label_2->hide();
		styleComboBox->hide();
	}
	dictList->addItems(app()->languageList());
	QString languageName = app()->settings->value("Common/Language", QString::fromUtf8("Русский")).toString();
	if (app()->languageList().contains(languageName)) {
		dictList->setCurrentRow(app()->languageList().indexOf(languageName));
	}
	languageChanged = false;

	textEdit->setInSettingsEditor(true);
	setLabels();

}

void SettingsDialog::loadSettingsToDialog()
{
	legacyFileDialogs->setChecked(app()->settings->value("UI/LegacyFileDialog", false).toBool());
	libEdit->setText(QDir::toNativeSeparators(app()->settings->value("Directories/StdModules", "").toString()));
	ioEdit->setText(QDir::toNativeSeparators(app()->settings->value("Directories/IO", "").toString()));
	htmlEdit->setText(QDir::toNativeSeparators(app()->settings->value("HyperText/Path", app()->httpDaemon->defaultPath()).toString()));
	QString startText;
	if (qApp->arguments().contains("-e")) {
		startText = QString::fromUtf8("алг \nнач\n\nкон\n");
	} else {
		startText = QString::fromUtf8("| использовать Робот\nалг \nнач\n\nкон\n");
	}
	textEdit->setText(app()->settings->value("Start/Text", startText).toString());
	setBtnColor(kwColor, QColor(app()->settings->value("SyntaxHighlighter/Keyword/Color", "#000000").toString()));
	setBtnColor(typeColor, QColor(app()->settings->value("SyntaxHighlighter/Type/Color", "#C05800").toString()));
	setBtnColor(valueColor, QColor(app()->settings->value("SyntaxHighlighter/Value/Color", "#0095FF").toString()));
	setBtnColor(stringColor, QColor(app()->settings->value("SyntaxHighlighter/QuotedValue/Color", "#0095FF").toString()));
	setBtnColor(ispColor, QColor(app()->settings->value("SyntaxHighlighter/Module/Color", "#00AA00").toString()));
	setBtnColor(nameColor, QColor(app()->settings->value("SyntaxHighlighter/Algorhitm/Color", "#0000C8").toString()));
	setBtnColor(docColor, QColor(app()->settings->value("SyntaxHighlighter/Documentation/Color", "#A3ACFF").toString()));
	setBtnColor(commColor, QColor(app()->settings->value("SyntaxHighlighter/Comment/Color", "#888888").toString()));

	setBtnColor(lineColor, QColor(app()->settings->value("Robot/LineColor", "#C8C800").toString()));
	setBtnColor(wallColor, QColor(app()->settings->value("Robot/WallColor", "#C8C800").toString()));
	setBtnColor(editColor, QColor(app()->settings->value("Robot/EditColor", "#00008C").toString()));
	setBtnColor(normalColor, QColor(app()->settings->value("Robot/NormalColor", "#289628").toString()));
	setBtnColor(textColor, QColor(app()->settings->value("Robot/TextColor", "#FFFFFF").toString()));

	kwBold->setChecked(app()->settings->value("SyntaxHighlighter/Keyword/Bold", true).toBool());
	typeBold->setChecked(app()->settings->value("SyntaxHighlighter/Type/Bold", true).toBool());
	valueBold->setChecked(app()->settings->value("SyntaxHighlighter/Value/Bold", true).toBool());
	stringBold->setChecked(app()->settings->value("SyntaxHighlighter/QuotedValue/Bold", true).toBool());
	ispBold->setChecked(app()->settings->value("SyntaxHighlighter/Module/Bold", true).toBool());
	nameBold->setChecked(app()->settings->value("SyntaxHighlighter/Algorhitm/Bold", true).toBool());
	docBold->setChecked(app()->settings->value("SyntaxHighlighter/Documentation/Bold", true).toBool());
	commBold->setChecked(app()->settings->value("SyntaxHighlighter/Comment/Bold", false).toBool());
	kwItalic->setChecked(app()->settings->value("SyntaxHighlighter/Keyword/Italic", false).toBool());
	typeItalic->setChecked(app()->settings->value("SyntaxHighlighter/Type/Italic", false).toBool());
	valueItalic->setChecked(app()->settings->value("SyntaxHighlighter/Value/Italic", false).toBool());
	stringItalic->setChecked(app()->settings->value("SyntaxHighlighter/QuotedValue/Italic", false).toBool());
	ispItalic->setChecked(app()->settings->value("SyntaxHighlighter/Module/Italic", false).toBool());
	nameItalic->setChecked(app()->settings->value("SyntaxHighlighter/Algorhitm/Italic", false).toBool());
	docItalic->setChecked(app()->settings->value("SyntaxHighlighter/Documentation/Italic", true).toBool());
	commItalic->setChecked(app()->settings->value("SyntaxHighlighter/Comment/Italic", true).toBool());
	useBoldFont->setChecked(app()->settings->value("Appearance/FontBold", false).toBool());


	latinItalic->setChecked(app()->settings->value("SyntaxHighlighter/OverloadLatinFont", true).toBool());
	if (latinItalic->isChecked()) {
		kwItalic->setDisabled(true);
		typeItalic->setDisabled(true);
		valueItalic->setDisabled(true);
		stringItalic->setDisabled(true);
		ispItalic->setDisabled(true);
		nameItalic->setDisabled(true);
		docItalic->setDisabled(true);
		commItalic->setDisabled(true);

	}

	indentRatio->setValue(app()->settings->value("Editor/IndentRatio", 2.0).toDouble());

	fixedWidth->setChecked(app()->settings->value("Terminal/Fixed", true).toBool());
	fixedSize->setValue(app()->settings->value("Terminal/FixedSize", 80).toInt());
	highlightInput->setChecked(app()->settings->value("Terminal/HighlightInput", true).toBool());
	setBtnColor(btnBackground, QColor(app()->settings->value("Terminal/InputRectColor", "papayawhip").toString()));

	showLineNumbers->setChecked(app()->settings->value("Appearance/ShowLineNumbers", true).toBool());

	labelRobotStartField->setText(QDir::toNativeSeparators(
			app()->settings->value("Robot/StartField/File",
				QCoreApplication::applicationDirPath() + QDir::separator() + strRobotStartField).toString()));

	editorEdit->setText(QDir::toNativeSeparators(app()->settings->value("System/TextEditor", defaultTextEditor()).toString()));
	libEdit->setText(QDir::toNativeSeparators(app()->settings->value("Directories/StdLib",
				QApplication::applicationDirPath() + "/Kumir/Include").toString()));
}

QString SettingsDialog::defaultTextEditor()
{
	QString defaultEditor;
#ifdef Q_OS_WIN32
	if (QFile::exists(QCoreApplication::applicationDirPath() + "/Notepad++/notepad++.exe")) {
		defaultEditor = QCoreApplication::applicationDirPath() + "/Notepad++/notepad++.exe";
	} else {
		defaultEditor = "notepad.exe";
	}
#endif
#ifdef Q_OS_UNIX
	bool isKDE = QProcess::systemEnvironment().contains("KDE_FULL_SESSION=true", Qt::CaseInsensitive);
	bool isGNOME = QProcess::systemEnvironment().contains("DESKTOP_SESSION=gnome", Qt::CaseInsensitive);
	bool isXFCE = QProcess::systemEnvironment().contains("DESKTOP_SESSION=xfce", Qt::CaseInsensitive);
	bool isLXDE = QProcess::systemEnvironment().contains("DESKTOP_SESSION=lxde", Qt::CaseInsensitive);
	if (isKDE) {
		defaultEditor = "kwrite";
	} else if (isGNOME) {
		defaultEditor = "gedit";
	} else if (isXFCE) {
		defaultEditor = "mousepad";
	} else if (isLXDE) {
		defaultEditor = "leafpad";
	} else {
		defaultEditor = "emacs";
	}
#endif
#ifdef Q_OS_MAC
	// TODO !!!
#endif
	return defaultEditor;
}

void getMimeFilesList(const QString &startPoint, QStringList &l)
{
	QDir start(startPoint);
	QStringList entries = start.entryList();
	foreach (const QString &entry, entries) {
		if (entry == "." || entry == "..") {
			continue;
		}
		if (QFileInfo(entry).isDir()) {
			getMimeFilesList(start.absoluteFilePath(entry), l);
		} else if (entry.endsWith(".desktop")) {
			l << start.absoluteFilePath(entry);
		}
	}
}

void SettingsDialog::setLabels()
{
	if (!app()->configPath().isEmpty()) {
		configPath->setText(tr("Configuration will be stored in") + " " + app()->configPath());
	} else {
		configPath->setVisible(false);
	}
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::fontFamilyChanged(const QFont &font)
{
	int pointSize = previewTextEdit->font().pointSize();
	QFont newFont = font;
	newFont.setPointSize(pointSize);
	newFont.setBold(useBoldFont->isChecked());
	previewTextEdit->setFont(newFont);
	QTextCharFormat f;
	f.setFont(newFont);
	QTextCursor c(previewTextEdit->document());
	c.select(QTextCursor::Document);
	c.setCharFormat(f);
}

void SettingsDialog::fontBoldChanged(int state)
{
	Q_UNUSED(state);
	int pointSize = previewTextEdit->font().pointSize();
	QFont font = previewTextEdit->font();
	QFont newFont = font;
	newFont.setPointSize(pointSize);
	newFont.setBold(useBoldFont->isChecked());
	previewTextEdit->setFont(newFont);
	QTextCharFormat f;
	f.setFont(newFont);
	QTextCursor c(previewTextEdit->document());
	c.select(QTextCursor::Document);
	c.setCharFormat(f);
}

void SettingsDialog::fontSizeChanged(int size)
{
	QFont font = previewTextEdit->font();
	font.setPointSize(size);
	font.setBold(useBoldFont->isChecked());
	previewTextEdit->setFont(font);
	QTextCharFormat f;
	f.setFont(font);
	QTextCursor c(previewTextEdit->document());
	c.select(QTextCursor::Document);
	c.setCharFormat(f);
}

void SettingsDialog::accept()
{
	app()->settings->setValue("UI/LegacyFileDialog", legacyFileDialogs->isChecked());
	app()->settings->setValue("Appearance/Style", styleComboBox->currentText());
	app()->settings->setValue("Appearance/Font", fontComboBox->currentFont().family());
	app()->settings->setValue("Appearance/FontSize", sizeSpinBox->value());
	app()->settings->setValue("Appearance/FontBold", useBoldFont->isChecked());
	if (useBoldFont->isChecked()) {
		QFont fnt = app()->mainWindow->editorsFont();
		fnt.setBold(true);
		app()->mainWindow->setEditorsFont(fnt);
	} else {
		QFont fnt = app()->mainWindow->editorsFont();
		fnt.setBold(false);
		app()->mainWindow->setEditorsFont(fnt);
	}

	app()->settings->setValue("Directories/StdModules", QDir::fromNativeSeparators(libEdit->text()));
	app()->settings->setValue("Directories/IO", QDir::fromNativeSeparators(ioEdit->text()));
	app()->settings->setValue("HyperText/Path", QDir::fromNativeSeparators(htmlEdit->text()));

	qApp->setStyle(QStyleFactory::create(styleComboBox->currentText()));
	app()->mainWindow->setEditorsFont(previewTextEdit->font());
	if (dictList->currentItem() != NULL) {
		app()->settings->setValue("Common/Language", dictList->currentItem()->text());
		if (app()->currentLanguage() != dictList->currentItem()->text()) {
			QMessageBox::information(this, tr("Kumir"), tr("Language will be changed at next Kumir start."));
		}
	}
	if (stdLibPathChanged) {
		app()->doCompilation();
		stdLibPathChanged = false;
	}
	app()->settings->setValue("Start/Text", textEdit->toPlainText());

	app()->settings->setValue("SyntaxHighlighter/Keyword/Color", btnColor(kwColor));
	app()->settings->setValue("SyntaxHighlighter/Type/Color", btnColor(typeColor));
	app()->settings->setValue("SyntaxHighlighter/Value/Color", btnColor(valueColor));
	app()->settings->setValue("SyntaxHighlighter/QuotedValue/Color", btnColor(stringColor));
	app()->settings->setValue("SyntaxHighlighter/Module/Color", btnColor(ispColor));
	app()->settings->setValue("SyntaxHighlighter/Algorhitm/Color", btnColor(nameColor));
	app()->settings->setValue("SyntaxHighlighter/Documentation/Color", btnColor(docColor));
	app()->settings->setValue("SyntaxHighlighter/Comment/Color", btnColor(commColor));

	app()->settings->setValue("Robot/LineColor", btnColor(lineColor));
	app()->settings->setValue("Robot/WallColor", btnColor(wallColor));
	app()->settings->setValue("Robot/EditColor", btnColor(editColor));
	app()->settings->setValue("Robot/NormalColor", btnColor(normalColor));
	app()->settings->setValue("Robot/TextColor", btnColor(textColor));


	app()->settings->setValue("Terminal/InputRectColor", btnColor(btnBackground));

	app()->settings->setValue("SyntaxHighlighter/Keyword/Bold", kwBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Type/Bold", typeBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Value/Bold", valueBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/QuotedValue/Bold", stringBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Module/Bold", ispBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Algorhitm/Bold", nameBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Documentation/Bold", docBold->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Comment/Bold", commBold->isChecked());

	app()->settings->setValue("SyntaxHighlighter/Keyword/Italic", kwItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Type/Italic", typeItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Value/Italic", valueItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/QuotedValue/Italic", stringItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Module/Italic", ispItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Algorhitm/Italic", nameItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Documentation/Italic", docItalic->isChecked());
	app()->settings->setValue("SyntaxHighlighter/Comment/Italic", commItalic->isChecked());

	app()->settings->setValue("Terminal/Fixed", fixedWidth->isChecked());
	app()->settings->setValue("Terminal/FixedSize", fixedSize->value());
	app()->settings->setValue("Terminal/HighlightInput", highlightInput->isChecked());
	app()->settings->setValue("Terminal/HighlightColorDone", btnColor(btnBackground));


	app()->settings->setValue("SyntaxHighlighter/OverloadLatinFont", latinItalic->isChecked());


	app()->settings->setValue("Appearance/ShowLineNumbers", showLineNumbers->isChecked());



	app()->settings->setValue("Editor/IndentRatio", indentRatio->value());

	app()->mainWindow->updateSHAppearance();

	QTabWidget *tabWidget = app()->mainWindow->tabWidget;
	for (int i = 0 ; i < tabWidget->count(); i++) {
		Tab *tab = dynamic_cast<Tab *>(tabWidget->widget(i));
		TextEdit *editor = tab->editor();
		if (!app()->settings->value("Editor/LegacyIndent", false).toBool()) {
			editor->clearIndents();
		}
		editor->applyIndentation();
		editor->repaint();
	}

	app()->settings->setValue("System/TextEditor", QDir::fromNativeSeparators(editorEdit->text()));
	app()->settings->setValue("Directories/StdLib", QDir::fromNativeSeparators(libEdit->text()));

	app()->compiler->loadKumirModules();
	app()->doCompilation();

	QDialog::accept();
}


void SettingsDialog::reject()
{
	QDialog::reject();
}

void SettingsDialog::browseStdLib()
{
	QString newPath = QFileDialog::getExistingDirectory(this, tr("Select standart modules directory..."), libEdit->text(), QFileDialog::ShowDirsOnly);
	if (!newPath.isEmpty()) {
		if (newPath != libEdit->text()) {
			stdLibPathChanged = true;
		}
		libEdit->setText(QDir::toNativeSeparators(newPath));
	}
}

void SettingsDialog::getTextFromEditor()
{
	QString text = app()->mainWindow->currentEditorText();
	textEdit->setText(text);
}

void SettingsDialog::chooseColor()
{
	QToolButton *src = (QToolButton *)sender();
	QColor newColor = QColorDialog::getColor(btnColor(src), this);
	if (newColor.isValid()) {
		setBtnColor(src, newColor);
	}

}

QColor SettingsDialog::btnColor(QToolButton *btn)
{
	//  QPalette palette = btn->palette();
	QString style = btn->styleSheet();
	QRegExp rxBgColor("background-color:\\s*(\\S+)");
	if (rxBgColor.indexIn(style) != -1) {
		QString color = rxBgColor.cap(1);
		return QColor(color);
	} else {
		return QColor("white");
	}
}

void SettingsDialog::setBtnColor(QToolButton *btn, QColor color)
{
	QString style = QString("QToolButton { border: 1px solid ") + color.darker().name() + QString("; ") +
		QString("              border-radius: 4px; ") +
		QString("              background-color: ") + color.name() +
		QString("            }") +
		QString("QToolButton::pressed { background-color: ") + color.darker(50).name() + QString(" }");

	btn->setStyleSheet(style);
}

void SettingsDialog::browseIO()
{
	QString newPath = QFileDialog::getExistingDirectory(this, tr("Select input/output directory..."), ioEdit->text(), QFileDialog::ShowDirsOnly);
	if (!newPath.isEmpty()) {
		ioEdit->setText(QDir::toNativeSeparators(newPath));
	}
}

void SettingsDialog::browseHtml()
{
	QString newPath = QFileDialog::getExistingDirectory(this, tr("Select input/output directory..."), htmlEdit->text(), QFileDialog::ShowDirsOnly);
	if (!newPath.isEmpty()) {
		htmlEdit->setText(QDir::toNativeSeparators(newPath));
	}
}


void SettingsDialog::fixedSizeChanged(int v)
{
	fixedSize->setEnabled(v == Qt::Checked);
}

void SettingsDialog::highlightInputChanged(int v)
{
	btnBackground->setEnabled(v == Qt::Checked);
	label_17->setEnabled(v == Qt::Checked);
}

int SettingsDialog::exec()
{
	loadSettingsToDialog();
	previewTextEdit->setText(tr("The Quick Brown Fox Jumps Over The Lazy Dog"));
	QFont currentFont = app()->mainWindow->editorsFont();
	fontComboBox->setCurrentFont(currentFont);
	sizeSpinBox->setValue(currentFont.pointSize());
	int langIndex = -1;
	for (int i = 0; i < dictList->count(); i++) {
		if (dictList->item(i)->text() == app()->currentLanguage()) {
			langIndex = i;
			break;
		}
	}
	if (langIndex > -1) {
		dictList->setCurrentRow(langIndex);
	}
	previewTextEdit->setFont(currentFont);

	int i;
	do {
		i = fontComboBox->findText(QString::fromUtf8("Mincho"), Qt::MatchContains);
		if (i != -1) {
			fontComboBox->removeItem(i);
		}
	} while (i != -1);
	do {
		i = fontComboBox->findText(QString::fromUtf8("Terminal"), Qt::MatchContains);
		if (i != -1) {
			fontComboBox->removeItem(i);
		}
	} while (i != -1);
	do {
		i = fontComboBox->findText(QString::fromUtf8("Fixedsys"), Qt::MatchContains);
		if (i != -1) {
			fontComboBox->removeItem(i);
		}
	} while (i != -1);

	labelRobotStartField->setText(QDir::toNativeSeparators(app()->settings->value("Robot/StartField/File", QCoreApplication::applicationDirPath() + "/Kumir/Environments/" + strRobotStartField).toString()));
	return QDialog::exec();
}


void SettingsDialog::setDefaultSettings()
{
	if (QMessageBox::question(this, tr("Default settings"), tr("Are you sure you want to revert to default settings?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		foreach (QString s, app()->settings->allKeys())
			if (s != "Common/Language") {
				app()->settings->remove(s);
			}
		app()->settings->setValue("Common/SetDefaultSettingsAtFirstRun", true);
		QMessageBox::information(this, tr("Kumir"), tr("Settings will be reverted at next Kumir start."));
	}
}

void SettingsDialog::browseExternalEditor()
{
	QString editor = editorEdit->text().trimmed();
	QString filter;
#ifdef Q_OS_WIN32
	filter = tr("Applications (*.exe)");
#endif
#ifdef Q_OS_UNIX
	filter = tr("Applications (*)");
#endif
	editor = QFileDialog::getOpenFileName(this,
			tr("Select external text editor"),
			editor,
			filter);
	if (!editor.isEmpty()) {
		editorEdit->setText(QDir::toNativeSeparators(editor));
	}
}
