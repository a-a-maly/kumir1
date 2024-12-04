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

#include "KumIOArea.h"
#include "application.h"
#include "kumfiledialog.h"
#include "application.h"
#include "integeroverflowchecker.h"
#include "doubleoverflowchecker.h"
#include <QScrollBar>
#include <QMenu>
#include <QKeyEvent>
#include <QSettings>
#include <QDateTime>
#include <QPainter>
#include <QTextCodec>
#include <QMimeData>
#include <QApplication>

#define FC_PLAIN char(0x00)
#define FC_INPUT char(0x01)
#define FC_ERROR_FLAG char(0x10)
#define FC_LATIN_FLAG char(0x20)

InputHighlighter::InputHighlighter(KumIOArea *editor)
	: QSyntaxHighlighter(editor)
{
	this->editor = editor;
	rx_latinSymbols = QRegExp("[a-zA-Z]+");
	cf_input.setForeground(QColor("blue"));
}

QByteArray InputHighlighter::generateFormatString(const QString &text, InputUserData *d)
{
	QByteArray result(text.size(), FC_PLAIN);
	if (d && d->highlightable && editor->waitForInput) {
		for (int i = editor->inputStartPosition; i < text.size(); i++) {
			result[i] = FC_INPUT;
		}
	}
	if (d && d->highlightable && editor->waitForInput) {
		foreach (InputTerm word, editor->words) {
			if (word.start > -1) {
				if (!word.valid) {
					for (int i = editor->inputStartPosition + word.start;
						i < editor->inputStartPosition + word.start + word.text.length();
						i++) {
						result[i] = result[i] | FC_ERROR_FLAG;
					}
				}
			}
		}
		if (editor->inputGarbageStart > -1) {
			int start = editor->inputStartPosition + editor->inputGarbageStart;
			for (int i = start; i < text.length(); i++) {
				result[i] = result[i] | FC_ERROR_FLAG;
			}
		}
	}
	if (app()->settings->value(
			"SyntaxHighlighter/OverloadLatinFont",
			true).toBool()) {
		int pos = 0;
		int len = 0;
		while (pos >= 0) {
			pos = rx_latinSymbols.indexIn(text, pos);
			if (pos != -1) {
				len = rx_latinSymbols.matchedLength();
				for (int i = pos; i < pos + len; i++) {
					result[i] = result[i] | FC_LATIN_FLAG;
				}
				pos += len;
			}
		}
	}
	return result;
}


QList<InputHighlighter::Block> InputHighlighter::splitFormatString(const QByteArray &in)
{
	QList<Block> result;
	for (int i = 0; i < in.size(); i++) {
		if (result.isEmpty()) {
			result << Block(0, 1, in[i]);
		} else {
			if (in[i] == result.last().val) {
				result.last().len++;
			} else {
				result << Block(i, 1, in[i]);
			}
		}
	}
	return result;
}

void InputHighlighter::highlightBlock(const QString &text)
{
	InputUserData *d = static_cast<InputUserData *>(currentBlockUserData());
	QByteArray format = generateFormatString(text, d);
	QList<Block> blocks = splitFormatString(format);
	foreach (const Block &b, blocks) {
		QTextCharFormat f;
		char baseval = b.val & 0x0F;
		switch (baseval) {
		case FC_INPUT:
			f = cf_input;
			break;
		default:
			f = cf_plain;
			break;
		}
		if (b.val & FC_ERROR_FLAG) {
			f.setUnderlineColor(Qt::red);
			f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
		}
		if (b.val & FC_LATIN_FLAG) {
			f.setFontItalic(true);
		}
		setFormat(b.pos, b.len, f);
	}
}

KumIOArea::KumIOArea(QWidget *parent)
	: QTextEdit(parent)
{

	inputStartPosition = 0;
	waitForInput = false;
	waitForKey = false;


	setLineWrapMode(QTextEdit::NoWrap);
	integerValidator = new QIntValidator(this);
	doubleValidator = new QDoubleValidator(this);
	characterValidator = new QRegExpValidator(QRegExp("."), this);
	booleanValidator = new QRegExpValidator(QRegExp(tr("yes") + "|" + tr("no"), Qt::CaseInsensitive), this);
	stringValidator = new QRegExpValidator(QRegExp(".*"), this);
	QImage keyImage(":/icons/32x32_key.png");
	document()->addResource(QTextDocument::ImageResource, QUrl("key"), keyImage);
	setReadOnly(true);
	sh = new InputHighlighter(this);
	setAcceptDrops(false);
}

void KumIOArea::scanKey(bool ignoreShiftCommandControl)
{



	setFocus();
	waitForKey = true;
	ignoreModifiers = ignoreShiftCommandControl;
	emit sendMessage(tr("Waiting for key pressed"), 0);
}

void KumIOArea::reset(const QString &fileName, bool isTestingMode)
{
	updateLastBlockBottomMargin();
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	runningFilename = fileName;
	inputStartPosition = 0;
	waitForInput = false;
	waitForKey = false;

	QTextCursor c(document());
	c.movePosition(QTextCursor::End);

	if (c.columnNumber() > 0) {
		c.insertText("\n");
	}

	for (QTextBlock bl = document()->begin();
		bl != document()->end();
		bl = bl.next()) {
		if (bl.userData() != NULL) {
			InputUserData *d = static_cast<InputUserData *>(bl.userData());
			if (d != NULL) {
				d->highlightable = false;
			}
		}
	}

	QDateTime dateTime = QDateTime::currentDateTime();
	QTextCharFormat fmt = c.charFormat();
	QTextBlockFormat bfmt = c.blockFormat();
	QTextCharFormat small;
	QTextBlockFormat bsmall;
	small.setForeground(QColor("gray"));
	bsmall.setTopMargin(7);
	c.mergeCharFormat(small);
	c.mergeBlockFormat(bsmall);
	QString dateLine = ">> ";
	QString s(isTestingMode ? tr("Testing started") : tr("Program started"));
	if (!app()->isBatchMode()) {
		dateLine += dateTime.time().toString("HH:mm:ss") + " - " + fileName + " - " + s;
	} else {
		dateLine += fileName;
	}
	QRect r = cursorRect(c);
	int y = r.top();
//	int hh = height();
//	int h = verticalScrollBar()->maximum();
//	int v = verticalScrollBar()->value();
//	double ratio = (double)v / (double)h;
	scrollContentsBy(0, y);
	c.insertText(dateLine);
	QTextCharFormat normal;
	normal.setForeground(QColor("black"));
	c.mergeCharFormat(normal);
	c.insertText("\n");


	c.setBlockFormat(bfmt);
	setTextCursor(c);
	ensureCursorVisible();
	setReadOnly(true);
	setCursorWidth(1);

}


