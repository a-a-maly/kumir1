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

#include "textedit.h"
#include "mainwindow.h"
#include "application.h"
#include "macro.h"
#include "undocommands.h"
#include "lineprop.h"
#include "kumundostack.h"

#include <QtGlobal>
#include <QScrollBar>
#include <QClipboard>

#ifdef Q_OS_UNIX
#include <X11/XKBlib.h>
#include <QX11Info>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent)
{
	teacherMode = QCoreApplication::arguments().contains("-t") || QCoreApplication::arguments().contains("--teacher");
	//qDebug() << "ARGS: " << QCoreApplication::arguments();
	//qDebug() << "Teacher: " << (teacherMode? "yes" : "no");
	dummyCursor = NULL;
	newUndoStack = new KumUndoStack;
	newUndoStack->inMacro = false;
	newRedoStack = new KumRedoStack;
	forceEmitCompilation = false;
	setLineWrapMode(QTextEdit::NoWrap);
	setUndoRedoEnabled(false);

	macro = NULL;
	prevIndexUndoStack = 0;

	applyIndentation();

	initKeymaps();
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(emitCursorPosition()));
	connect(this, SIGNAL(textChanged()), this, SLOT(checkUndoStackModified()));
//  connect ( this, SIGNAL(textChanged()), this, SLOT(keepEmptyLine()) );

	lockKeepEmptyLine = false;
	//connect ( &undoStack, SIGNAL(indexChanged(int)), this, SLOT(checkUndoStackModified(int)) );
	setFont(font());
}

void TextEdit::setFont(const QFont &f)
{
	QTextEdit::setFont(f);
	QFont ff(f);
	ff.setPointSize(f.pointSize() - 2);
	i_lineNumberingMarginWidth = QFontMetrics(f).width("000") + 16;
}

