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

#include "programtab.h"
#include "application.h"
#include "mainwindow.h"
#include <QtGlobal>
#include <QScrollBar>
#include <QPrinter>
#include <QSettings>
#include <QPainter>

ProgramTab::ProgramTab(QWidget *parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), Ui::ProgramTab()
{
	setupUi(this);
	QString defaultFont;
#ifdef Q_OS_UNIX
	defaultFont = "DejaVu Sans Mono";
#endif
#ifdef Q_OS_WIN32
	if (QSysInfo::WindowsVersion >= 0x0080) {
		defaultFont = "Consolas";
	} else {
		defaultFont = "Courier";
	}
#endif
	QString editorsFont = app()->settings->value("Appearance/Font", defaultFont).toString();
	QFont font(editorsFont);
	int fontSize = app()->settings->value("Appearance/FontSize", 12).toInt();
	font.setPointSize(fontSize);
	bool boldFont = app()->settings->value("Appearance/FontBold", false).toBool();
	font.setBold(boldFont);
	setFont(font);
	QList<QVariant> lv = app()->settings->value("MainWindow/Splitter1Sizes", QList<QVariant>() << QVariant(800 - 16 - 200) << QVariant(200)).toList();
	QList<int> li;
	for (int i = 0; i < lv.size(); i++) {
		li << lv[i].toInt();
	}
//	splitter->restoreState(app()->settings->value("MainWindow/Splitter1","").toByteArray());
	splitter->setSizes(li);
	//fixSplitterRight();

	connect(kumirEdit, SIGNAL(copyAvailable(bool)), this, SLOT(emitCopyAvailable(bool)));
	connect(kumirEdit, SIGNAL(undoAvailable(bool)), this, SLOT(emitUndoAvailable(bool)));
	connect(kumirEdit, SIGNAL(redoAvailable(bool)), this, SLOT(emitRedoAvailable(bool)));
	connect(kumirEdit, SIGNAL(uncommentAvailable(bool)), this, SLOT(emitUncommentAvailable(bool)));
	connect(kumirEdit, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));
	connect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));
	connect(kumirEdit, SIGNAL(ensureMarginCursorVisible(int)), this, SLOT(syncCursorPosition(int)));
	connect(kumirEdit, SIGNAL(clearError(int)), kumirMargin, SLOT(clearError(int)));
	connect(kumirEdit, SIGNAL(linesRemoved(int, int)), kumirMargin, SLOT(removeLines(int, int)));
	connect(kumirEdit, SIGNAL(linesInserted(int, int)), kumirMargin, SLOT(insertEmptyLines(int, int)));
	scrollLock = false;
	kumirEdit->setFocus();
	m_fileName = "";
	module = false;
	kumirMargin->setEditor(kumirEdit);
	kumirMargin->setTab(this);
	kumirEdit->setMargin(kumirMargin);
	textCursorPosition = 0;
}

void ProgramTab::syncCursorPosition(int lineNo)
{
	disconnect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));


//  int v = kumirEdit->verticalScrollBar()->value();
//  kumirMargin->verticalScrollBar()->setValue(v);

	QTextCursor c = kumirMargin->textCursor();
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNo);
	kumirMargin->setTextCursor(c);
	kumirMargin->ensureCursorVisible();
	c.insertText(QString::fromUtf8("   "));
	connect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));

}

void ProgramTab::saveCursorPosition()
{
	textCursorPosition = kumirEdit->textCursor().position();
}

void ProgramTab::restoreCursorPosition()
{
	kumirEdit->textCursor().setPosition(textCursorPosition);
}

void ProgramTab::emitUncommentAvailable(bool v)
{
	emit uncommentAvailable(v);
}

void ProgramTab::emitCopyAvailable(bool v)
{
	emit copyAvailable(v);
}

void ProgramTab::emitUndoAvailable(bool v)
{
	emit undoAvailable(v);
}

void ProgramTab::emitRedoAvailable(bool v)
{
	emit redoAvailable(v);
}

bool ProgramTab::isCopyAvailable()
{
	return kumirEdit->textCursor().hasSelection();
}

bool ProgramTab::isUndoAvailable()
{
	return kumirEdit->isUndoAvailable();
}

bool ProgramTab::isRedoAvailable()
{
	return kumirEdit->isRedoAvailable();
}

bool ProgramTab::isUncommentAvailable()
{
	return kumirEdit->isUncommentAvailable();
}