void KumIOArea::stop(const QString &reason, const QString &color)
{
	inputStartPosition = 0;
	waitForInput = false;

	QTextFrameFormat frameFormat;
	frameFormat.setBorder(3);
	ensureCursorVisible();
	setReadOnly(true);
	setCursorWidth(1);

	QTextCursor c(document());
	c.movePosition(QTextCursor::End);
	if (c.columnNumber() > 0) {
		c.insertText("\n");
	}
	QDateTime dateTime = QDateTime::currentDateTime();
	QTextCharFormat fmt = c.charFormat();
	QTextBlockFormat bfmt = c.blockFormat();
	QTextCharFormat small;
	QTextBlockFormat bsmall;
	small.setForeground(QColor(color));
	bsmall.setTopMargin(15);
	c.mergeCharFormat(small);
	c.mergeBlockFormat(bsmall);
	QString dateLine = ">> ";
	if (!app()->isBatchMode()) {
		dateLine += dateTime.time().toString("HH:mm:ss") + " - " + runningFilename + " - " + reason;
	} else {
		dateLine += reason;
	}
	c.insertText(dateLine);
	if (c.block().userData() != NULL) {
		InputUserData *d = static_cast<InputUserData *>(c.block().userData());
		Q_CHECK_PTR(d);
		d->highlightable = false;
	}
	QTextCharFormat normal;
	normal.setForeground(QColor("black"));
	c.mergeCharFormat(normal);

	c.setBlockFormat(bfmt);
	setTextCursor(c);
	ensureCursorVisible();


	runSeparators << c.blockNumber();
	horizontalScrollBar()->setValue(0);

	if (runSeparators.count() > 1) {
//		saveAll->setEnabled(true);
	}
//  uint nnn = 0;
//  for (QTextBlock bl = document()->begin();
//              bl != document()->end();
//              bl = bl.next() )
//  {
//      QString st = "NULL";
//      if ( bl.userData()!=NULL ) {
//          InputUserData* d;
//          st = d->highlightable? "true" : "false";
//      }
//      qDebug() << nnn << " " << bl.text().toUtf8().data() << st;
//      nnn++;
//  }
//
	updateLastBlockBottomMargin();
	forceRepaint();
}

void KumIOArea::paintEvent(QPaintEvent *event)
{

	QPainter painter(viewport());

	QTextBlock bl;

	for (bl = document()->begin(); bl != document()->end(); bl = bl.next()) {
		QTextCursor c(bl);
		if (runSeparators.contains(c.blockNumber())) {
			QRect r = cursorRect(c);
			painter.drawLine(0, r.bottom(), width(), r.bottom());
		}
	}

	if (waitForInput && app()->settings->value("Terminal/HighlightInput", true).toBool()) {
		QTextCursor c = textCursor();
		c.movePosition(QTextCursor::End);
		c.movePosition(QTextCursor::StartOfBlock);
		c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, inputStartPosition);
		QRect r = cursorRect(c);
		// http://www.w3.org/TR/SVG/types.html#ColorKeywords
		painter.setBrush(QBrush(QColor(app()->settings->value("Terminal/InputRectColor", "papayawhip").toString())));
		painter.setPen(Qt::NoPen);
		QFontMetrics fm(font());
		int x = r.left();// + fm.width("w")/2;
		painter.drawRect(x, r.top(), width() - x, r.height());
	}
	painter.end();
	QTextEdit::paintEvent(event);
}