void TextEdit::initKeymaps()
{
	rusLatMap[QString::fromUtf8("ё")] = "`";
	rusLatMap[QString::fromUtf8("Ё")] = "~";
	rusLatMap[QString::fromUtf8("1")] = "|";
	rusLatMap[QString::fromUtf8("!")] = "!";
	rusLatMap[QString::fromUtf8("2")] = "2";
	rusLatMap[QString::fromUtf8("\"")] = "@";
	rusLatMap[QString::fromUtf8("3")] = "3";
	rusLatMap[QString::fromUtf8("№")] = "#";
	rusLatMap[QString::fromUtf8("4")] = "4";
	rusLatMap[QString::fromUtf8(";")] = "$";
	rusLatMap[QString::fromUtf8("5")] = "5";
	rusLatMap[QString::fromUtf8("%")] = "%";
	rusLatMap[QString::fromUtf8("6")] = "6";
	rusLatMap[QString::fromUtf8(":")] = "^";
	rusLatMap[QString::fromUtf8("7")] = "7";
	rusLatMap[QString::fromUtf8("?")] = "&";
	rusLatMap[QString::fromUtf8("8")] = "8";
	rusLatMap[QString::fromUtf8("*")] = "*";
	rusLatMap[QString::fromUtf8("9")] = "9";
	rusLatMap[QString::fromUtf8("(")] = "(";
	rusLatMap[QString::fromUtf8("0")] = "0";
	rusLatMap[QString::fromUtf8(")")] = ")";
	rusLatMap[QString::fromUtf8("=")] = ":=";
	rusLatMap[QString::fromUtf8("+")] = "<>";
	rusLatMap[QString::fromUtf8("\\")] = "\\";
	rusLatMap[QString::fromUtf8("/")] = "|";
	rusLatMap[QString::fromUtf8("й")] = "q";
	rusLatMap[QString::fromUtf8("Й")] = "Q";
	rusLatMap[QString::fromUtf8("ц")] = "w";
	rusLatMap[QString::fromUtf8("Ц")] = "W";
	rusLatMap[QString::fromUtf8("у")] = "e";
	rusLatMap[QString::fromUtf8("У")] = "E";
	rusLatMap[QString::fromUtf8("к")] = "r";
	rusLatMap[QString::fromUtf8("К")] = "R";
	rusLatMap[QString::fromUtf8("е")] = "t";
	rusLatMap[QString::fromUtf8("Е")] = "T";
	rusLatMap[QString::fromUtf8("н")] = "y";
	rusLatMap[QString::fromUtf8("Н")] = "Y";
	rusLatMap[QString::fromUtf8("г")] = "u";
	rusLatMap[QString::fromUtf8("Г")] = "U";
	rusLatMap[QString::fromUtf8("ш")] = "i";
	rusLatMap[QString::fromUtf8("Ш")] = "I";
	rusLatMap[QString::fromUtf8("щ")] = "o";
	rusLatMap[QString::fromUtf8("Щ")] = "O";
	rusLatMap[QString::fromUtf8("з")] = "p";
	rusLatMap[QString::fromUtf8("З")] = "P";
	rusLatMap[QString::fromUtf8("х")] = "[";
	rusLatMap[QString::fromUtf8("Х")] = "{";
	rusLatMap[QString::fromUtf8("ъ")] = "]";
	rusLatMap[QString::fromUtf8("Ъ")] = "}";
	rusLatMap[QString::fromUtf8("ф")] = "a";
	rusLatMap[QString::fromUtf8("Ф")] = "A";
	rusLatMap[QString::fromUtf8("ы")] = "s";
	rusLatMap[QString::fromUtf8("Ы")] = "S";
	rusLatMap[QString::fromUtf8("в")] = "d";
	rusLatMap[QString::fromUtf8("В")] = "D";
	rusLatMap[QString::fromUtf8("а")] = "f";
	rusLatMap[QString::fromUtf8("А")] = "F";
	rusLatMap[QString::fromUtf8("п")] = "g";
	rusLatMap[QString::fromUtf8("П")] = "G";
	rusLatMap[QString::fromUtf8("р")] = "h";
	rusLatMap[QString::fromUtf8("Р")] = "H";
	rusLatMap[QString::fromUtf8("о")] = "j";
	rusLatMap[QString::fromUtf8("О")] = "J";
	rusLatMap[QString::fromUtf8("л")] = "k";
	rusLatMap[QString::fromUtf8("Л")] = "K";
	rusLatMap[QString::fromUtf8("д")] = "l";
	rusLatMap[QString::fromUtf8("Д")] = "L";
	rusLatMap[QString::fromUtf8("ж")] = ";";
	rusLatMap[QString::fromUtf8("Ж")] = ":";
	rusLatMap[QString::fromUtf8("э")] = "'";
	rusLatMap[QString::fromUtf8("Э")] = "\"";
	rusLatMap[QString::fromUtf8("я")] = "z";
	rusLatMap[QString::fromUtf8("Я")] = "Z";
	rusLatMap[QString::fromUtf8("ч")] = "x";
	rusLatMap[QString::fromUtf8("Ч")] = "X";
	rusLatMap[QString::fromUtf8("с")] = "c";
	rusLatMap[QString::fromUtf8("С")] = "C";
	rusLatMap[QString::fromUtf8("м")] = "v";
	rusLatMap[QString::fromUtf8("М")] = "V";
	rusLatMap[QString::fromUtf8("и")] = "b";
	rusLatMap[QString::fromUtf8("И")] = "B";
	rusLatMap[QString::fromUtf8("т")] = "n";
	rusLatMap[QString::fromUtf8("Т")] = "N";
	rusLatMap[QString::fromUtf8("ь")] = "m";
	rusLatMap[QString::fromUtf8("Ь")] = "M";
	rusLatMap[QString::fromUtf8("Б")] = ",";
	rusLatMap[QString::fromUtf8("б")] = "<";
	rusLatMap[QString::fromUtf8("ю")] = ">";
	rusLatMap[QString::fromUtf8("Ю")] = ".";
	rusLatMap[QString::fromUtf8(".")] = "/";
	rusLatMap[QString::fromUtf8(",")] = "?";

	rusLatMap[QString::fromUtf8("@")] = QString::fromUtf8("\"");
	rusLatMap[QString::fromUtf8("#")] = QString::fromUtf8("№");
	rusLatMap[QString::fromUtf8("$")] = QString::fromUtf8(";");
	rusLatMap[QString::fromUtf8("^")] = QString::fromUtf8(":");
	rusLatMap[QString::fromUtf8("&")] = QString::fromUtf8("?");


	rusLatMap[QString::fromUtf8("`")] = QString::fromUtf8("ё");
	rusLatMap[QString::fromUtf8("~")] = QString::fromUtf8("Ё");
	rusLatMap[QString::fromUtf8("q")] = QString::fromUtf8("й");
	rusLatMap[QString::fromUtf8("Q")] = QString::fromUtf8("Й");
	rusLatMap[QString::fromUtf8("w")] = QString::fromUtf8("ц");
	rusLatMap[QString::fromUtf8("W")] = QString::fromUtf8("Ц");
	rusLatMap[QString::fromUtf8("e")] = QString::fromUtf8("у");
	rusLatMap[QString::fromUtf8("E")] = QString::fromUtf8("У");
	rusLatMap[QString::fromUtf8("r")] = QString::fromUtf8("к");
	rusLatMap[QString::fromUtf8("R")] = QString::fromUtf8("К");
	rusLatMap[QString::fromUtf8("t")] = QString::fromUtf8("е");
	rusLatMap[QString::fromUtf8("T")] = QString::fromUtf8("Е");
	rusLatMap[QString::fromUtf8("y")] = QString::fromUtf8("н");
	rusLatMap[QString::fromUtf8("Y")] = QString::fromUtf8("Н");
	rusLatMap[QString::fromUtf8("u")] = QString::fromUtf8("г");
	rusLatMap[QString::fromUtf8("U")] = QString::fromUtf8("Г");
	rusLatMap[QString::fromUtf8("i")] = QString::fromUtf8("ш");
	rusLatMap[QString::fromUtf8("I")] = QString::fromUtf8("Ш");
	rusLatMap[QString::fromUtf8("o")] = QString::fromUtf8("щ");
	rusLatMap[QString::fromUtf8("O")] = QString::fromUtf8("Щ");
	rusLatMap[QString::fromUtf8("p")] = QString::fromUtf8("з");
	rusLatMap[QString::fromUtf8("P")] = QString::fromUtf8("З");
	rusLatMap[QString::fromUtf8("[")] = QString::fromUtf8("х");
	rusLatMap[QString::fromUtf8("{")] = QString::fromUtf8("Х");
	rusLatMap[QString::fromUtf8("]")] = QString::fromUtf8("ъ");
	rusLatMap[QString::fromUtf8("}")] = QString::fromUtf8("Ъ");
	rusLatMap[QString::fromUtf8("a")] = QString::fromUtf8("ф");
	rusLatMap[QString::fromUtf8("A")] = QString::fromUtf8("Ф");
	rusLatMap[QString::fromUtf8("s")] = QString::fromUtf8("ы");
	rusLatMap[QString::fromUtf8("S")] = QString::fromUtf8("Ы");
	rusLatMap[QString::fromUtf8("d")] = QString::fromUtf8("в");
	rusLatMap[QString::fromUtf8("D")] = QString::fromUtf8("В");
	rusLatMap[QString::fromUtf8("f")] = QString::fromUtf8("а");
	rusLatMap[QString::fromUtf8("F")] = QString::fromUtf8("А");
	rusLatMap[QString::fromUtf8("g")] = QString::fromUtf8("п");
	rusLatMap[QString::fromUtf8("G")] = QString::fromUtf8("П");
	rusLatMap[QString::fromUtf8("h")] = QString::fromUtf8("р");
	rusLatMap[QString::fromUtf8("H")] = QString::fromUtf8("Р");
	rusLatMap[QString::fromUtf8("j")] = QString::fromUtf8("о");
	rusLatMap[QString::fromUtf8("J")] = QString::fromUtf8("О");
	rusLatMap[QString::fromUtf8("k")] = QString::fromUtf8("л");
	rusLatMap[QString::fromUtf8("K")] = QString::fromUtf8("Л");
	rusLatMap[QString::fromUtf8("l")] = QString::fromUtf8("д");
	rusLatMap[QString::fromUtf8("L")] = QString::fromUtf8("Д");
	rusLatMap[QString::fromUtf8(";")] = QString::fromUtf8("ж");
	rusLatMap[QString::fromUtf8(":")] = QString::fromUtf8("Ж");
	rusLatMap[QString::fromUtf8("'")] = QString::fromUtf8("э");
//   rusLatMap[QString::fromUtf8("\"")] = QString::fromUtf8("Э");
	rusLatMap[QString::fromUtf8("z")] = QString::fromUtf8("я");
	rusLatMap[QString::fromUtf8("Z")] = QString::fromUtf8("Я");
	rusLatMap[QString::fromUtf8("x")] = QString::fromUtf8("ч");
	rusLatMap[QString::fromUtf8("X")] = QString::fromUtf8("Ч");
	rusLatMap[QString::fromUtf8("c")] = QString::fromUtf8("с");
	rusLatMap[QString::fromUtf8("C")] = QString::fromUtf8("С");
	rusLatMap[QString::fromUtf8("v")] = QString::fromUtf8("м");
	rusLatMap[QString::fromUtf8("V")] = QString::fromUtf8("М");
	rusLatMap[QString::fromUtf8("b")] = QString::fromUtf8("и");
	rusLatMap[QString::fromUtf8("B")] = QString::fromUtf8("И");
	rusLatMap[QString::fromUtf8("n")] = QString::fromUtf8("т");
	rusLatMap[QString::fromUtf8("N")] = QString::fromUtf8("Т");
	rusLatMap[QString::fromUtf8("m")] = QString::fromUtf8("ь");
	rusLatMap[QString::fromUtf8("M")] = QString::fromUtf8("Ь");
//   rusLatMap[QString::fromUtf8(",")] = QString::fromUtf8("б");
	rusLatMap[QString::fromUtf8("<")] = QString::fromUtf8("Б");
//   rusLatMap[QString::fromUtf8(".")] = QString::fromUtf8("ю");
	rusLatMap[QString::fromUtf8(">")] = QString::fromUtf8("Ю");
//   rusLatMap[QString::fromUtf8("/")] = QString::fromUtf8(".");
	rusLatMap[QString::fromUtf8("?")] = QString::fromUtf8(",");


}

void TextEdit::applyIndentation()
{
	QTextBlock block = document()->begin();
	while (block.isValid()) {
		QTextBlockFormat format = block.blockFormat();
		if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
			format.setTextIndent(i_lineNumberingMarginWidth);
		} else {
			format.setTextIndent(0);
		}
		QTextCursor c = QTextCursor(block);
		c.setBlockFormat(format);
		block = block.next();
	}
	update();

}

void TextEdit::deleteCurrentLine()
{
	if (!textCursor().hasSelection()) {
		QTextBlock b = textCursor().block();
		int start = b.position();
		int end = start + b.length();
		QTextCursor c(document());
		c.setPosition(start, QTextCursor::MoveAnchor);
		c.setPosition(end, QTextCursor::KeepAnchor);
		setTextCursor(c);
	}
	doDelete();
	qDebug("Delete current line");
}

