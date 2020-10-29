//
// C++ Implementation:
//
// Description:
//
//
// Author: Виктор Яковлев <V.Yacovlev@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "textwindow.h"
#include "kumfiledialog.h"
#include "application.h"
#include <QMessageBox>
#include <QCloseEvent>

QString TextWindow::fakeLabel()
{
	return tr("New text");
}

TextWindow::TextWindow(QWidget *parent, Qt::WindowFlags fl)
	: QMainWindow(parent, fl), Ui::TextWindow1()
{
	setupUi(this);
	restoreGeometry(app()->settings->value("TextWindow/Geometry", "").toByteArray());
	move(x() + 25, y() + 25);
	app()->settings->setValue("TextWindow/Geometry", saveGeometry());

	QString fntF = app()->settings->value("Appearance/Font", "").toString();
	int fntS = app()->settings->value("Appearance/FontSize", 0).toInt();
	bool boldFont = app()->settings->value("Appearance/FontBold", false).toBool();
	if (!fntF.isEmpty() && fntS > 0) {
		QFont edFont(fntF, fntS);
		edFont.setBold(boldFont);
		editor->setFont(edFont);
	}

	connect(editor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
	connect(editor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
	actionCut->setEnabled(false);
	actionCopy->setEnabled(false);

	connect(actionCut, SIGNAL(triggered()), editor, SLOT(cut()));
	connect(actionCopy, SIGNAL(triggered()), editor, SLOT(copy()));
	connect(actionPaste, SIGNAL(triggered()), editor, SLOT(paste()));

	connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

	connect(actionUndo, SIGNAL(triggered()), editor, SLOT(undo()));
	connect(actionRedo, SIGNAL(triggered()), editor, SLOT(redo()));

	connect(editor, SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
	connect(editor, SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));

	actionUndo->setEnabled(false);
	actionRedo->setEnabled(false);

	actionSave->setEnabled(false);

	connect(editor, SIGNAL(textChanged()), this, SLOT(checkChanges()));

	QString label(fakeLabel());
	int n = 0;
	bool flag;
	do {
		flag = false;
		++n;
		for (int i = 0; i < app()->secondaryWindows.count(); ++i) {
			QString s(app()->secondaryWindows[i]->windowTitle());
			if ((s.startsWith(label) || "*" + s.startsWith(label)) && s.endsWith(" " + QString::number(n))) {
				flag = true;
			}
		}
	} while (flag);
	label += " " + QString::number(n);
	setFileName(label);

	app()->secondaryWindows << this;

}

TextWindow::~TextWindow()
{
}

void TextWindow::enterEvent(QEvent *event)
{
	if (QApplication::overrideCursor()) {
		QApplication::restoreOverrideCursor();
	}
	QWidget::enterEvent(event);
}

bool TextWindow::save()
{
	if (m_fileDir.isEmpty()) {
		return saveAs();
	} else {
		return saveFileTo(QDir(m_fileDir).absoluteFilePath(m_fileName), m_textEncoding);
	}
}

bool TextWindow::saveAs()
{
	QString dir = m_fileDir.isEmpty() ? app()->settings->value("History/KumDir").toString() : m_fileDir;
	KumFileDialog *saveDialog = new KumFileDialog(this, tr("Save text as..."), QDir(dir).absoluteFilePath(m_fileName), tr("Text files (*.txt)"), true);
	saveDialog->setEncoding(m_textEncoding);
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	bool result = false;
	if (saveDialog->exec() == QDialog::Accepted && !saveDialog->selectedFiles().isEmpty()) {
		QString fileName = saveDialog->selectedFiles().first();
		if (!fileName.isEmpty()) {
			app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
			if (!fileName.endsWith(".txt")) {
				fileName += ".txt";
			}
			result = saveFileTo(fileName, saveDialog -> encoding());
		}
	}
	delete saveDialog;
	return result;
}

bool TextWindow::saveFileTo(const QString &path, const QString &enc)
{
	QFile f(path);
	QTextStream ts(&f);
	if (f.open(QIODevice::WriteOnly)) {
		ts.setCodec(enc.toUtf8());
		ts << editor->toPlainText();
		f.close();
		m_fileDir = QFileInfo(path).dir().absolutePath();
		m_fileName = path;
		m_fileName.remove(0, m_fileDir.length() + 1);
		editor->setModified(false);
		checkChanges();
		return true;
	} else {
		QMessageBox::critical(this,
			tr("Error saving file"),
			tr("There is an error occured while writing file.\nPossible reason is out of space on drive or you do not have write access to the selected directory."));
		return false;
	}
}

void TextWindow::setFileName(const QString &fileName)
{

	m_fileName = fileName;
	setWindowTitle(fileName);
}

void TextWindow::checkChanges()
{
	if (editor->isModified()) {
		actionSave->setEnabled(true);
		setWindowTitle("*" + m_fileName);
	} else {
		actionSave->setEnabled(false);
		setWindowTitle(m_fileName);
	}
}

void TextWindow::closeEvent(QCloseEvent *event)
{

	bool allowClose = true;

	if (editor->isModified()) {
		QMessageBox::StandardButton result = QMessageBox::question(this, tr("Close text window"),
				tr("You have modified text, but not saved. Save it?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Save);
		if (result == QMessageBox::Save) {
			allowClose = save();
		} else if (result == QMessageBox::Discard) {
			allowClose = true;
		} else {
			allowClose = false;
		}
	}

	if (allowClose) {
		app()->secondaryWindows.removeAll(this);
		app()->settings->setValue("TextWindow/Geometry", saveGeometry());
		QWidget::closeEvent(event);
	} else {
		event->ignore();
	}
}

void TextWindow::open(QString fileName)
{
	bool allowClose = true;

	if (editor->isModified()) {
		QMessageBox::StandardButton result = QMessageBox::question(this, tr("Close text window"),
				tr("You have modified text, but not saved. Save it?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Save);
		if (result == QMessageBox::Save) {
			allowClose = save();
		} else if (result == QMessageBox::Discard) {
			allowClose = true;
		} else {
			allowClose = false;
		}
	}

	if (!allowClose) {
		return;
	}

	QString enc = "UTF-8";
	if (fileName.isEmpty()) {
		KumFileDialog *openDialog;

		QString defaultDir = QDir::currentPath();
		QDir lastDir = QDir(app()->settings->value("History/KumDir", defaultDir).toString());
		QString lastDirPath = lastDir.absolutePath().replace("/", QDir::separator()) + QDir::separator();
		openDialog = new KumFileDialog(this, tr("Open..."), lastDirPath, tr("Text files (*.txt);;All files (*.*)"), true);
		if (openDialog->exec() == QDialog::Accepted) {
			if (! openDialog->selectedFiles().isEmpty()) {
				fileName = openDialog->selectedFiles().first();
				bool flag = false;
				foreach (QWidget *pqw, app()->secondaryWindows) {
					QString fn = pqw->windowTitle();
					if (fn == QFileInfo(fileName).fileName() || fn == "*" + QFileInfo(fileName).fileName())
						if (pqw != this) {
							flag = true;
						}
				}
				if (flag) {
					fileName = "";
					QMessageBox::warning(this,
						tr("Error opening text file"),
						tr("File already opened in another text editor"));
				} else {
					if (openDialog->encoding() != "") {
						enc = openDialog->encoding();
					}
					app()->settings->setValue("History/KumDir", QFileInfo(fileName).dir().absolutePath());
				}
			}
		}
		delete openDialog;
	}

	if (!fileName.isEmpty()) {
		QFile f(fileName);
		QTextStream ts(&f);
		if (!f.open(QIODevice::ReadOnly)) {
			QMessageBox::critical(this,
				tr("Error opening text file"),
				tr("There is an error occured while reading file.\nCheck your access rights to this file."));
			//emit sendMessage(tr("Error opening file"),3000);
			return;
		}
		ts.setCodec(enc.toUtf8());
		QString data = ts.readAll();
		//TextTab *tab = dynamic_cast<TextTab*>(tabWidget->currentWidget());
		QFileInfo fi(fileName);
		//tabWidget->setTabText(tabWidget->currentIndex(),fi.fileName());
		//setWindowTitle(fi.fileName()+" - "+tr("Kumir"));
		//tab->setFileName(fileName);
		editor->setText(data);
		//emit sendMessage(tr("Text loaded")+" ("+enc+")",3000);
		//tabWidget->setTabToolTip(tabWidget->currentIndex(), fileName);
		setFileName(fi.fileName());
		m_fileDir = fi.absolutePath();
		m_textEncoding = enc;
		editor->setModified(false);
		//qDebug()<<m_fileName<<m_fileDir<<m_textEncoding;
	}

}