void ProgramTab::saveSplitterState()
{
	QList<int> li = splitter->sizes();
	QList<QVariant> lv;
	for (int i = 0; i < li.size(); i++) {
		lv << QVariant(li[i]);
	}
	app()->settings->setValue("MainWindow/Splitter1Sizes", lv);
	app()->settings->sync();
}

ProgramTab::~ProgramTab()
{

}

QFont ProgramTab::font()
{
	return kumirEdit->font();
}

void ProgramTab::setFont(const QFont &font)
{
	kumirEdit->setFont(font);
	QTextCharFormat f;
	f.setFont(font);
	QTextCursor c1(kumirEdit->document());
	c1.select(QTextCursor::Document);
	c1.setCharFormat(f);
	kumirMargin->setFont(font);
	QTextCursor c2(kumirMargin->document());
	c2.select(QTextCursor::Document);
	c2.setCharFormat(f);
}

QString ProgramTab::editorText(bool withIndentation)
{
	QString txt = kumirEdit->toPlainText();
	if (app()->settings->value("Editor/LegacyIndent", false).toBool()) {
		if (withIndentation) {
			QStringList lines = txt.split("\n", QString::KeepEmptyParts);
			QTextDocument *doc = kumirEdit->document();
			int i = 0;
			for (QTextBlock bl = doc->begin(); bl != doc->end(); bl = bl.next()) {
				if (bl.userData() != NULL) {
					LineProp *lp = static_cast<LineProp *>(bl.userData());
					int indent_count = 0;
					QString line = bl.text();
					while (line.startsWith(". ")) {
						indent_count ++;
						line.remove(0, 2);
					}
					int shift = indent_count * 2;
//                  foreach ( QPoint ep, lp->errorPositions ) {
//                      int start = ep.x();
//                      int end = ep.y();
//                      lines[i].insert(start+shift,"<u>");
//                      shift += 3;
//                      lines[i].insert(end+shift,"</u>");
//                      shift += 4;
//                  }
					bool err = false;
					for (int j = 0; j < lp->errorMask.count(); j++) {
						if (lp->errorMask[j] && !err) {
							lines[i].insert(j + shift, "<u>");
							err = true;
							shift += 3;
						} else if (!lp->errorMask[j] && err) {
							lines[i].insert(j + shift, "</u>");
							shift += 4;
							err = false;
						}
					}
				}
				i++;
			}
			return txt;
		} else {
			int indent_count = 0;
			Q_UNUSED(indent_count);
			QStringList lines = txt.split("\n", QString::KeepEmptyParts);
			QTextDocument *doc = kumirEdit->document();
			int i = 0;
			for (QTextBlock bl = doc->begin(); bl != doc->end(); bl = bl.next()) {
				if (bl.userData() != NULL) {
					LineProp *lp = (LineProp *)(bl.userData());
					lines[i].remove(0, lp->indentCount * 2);
				}
				i++;
			}
			return lines.join("\n");
		}
	} else {
		if (!withIndentation) {
			return txt;
		} else {
			int indent_count = 0;
			QStringList lines = txt.split("\n", QString::KeepEmptyParts);
			QTextDocument *doc = kumirEdit->document();
			int i = 0;
			for (QTextBlock bl = doc->begin(); bl != doc->end(); bl = bl.next()) {
				if (bl.userData() != NULL) {
					LineProp *lp = (LineProp *)(bl.userData());
					indent_count += lp->indentRang;
					for (int j = 0; j < indent_count; j++) {
						lines[i] = ". " + lines[i];
					}
					indent_count += lp->indentRangNext;
					int shift = indent_count * 2;
//                  foreach ( QPoint ep, lp->errorPositions ) {
//                      int start = ep.x();
//                      int end = ep.y();
//                      lines[i].insert(start+shift,"<u>");
//                      shift += 3;
//                      lines[i].insert(end+shift,"</u>");
//                      shift += 4;
//                  }
					bool err = false;
					for (int j = 0; j < lp->errorMask.count(); j++) {
						if (lp->errorMask[j] && !err) {
							lines[i].insert(j + shift, "<u>");
							err = true;
							shift += 3;
						} else if (!lp->errorMask[j] && err) {
							lines[i].insert(j + shift, "</u>");
							shift += 4;
							err = false;
						}
					}
				}
				i++;
			}
			return lines.join("\n");
		}
	}
}