void TextEdit::keyPressEvent(QKeyEvent *event)
{
	//qDebug() << "Keyboard: " << QApplication::keyboardInputLocale().name();
	if (isReadOnly()) {
		return;
	}
	if (event->text().length() > 0) {
		app()->mainWindow->sMessage->setText("");    // Do not react for SHIFT, CTRL, ALT, etc.
	}
	QString sDbg = event->text();
	QChar cDbg;
	ushort code = 0;
	if (sDbg.length() > 0) {
		cDbg = sDbg[0];
		code = cDbg.unicode();
	}


	if (code == 27) {
		event->accept();
		return;
	}
	if (macro != NULL) {
		// app()->addEventToHistory("AddMacro KeySequence pressed");
		macro->addCommand(event);
	}
	if (event->key() == Qt::Key_Insert && event->modifiers() == 0) {
		event->accept();
		setOverwriteMode(!overwriteMode());
		emit overwriteModeChanged(overwriteMode());

	} else if (event->matches(QKeySequence::Undo)) {
		if (isReadOnly()) {
			event->ignore();

			return;
		}
		// app()->addEventToHistory("Undo KeySequence pressed");
		undo();
	} else if (event->matches(QKeySequence::Redo)) {
		if (isReadOnly()) {
			event->ignore();

			return;
		}
		// app()->addEventToHistory("Redo KeySequence pressed");
		redo();

	} else if (event->matches(QKeySequence::Cut)) {
		if (isReadOnly()) {
			event->ignore();

			return;
		}
		// app()->addEventToHistory("Cut KeySequence pressed");
		cut();

	} else if (event->modifiers() & Qt::ControlModifier) {

//		// Если нажат Ctrl или Command,
//		// то передаем управление стандартному обработчику
//		// для вызова Hotkey
//		// app()->addEventToHistory("Ctrl or Command pressed");
//		if (event->key()==89)
//		{
//			if ( isReadOnly() )
//			{
//				event->ignore();

//				return;
//			}
//			// app()->addEventToHistory("Ctrl-Y pressed");
//			deleteCurrentLine();

//		}
//		else if (event->key()==Qt::Key_C && (event->modifiers() & Qt::ControlModifier)&& ( event->modifiers()& Qt::ShiftModifier))  {
//			event->accept();
//			copyWithFormatting();

//		}
//		else {

		QTextEdit::keyPressEvent(event);
//		}
	}
#ifndef Q_OS_MAC
	else if (event->key() == Qt::Key_Meta) {
		event->accept();
//      qDebug("Meta pressed");

		return;
	}
#endif
	else if ((event->modifiers() & Qt::AltModifier) && (event->key() == Qt::Key_Backspace)) {
		// app()->addEventToHistory("Undo KeySequence pressed <2>");
		undo();

	} else if ((event->modifiers() & Qt::AltModifier) && ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))) {
		// app()->addEventToHistory("Redo KeySequence pressed <2>");
		redo();

	} else if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
		// Перехватываем Enter
		// app()->addEventToHistory("Enter pressed <2>");

		keyPressEnter(event);

		return;
	} else if (event->key() == Qt::Key_Delete) {
		// Перехватываем Delete
		// app()->addEventToHistory("Delete pressed");
		QString txt = textCursor().block().text();
		int p = textCursor().columnNumber();
		if (txt.length() == p) {
			forceEmitCompilation = true;
		}
		keyPressDelete(event);
		emit cursorPositionChanged(textCursor().blockNumber(), textCursor().columnNumber());

		return;
	} else if (event->key() == Qt::Key_Backspace) {
		// Перехватываем Backspace
		// app()->addEventToHistory("Backspace pressed");
		emit cursorPositionChanged(textCursor().blockNumber(), textCursor().columnNumber());
		int p = textCursor().columnNumber();
		if (p == 0) {
			forceEmitCompilation = true;
		}
		keyPressBackspace(event);

		ensureCursorVisible();
		if (textCursor().columnNumber() == 0) {
			horizontalScrollBar()->setValue(0);
		}
		return;
	}
#ifdef Q_OS_MAC
	else if (event->text().length() > 0 &&
		QChar(event->text()[0]).isPrint()
	)
#else
	else if (event->text().length() > 0)
#endif
	{
		// Перехватываем изменение текста
		// app()->addEventToHistory(event->text()+" pressed");
		keyPressCharect(event);

		return;
	} else if (event->key() == Qt::Key_Up) {
		keyPressUp(event);
	} else if (event->key() == Qt::Key_Left) {
		bool filter = false;
		if (dummyCursor != NULL) {
			if (dummyCursor->isVisible()) {
				uint dci = dummyCursor->indent;
				if (dci > 0) {
					filter = true;
					dummyCursor->moveLeft();
				} else {
					filter = false;
					dummyCursor->hide();
				}
			} else {
				uint line = textCursor().blockNumber();
				uint col = textCursor().columnNumber();
				if (col == 0) {
					QFontMetrics fm(font());
					double baseWidth = 2 * fm.width("w");
					uint indentCount = 0;
					if (app()->settings->value("Appearance/ShowLineNumbers", false).toBool()) {
						double indentWidth = textCursor().block().blockFormat().textIndent() - i_lineNumberingMarginWidth;
						indentCount = (uint)(indentWidth / baseWidth);
					} else {
						double indentWidth = textCursor().block().blockFormat().textIndent();
						indentCount = (uint)(indentWidth / baseWidth);
					}
					if (indentCount > 0) {
						filter = true;
						dummyCursor->show(line, indentCount - 1);
					}
				}
			}
		}
		if (filter) {
			event->accept();
		} else {
			QTextEdit::keyPressEvent(event);
		}
		ensureCursorVisible();
		if (textCursor().columnNumber() == 0) {
			horizontalScrollBar()->setValue(0);
		}
	} else if (event->key() == Qt::Key_Right) {
		bool filter = false;

		if (dummyCursor != NULL) {
			if (dummyCursor->isVisible()) {
				QFontMetrics fm(font());
				double baseWidth = 2 * fm.width("w");
				uint indentCount = 0;
				if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
					double indentWidth = textCursor().block().blockFormat().textIndent() - i_lineNumberingMarginWidth;
					indentCount = (uint)(indentWidth / baseWidth);
				} else {
					double indentWidth = textCursor().block().blockFormat().textIndent();
					indentCount = (uint)(indentWidth / baseWidth);
				}
				uint dci = dummyCursor->indent;
				if ((dci + 1) < indentCount) {
					filter = true;
					dummyCursor->moveRight();
				} else {
					filter = true;
					dummyCursor->hide();
				}
				if (filter) {
					event->accept();
				} else {
					QTextEdit::keyPressEvent(event);
				}
			} else {
				QTextEdit::keyPressEvent(event);
				uint line = textCursor().blockNumber();
				uint col = textCursor().columnNumber();
				if (col == 0) {
					QFontMetrics fm(font());
					double baseWidth = 2 * fm.width("w");
					uint indentCount = 0;
					if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
						double indentWidth = textCursor().block().blockFormat().textIndent() - i_lineNumberingMarginWidth;
						indentCount = (uint)(indentWidth / baseWidth);
					} else {
						double indentWidth = textCursor().block().blockFormat().textIndent();
						indentCount = (uint)(indentWidth / baseWidth);
					}
					if (indentCount > 0) {
						filter = true;
						dummyCursor->show(line, 0);
					}
				}
			}
		} else {
			QTextEdit::keyPressEvent(event);
		}

	} else if (event->key() == Qt::Key_Down) {
		keyPressDown(event);
	} else if (event->key() == Qt::Key_End || event->key() == Qt::Key_Home) {
		if (dummyCursor != NULL) {
			dummyCursor->hide();
		}
		QTextEdit::keyPressEvent(event);
	} else {
		//app()->addEventToHistory("Something else pressed");

		QTextEdit::keyPressEvent(event);
	}
}