void KumIOArea::keyPressEvent(QKeyEvent *event)
{
	if (waitForKey) {
		event->accept();
		uint code = event->key();
		if (ignoreModifiers && (code == Qt::Key_Shift || code == Qt::Key_Control || code == Qt::Key_Meta || code == Qt::Key_Alt || code == Qt::Key_AltGr)) {
			return;
		}
		bool shift = event->modifiers() & Qt::ShiftModifier;
		bool control = event->modifiers() & Qt::ControlModifier;
		bool alt = event->modifiers() & Qt::AltModifier;
		bool meta = event->modifiers() & Qt::MetaModifier;
		waitForKey = false;
		emit sendMessage("", 0);
		emit scanKeyFinished(code, shift, control, alt, meta);
		return;
	}
	if (!waitForInput) {
		if (event->key() == Qt::Key_Home) {
			event->accept();
			if (event->modifiers() & Qt::ControlModifier) {
				verticalScrollBar()->setValue(0);
			}
		} else if (event->key() == Qt::Key_End) {
			event->accept();
			if (event->modifiers() & Qt::ControlModifier) {
				verticalScrollBar()->setValue(verticalScrollBar()->maximum());
			}
		} else {
			QTextEdit::keyPressEvent(event);
		}
		return;
	} else {
		int curBlock = textCursor().blockNumber();
		int curCol = textCursor().columnNumber();
		bool mustBlock = false;
		if (!(event->matches(QKeySequence::Copy) || event->matches(QKeySequence::SelectAll))) {
			mustBlock |= curBlock != document()->blockCount() - 1;
			mustBlock |= curCol < inputStartPosition;
			if (textCursor().hasSelection()) {
				QTextBlock startBlock = document()->findBlock(textCursor().selectionStart());
				QTextBlock endBlock = document()->findBlock(textCursor().selectionEnd());
				int sb = startBlock.blockNumber();
				int eb = endBlock.blockNumber();
				mustBlock |= sb != document()->blockCount() - 1;
				mustBlock |= eb != document()->blockCount() - 1;
				if ((sb == eb) && (sb == document()->blockCount() - 1)) {
					int startBlockPos = startBlock.position();
					int columnStart = textCursor().selectionStart() - startBlockPos;
					int columnEnd = textCursor().selectionEnd() - startBlockPos;
					mustBlock |= columnStart < inputStartPosition;
					mustBlock |= columnEnd < inputStartPosition;
				}
			}
		}
		if
//                        ( (curBlock != document()->blockCount()-1 || curCol < inputStartPosition) &&
//						!(event->key()==Qt::Key_Control || event->matches(QKeySequence::Copy)) )
		(mustBlock) {
			event->accept();
			QTextCursor c = textCursor();
			c.movePosition(QTextCursor::End);
			setTextCursor(c);
			return;
		}
		if (event->matches(QKeySequence::SelectAll)) {
			event->accept();
			QTextCursor c = textCursor();
			c.movePosition(QTextCursor::StartOfBlock);
			c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, inputStartPosition);
			c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
			setTextCursor(c);
		} else if (event->key() == Qt::Key_Left) {
			if (textCursor().columnNumber() > inputStartPosition) {
				QTextEdit::keyPressEvent(event);
			} else {
				event->accept();
			}
		} else if (event->key() == Qt::Key_Backspace) {
			if (textCursor().columnNumber() > inputStartPosition) {
				QTextEdit::keyPressEvent(event);
				validateInput();
				for (int i = 0; i < words.count(); i++) {
					words[i].valid = true;
				}
				inputGarbageStart = -1;
				sh->rehighlight();

			} else {
				event->accept();
			}
		} else if (event->key() == Qt::Key_Delete) {
			QTextEdit::keyPressEvent(event);
			validateInput();
			for (int i = 0; i < words.count(); i++) {
				words[i].valid = true;
			}
			inputGarbageStart = -1;
			sh->rehighlight();
		} else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
			event->accept();
			tryFinishInput();
		} else if (event->key() == Qt::Key_Up) {
			event->accept();
		} else if (event->key() == Qt::Key_Down) {
			event->accept();
		} else if (event->key() == Qt::Key_Home) {
			event->accept();
			bool selection = (event->modifiers() & Qt::ShiftModifier) > 0;
			int cp = textCursor().columnNumber();
			int tp = inputStartPosition;
			QTextCursor c = textCursor();
			c.movePosition(QTextCursor::Left, selection ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, cp - tp);
			setTextCursor(c);
		} else if (event->text().length() > 0) {
			QTextEdit::keyPressEvent(event);
			validateInput();
			for (int i = 0; i < words.count(); i++) {
				words[i].valid = true;
			}
			inputGarbageStart = -1;
			sh->rehighlight();
		} else if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
			event->accept();
		} else {
			if (event->key() == Qt::Key_F2 ||
				event->key() == Qt::Key_F3 ||
				event->key() == Qt::Key_F4 ||
				event->key() == Qt::Key_F5 ||
				event->key() == Qt::Key_F6 ||
				event->key() == Qt::Key_F7 ||
				event->key() == Qt::Key_F8 ||
				event->key() == Qt::Key_F9 ||
				event->key() == Qt::Key_F10 ||
				event->key() == Qt::Key_F11 ||
				event->key() == Qt::Key_F12 ||
				event->key() == Qt::Key_F13 ||
				event->key() == Qt::Key_F14 ||
				event->key() == Qt::Key_F15 ||
				event->key() == Qt::Key_F16 ||
				event->key() == Qt::Key_Shift ||
				event->key() == Qt::Key_Control ||
				event->key() == Qt::Key_Meta ||
				event->key() == Qt::Key_Alt ||
				event->key() == Qt::Key_AltGr ||
				event->key() == Qt::Key_CapsLock ||
				event->key() == Qt::Key_Insert ||
				event->key() == Qt::Key_ScrollLock ||
				event->key() == Qt::Key_NumLock ||
				event->key() == Qt::Key_Pause) {
				return;
			}

			QTextEdit::keyPressEvent(event);
		}
	}
}


void KumIOArea::mousePressEvent(QMouseEvent *event)
{
	if (waitForKey) {
		return;
	}
	QTextEdit::mousePressEvent(event);
//  QTextCursor mc = cursorForPosition(event->pos());
//  bool allow = mc.blockNumber()==document()->blockCount()-1;
//  allow = allow && mc.columnNumber() >= inputStartPosition;
//  allow = allow || (!waitForInput);
//  if (allow) {
//      mouseSelectionStart = -1;
//      QTextEdit::mousePressEvent(event);
//  }
//  else {
//      event->accept();
//      mouseSelectionStart = mc.position()
//  }
}

void KumIOArea::mouseReleaseEvent(QMouseEvent *event)
{
	QTextEdit::mousePressEvent(event);
//  QTextCursor mc = cursorForPosition(event->pos());
//  bool allow = mc.blockNumber()==document()->blockCount()-1;
//  allow = allow && mc.columnNumber() >= inputStartPosition;
//  allow = allow || (!waitForInput);
//  if (allow)
//      QTextEdit::mousePressEvent(event);
//  else
//      event->accept();
}

void KumIOArea::mouseMoveEvent(QMouseEvent *event)
{
	QTextEdit::mouseMoveEvent(event);
//  QTextCursor mc = cursorForPosition(event->pos());
//  bool allow = mc.blockNumber()==document()->blockCount()-1;
//  allow = allow && mc.columnNumber() >= inputStartPosition;
//  allow = allow || (!waitForInput);
//  if (allow)
//      QTextEdit::mousePressEvent(event);
//  else
//      event->accept();
//  if ( mouseSelectionStart >= 0 ) {
//      textCursor().setSelection
//  }
}