QString ProgramTab::marginText()
{
	return kumirMargin->toPlainText();
}

void ProgramTab::setEditorText(const QString &txt)
{
	kumirEdit->setText(txt);
	for (QTextBlock b = editor()->document()->begin(); b.isValid(); b = b.next()) {
		QTextCursor c(b);
		QTextBlockFormat fmt = b.blockFormat();
		fmt.setTopMargin(0);
		c.mergeBlockFormat(fmt);
	}
}

void ProgramTab::updateLanguage()
{
	retranslateUi(this);
	kumirEdit->updateLanguage();
}

void ProgramTab::updateSHAppearance()
{
	kumirEdit->updateSHAppearance();
}

bool ProgramTab::isProgram()
{
	return true;
}

void ProgramTab::disconnectEditActions()
{
	disconnect(SIGNAL(copyAvailable(bool)));
	disconnect(SIGNAL(undoAvailable(bool)));
	disconnect(SIGNAL(redoAvailable(bool)));
}

void ProgramTab::editCut()
{
	kumirEdit->cut();
}

void ProgramTab::editCopy()
{
	kumirEdit->copy();
}

void ProgramTab::editPaste()
{
	kumirEdit->paste();
}

void ProgramTab::editUndo()
{
	kumirEdit->undo();
}

void ProgramTab::editRedo()
{
	kumirEdit->redo();
}

void ProgramTab::editComment()
{
	kumirEdit->comment();
}

void ProgramTab::editUncomment()
{
	kumirEdit->uncomment();
}

// void ProgramTab::clearIndents()
// {
//  kumirEdit->clearIndents();
// }

void ProgramTab::applyIndentation()
{
	kumirEdit->applyIndentation();
}

void ProgramTab::setTabNo(int i)
{
	kumirEdit->setTabNo(i);
}

KumirEdit *ProgramTab::editor()
{
	return kumirEdit;
}

KumirMargin *ProgramTab::margin()
{
	return kumirMargin;
}

void ProgramTab::synchonizeScroll(int dx, int dy)
{
	Q_UNUSED(dx);
	if (sender() == kumirEdit) {
		int y = kumirMargin->verticalScrollBar()->value();
		y -= dy;
		kumirMargin->disconnect(SIGNAL(contentsMoving(int, int)));
		kumirMargin->verticalScrollBar()->setValue(y);
		connect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));
	}
	if (sender() == kumirMargin) {
		int y = kumirEdit->verticalScrollBar()->value();
		y -= dy;
		scrollLock = true;
		kumirEdit->disconnect(SIGNAL(contentsMoving(int, int)));
		kumirEdit->verticalScrollBar()->setValue(y);
		connect(kumirEdit, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));
	}
}

void ProgramTab::setMarginText(const QString &txt)
{
	kumirMargin->disconnect(SIGNAL(contentsMoving(int, int)));
	int y = kumirMargin->verticalScrollBar()->value();
	QStringList lines = txt.split("\n", QString::KeepEmptyParts);
	for (int i = 0; i < lines.count(); i++) {
		lines[i] = lines[i].trimmed();
	}
	kumirMargin->clear();
	QString text = lines.join("\n");
//  qDebug() << text;
	kumirMargin->setText(text);
	syncronizeLineHeights();
	kumirMargin->verticalScrollBar()->setValue(y);
	connect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(syncronizeLineHeights()));
	connect(kumirMargin, SIGNAL(contentsMoving(int, int)), this, SLOT(synchonizeScroll(int, int)));
	kumirEdit->checkScrollBars();

}