uint TextEdit::getIndentCount(QTextBlock block)
{
	uint result = 0;
	QFontMetrics fm(font());
	double baseWidth = 2 * fm.width("w");
	if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
		double indentWidth = block.blockFormat().textIndent() - i_lineNumberingMarginWidth;
		result = (uint)(indentWidth / baseWidth);
	} else {
		double indentWidth = block.blockFormat().textIndent();
		result = (uint)(indentWidth / baseWidth);
	}
	return result;
}

void TextEdit::keyPressUp(QKeyEvent *event)
{

	event->accept();
	bool sel = event->modifiers() & Qt::ShiftModifier;
	uint indentRatio = app()->settings->value("Editor/IndentRatio", 2).toUInt();

	uint line = textCursor().blockNumber();
	uint col = textCursor().columnNumber();

	if (line == 0) {
		return;
	}

	QTextCursor cur = textCursor();

	uint pind = getIndentCount(cur.block());

	cur.movePosition(QTextCursor::StartOfBlock,
		sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
	cur.movePosition(QTextCursor::Left,
		sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

	uint len = cur.block().text().length();
	uint ind = getIndentCount(cur.block());

	if (dummyCursor != NULL && dummyCursor->isVisible()) {
		col += dummyCursor->indent * indentRatio;
	} else {
		col += pind * indentRatio;
	}

	uint newCol = qMin(col, len + ind * indentRatio);
	if (newCol <= ind * indentRatio) {
		cur.movePosition(QTextCursor::StartOfBlock,
			sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
		if (dummyCursor != NULL && newCol < ind * indentRatio) {
			if (newCol % indentRatio == 0) {
				dummyCursor->show(line - 1, newCol / indentRatio);
			} else {
				dummyCursor->hide();
			}
		} else if (dummyCursor != NULL) {
			dummyCursor->hide();
		}
	} else {
		cur.movePosition(QTextCursor::Left,
			sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor,
			(len + ind * indentRatio) - newCol);
		if (dummyCursor != NULL && dummyCursor->isVisible()) {
			dummyCursor->hide();
		}
	}

	setTextCursor(cur);
}

void TextEdit::keyPressDown(QKeyEvent *event)
{
	event->accept();
	bool sel = event->modifiers() & Qt::ShiftModifier;
	int indentRatio = app()->settings->value("Editor/IndentRatio", 2).toInt();
	K_ASSERT(indentRatio >= 0);

	int line = textCursor().blockNumber();
	int col = textCursor().columnNumber();

	if (line == document()->blockCount() - 1) {
		return;
	}

	QTextCursor cur = textCursor();

	int pind = getIndentCount(cur.block());
	K_ASSERT(pind >= 0);

	cur.movePosition(QTextCursor::EndOfBlock,
		sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
	cur.movePosition(QTextCursor::Right,
		sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

	int len = cur.block().text().length();
	int ind = getIndentCount(cur.block());

	if (dummyCursor != NULL && dummyCursor->isVisible()) {
		col += dummyCursor->indent * indentRatio;
	} else {
		col += pind * indentRatio;
	}

	int newCol = qMin(col, len + ind * indentRatio);
	if (dummyCursor != NULL && newCol < ind * indentRatio) {
		if (newCol % indentRatio == 0) {
			dummyCursor->show(line + 1, newCol / indentRatio);
		} else if (dummyCursor != NULL) {
			dummyCursor->hide();
		}
	} else {
		cur.movePosition(QTextCursor::Right,
			sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor,
			newCol - ind * indentRatio);
		if (dummyCursor != NULL && dummyCursor->isVisible()) {
			dummyCursor->hide();
		}
	}

	setTextCursor(cur);
}

void TextEdit::keyPressCharect(QKeyEvent *event)
{
	if (dummyCursor != NULL && dummyCursor->isVisible()) {
		event->accept();
		dummyCursor->hide();
//      return;
	}
	QMutexLocker l(&editMutex);
	QTextCursor c = textCursor();
	int cn = c.columnNumber();
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	if (lp != NULL) {
		bool sel = event->modifiers() & Qt::ShiftModifier;
		int i = lp->indentCount * 2;
		if (cn < i) {
			c.movePosition(QTextCursor::Right, sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, i - cn);
			setTextCursor(c);
			return;
		}
	}


	if (event->modifiers() & Qt::AltModifier) {
		QString newText = event->text();
#ifndef Q_OS_MAC
		if (rusLatMap.contains(event->text())) {
			bool capsLock = false;
#ifdef Q_OS_UNIX
			Display *d = QX11Info::display();
			if (d) {
				unsigned n;
				XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
				capsLock = (n & 0x01) == 1;
			}
#endif
#ifdef Q_OS_WIN32
			capsLock = GetKeyState(VK_CAPITAL) == 1;
#endif
			bool shift = event->modifiers() & Qt::ShiftModifier;
			bool cap = (capsLock && !shift) || (shift && !capsLock);
			qDebug() << event->text();
#if QT_VERSION >= 0x50000
			QString keybLang = QApplication::inputMethod()->locale().name();
#else
			QString keybLang = QApplication::keyboardInputLocale().name();
#endif
			qDebug() << keybLang;
			bool russianKeyboard = keybLang.contains("ru");
#ifdef Q_OS_WIN32
			HKL l = GetKeyboardLayout(0);
			russianKeyboard = unsigned(l) == 0x4190419;
			qDebug() << l;
#endif
			if (event->text() == "," && !russianKeyboard) {
				newText = cap ? QString::fromUtf8("Б") : QString::fromUtf8("б");
			} else if (event->text() == "." && !russianKeyboard) {
				newText = cap ? QString::fromUtf8("Ю") : QString::fromUtf8("ю");
			} else if (event->text() == ";" && !russianKeyboard) {
				newText = cap ? QString::fromUtf8("Ж") : QString::fromUtf8("ж");
			} else if (event->text() == "'" && !russianKeyboard) {
				newText = cap ? QString::fromUtf8("Э") : QString::fromUtf8("э");
			} else if (event->text() == "/" && !russianKeyboard) {
				newText = QString::fromUtf8(".");
			} else if (event->text() == "[" && cap && !russianKeyboard) {
				newText = QString::fromUtf8("Х");
			} else if (event->text() == "]" && cap && !russianKeyboard) {
				newText = QString::fromUtf8("Ъ");
			} else {
				newText = rusLatMap[event->text()];
			}
		}
#endif
		event->ignore();

		doCharect(newText);

	} else {

		doCharect(event->text());
		event->accept();

	}
	ensureCursorVisible();
}

void TextEdit::keyPressEnter(QKeyEvent *event)
{
	QTextCursor c = textCursor();
	int cn = c.columnNumber();
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	if (lp != NULL) {
		bool sel = event->modifiers() & Qt::ShiftModifier;
		int i = lp->indentCount * 2;
		if (cn < i) {
			c.movePosition(QTextCursor::Right, sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, i - cn);
			setTextCursor(c);
			return;
		}
	}
	// app()->addEventToHistory("Enter pressed");
	if (event->modifiers() & Qt::AltModifier) {
		// app()->addEventToHistory("Redo KeySequence pressed <3>");
		redo();
		return;
	}
	if (macro != NULL) {
		macro->addCommand(event);
	}
	QMutexLocker l(&editMutex);
	QTextBlock curBlock = textCursor().block();

	doEnter();
	QTextBlock nextBlock = textCursor().block();
//  emit undoAvailable(undoStack.canUndo());
	event->accept();
	initNewBlock(curBlock, nextBlock);
	forceEmitCompilation = true;
	emit cursorPositionChanged(textCursor().blockNumber(), textCursor().columnNumber());
//  applyIndentation();
	ensureCursorVisible();
}

void TextEdit::keyPressDelete(QKeyEvent *event)
{
	if (dummyCursor != NULL && dummyCursor->isVisible()) {
		event->accept();
		dummyCursor->hide();
		if (!textCursor().hasSelection()) {
			return;
		}
	}
	QTextCursor c = textCursor();
	int cn = c.columnNumber();
	if (c.atBlockEnd() && c.block().next().isValid() && c.block().next().userState() == PROTECTED && !teacherMode) {
		event->accept();
		return;
	}
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	if (lp != NULL) {
		bool sel = event->modifiers() & Qt::ShiftModifier;
		int i = lp->indentCount * 2;
		if (cn < i) {
			c.movePosition(QTextCursor::Right, sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, i - cn);
			setTextCursor(c);
			return;
		}
	}
	QMutexLocker l(&editMutex);
	if ((textCursor().position() < toPlainText().length()) || (textCursor().hasSelection())) {

		doDelete();
		event->accept();
	} else {
		event->ignore();
	}
}

void TextEdit::mouseReleaseEvent(QMouseEvent *event)
{
	setCursorWidth(0);
	QTextEdit::mouseReleaseEvent(event);
	int line = textCursor().blockNumber();
	int col = textCursor().columnNumber();
	int ind = getIndentCount(textCursor().block());
	if (dummyCursor != NULL) {
		dummyCursor->hide();
	}
	if (col > 0 || ind == 0 || event->button() != Qt::LeftButton) {
		setCursorWidth(1);
		return;
	}
	double x = event->x();
	uint indentRatio = app()->settings->value("Editor/IndentRatio", 2).toUInt();
	int x_offset = 0;
	if (horizontalScrollBar()->maximum() > 0) {
		double ratio;
		int s_max = horizontalScrollBar()->maximum();
		int s_val = horizontalScrollBar()->value();
		int v_width = width();
		double a_width = document()->size().width();
		ratio = (double)s_val / (double)s_max;
		int pixelsHidden = (int)((a_width - v_width) * ratio);
		x_offset = pixelsHidden;
	}
	x += x_offset;
	QFontMetrics fm(font());
	double indentWidth = indentRatio * fm.width("w");

	if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
		x -= i_lineNumberingMarginWidth;
	}
	int clickedIndent = ind;
	double DELTA = 3.0;
	for (int i = 0; i < ind; i++) {
		x -= indentWidth;
		if (x < DELTA) {
			clickedIndent = i;
			break;
		}
	}


	if (clickedIndent < ind) {
		if (dummyCursor != NULL) {
			dummyCursor->show(line, clickedIndent);
		}
	}
}

void TextEdit::keyPressBackspace(QKeyEvent *event)
{
	if (dummyCursor != NULL && dummyCursor->isVisible() && dummyCursor->indent > 0) {
		event->accept();
		dummyCursor->hide();
		return;
	}
	if (dummyCursor != NULL) {
		dummyCursor->hide();
	}
	QTextCursor c = textCursor();
	if (c.columnNumber() == 0 && c.block().previous().isValid() && c.block().previous().userState() == PROTECTED && !teacherMode) {
		event->accept();
		c.movePosition(QTextCursor::PreviousBlock);
		c.movePosition(QTextCursor::EndOfBlock);
		setTextCursor(c);
		return;
	}
	int cn = c.columnNumber();
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	if (lp != NULL) {
		bool sel = event->modifiers() & Qt::ShiftModifier;
		int i = lp->indentCount * 2;
		if (cn < i) {
			c.movePosition(QTextCursor::Right, sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, i - cn);
			setTextCursor(c);
			return;
		}
	}
	QMutexLocker l(&editMutex);
	if ((textCursor().position() > 0) || (textCursor().hasSelection())) {

		doBackspace();
		event->accept();
	} else {
		event->ignore();
	}
}

bool TextEdit::isKumirEditor()
{
	return false;
}



void TextEdit::paintWidget(QPainter *painter)
{
	int x_offset = 0;
	if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
		if (horizontalScrollBar()->maximum() > 0) {
			double ratio;
			int s_max = horizontalScrollBar()->maximum();
			int s_val = horizontalScrollBar()->value();
			int v_width = width();
			double a_width = document()->size().width();
			ratio = (double)s_val / (double)s_max;
			int pixelsHidden = (int)((a_width - v_width) * ratio);
			x_offset = pixelsHidden;
		}
		painter->setPen(Qt::NoPen);
		painter->setBrush(QColor("#FFFFDD"));
		QRect marginRect = QRect(-x_offset, 0, i_lineNumberingMarginWidth, this->height());
		painter->drawRect(marginRect);
		QFont smallFont = this->font();
		smallFont.setPointSize(this->font().pointSize() - 2);
		painter->setFont(smallFont);
		QTextBlock block = document()->begin();
		int i = 0;
		while (block.isValid()) {
			QTextCursor cursor(block);
			QRect rect = cursorRect(cursor);
			rect.moveLeft(-x_offset);
			rect.setWidth((int)(document()->size().width()));
			QPen pen;
			pen.setStyle(Qt::SolidLine);
			pen.setWidth(1);
			painter->setPen(pen);
			painter->setBrush(Qt::red);
			i++;
			rect = cursorRect(cursor);
			QRect lineNumber = QRect(-x_offset,
					rect.y(),
					i_lineNumberingMarginWidth - QFontMetrics(font()).width(QString::number(i)),
					rect.height());
			painter->drawText(lineNumber, Qt::AlignRight | Qt::AlignVCenter, QString::number(i));

			block = block.next();
		}
	}
}


void TextEdit::paintEvent(QPaintEvent *event)
{
	QPainter painter(viewport());
	paintWidget(&painter);
	painter.end();
	QTextEdit::paintEvent(event);
}

void TextEdit::setText(const QString &txt)
{
	QMutexLocker l(&editMutex);
	QTextEdit::setPlainText(txt);
	applyIndentation();
	update();
}

void TextEdit::insertFromMimeData(const QMimeData *source)
{
	QMutexLocker l(&editMutex);

	doInsert(source->text());
}

bool TextEdit::isRedoAvailable()
{
	return newRedoStack->stack.count() > 0;
}

bool TextEdit::isUndoAvailable()
{
	return newUndoStack->stack.count() > 0;
}

void TextEdit::undo()
{
	if (isReadOnly()) {
		return;
	}
	QMutexLocker l(&editMutex);
//  undoStack.undo();
//  emit undoAvailable(undoStack.canUndo());
//  emit redoAvailable(undoStack.canRedo());
	revert();
}

void TextEdit::redo()
{
	if (isReadOnly()) {
		return;
	}
	QMutexLocker l(&editMutex);
//  undoStack.redo();
//  emit undoAvailable(undoStack.canUndo());
//  emit redoAvailable(undoStack.canRedo());
	revert(true);
}

void TextEdit::initLineProps(QPointer<QTextDocument>)
{
}

void TextEdit::cut()
{
	if (isReadOnly()) {
		return;
	}

	QTextBlock block = document()->findBlock(textCursor().selectionStart());
	QTextBlock endBlock = document()->findBlock(textCursor().selectionEnd());
	bool hasProtectedLines = false;
	do {
		if (block.userState() == PROTECTED && !teacherMode) {
			hasProtectedLines = true;
			break;
		}
		if (block == endBlock) {
			break;
		}
		block = block.next();
	} while (block != endBlock);
	if (hasProtectedLines) {
		return;
	}
	QMutexLocker l(&editMutex);

	doCut();
}

void TextEdit::initNewBlock(QTextBlock prevBlock, QTextBlock newBlock)
{
	Q_UNUSED(prevBlock);
	Q_UNUSED(newBlock);
}

void TextEdit::lock()
{
	editMutex.lock();
}

void TextEdit::unlock()
{
	editMutex.unlock();
}

void TextEdit::recordMacro(Macro *m)
{
	macro = m;
}

void TextEdit::playMacro(Macro *m)
{

//  undoStack.beginMacro("Macro "+m->name());
	int i = 0;
	save(0, (uint)MacroCMD, QChar('\0'), false);
	newUndoStack->inMacro = true;
	foreach (QKeyEvent e, m->commands()) {
		i++;
		QKeyEvent *ee = new QKeyEvent(e.type(),
			e.key(),
			e.modifiers(),
			e.text(),
			e.isAutoRepeat(),
			e.count());
		keyPressEvent(ee);
	}
	newUndoStack->inMacro = false;
//  undoStack.endMacro();
//  emit undoAvailable(undoStack.canUndo());
//  emit redoAvailable(undoStack.canRedo());
}

void TextEdit::stopMacro()
{
	macro = NULL;
}

void TextEdit::changeText(const QString &txt)
{
	doSetText(txt);
}

void TextEdit::emitCursorPosition()
{
	int row = textCursor().blockNumber();
	int col = textCursor().columnNumber();
	emit cursorPositionChanged(row, col);
}

void TextEdit::setModified(bool f)
{
	//qDebug()<<"TextEdit::setModified("<<f<<")";
	if (f) {
		QTabWidget *tw = app()->mainWindow->tabWidget;
		int cur = tw->currentIndex();
		if (!tw->tabText(cur).endsWith("*")) {
			tw->setTabText(cur, tw->tabText(cur) + "*");
		}
	} else {
		clearUndoStack();
		prevIndexUndoStack = newUndoStack->stack.count();
		QTabWidget *tw = app()->mainWindow->tabWidget;
		int cur = tw->currentIndex();
		QString s = tw->tabText(cur);
		if (s.endsWith("*")) {
			tw->setTabText(cur, s.mid(0, s.length() - 1));
		}
	}
}

bool TextEdit::isModified()
{
//  return undoStack.index() != prevIndexUndoStack;
	return newUndoStack->stack.count() != prevIndexUndoStack;
}

void TextEdit::checkUndoStackModified(int c)
{
	Q_UNUSED(c);
	setModified(undoStack.index() != prevIndexUndoStack);
}

void TextEdit::checkUndoStackModified()
{
	//qDebug()<<"TextEdit::checkUndoStackModified() "<<newUndoStack->stack.count()<<prevIndexUndoStack;
	setModified(newUndoStack->stack.count() != prevIndexUndoStack);
}

void TextEdit::clearUndoStack()
{
	//qDebug()<<"TextEdit::clearUndoStack()";
	//undoStack.clear();
	newUndoStack->stack.clear();
	//newRedoStack->stack.clear();
	emit undoAvailable(false);
	//emit redoAvailable(false);
}

void TextEdit::copyWithFormatting()
{
	qDebug("TextEdit::copyWithFormatting()");
	if (!textCursor().hasSelection()) {
		return;
	}
	int start = textCursor().selectionStart();
	int end = textCursor().selectionEnd();
	QTextCursor cur(document());
	cur.setPosition(start);
	int ls = cur.blockNumber();
	cur = QTextCursor();
	cur.setPosition(end);
	int le = cur.blockNumber();
	QTextBlock bl;
	QString result;
	int i = 0;
	for (bl = document()->begin(); bl.isValid(); bl = bl.next()) {
		if (i >= ls && i <= le) {
			if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
				if (i + 1 < 10) {
					result += " ";
				}
				if (i + 1 < 100) {
					result += " ";
				}
				result += QString::number(i + 1) + "\t";
			}
			QString line = bl.text();
			result += line + "\n";
		}
		i++;
	}
	QApplication::clipboard()->setText(result);
}

void TextEdit::setOverwriteMode(bool m)
{
	QTextEdit::setOverwriteMode(m);
	if (m)
//      setCursorWidth(QFontMetrics(font()).width("N")-4);
	{
		setCursorWidth(1);    // FIXME разобраться с перерисовкой курсора (после перехода в режим вставки остаётся грязь от старого курсора)
	} else {
		setCursorWidth(1);
	}
}



void TextEdit::connectNotify(const char *signal)
{
	Q_UNUSED(signal);
//  qDebug("TextEdit: connected signal %s",signal);
}

void TextEdit::disconnectNotify(const char *signal)
{
	Q_UNUSED(signal);
//  qDebug("TextEdit: disconnected signal %s",signal);
}

void TextEdit::normalizeLines(QPointer<QTextDocument> doc)
{
	Q_UNUSED(doc);
}

void TextEdit::normalizeLines(QPointer<QTextDocument> doc, QList<QTextBlock>)
{
	Q_UNUSED(doc);
}

void TextEdit::strictSelection()
{
	QTextCursor c = textCursor();
	if (!c.hasSelection()) {
		return;
	}
	QTextCursor cs(document());
	if (c.selectionStart() == c.position()) {
		cs.setPosition(c.selectionEnd());
		LineProp *lp = static_cast<LineProp *>(cs.block().userData());
		if (lp != NULL) {
			int i = lp->indentCount * 2;
			int p = cs.columnNumber();
			if (p < i) {
				cs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, i - p);
			}
		}
		cs.setPosition(c.selectionStart(), QTextCursor::KeepAnchor);
		lp = static_cast<LineProp *>(cs.block().userData());
		if (lp != NULL) {
			int i = lp->indentCount * 2;
			int p = cs.columnNumber();
			if (p < i) {
				cs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, i - p);
			}
		}
		setTextCursor(cs);
	} else {
		cs.setPosition(c.selectionStart());
		LineProp *lp = static_cast<LineProp *>(cs.block().userData());
		if (lp != NULL) {
			int i = lp->indentCount * 2;
			int p = cs.columnNumber();
			if (p < i) {
				cs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, i - p);
			}

		}
		cs.setPosition(c.selectionEnd(), QTextCursor::KeepAnchor);
		lp = static_cast<LineProp *>(cs.block().userData());
		if (lp != NULL) {
			int i = lp->indentCount * 2;
			int p = cs.columnNumber();
			if (p < i) {
				cs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, i - p);
			}
		}
		setTextCursor(cs);
	}
}