QString formatOutLine(int startPos, int width, const QString &txt)
{
	if (width == -1) {
		return txt;
	}
	int nextTabPos = (startPos / 4) * 4 + 4;
	int column = startPos;
	QString result;
	for (int i = 0; i < txt.length(); i++) {
		if (txt[i].unicode() == 10/* || txt[i].unicode()==13 */) {
			result += "\n";
			nextTabPos = 4;
			column = 0;
		} else if (txt[i].unicode() == 9) {
			if (width != -1 && (width - column) < 4) {
				result += "\n";
				nextTabPos = 4;
				column = 0;
			} else {
				while (column < nextTabPos) {
					result += " ";
					column ++;
				}
				nextTabPos += 4;
			}
		} else {
			QChar ch = txt[i];
			if (ch.unicode() < 32 || ch.unicode() == 127) {
				ch = QChar(0x22a0);
			} else if (ch.unicode() == 0x00A0) {
				ch = ' ';
			}
			if (column < width) {
				result += ch;
				column ++;
				nextTabPos = (column / 4) * 4 + 4;
			} else {
				result += QString("\n") + ch;
				column = 1;
				nextTabPos = 4;
			}
		}
	}
	return result;
}

void KumIOArea::output(const QString &txt, bool isErrorMessage)
{
	if (isErrorMessage) {
		textCursor().movePosition(QTextCursor::End);
		textCursor().insertHtml("<p><font color='red'>" + txt + "</font></p>");
	} else {
		QTextCursor c = textCursor();
		QString tmp;
		if (app()->settings->value("Terminal/Fixed", true).toBool()) {
			int w = app()->settings->value("Terminal/FixedSize", 80).toInt();
			tmp = formatOutLine(c.columnNumber(), w, txt);
//                    QString tmp = txt;
//                    int a = w-c.columnNumber();
//                    c.insertText(txt.left(a));
//                    tmp.remove(0,a);
//                    while ( !tmp.isEmpty() ) {
//                            append(tmp.left(w));
//                            tmp.remove(0,w);
//                    }
		} else {
			tmp = formatOutLine(c.columnNumber(), -1, txt);
//                    if ( txt.startsWith("\n")  )
//                            append(txt.mid(1));
//                    else
//                            c.insertText(txt);
		}
		c.insertText(tmp);
		ensureCursorVisible();
//            saveLast->setEnabled(true);
		if (runSeparators.count() > 1) {
//                    saveAll->setEnabled(true);
		}
		emit outputAvailable(true);
	}
}

void KumIOArea::clear()
{
	runSeparators.clear();
	QTextEdit::clear();
//	saveAll->setEnabled(false);
//	saveLast->setEnabled(false);
	emit outputAvailable(true);
}

void KumIOArea::scrollUp()
{
	int curBlock = textCursor().blockNumber();
	int p = 0;
	if (!runSeparators.isEmpty()) {
		p = runSeparators.first();
	}
	foreach (int n, runSeparators) {
		if (curBlock >= p && curBlock <= n) {
			break;
		}
		p = n;
	}
	QTextBlock bl;
	int k = 0;
	for (bl = document()->begin(); bl != document()->end(); bl = bl.next()) {
		if (p == k) {
			QTextCursor c(bl);
			setTextCursor(c);
			break;
		}
		k++;
	}
	ensureCursorVisible();
}

void KumIOArea::scrollDown()
{
}

void KumIOArea::saveAllOutput()
{
	QString lastSavedOut = app()->settings->value("History/SaveOutputDir", QDir::homePath()).toString();
	QString lastSavedEnc = app()->settings->value("History/SaveOutputEncoding", "UTF-8").toString();
	KumFileDialog *saveDialog = new KumFileDialog(this, tr("Save output"), lastSavedOut, tr("Any files (*)"));
	saveDialog->setFileMode(QFileDialog::AnyFile);
	saveDialog->setEncoding(lastSavedEnc);
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	saveDialog->setModal(true);
	if (saveDialog->exec() == QDialog::Accepted) {
		if (saveDialog->selectedFiles().isEmpty()) {
			delete saveDialog;
			return;
		}
		QString fileName = saveDialog->selectedFiles().first();
		if (fileName.count(".") == 0) {
			fileName += ".txt";
		}
		app()->settings->setValue("History/SaveOutputDir", QFileInfo(fileName).dir().absolutePath());
		QString encoding = saveDialog->encoding();
		app()->settings->setValue("History/SaveOutputEncoding", encoding);
		QFile f(fileName);
		if (f.open(QIODevice::WriteOnly)) {
			QTextStream ts(&f);
			QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8().data());
			ts.setCodec(codec);
			if (encoding.startsWith("UTF")) {
				ts.setGenerateByteOrderMark(true);
			}
			QString txt = toPlainText();
			QStringList lines = txt.split("\n", QString::KeepEmptyParts);
			QString hLine;
			uint hLineLength = 80;
			if (app()->settings->value("Terminal/Fixed", true).toBool()) {
				hLineLength = app()->settings->value("Terminal/FixedSize", true).toUInt();
			}
			hLine.fill('-', hLineLength);
			QString result;
			for (int i = 0; i < lines.count(); i++) {
				result += lines[i];
				if (runSeparators.contains(i)) {
					result += "\n" + hLine;
				}
				if (i < lines.count() - 1) {
					result += "\n";
				}
			}
			ts << result;
		}
		f.close();
	}
	delete saveDialog;
}

void KumIOArea::getLastOutput(QStringList &list)
{
	QStringList lines = toPlainText().split("\n", QString::KeepEmptyParts);
	int last = runSeparators.isEmpty() ? 0 : runSeparators.last() + 1;
	list.clear();
	for (int i = last; i < lines.count(); i++) {
		list << lines[i];
	}
}

