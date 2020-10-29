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

#include "kumirmargin.h"
#include "programtab.h"
#include "kumiredit.h"
#include "application.h"
#include "latinhighlighter.h"

#include <QScrollBar>
#include <QMenu>
#include <QKeyEvent>

KumirMargin::KumirMargin(QWidget *parent) : QTextEdit(parent)
{
	scrollBarHeight = 0;
	setReadOnly(true);
	connect(this, SIGNAL(textChanged()), this, SLOT(checkScrollBars()));
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setLineWrapMode(QTextEdit::NoWrap);
	editor = NULL;
	checkScrollBars();
	new LatinHighlighter(this);
}

void KumirMargin::setTab(ProgramTab *t)
{
	tab = t;
}

void KumirMargin::resizeEvent(QResizeEvent *event)
{
	QTextEdit::resizeEvent(event);
	checkScrollBars();
}

void KumirMargin::checkScrollBars()
{
	if (horizontalScrollBar()->isVisible()) {
		setViewportMargins(0, 0, 0, 0);
	} else {
		setViewportMargins(0, 0, 0, horizontalScrollBar()->height());
	}
}

void KumirMargin::scrollContentsBy(int dx, int dy)
{
	QTextEdit::scrollContentsBy(dx, dy);
	emit contentsMoving(dx, dy);
}

void KumirMargin::setText(const QString &text)
{
//  QTextEdit::clear();
//  QStringList lines = text.split("\n",QString::KeepEmptyParts);
//  for ( int i=0; i<lines.count(); i++ )
//  {
//      lines [i] = QString::number(i+1)+lines[i];
//  }
	QTextEdit::setText(text);
	debugTable.clear();

//  for (QTextBlock b=document()->begin();
//              b!=document()->end();
//              b=b.next()
//          )
//  {
//      QString txt = b.text();
//      int spacesCount = 0;
//      while (txt.startsWith(" ")) {
//          spacesCount++;
//          txt.remove(0,1);
//      }
//      qDebug() << spacesCount;
//  }

//  if (editor==NULL)
//      return;
//
//  setFont(editor->font());
//
//  QFontMetricsF fmMain(font());
//  QFontMetricsF fmLatin(font());
//
//  bool overLoadLatinFont = app()->settings->value("SyntaxHighlighter/OverloadLatinFont",false).toBool();
//
//  if ( app()->settings->value("SyntaxHighlighter/OverloadLatinFont",false).toBool() ) {
//      QString mainFontFamily = app()->settings->value("Appearance/Font","Courier").toString();
//      QString latinFontFamily = app()->settings->value("SyntaxHighlighter/LatinFont","Courier").toString();
//      int fontSize = app()->settings->value("Appearance/FontSize",12).toInt();
//      QFont mainFont = QFont(mainFontFamily,fontSize,QFont::Bold);
//      QFont latinFont = QFont(latinFontFamily,fontSize,QFont::Bold);
//      fmMain = QFontMetricsF(mainFont);
//      fmLatin = QFontMetricsF(latinFont);
//  }
//  else {
//      QString mainFontFamily = app()->settings->value("Appearance/Font","Courier").toString();
//      int fontSize = app()->settings->value("Appearance/FontSize",12).toInt();
//      QFont mainFont = QFont(mainFontFamily,fontSize,QFont::Bold);
//      fmMain = QFontMetricsF(mainFont);
//  }
//
//  QFontMetricsF fmThis = QFontMetrics(font());
//
//  QTextBlock ebl = editor->document()->begin();
//  for (QTextBlock bl = document()->begin(); bl.isValid(); bl = bl.next() ) {
//      if (!ebl.isValid())
//          break;
//      QString editorLine = ebl.text();
//
//      QSizeF mainEditorLineSize = fmMain.size(Qt::TextSingleLine,editorLine);
//      QSizeF latinEditorLineSize = fmLatin.size(Qt::TextSingleLine,editorLine);
//      qreal maxEditorLineHeight = 0;
//      for (int j=0; j<editorLine.length(); j++) {
//          if (editorLine[j].isLetter()&&editorLine[j].toAscii()>0&&overLoadLatinFont)
//              maxEditorLineHeight = qMax(maxEditorLineHeight,fmLatin.size(Qt::TextSingleLine,editorLine).height());
//          else
//              maxEditorLineHeight = qMax(maxEditorLineHeight,fmMain.size(Qt::TextSingleLine,editorLine).height());
//      }
//      QSizeF thisLineSize = fmThis.size(Qt::TextSingleLine,bl.text());
//      qreal thisLineHeight = thisLineSize.height();
// //       qDebug("Line: %s, ed: %f, m: %f",editorLine.toUtf8().data(),maxEditorLineHeight,thisLineHeight);
//      if (maxEditorLineHeight>thisLineHeight) {
//          qreal diff = maxEditorLineHeight - thisLineHeight;
//          QTextBlockFormat fmt;
//          fmt.setTopMargin(diff/2);
//          fmt.setBottomMargin(diff/2);
//          QTextCursor c(bl);
//          c.mergeBlockFormat(fmt);
//
//      }
//      qreal debug = bl.blockFormat().bottomMargin();
//      ebl = ebl.next();
//  }
}