void ProgramTab::syncronizeLineHeights()
{
//  qDebug() << "FONT SIZE: " << editor()->font().pointSize();
	K_ASSERT(kumirEdit->document()->blockCount() == kumirMargin->document()->blockCount());
	QTextBlock eb = kumirEdit->document()->begin();
	QTextBlock mb = kumirMargin->document()->begin();
//  qDebug()<<"kumirEdit->document()->blockCount()="<<kumirEdit->document()->blockCount();
//  qDebug()<<"kumirMargin->document()->blockCount()="<<kumirMargin->document()->blockCount();

	int i = 1;
	while (eb.isValid()) {
		K_ASSERT(eb.isValid());
		K_ASSERT(mb.isValid());
		QTextCursor ce = QTextCursor(eb);
		QTextCursor cm = QTextCursor(mb);
		QRect re = kumirEdit->cursorRect(ce);
		QRect rm = kumirMargin->cursorRect(cm);
//      qDebug() << "LINE HEIGHT: " << re.height() << " TOP SPACING: " << eb.blockFormat().topMargin();
		// TODO send bugreport to Nokia: comment && abs "abs(re.height()-rm.height())<re.height()" and see :-(
		if (re.height() != rm.height() && abs(re.height() - rm.height()) < re.height()) {
			if (re.height() > rm.height()) {
				int diff = re.height() - rm.height();
				QTextBlockFormat fmt = mb.blockFormat();
				fmt.setTopMargin(diff);
				cm.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
				cm.mergeBlockFormat(fmt);
//              QMessageBox::information(this, "", "INCREASED MARGIN LINE "+QString::number(i)+": "+QString::number(diff));
			}
			if (re.height() < rm.height()) {
				int diff = rm.height() - re.height();
				QTextBlockFormat fmt = eb.blockFormat();
				fmt.setTopMargin(diff);

				ce.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
				ce.mergeBlockFormat(fmt);
//              QMessageBox::information(this, "", "INCREASED EDITOR LINE "+QString::number(i)+": "+QString::number(diff));
			}
			i++;
		}
		eb = eb.next();
		if (mb.isValid()) {
			mb = mb.next();
		}
	}
}

void ProgramTab::paste()
{
	if (kumirEdit->hasFocus()) {
		kumirEdit->paste();
	} else if (kumirMargin->hasFocus()) {
		kumirMargin->paste();
	}
}

/**
 * Проверяет, находится ли позиция pos внутри литеральной константы
 * @param s исходная ненормализованная строка
 * @param pos позиция
 * @return true, если внутри литеральной константы или false, если нет
 */

bool PTinLit(QString s, int pos)
{
	bool isDoubleQuote = false;
	bool inL = false;
	int cur_pos = -1;
	int len = s.length();
	int pos_DQ, pos_SQ;
	while (cur_pos < len) {
		pos_DQ = s.indexOf("\"", cur_pos + 1);
		pos_SQ = s.indexOf("'", cur_pos + 1);
		if ((pos_DQ == -1) && (pos_SQ == -1)) {
			return inL;
		}
		if (
			(pos_DQ == -1)
			||
			((pos_SQ < pos_DQ) && (pos_SQ != -1))
		) { // next is '
			cur_pos = pos_SQ;
			if (pos < cur_pos) {
				return inL;
			}
			if (!inL) { // begin lit
				inL = true;
				isDoubleQuote = false;
			} else {
				if (!isDoubleQuote) {
					inL = false;
				}
			}
		} else { // next is "
			cur_pos = pos_DQ;
			if (pos < cur_pos) {
				return inL;
			}
			if (!inL) {
				isDoubleQuote = true;
				inL = true;
			} else {
				if (isDoubleQuote) {
					inL = false;
				}
			}
		}
	}
	return false;
}


struct Part {
	QString text;
	bool bold;
	bool underline;
	bool italic;
};


static const char *keywords[] {
	"алг",
	"нач",
	"кон",
	"для",
	"нц",
	"кц",
	"пока",
	"если",
	"то",
	"иначе",
	"все",
	"ввод",
	"вывод",
	"ф_ввод",
	"ф_вывод",
	"от",
	"до",
	"выбор",
	"при",
	"выход",
	"арг",
	"рез",
	"аргрез",
	"дано",
	"надо",
	"раз",
	"и",
	"или",
	"не",
	"кц_при",
	"утв",
	"дано",
	"нс",
	"исп",
	"кон_исп",
	"знач",
	"шаг",
	"использовать",
	"вещ",
	"цел",
	"лит",
	"сим",
	"лог",
	"вещтаб",
	"целтаб",
	"литтаб",
	"симтаб",
	"логтаб",
	"вещ(\\s+)таб",
	"цел(\\s+)таб",
	"лит(\\s+)таб",
	"сим(\\s+)таб",
	"лог(\\s+)таб"
};