void KumIOArea::saveLastOutput()
{
	QString def = app()->settings->value("Directories/IO", "").toString();
	QString lastSavedOut = app()->settings->value("History/SaveOutputDir", def).toString();
	QString lastSavedEnc = app()->settings->value("History/SaveOutputEncoding", "UTF-8").toString();
	KumFileDialog *saveDialog = new KumFileDialog(this, tr("Save output"), lastSavedOut, tr("Any files (*)"));
	saveDialog->setFileMode(QFileDialog::AnyFile);
	saveDialog->setEncoding(lastSavedEnc);
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	saveDialog->setModal(true);
	if (saveDialog->exec() == QDialog::Accepted) {
		if (saveDialog->selectedFiles().isEmpty()) {
			delete saveDialog;
			return;
		}
		QString fileName = saveDialog->selectedFiles().first();
		if (fileName.count(".") == 0) {
			fileName += ".txt";
		}
		app()->settings->setValue("History/SaveOutputDir", QFileInfo(fileName).dir().absolutePath());
		QString encoding = saveDialog->encoding();
		app()->settings->setValue("History/SaveOutputEncoding", encoding);
		QFile f(fileName);
		if (f.open(QIODevice::WriteOnly)) {
			QTextStream ts(&f);
			QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8().data());
			ts.setCodec(codec);
			if (encoding.startsWith("UTF")) {
				ts.setGenerateByteOrderMark(true);
			}
			QStringList lines = toPlainText().split("\n", QString::KeepEmptyParts);
			int last = runSeparators.count() > 1 ? runSeparators[runSeparators.count() - 2] + 1 : 0;
			QString hLine;
			uint hLineLength = 80;
			if (app()->settings->value("Terminal/Fixed", true).toBool()) {
				hLineLength = app()->settings->value("Terminal/FixedSize", true).toUInt();
			}
			hLine.fill('-', hLineLength);
			for (int i = last; i < lines.count(); i++) {
				ts << lines[i];
				if (runSeparators.contains(i)) {
					ts << "\n" + hLine;
				}
				if (i < lines.count() - 1) {
					ts << "\n";
				}
			}
			f.close();
		}
	}
	delete saveDialog;
}

void KumIOArea::saveSelectedOutput()
{
	QString def = app()->settings->value("Directories/IO", "").toString();
	QString lastSavedOut = app()->settings->value("History/SaveOutputDir", def).toString();
	QString lastSavedEnc = app()->settings->value("History/SaveOutputEncoding", "UTF-8").toString();
	KumFileDialog *saveDialog = new KumFileDialog(this, tr("Save output"), lastSavedOut, tr("Any files (*)"));
	saveDialog->setFileMode(QFileDialog::AnyFile);
	saveDialog->setEncoding(lastSavedEnc);
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	saveDialog->setModal(true);
	if (saveDialog->exec() == QDialog::Accepted) {
		if (saveDialog->selectedFiles().isEmpty()) {
			delete saveDialog;
			return;
		}
		QString fileName = saveDialog->selectedFiles().first();
		if (fileName.count(".") == 0) {
			fileName += ".txt";
		}
		app()->settings->setValue("History/SaveOutputDir", QFileInfo(fileName).dir().absolutePath());
		QString encoding = saveDialog->encoding();
		app()->settings->setValue("History/SaveOutputEncoding", encoding);
		QFile f(fileName);
		if (f.open(QIODevice::WriteOnly)) {
			QTextStream ts(&f);
			QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8().data());
			ts.setCodec(codec);
			ts << toPlainText().mid(textCursor().selectionStart(), textCursor().selectionEnd() - textCursor().selectionStart());
			f.close();
		}
	}
	delete saveDialog;
}

void KumIOArea::forceRepaint()
{
	if (textCursor().block().userData() == NULL) {
		InputUserData *d = new InputUserData();
		d->highlightable = true;
		textCursor().block().setUserData(d);
	}
	viewport()->update();
	sh->rehighlight();
}

void KumIOArea::contextMenuEvent(QContextMenuEvent *event)
{
	QTextCursor cur = cursorForPosition(event->pos());
	int para = cur.blockNumber();
	int index = cur.columnNumber();

	bool editAvailable = waitForInput && para == document()->blockCount() - 1 && index >= inputStartPosition;

	if (textCursor().hasSelection() || waitForInput) {
		QMenu *menu = new QMenu(this);
		if (editAvailable && textCursor().hasSelection()) {
			menu->addAction(tr("Cut"), this, SLOT(cut()));
		}
		if (textCursor().hasSelection()) {
			menu->addAction(tr("Copy"), this, SLOT(copy()));
		}
		if (editAvailable) {
			menu->addAction(tr("Paste"), this, SLOT(paste()));
		}
		QTextCursor newPos = cursorForPosition(event->pos());
		if (!menu->isEmpty()) {
			menu->exec(event->globalPos());
		}
		event->accept();
		delete menu;
	} else {
		return;
	}

}

void KumIOArea::input(QString format)
{
	words.clear();
	types.clear();
	formats = format.split("%");
	foreach (QString fmt, formats) {
		if (fmt.isEmpty()) {
			continue;
		}
		PeremType pType;
		if (fmt == "s") {
			pType = kumString;
		}
		if (fmt == "i") {
			pType = integer;
		}
		if (fmt == "c") {
			pType = charect;
		}
		if (fmt == "f") {
			pType = real;
		}
		if (fmt == "b") {
			pType = kumBoolean;
		}
		types << pType;
	}
	waitForInput = true;
	setReadOnly(false);
	tmpInput = "";
	inputStartPosition = textCursor().columnNumber();
	inputGarbageStart = -1;
	if (textCursor().block().userData() == NULL) {
		InputUserData *d = new InputUserData();
		d->highlightable = true;
		textCursor().block().setUserData(d);
	}
//	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	ensureCursorVisible();
	validateInput();
	forceRepaint();
}



int findTermStart(int termNo, QString line)
{
	int c = 0;
	bool found = false;
	int p = -1;
	for (int i = 0; i < line.length(); i++) {
		if (line[i] == ',') {
			c++;
		}
		if (c == termNo) {
			p = i;
			if (c > 0) {
				p++;
			}
			found = true;
			break;
		}
	}
	if (found && p >= line.length()) {
		p = -1;
	}
	return p;
}