void KumirMargin::setEditor(KumirEdit *editor)
{
	this->editor = editor;
}

void KumirMargin::setDebugText(const QString &txt, int line, int col)
{
	if (line < 0) {
		return;
	}
	QString text;
	for (int i = 0; i < txt.length(); i++) {
		if (txt[i].unicode() == 10 || txt[i].unicode() == 13) {
			text += QChar(0x21b2);
		} else if (txt[i].unicode() == 9) {
			text += QChar(0x21a6);
		} else if (txt[i].unicode() < 32) {
			text += QChar(0x22a0);
		} else {
			text += txt[i];
		}
	}
	int scrollValue = verticalScrollBar()->value();
	int blockCount = document()->blockCount();
	int textCursorPosition = textCursor().position();
	Q_UNUSED(textCursorPosition);

	if (debugTable.isEmpty()) {
		debugTable.resize(document()->blockCount());
	}

	QStringList row = debugTable[line];
	while (row.count() - 1 < col) {
		row.append("");
	}

	row[col] = text;
	debugTable[line] = row;

	QString newTextLine;

	row.removeAll("");
	newTextLine = row.join("; ");
	QTextCursor c(document());

	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.insertText(newTextLine);

	checkScrollBars();
	if (horizontalScrollBar()->isVisible()) {
		setViewportMargins(0, 0, 0, 0);
	} else {
		setViewportMargins(0, 0, 0, scrollBarHeight);
	}
	tab->syncronizeLineHeights();
	int newBlockCount = document()->blockCount();
	K_ASSERT(newBlockCount == blockCount);
	verticalScrollBar()->setValue(scrollValue);
}

void KumirMargin::contextMenuEvent(QContextMenuEvent *event)
{

	if (textCursor().hasSelection()) {
		QMenu *menu = new QMenu(this);
		menu->addAction(tr("Copy"), this, SLOT(copy()));
		QTextCursor newPos = cursorForPosition(event->pos());
		menu->exec(event->globalPos());
		event->accept();
		delete menu;
	} else {
		return;
	}

}

void KumirMargin::clearError(int lineNo)
{
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNo);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.removeSelectedText();
}

void KumirMargin::keyPressEvent(QKeyEvent *event)
{
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
}

void KumirMargin::removeLines(int from, int count)
{
//  qDebug("removeLines(%i,%i)",from,count);
	if (count == 0) {
		return;
	}
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, count);
	c.removeSelectedText();
}

void KumirMargin::insertEmptyLines(int from, int count)
{
//  qDebug("insertEmptyLines(%i,%i)",from,count);
	if (count == 0) {
		return;
	}
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	c.movePosition(QTextCursor::EndOfBlock);
	QString txt;
	for (int i = 0; i < count; i++) {
		txt += "\n";
	}
	c.insertText(txt);

}