QList<Part> split(QString text, QList<QPoint> errorPositions)
{
	int keynum = sizeof(keywords)  / sizeof(keywords[0]);
	QStringList keyWordList;

	for (int i = 0; i < keynum; i++) {
		keyWordList.append(QObject::trUtf8("SyntaxHighlighter", keywords[i]));
	}
#if 0
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "алг", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "нач", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "кон", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "для", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "нц", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "кц", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "пока", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "если", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "то", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "иначе", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "все", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "ввод", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "вывод", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "ф_ввод", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "ф_вывод", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "от", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "до", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "выбор", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "при", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "выход", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "арг", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "рез", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "аргрез", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "дано", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "надо", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "раз", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "и", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "или", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "не", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "кц_при", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "утв", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "дано", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "нс", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "исп", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "кон_исп", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "знач", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "шаг", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "использовать", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "вещ", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "цел", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "лит", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "сим", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "лог", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "вещтаб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "целтаб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "литтаб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "симтаб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "логтаб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "вещ(\\s+)таб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "цел(\\s+)таб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "лит(\\s+)таб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "сим(\\s+)таб", 0, QCoreApplication::UnicodeUTF8));
	keyWordList.append(QApplication::translate("SyntaxHighlighter", "лог(\\s+)таб", 0, QCoreApplication::UnicodeUTF8));
#endif

	for (int i = 0; i < keyWordList.count(); i++) {
		keyWordList[i] = "\\b" + keyWordList[i] + "\\b";
	}

	QRegExp rxKeyWords;
	rxKeyWords = QRegExp(keyWordList.join("|"));

	QString format;
	for (int i = 0; i < text.length(); i++) {
		format += "p";
	}

	int p = 0;

	forever {
		p = rxKeyWords.indexIn(text, p);
		if (p == -1) {
			break;
		}
		if (!PTinLit(text, p)) {
			for (int i = 0; i < rxKeyWords.matchedLength(); i++) {
				format[p + i] = 'k';
			}
		}
		p += rxKeyWords.matchedLength();
	}

	foreach (QPoint p, errorPositions) {
		for (int i = p.x(); i < p.y(); i ++)
			if (text[i] != ' ') {
				format[i] = format[i].toUpper();
			}
	}

	p = 0;
	forever {
		p = text.indexOf("|", p);
		if (p == -1) {
			break;
		}
		if (!PTinLit(text, p)) {
			for (int i = p; i < format.length(); i++) {
				format[i] = 'c';
			}
			break;
		}
		p++;
	}

	QList<Part> result;
	Part cur;
	QChar cf;
	for (int i = 0; i < format.length(); i++) {
		if (format[i] != cf && !cur.text.isEmpty()) {
			if (cf == 'p') {
				cur.bold = false;
				cur.underline = false;
				cur.italic = false;
			}
			if (cf == 'P') {
				cur.bold = false;
				cur.underline = true;
				cur.italic = false;
			}
			if (cf == 'k') {
				cur.bold = true;
				cur.underline = false;
				cur.italic = false;
			}
			if (cf == 'K') {
				cur.bold = true;
				cur.underline = true;
				cur.italic = false;
			}
			if (cf == 'c') {
				cur.bold = false;
				cur.underline = false;
				cur.italic = true;
			}
			result << cur;
			cur.text = "";
		}
		cur.text += text[i];
		cf = format[i];
	}
	if (!cur.text.isEmpty()) {
		if (cf == 'p') {
			cur.bold = false;
			cur.underline = false;
			cur.italic = false;
		}
		if (cf == 'P') {
			cur.bold = false;
			cur.underline = true;
			cur.italic = false;
		}
		if (cf == 'k') {
			cur.bold = true;
			cur.underline = false;
			cur.italic = false;
		}
		if (cf == 'K') {
			cur.bold = true;
			cur.underline = true;
			cur.italic = false;
		}
		if (cf == 'c') {
			cur.bold = false;
			cur.underline = false;
			cur.italic = true;
		}
		result << cur;
		cur.text = "";
	}
	return result;
}