void TextEdit::doCharect(const QString &txt)
{
	QList<QTextBlock> blocks;
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		int saveStart = c.blockNumber();
		blocks << c.block();
		save(saveStart, (uint)Charect, QChar('0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		strictSelection();
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		emit linesRemoved(saveStart, blockCount - document()->blockCount());
		blockCount = document()->blockCount();
		textCursor().insertText(txt);
		emit linesInserted(saveStart, document()->blockCount() - blockCount);
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	} else {
		save(textCursor().blockNumber(), (uint)Charect, txt[0], false);
		if (overwriteMode() && textCursor().columnNumber() < textCursor().block().text().length()) {
			textCursor().deleteChar();
		}
		textCursor().insertText(txt);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	}
	normalizeLines(document(), QList<QTextBlock>() << textCursor().block());
}

void TextEdit::doDelete()
{
	int linesCount = document()->blockCount();
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		save(c.blockNumber(), (uint)Delete, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		strictSelection();
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		emit linesRemoved(c.blockNumber(), blockCount - document()->blockCount());
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	} else {
		QString txt = textCursor().block().text();
		QChar nextChar = txt[textCursor().columnNumber()];
		save(textCursor().blockNumber(), (uint)Delete, nextChar, false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		int extraDelete = 0;
		if (textCursor().columnNumber() == txt.length()) {
			QTextBlock bl = textCursor().block().next();
			if (bl.isValid()) {
				LineProp *lp2 = static_cast<LineProp *>(bl.userData());
				Q_CHECK_PTR(lp2);
				if (lp != NULL) {
					extraDelete = lp2->indentCount * 2;
				}
			}
		}
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		int blockCount = document()->blockCount();
		textCursor().deleteChar();
		for (int i = 0; i < extraDelete; i++) {
			textCursor().deleteChar();
		}
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		if (document()->blockCount() != blockCount) {
			emit linesRemoved(textCursor().blockNumber() + 1, 1);
		}
	}
	normalizeLines(document(), QList<QTextBlock>() << textCursor().block());
	if (document()->blockCount() != linesCount) {
		applyIndentation();
		checkForCompilationRequired();
	}
}

void TextEdit::checkForCompilationRequired() {}

void TextEdit::doCut()
{
	int linesCount = document()->blockCount();
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		save(c.blockNumber(), (uint)Cut, QChar('\0'), false);
		strictSelection();
		QClipboard *clipboard = QApplication::clipboard();
		QTextCursor s(document());
		s.setPosition(textCursor().selectionStart());
		QTextCursor e(document());
		e.setPosition(textCursor().selectionEnd());
		QStringList lines = toPlainText().split("\n", QString::KeepEmptyParts);
		QString txt;
		int ls = s.blockNumber();
		int le = e.blockNumber();
		int cs = s.columnNumber();
		int ce = e.columnNumber();
		for (int i = ls; i <= le; i++) {
			QString line = lines[i];
			if (i == ls)
				for (int j = 0; j < cs; j++) {
					line[j] = QChar(0x0698);
				}
			if (i == le)
				for (int j = ce; j < line.length(); j++) {
					line[j] = QChar(0x0698);
				}
			line.remove(QChar(0x0698));
			txt += line;
			if (i != le) {
				txt += "\n";
			}
		}

		clipboard->setText(txt);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		emit linesRemoved(c.blockNumber(), blockCount - document()->blockCount());
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	}
	normalizeLines(document(), QList<QTextBlock>() << textCursor().block());
	if (document()->blockCount() != linesCount) {
		applyIndentation();
		checkForCompilationRequired();
	}
}

void TextEdit::doBackspace()
{
	int linesCount = document()->blockCount();
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		save(c.blockNumber(), (uint)Backspace, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		strictSelection();
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		emit linesRemoved(c.blockNumber(), blockCount - document()->blockCount());
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	} else {
		int blockCount = document()->blockCount();
		uint fromLine;
		if (textCursor().blockNumber() > 0 && textCursor().columnNumber() == 0) {
			fromLine = textCursor().blockNumber() - 1;
		} else {
			fromLine = textCursor().blockNumber();
		}
		QString txt = textCursor().block().text();
		QChar prevChar = QChar('\0');
		if (textCursor().columnNumber() > 0) {
			prevChar = txt[textCursor().columnNumber() - 1];
		}
		save(fromLine, (uint)Backspace, prevChar, false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		int extraDelete = 0;
		if (lp != NULL)
			if (textCursor().columnNumber() == lp->indentCount * 2) {
				extraDelete = lp->indentCount * 2;
			}
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		textCursor().deletePreviousChar();
		for (int i = 0; i < extraDelete; i++) {
			textCursor().deletePreviousChar();
		}
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		if (document()->blockCount() != blockCount) {
			emit linesRemoved(textCursor().blockNumber() + 1, 1);
		}
	}
	normalizeLines(document(), QList<QTextBlock>() << textCursor().block());
	if (document()->blockCount() != linesCount) {
		applyIndentation();
		checkForCompilationRequired();
	}
}

void TextEdit::doInsertFirst(const QString &txt)
{
	int cursorPos = textCursor().position();
	save(0, (uint)Insert, QChar('\0'), false);
	QTextCursor c = QTextCursor(document()->begin());
	c.insertText(txt);
	textCursor().setPosition(txt.length() + cursorPos);
}

void TextEdit::doInsert(const QString &txt)
{
	QList<QTextBlock> blocks;
	int bn = 0;
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		while (c.position() < textCursor().selectionEnd()) {
			if (c.block().userState() == PROTECTED && !teacherMode) {
				return;
			}
			c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		}
		c.setPosition(textCursor().selectionStart());
		bn = c.blockNumber();
		save(bn, (uint)Insert, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		strictSelection();
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		emit linesRemoved(bn, blockCount - document()->blockCount());
		blockCount = document()->blockCount();
		QTextBlock bl = textCursor().block();
		blocks << bl;
		textCursor().insertText(txt);
		for (int i = 0; i < txt.count("\n"); i++) {
			bl = bl.next();
			blocks << bl;
		}

		emit linesInserted(bn, document()->blockCount() - blockCount);
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	} else {
		if (textCursor().block().userState() == PROTECTED && !teacherMode) {
			return;
		}
		bn = textCursor().blockNumber();
		save(bn, (uint)Insert, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		QTextBlock bl = textCursor().block();
		blocks << bl;
		textCursor().insertText(txt);
		for (int i = 0; i < txt.count("\n"); i++) {
			bl = bl.next();
			blocks << bl;
		}
		emit linesInserted(bn, txt.count("\n"));
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	}
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, bn);
	QTextBlock bl = c.block();
	initLineProps(document());
	for (int i = 0; i < txt.count("\n"); i++) {
		LineProp *lp = static_cast<LineProp *>(bl.userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		bl = bl.next();
	}
	ensureCursorVisible();
	if (textCursor().columnNumber() == 0) {
		horizontalScrollBar()->setValue(0);
	}
	normalizeLines(document(), blocks);
	applyIndentation();
	checkForCompilationRequired();
}

void TextEdit::doComment(uint from, uint to)
{
	Q_UNUSED(from);
	Q_UNUSED(to);
}

void TextEdit::doUncomment(uint from, uint to)
{
	Q_UNUSED(from);
	Q_UNUSED(to);
}

void TextEdit::doSetText(const QString &txt)
{
	Q_UNUSED(txt);
	save();
	QTextEdit::setPlainText(txt);
	initLineProps(document());
	normalizeLines(document());
	applyIndentation();
	checkForCompilationRequired();
}

void TextEdit::doInsertLine(uint line, const QString &txt)
{
	Q_UNUSED(txt);
	Q_UNUSED(line);
}

void TextEdit::doChangeLineText(uint line, const QString &txt)
{
	Q_UNUSED(txt);
	Q_UNUSED(line);
}

void TextEdit::doEnter()
{
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		save(c.blockNumber(), (uint)Enter, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		textCursor().removeSelectedText();
		textCursor().insertText("\n");
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	} else {
		save(textCursor().blockNumber(), (uint)Enter, QChar('\0'), false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
		textCursor().insertText("\n");
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp != NULL) {
			lp->lineChanged = true;
			emit yellowIndicator();
		}
	}
	initLineProps(document());
	normalizeLines(document());
	applyIndentation();
	ensureCursorVisible();
	if (textCursor().columnNumber() == 0) {
		horizontalScrollBar()->setValue(0);
	}
}

void TextEdit::doFastInsertText(const QString &txt, uint removeCount)
{
	Q_UNUSED(txt);
	Q_UNUSED(removeCount);
}


void TextEdit::save(uint fromLine, uint ucmd, QChar charect, bool saveToRedoStack)
{
	EditorCommand cmd = (EditorCommand)ucmd;
	bool canMerge = false;
	if ((!saveToRedoStack) && cmd == newUndoStack->lastCommand && (cmd == Charect || cmd == Delete || cmd == Backspace) && newUndoStack->stack.count() > 0) {
		bool contiguous;
		uint cpos = textCursor().position();
		uint lcpos = newUndoStack->stack.last().absoluteCursorPosition;
		if (cmd == Charect) {
			contiguous = lcpos == cpos - 1;
		} else if (cmd == Delete) {
			contiguous = lcpos == cpos;
		} else if (cmd == Backspace) {
			contiguous = lcpos == cpos + 1;
		}
		bool noSelection = (!textCursor().hasSelection()) && (!newUndoStack->stack.last().cursorHasSelection);
		bool charectGroupMatch;
		QChar pc = newUndoStack->lastCommandCharect;
		bool spaceCharects = charect.isSpace() && pc.isSpace();
		bool digitLetterCharects = (charect.isLetter() || charect.isDigit()) && (pc.isLetter() || pc.isDigit());
		charectGroupMatch = spaceCharects || digitLetterCharects;
		canMerge = contiguous && noSelection && charectGroupMatch;
	}
	if ((!canMerge && !newUndoStack->inMacro) || saveToRedoStack) {
		KumUndoElement element;
		element.cursorHasSelection = textCursor().hasSelection();
		element.cursorSelectionStart = textCursor().selectionStart();
		element.cursorSelectionEnd = textCursor().selectionEnd();
		element.absoluteCursorPosition = textCursor().position();
		element.fromLine = fromLine;
		element.command = cmd;
		QTextBlock block = document()->findBlockByNumber(fromLine);

		while (block != document()->end()) {
			int skipIndents = 0;
			LineProp *tlp = static_cast<LineProp *>(block.userData());
			if (tlp != NULL) {
				skipIndents = tlp->indentCount * 2;
				LineProp lp;
//              lp.visible = tlp->visible;
				lp.str_type = tlp->str_type;
				lp.lineChanged = true;
//              lp.isBreak = tlp->isBreak;
				lp.indentRang = tlp->indentRang;
				lp.indentRangNext = tlp->indentRangNext;
				lp.hyperlinks = tlp->hyperlinks;
//              lp.errorPositions = tlp->errorPositions;
				lp.errorMask = tlp->errorMask;
//              lp.hiddenText = tlp->hiddenText;
				lp.error_count = tlp->error_count;
				lp.error = tlp->error;
//              lp.editable = tlp->editable;
//              lp.allow_insert_before = tlp->allow_insert_before;
//              lp.allow_insert_after = tlp->allow_insert_after;
				element.savedLineProps << lp;
			}
			element.savedBlockStates << block.userState();
			element.savedText += block.text().mid(skipIndents);

			if (block.next().isValid()) {
				element.savedText += "\n";
			}
			block = block.next();
		}
		if (!saveToRedoStack) {
			newUndoStack->stack.push(element);
			newUndoStack->lastCommand = cmd;
			newUndoStack->lastCommandCharect = charect;
			if (cmd != Redo) {
				newRedoStack->stack.clear();
			}
			emit redoAvailable(false);
			emit undoAvailable(newUndoStack->stack.count() > 0);
		} else {
			newRedoStack->stack.push(element);
			emit redoAvailable(newRedoStack->stack.count() > 0);
		}
	} else if ((!saveToRedoStack) && newUndoStack->stack.count() > 0) {
		newUndoStack->stack.last().absoluteCursorPosition = textCursor().position();
	}

}

void TextEdit::revert(bool fromRedo)
{
	if (newUndoStack->stack.isEmpty() && (!fromRedo)) {
		return;
	}
	if (newRedoStack->stack.isEmpty() && fromRedo) {
		return;
	}
	KumUndoElement element;
	if (fromRedo) {
		element = newRedoStack->stack.pop();
		save(element.fromLine, Redo, QChar('\0'), false);
	} else {
		element = newUndoStack->stack.pop();
		save(element.fromLine, Undo, QChar('\0'), true);
	}
	QTextCursor c(document());
	for (int i = 0; i < element.fromLine; i++) {
		c.movePosition(QTextCursor::NextBlock);
	}
	c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	c.removeSelectedText();
	c.movePosition(QTextCursor::Start);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, element.fromLine);

	c.insertText(element.savedText);

	QTextBlock block = document()->findBlockByNumber(element.fromLine);
	qDebug() << block.text();

	foreach (int st, element.savedBlockStates) {
		block.setUserState(st);
		block = block.next();
	}

	block = document()->findBlockByNumber(element.fromLine);;
	foreach (LineProp lp, element.savedLineProps) {
		LineProp *elp = new LineProp();
//      elp->visible = lp.visible;
		elp->str_type = lp.str_type;
		elp->lineChanged = true;
//      elp->isBreak = lp.isBreak;
		elp->indentRang = lp.indentRang;
		elp->indentRangNext = lp.indentRangNext;
		elp->hyperlinks = lp.hyperlinks;
//      elp->hiddenText = lp.hiddenText;
		elp->error_count = lp.error_count;
		elp->error = lp.error;
//      elp->errorPositions = lp.errorPositions;
		elp->errorMask = lp.errorMask;
//      elp->editable = lp.editable;
//      elp->allow_insert_before = lp.allow_insert_before;
//      elp->allow_insert_after = lp.allow_insert_after;
		block.setUserData(elp);

		block = block.next();
	}

	normalizeLines(document());
	applyIndentation();
	if (element.cursorHasSelection) {
		if (element.absoluteCursorPosition == element.cursorSelectionStart) {
			c.setPosition(element.cursorSelectionEnd);
			c.setPosition(element.cursorSelectionStart, QTextCursor::KeepAnchor);
		} else {
			c.setPosition(element.cursorSelectionStart);
			c.setPosition(element.cursorSelectionEnd, QTextCursor::KeepAnchor);
		}
	} else {
		c.setPosition(element.absoluteCursorPosition);
	}
	setTextCursor(c);
	emit undoAvailable(newUndoStack->stack.count() > 0);
	emit redoAvailable(newRedoStack->stack.count() > 0);
}

void TextEdit::clearIndents()
{
	if (isKumirEditor()) {
		QTextCursor c(document());
		for (int i = 0; i < document()->blockCount(); i++) {
			QString txt = c.block().text();
			while (txt.startsWith(".") || txt.startsWith(" ")) {
				c.deleteChar();
				txt.remove(0, 1);
			}
			LineProp *lp = static_cast<LineProp *>(c.block().userData());
			if (lp != NULL) {
				lp->indentCount = 0;
			}
			c.movePosition(QTextCursor::NextBlock);
		}
	}
}

/**
* Поддержка пустой строки в конце.
*/
void TextEdit::keepEmptyLine()
{
	QTextCursor c(document());
	c.movePosition(QTextCursor::End);
	QTextBlock block = c.block();
	if (! block.text().isEmpty()) {
		c.insertText("\n");
		c.movePosition(QTextCursor::PreviousCharacter);
	}
}

DummyCursor::DummyCursor(QTextEdit *parent)
	: QObject(parent)
{
	visible = false;
	line = indent = 0;
	state = On;
	timer = new QTimer(this);
	QApplication *a = qobject_cast<QApplication *>(qApp);
	if (a) {
		timer->setInterval(a->cursorFlashTime() / 2);
		timer->setSingleShot(false);
		connect(timer, SIGNAL(timeout()), this, SLOT(blinkCursor()));
		timer->start();
	}
	this->parent = parent;
}

void DummyCursor::blinkCursor()
{

	if (!visible) {
		return;
	}
	if (state == On) {
		state = Off;
	} else {
		state = On;
	}
	emit needRepaint();
}

void DummyCursor::hide()
{
	visible = false;
	parent->setCursorWidth(1);
	emit needRepaint();
}

void DummyCursor::show(uint line, uint indent)
{
	this->line = line;
	this->indent = indent;
	visible = true;
	parent->setCursorWidth(0);
	emit needRepaint();
}

void DummyCursor::moveLeft()
{
	indent--;
	emit needRepaint();
}

void DummyCursor::moveRight()
{
	indent++;
	emit needRepaint();
}