int prepareString(QString &s)
{
	bool inQuote = false;
	QChar quote;
	int quotePos = -1;
	for (int i = 0; i < s.length(); i++) {
		if (!inQuote) {
			if (s[i] == '"' || s[i] == '\'') {
				inQuote = true;
				quote = s[i];
				quotePos = i;
			}
		} else {
			if (s[i] == quote) {
				inQuote = false;
			}
			if (s[i] == '"' && s[i] != quote) {
				s[i] = QChar(1003);
			}
			if (s[i] == '\'' && s[i] != quote) {
				s[i] = QChar(1004);
			}
			if (s[i] == ' ') {
				s[i] = QChar(1001);
			}
			if (s[i] == '\t') {
				s[i] = QChar(1002);
			}
			if (s[i] == ',') {
				s[i] = QChar(1000);
			}
		}
	}
	if (inQuote) {
		for (int i = quotePos + 1; i < s.length(); i++) {
			if (s[i] == QChar(1003)) {
				s[i] = '"';
			}
			if (s[i] == QChar(1004)) {
				s[i] = '\'';
			}
			if (s[i] == QChar(1001)) {
				s[i] = ' ';
			}
			if (s[i] == QChar(1002)) {
				s[i] = '\t';
			}
			if (s[i] == QChar(1000)) {
				s[i] = ',';
			}
		}
		return quotePos;
	} else {
		return -1;
	}
}

void split(QString S, QRegExp R, QList<InputTerm> &T)
{
	int cur;
	int prev;
	cur = 0;
	QRegExp nonSpace("\\S");
	prev = nonSpace.indexIn(S) > -1 ? nonSpace.indexIn(S) : 0;
	forever {
		InputTerm t;
		bool stop = false;
		cur = R.indexIn(S, prev);
		if (cur > -1) {
			t.text = S.mid(prev, cur - prev);
			t.allow_left = !(t.text.startsWith("\"") || t.text.startsWith("'"));
			t.allow_right = !(t.text.endsWith("\"") || t.text.endsWith("'"));
			t.allow_right = t.allow_right && (! R.cap().contains(","));
			t.start = prev;
			prev = cur + R.matchedLength();
		} else {
			t.text = S.mid(prev);
			if (t.text.trimmed().isEmpty()) {
				break;
			}
			t.allow_left = !(t.text.startsWith("\"") || t.text.startsWith("'"));
			t.allow_right = false;
			t.start = prev;
			stop = true;
		}
		T << t;
		if (stop) {
			break;
		}
	}
}

void validate(QList<PeremType> L, QList<InputTerm> S, QList<InputTerm> &T, bool &R, int &G)
{
	int i;
	InputTerm t;
	PeremType l;
	i = 0;
	foreach (l, L) {
		if (i >= S.size()) {
			break;
		}
		t = InputTerm();
		if (l == kumString) {
			t = S[i];
			while (t.allow_right && i < (S.size() - 1) && S[i].allow_left) {
				i = i + 1;
				t = t + S[i];
			}
			if (t.text.contains("\"")) {
				bool valid = t.text.startsWith("\"") && t.text.endsWith("\"") && t.text.count("\"") == 2;
				if (valid) {
					t.error = OK;
				} else {
					if (t.text.count("\"") != 2) {
						t.error = unpairedQuote;
					} else {
						t.error = quoteMisplace;
					}
				}
				t.valid = valid;

			} else if (t.text.contains("'")) {
				bool valid = t.text.startsWith("'") && t.text.endsWith("'") && t.text.count("'") == 2;
				if (valid) {
					t.error = OK;
				} else {
					if (t.text.count("'") != 2) {
						t.error = unpairedQuote;
					} else {
						t.error = quoteMisplace;
					}
				}
				t.valid = valid;
			} else {
				t.valid = true;
			}
		}
		if (l == charect) {
			t = S[i];
			if (t.text.length() == 1) {
				t.valid = true;
				t.error = OK;
			} else if (S[i].text.length() == 3 && (S[i].text[0] == '"' || S[i].text[0] == '\'') && S[i].text[0] == S[i].text[2]) {
				t.error = OK;
				t.valid = true;
			} else {
				bool hasQuote = S[i].text.contains("'") || S[i].text.contains("\"");
				if (S[i].text.length() > 3 || !hasQuote) {
					t.error = longCharect;
				} else if (S[i].text[0] != S[i].text[2]) {
					t.error = differentQuotes;
				} else {
					t.error = unpairedQuote;
				}
				t.valid = false;
			}
		}
		if (l == integer) {
			t = S[i];
			bool ok;
			t.text.toInt(&ok);
			if (!ok && t.text.trimmed().startsWith("$")) {
				QString h = t.text;
				h.remove(0, 1);
				h.toInt(&ok, 16);
			}
//          if ( ok )
//              t.error = OK;
//          else
//              t.error = notInteger;
//          t.valid = ok;
			t.valid = ok && IntegerOverflowChecker::checkFromString(t.text);
		}
		if (l == real) {
			t = S[i];
			bool ok;
			t.text.replace(QString::fromUtf8("Е"), "E");
			t.text.replace(QString::fromUtf8("е"), "e");
			t.text.toDouble(&ok);
			t.valid = ok && t.text.toLower() != "nan" && t.text.toLower() != "inf";
			if (t.valid) {
				t.error = OK;
			} else {
				t.error = notReal;
			}
		}
		if (l == kumBoolean) {
			t = S[i];
			bool ok;
			ok = t.text.toLower() == QApplication::translate("KumIOArea", "yes");
			ok = ok || t.text.toLower() == QApplication::translate("KumIOArea", "no");
			t.valid = ok;
			if (ok) {
				t.error = OK;
			} else {
				t.error = notBool;
			}
		}
		t.pType = l;
		T << t;
		i = i + 1;
	}
	if (i < S.size()) {
		R = false;
		G = S[i].start;
	} else if (T.size() < L.size()) {
		R = false;
		G = -1;
	} else {
		R = true;
		G = -1;
		foreach (t, T) {
			R = R && t.valid;
		}
	}
}