void ProgramTab::print(QPrinter *printer)
{
	QPainter painter;
	painter.begin(printer);
	QFont fnt = editor()->font();
	fnt.setPointSize(8);
	int pageNo = 1;
	int lineNo = 3;
	int realLineNo = 1;
	QFont test = fnt;
	test.setBold(true);
	test.setUnderline(true);
	painter.setFont(fnt);
	qreal lineHeight = QFontMetricsF(test).lineSpacing();
	painter.setBrush(QColor("lightgray"));
	painter.setPen(Qt::NoPen);
	painter.drawRect(0, 0, 30, printer->pageRect().height());
	painter.drawRect(30 + printer->pageRect().width() * 0.75, 0, printer->pageRect().width() * 0.25 - 30, printer->pageRect().height());
	painter.drawRect(0, 0, printer->pageRect().width(), 1.5 * lineHeight);
	painter.drawRect(0, printer->pageRect().height() - 1.5 * lineHeight, printer->pageRect().width(), 1.5 * lineHeight);
	painter.setPen(Qt::black);
	painter.drawText(QRect(0, 0, printer->pageRect().width(), 1.5 * lineHeight),
		Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
		getFileName());
	painter.drawText(QRect(0, printer->pageRect().height() - 1.5 * lineHeight, printer->pageRect().width(), 1.5 * lineHeight),
		Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
		tr("Page ") + QString::number(pageNo));
	QTextBlock ed = editor()->document()->begin();
	QTextBlock mr = margin()->document()->begin();
	int indent_count = 0;
	while (ed.isValid()) {
		if ((lineNo + 2)*lineHeight >= printer->pageRect().height()) {
			printer->newPage();
			pageNo++;
			painter.setBrush(QColor("lightgray"));
			painter.setPen(Qt::NoPen);
			painter.drawRect(0, 0, 30, printer->pageRect().height());
			painter.drawRect(30 + printer->pageRect().width() * 0.75, 0, printer->pageRect().width() * 0.25 - 30, printer->pageRect().height());
			painter.drawRect(0, 0, printer->pageRect().width(), 1.5 * lineHeight);
			painter.drawRect(0, printer->pageRect().height() - 1.5 * lineHeight, printer->pageRect().width(), 1.5 * lineHeight);
			painter.setPen(Qt::black);
			painter.drawText(QRect(0, 0, printer->pageRect().width(), 1.5 * lineHeight),
				Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
				getFileName());
			painter.drawText(QRect(0, printer->pageRect().height() - 1.5 * lineHeight, printer->pageRect().width(), 1.5 * lineHeight),
				Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
				tr("Page ") + QString::number(pageNo));
			lineNo = 3;
		}
		painter.setFont(fnt);
		QRect lineNumberRect = QRect(0, (lineNo - 1) * lineHeight, 30, lineHeight);
		painter.drawText(lineNumberRect, Qt::AlignCenter, QString::number(realLineNo));
		qreal shift = 5.0;
		QList<QPoint> errorPositions;
		LineProp *lp = static_cast<LineProp *>(ed.userData());
		if (lp != NULL) {
			indent_count += lp->indentRang;
			int real_indent = qMax(0, indent_count);
			shift += 15 * real_indent;
			indent_count += lp->indentRangNext;
//          errorPositions = lp->errorPositions;
		}
		QList<Part> parts = split(ed.text(), errorPositions);
		foreach (Part p, parts) {
			QFont cf = fnt;
			cf.setBold(p.bold);
			cf.setUnderline(p.underline);
			cf.setItalic(p.italic);
			painter.setFont(cf);
			QRect textRect = QRect(30 + shift, (lineNo - 1) * lineHeight, printer->pageRect().width() * 0.75 - shift - 5.0, lineHeight);
			QFontMetrics test(cf);
			while (test.width(p.text) > textRect.width()) {
				p.text.chop(1);
			}
			painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, p.text);
			shift += QFontMetrics(cf).width(p.text);
		}
		painter.setFont(fnt);
		QRect mt = QRect(35 + printer->pageRect().width() * 0.75, (lineNo - 1) * lineHeight, printer->pageRect().width() * 0.25 - 40, lineHeight);
		QFontMetrics test(fnt);
		QString line = mr.text();
		qreal boxWidth = mt.width();
		qreal textWidth = test.width(line);
		while (textWidth > boxWidth) {
			line.chop(1);
			boxWidth = mt.width();
			textWidth = test.width(line);
		}
		painter.drawText(mt, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, line);
		realLineNo++;
		lineNo++;
		ed = ed.next();
		if (mr.isValid()) {
			mr = mr.next();
		}
	}
	painter.end();
}

/*void ProgramTab::fixSplitterAfterResize()
{
    splitter->setSizes(splitter->sizes());
}

QList<int> ProgramTab::getSplitterSizes()
{
    return splitter->sizes();
}

void ProgramTab::setSplitterSizes(const QList<int>& l)
{
    splitter->setSizes(l);
}

void ProgramTab::fixSplitterLeft()
{
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
}

void ProgramTab::fixSplitterRight()
{
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
}

int ProgramTab::horizStretchAtW0()
{
    return splitter->widget(0)->sizePolicy().horizontalStretch();
}*/