bool KumIOArea::validateInput()
{

	QString txt = textCursor().block().text().mid(inputStartPosition);

	prepareString(txt);
//  if ( types.count() == 1 && types[0]==charect )
//      inputUnpairedQuote = -1;
//  if ( inputUnpairedQuote != -1 )
//      return false;
	QList<InputTerm> terms;
	QRegExp rxDelim = QRegExp("(\\s*,\\s*)|(\\s+)");
	split(txt, rxDelim, terms);
	bool correct;
	words.clear();

	validate(types, terms, words, correct, inputGarbageStart);
//	if (types.count()==1 && types.first()==kumString) {
//		InputTerm term;
//		term.pType = kumString;
//		term.text = "";
//		term.start = 0;
//		term.valid = true;
//		terms << term;
//		words << term;
//		correct = true;
//	}
	if (txt.trimmed().endsWith(",") && correct && (types.last() != kumString || words.last().text.endsWith("\"") || words.last().text.endsWith("'"))) {
		inputGarbageStart = txt.lastIndexOf(",");
		correct = false;
	} else if (txt.trimmed().endsWith(",") && correct && types.last() == kumString && !(words.last().text.endsWith("\"") || words.last().text.endsWith("'"))) {
		words.last().text += ",";
	}

	QString msg = tr("INPUT ");
	QStringList m;
	foreach (PeremType pType, types) {
		QString t;
		if (pType == integer) {
			t = tr("int");
		} else if (pType == real) {
			t = tr("float");
		} else if (pType == charect) {
			t = tr("char");
		} else if (pType == kumString) {
			t = tr("string");
		} else if (pType == kumBoolean) {
			t = tr("bool");
		}
		m << t;
	}
	msg += m.join(", ");
	emit sendMessage(msg, 0);
	return correct;
}

void KumIOArea::tryFinishInput()
{
	QString msg;
	bool ok = validateInput();

	sh->rehighlight();

	if (ok) {
		finishInput();
	} else {
		foreach (InputTerm word, words) {
			if (!word.valid && word.start > -1) {
				QTextCursor c = textCursor();
				c.movePosition(QTextCursor::StartOfBlock);
				c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, inputStartPosition + word.start + word.text.length());
				c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, word.text.length());
				setTextCursor(c);
				msg = tr("INPUT ");
				QStringList m;
				foreach (InputTerm word, words) {
					QString t;
					if (word.pType == integer) {
						t = tr("int");
					} else if (word.pType == real) {
						t = tr("float");
					} else if (word.pType == charect) {
						t = tr("char");
					} else if (word.pType == kumString) {
						t = tr("string");
					} else if (word.pType == kumBoolean) {
						t = tr("bool");
					}
					if (word.valid) {
						m << "<font color='green'><b>" + t + "</b></font>";
					} else if (word.text.length() > 0) {
						m << "<font color='red'><b>" + t + "</b></font>";
					} else {
						m << t;
					}

				}
				for (int i = words.count(); i < types.count() ; i++) {
					if (types[i] == integer) {
						m << tr("int");
					}
					if (types[i] == real) {
						m << tr("float");
					}
					if (types[i] == charect) {
						m << tr("char");
					}
					if (types[i] == kumString) {
						m << tr("string");
					}
					if (types[i] == kumBoolean) {
						m << tr("bool");
					}
				}
				msg += m.join(", ") + ".";
				if (word.text.length() > 0) {
					if (word.error == notInteger) {
						msg += " <font color='red'>" + tr("It is not integer.") + " </font>";
					} else if (word.error == notReal) {
						msg += " <font color='red'>" + tr("It is not real.") + " </font>";
					} else if (word.error == notBool) {
						msg += " <font color='red'>" + tr("It is not kumBoolean.") + " </font>";
					} else if (word.error == unpairedQuote) {
						msg += " <font color='red'>" + tr("Unpaired quote.") + " </font>";
					} else if (word.error == longCharect) {
						msg += " <font color='red'>" + tr("More then one character.") + " </font>";
					} else if (word.error == quoteMisplace) {
						msg += " <font color='red'>" + tr("Quotes misplaced.") + " </font>";
					} else if (word.error == differentQuotes) {
						msg += " <font color='red'>" + tr("Different quotes.") + " </font>";
					}
				} else {
					if (word.pType == integer) {
						msg += " <font color='red'>" + tr("Integer not entered.") + " </font>";
					}
					if (word.pType == real) {
						msg += " <font color='red'>" + tr("Real not entered.") + " </font>";
					}
					if (word.pType == kumBoolean) {
						msg += " <font color='red'>" + tr("Boolean not entered.") + " </font>";
					}
					if (word.pType == charect) {
						msg += " <font color='red'>" + tr("Charect not entered.") + " </font>";
					}
				}
				emit sendMessage(msg, 0);
				return;
			} else if (!word.valid) {
			}
		}
		if (words.count() < types.count()) {
			msg = tr("INPUT ");
			QStringList m;
			foreach (InputTerm word, words) {
				QString t;
				if (word.pType == integer) {
					t = tr("int");
				} else if (word.pType == real) {
					t = tr("float");
				} else if (word.pType == charect) {
					t = tr("char");
				} else if (word.pType == kumString) {
					t = tr("string");
				} else if (word.pType == kumBoolean) {
					t = tr("bool");
				}
				if (word.valid) {
					m << "<font color='green'><b>" + t + "</b></font>";
				} else if (word.text.length() > 0) {
					m << "<font color='red'><b>" + t + "</b></font>";
				} else {
					m << t;
				}
			}
			for (int i = words.count(); i < types.count(); i++) {
				QString t;
				PeremType pType = types[i];
				if (pType == integer) {
					t = tr("int");
				} else if (pType == real) {
					t = tr("float");
				} else if (pType == charect) {
					t = tr("char");
				} else if (pType == kumString) {
					t = tr("string");
				} else if (pType == kumBoolean) {
					t = tr("bool");
				}
				m << t;
			}
			msg += m.join(", ") + ".";
			PeremType pType = types[words.count()];
			if (pType == integer) {
				msg += " <font color='red'>" + tr("Integer not entered.") + " </font>";
			}
			if (pType == real) {
				msg += " <font color='red'>" + tr("Real not entered.") + " </font>";
			}
			if (pType == kumBoolean) {
				msg += " <font color='red'>" + tr("Boolean not entered.") + " </font>";
			}
			if (pType == charect) {
				msg += " <font color='red'>" + tr("Charect not entered.") + " </font>";
			}
			if (pType == kumString) {
				msg += " <font color='red'>" + tr("String not entered.") + " </font>";
			}
			emit sendMessage(msg, 0);
			QTextCursor c = textCursor();
			c.movePosition(QTextCursor::EndOfBlock);
			setTextCursor(c);

		}
		if (inputGarbageStart > -1) {
			QTextCursor c = textCursor();
			c.movePosition(QTextCursor::EndOfBlock);
			c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, textCursor().block().text().length() - (inputStartPosition + inputGarbageStart));
			setTextCursor(c);
			msg = tr("INPUT ");
			QStringList m;
			foreach (InputTerm word, words) {
				QString t;
				if (word.pType == integer) {
					t = tr("int");
				} else if (word.pType == real) {
					t = tr("float");
				} else if (word.pType == charect) {
					t = tr("char");
				} else if (word.pType == kumString) {
					t = tr("string");
				} else if (word.pType == kumBoolean) {
					t = tr("bool");
				}
				if (word.valid) {
					m << "<font color='green'><b>" + t + "</b></font>";
				} else if (word.text.length() > 0) {
					m << "<font color='red'><b>" + t + "</b></font>";
				} else {
					m << t;
				}

			}
			msg += m.join(", ") + ".";
			msg += " <font color='red'>" + tr("Garbage after input string.") + " </font>";
			emit sendMessage(msg, 0);
		}
	}
}

void KumIOArea::finishInput()
{
	InputUserData *d = static_cast<InputUserData *>(textCursor().block().userData());
	d->highlightable = false;
	sh->rehighlight();
	setReadOnly(true);
	waitForInput = false;
	QTextCursor c = textCursor();
	c.movePosition(QTextCursor::EndOfBlock);
	setTextCursor(c);
	textCursor().insertText("\n");
	QStringList ret;
	foreach (InputTerm word, words) {
		if (word.pType == kumString) {
			if (word.text.startsWith("\"") && word.text.endsWith("\"")) {
				word.text = word.text.mid(1, word.text.length() - 2);
			}
			if (word.text.startsWith("'") && word.text.endsWith("'")) {
				word.text = word.text.mid(1, word.text.length() - 2);
			}
			word.text.replace(QString(QChar(1000)), ",");
			word.text.replace(QString(QChar(1001)), " ");
			word.text.replace(QString(QChar(1002)), "\t");
			word.text.replace(QString(QChar(1003)), "\"");
			word.text.replace(QString(QChar(1004)), "'");
			ret << word.text;
		} else if (word.pType == charect)  {
			QString QUOTE = QString(QChar(1001)) + QString(QChar(1001));
			if (word.text.length() == 3) {
				word.text.remove(0, 1);
				word.text.remove(1, 1);
				word.text.replace(QString(QChar(1000)), ",");
				word.text.replace(QString(QChar(1001)), " ");
				word.text.replace(QString(QChar(1002)), "\t");
				word.text.replace(QString(QChar(1003)), "\"");
				word.text.replace(QString(QChar(1004)), "'");
			}
			if (word.text.length() > 1) {
				word.text = word.text.trimmed();
			}
			if (word.text.isEmpty()) {
				word.text = " ";
			}
			ret << word.text;
		} else if (word.pType == integer) {
			if (word.text.trimmed().startsWith("$")) {
				word.text.remove(0, 1);
				bool ok;
				int value = word.text.toInt(&ok, 16);
				K_ASSERT(ok);
				ret << QString::number(value);
			} else {
				ret << word.text.trimmed();
			}
		} else if (word.pType == real) {
			word.text.replace(QString::fromUtf8("Е"), "E");
			word.text.replace(QString::fromUtf8("е"), "e");
			ret << word.text.trimmed();
		} else {
			ret << word.text.trimmed();
		}
	}
	sendMessage("", 0);
	emit inputFinished(ret);
}

void KumIOArea::insertFromMimeData(const QMimeData *source)
{
	if (waitForInput) {
		if (textCursor().blockNumber() != document()->blockCount() - 1) {
			return;
		}
		int cp = textCursor().columnNumber();
		if (cp < inputStartPosition) {
			return;
		}
		QTextCursor c = textCursor();
		QString txt = source->text();
		int p = txt.indexOf("\n");
		if (p != -1) {
			txt = txt.left(p - 1);
		}
		c.insertText(txt);
		setTextCursor(c);
		validateInput();
	}
}

void KumIOArea::resizeEvent(QResizeEvent *e)
{
	QTextEdit::resizeEvent(e);
	updateLastBlockBottomMargin();
}

void KumIOArea::updateLastBlockBottomMargin()
{
	if (runSeparators.isEmpty()) {
		return;
	}
	int realHeight = height();
	if (horizontalScrollBar()->isVisible()) {
		realHeight -= horizontalScrollBar()->height();
	}
	int lineHeight = QFontMetrics(font()).lineSpacing();
	int outputStartLine = runSeparators.last();
	int outputCurLine = textCursor().blockNumber();
	int outputHeight = (outputCurLine - outputStartLine) * lineHeight;
	int spaceToAdd = realHeight - outputHeight;
	if (spaceToAdd > 0) {
		QTextFrameFormat fmt = document()->rootFrame()->frameFormat();
		fmt.setBottomMargin((qreal)(spaceToAdd));
		document()->rootFrame()->setFrameFormat(fmt);
	}

}
