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

#define DOT_WIDTH 4

#include "kumiredit.h"
#include "syntaxhighlighter.h"
#include "application.h"
#include "macro.h"
#include "undocommands.h"
#include "application.h"
#include "compiler.h"
#include "int_proga.h"
#include "tooltip.h"
#include "kumirmargin.h"
#include "tools.h"
#include "kassert.h"
#include "kumundostack.h"
#include "kum_tables.h"
#include "messagesprovider.h"
#include "mainwindow.h"
#include "kumsinglemodule.h"

#include <QScrollBar>
#include <QClipboard>
#include <QTextLayout>
#include <QDesktopServices>

KumirEdit::KumirEdit(QWidget *parent) : TextEdit(parent)
{
	m_blockKeyboardFlag = false;
	m_blockKeyboardCounter = 0;
	hiddenProlog = QPointer<QTextDocument>();
	hiddenEpilog = QPointer<QTextDocument>();
	QTextDocument *psDoc = new QTextDocument(this);
	postScriptum = QPointer<QTextDocument>(psDoc);
	m_settingsEditor = false;
	allowRobotToInsertLines = false;
	dummyCursor = new DummyCursor(this);
	setAcceptDrops(false);

	messageEmited = false;
	shiftPressed = false;
	lastCompileTime = -1;
	tabNo = -1;
	m_indentSize = app()->settings->value("Editor/IndentScale", 1.0).toDouble();
	m_runLine = -1;
	m_errorLine = -1;
	initLineProps(document());
	prevCursorBlock = 0;
	prevLinesCount = 1;
	lockTextChangedFlag = false;
	textChanged = false;
	lineChanged = false;
	needCompilation = true;
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(this, SIGNAL(textChanged()), this, SLOT(checkScrollBars()));
	connect(this, SIGNAL(textChanged()), this, SLOT(checkUncommentAvailability()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(checkUncommentAvailability()));
	//connect ( this, SIGNAL(cursorPositionChanged()), this, SLOT(checkJumpErrorsAvailability()) );
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightBrackets()));
	connect(this, SIGNAL(requestCompilation(int)), app(), SLOT(doCompilation()));

	contextMenuBreakLine = -1;
	sh = new SyntaxHighlighter(this);
	setMouseTracking(true);
	underlineHyper = false;
	toolTipWindow = new ToolTip(this);
//    connect(toolTipWindow->list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(insertTxtFromPopup(QListWidgetItem*)));
	connect(toolTipWindow, SIGNAL(itemSelected(QListWidgetItem *, QString)), this, SLOT(insertTxtFromPopup(QListWidgetItem *, QString)));
	connect(toolTipWindow, SIGNAL(closed()), this, SLOT(setFocus()));
//    connect(toolTipWindow, SIGNAL (closed()), this, SLOT(unlockEditor()) );
	normLinesTotalCount = 0;

	scrollRepaintLock = false;

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	checkScrollBars();

	//menu->addAction(toggleRobotText, this, SLOT(toggleRobotAllowToInsertText()) );
	connect(app()->mainWindow->actionCatchPultCommands, SIGNAL(triggered()), this, SLOT(toggleRobotAllowToInsertText()));
}

void KumirEdit::unlockEditor()
{
	m_blockKeyboardFlag = false;
}

void KumirEdit::setMargin(KumirMargin *m)
{
	margin = m;
}


void KumirEdit::rehighlight()
{
	sh->rehighlight();
}

void KumirEdit::setIndentSize(qreal size)
{
	m_indentSize = size;
	applyIndentation();
}

qreal KumirEdit::indentSize()
{
	return m_indentSize;
}

void KumirEdit::applyIndentation()
{
	applyIndentation(document()->begin());
	keepEmptyLine();
}

void KumirEdit::applyIndentation(QTextBlock from)
{
	QMutexLocker locker(&indentationLocker);
	lockTextChangedFlag = true;
	QTextBlock block = from;
	int lastIndent = 0;
	Q_UNUSED(lastIndent);
	int indent_count = 0;
	QFontMetrics fm(font());
	double indentRatio = app()->settings->value("Editor/IndentRatio", 2.0).toDouble();
	double baseWidth = indentRatio * fm.width("w");
	QTextBlock prev = from.previous();
	if (prev.isValid()) {
		LineProp *lp = static_cast<LineProp *>(prev.userData());
		if (app()->settings->value("Editor/LegacyIndent", false).toBool()) {
			indent_count = lp->indentCount + lp->indentRangNext;
		} else {
			qreal indent_size = prev.blockFormat().textIndent();
			if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
				indent_size -= i_lineNumberingMarginWidth - 8;
			}
			indent_count = (int)(indent_size / baseWidth) + lp->indentRangNext;
		}
	}

	while (block.isValid()) {
		int debug;
		Q_UNUSED(debug);
		LineProp *lp = (LineProp *)block.userData();
		int real_indent = 0;
		Q_CHECK_PTR(lp);
		if (lp != NULL) {
			indent_count += lp->indentRang;
			real_indent = qMax(0, indent_count);
		}
		//      lp->errorMask = KumTools::instance()->recalculateErrorPositions(block.text().mid(lp->indentCount*2),lp->normalizedLines);
		QTextBlockFormat format = block.blockFormat();
		QTextCursor c = QTextCursor(block);
		if (app()->settings->value("Editor/LegacyIndent", false).toBool()) {
			if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
				format.setTextIndent(i_lineNumberingMarginWidth);
				if (c.block().blockFormat().textIndent() != i_lineNumberingMarginWidth) {
					c.setBlockFormat(format);
				}
			}
			if (lp->indentCount != real_indent) {
				int p = 0;
				while (lp->indentCount > real_indent) {
					p += 2;
					lp->indentCount--;
				}

				if (p > 0) {
					c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, p);
					c.removeSelectedText();
				}
				QString append = "";
				while (lp->indentCount < real_indent) {
					append += ". ";
					lp->indentCount++;
				}
				if (!append.isEmpty()) {
					c.insertText(append);
				}
			}
		} else {
			if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
				format.setTextIndent(i_lineNumberingMarginWidth + baseWidth * (real_indent));
				if (c.block().blockFormat().textIndent() != i_lineNumberingMarginWidth + baseWidth * (real_indent)) {
					c.setBlockFormat(format);
				}
			} else {
				format.setTextIndent(baseWidth * (real_indent));
				if (c.block().blockFormat().textIndent() != baseWidth * (real_indent)) {
					c.setBlockFormat(format);
				}
			}
		}
		indent_count += lp->indentRangNext;
		block = block.next();
	}

	update(); // FIXME update() ?
	lockTextChangedFlag = false;
}

void KumirEdit::keyPressEvent(QKeyEvent *event)
{
//    m_errorLine = -1;
	if (event->key() == Qt::Key_Shift) {
		shiftPressed = true;
	}
//    if ( toolTipWindow->isVisible() || m_blockKeyboardFlag) {
//        event->ignore();
//        return;
//    }
	if (m_blockKeyboardCounter > 0) {
		m_blockKeyboardCounter--;
		event->ignore();
		return;
	}
#ifndef WIN32
	if (event->key() == Qt::Key_Tab) {
		QTextCursor c = textCursor();
		if (c.block().userState() == PROTECTED && !teacherMode) {
			event->accept();
			return;
		}
		showHelpMenu();
		event->accept();
		return;
	}
#endif
#ifndef Q_OS_MAC
	if (event->key() == Qt::Key_Space && (event->modifiers() & Qt::ControlModifier)) {
		QTextCursor c = textCursor();
		if (c.block().userState() == PROTECTED && !teacherMode) {
			event->accept();
			return;
		}
		showHelpMenu();
		event->accept();
		return;
	}
#endif
	if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier)) {
		event->accept();
		copy();
	}
	if (event->key() == Qt::Key_Y && (event->modifiers() & Qt::ControlModifier)) {
		qDebug() << "Ctrl+Y";
		deleteCurrentLine();
	} else if (event->key() == Qt::Key_Comma && (event->modifiers() & Qt::ControlModifier)) {
		//      if ( isJumpPrevErrorAvailable() ) {
		//          gotoPrevError();
		//          event->accept();
		//      }
	} else if (event->key() == Qt::Key_Period && (event->modifiers() & Qt::ControlModifier)) {
		//      if ( isJumpNextErrorAvailable() ) {
		//          gotoNextError();
		//          event->accept();
		//      }
	} else if (event->key() == Qt::Key_Home) {
		TextEdit::keyPressEvent(event);
		horizontalScrollBar()->setValue(0);
	} else if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
		// Перехватываем Enter
		QTextCursor c = textCursor();
		if (c.block().userState() == PROTECTED && !teacherMode) {
			event->accept();
			c.movePosition(QTextCursor::NextBlock);
			c.movePosition(QTextCursor::EndOfBlock);
			setTextCursor(c);
			return;
		}
		keyPressEnter(event);
		if (macro != NULL) {
			// app()->addEventToHistory("AddMacro KeySequence pressed");
			macro->addCommand(event);
		}
		return;
	} else if (event->key() == Qt::Key_Backspace) {
		QTextCursor c = textCursor();
		if (c.block().userState() == PROTECTED && !teacherMode) {
			event->accept();
			QKeyEvent *newEvent = new QKeyEvent(event->type(), Qt::Key_Left, event->modifiers());
			keyPressEvent(newEvent);
			return;
		}
		if (c.blockNumber() == document()->blockCount() - 1) {
			if (c.columnNumber() == 0 && !c.hasSelection()) {
				event->accept();
				QKeyEvent *newEvent = new QKeyEvent(event->type(), Qt::Key_Left, event->modifiers());
				keyPressEvent(newEvent);
				return;
			}
		}
		// Backspace посылает сигнал "курсор перемещен" раньше,
		// чем сигнал "текст изменен" и фактическое удаление текста.
		// В связи с этим временно отключаем обработку сигнала
		// "курсор перемещен" и вызываем запрос на компиляцию вручную.
		lockTextChangedFlag = true;
		TextEdit::keyPressEvent(event);
		lockTextChangedFlag = false;
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

		keyPressCharect(event);
		return;
	}

	else {
		TextEdit::keyPressEvent(event);
		if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
			checkLineChanged();
		}
	}
	if (m_errorLine != -1) { // ???
		viewport()->update();
	}
}

void KumirEdit::keyPressCharect(QKeyEvent *event)
{
	if (textCursor().block().userState() == PROTECTED && !teacherMode) {
		emit sendMessage(tr("Text is protected in this line"), 1000);
		event->ignore();
		return;
	}
	TextEdit::keyPressEvent(event);
}

void KumirEdit::keyPressEnter(QKeyEvent *event)
{
	if (isReadOnly()) {
		event->ignore();
		return;
	}
	if (event->modifiers() & Qt::AltModifier) {
		TextEdit::keyPressEnter(event);
	} else {
		LineProp *lp = (LineProp *)textCursor().block().userData();
		if (textCursor().block().userState() == PROTECTED && !teacherMode) {
			emit sendMessage(tr("Text is protected in this line"), 1000);
			event->ignore();
			return;
		}
		QTextCursor c = textCursor();
		int cn = c.columnNumber();
		lp = static_cast<LineProp *>(c.block().userData());
		if (lp != NULL) {
			bool sel = event->modifiers() & Qt::ShiftModifier;
			int i = lp->indentCount * 2;
			if (cn < i) {
				c.movePosition(QTextCursor::Right, sel ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, i - cn);
				setTextCursor(c);
				return;
			}
		}
		doEnter();
		checkLineChanged();
	}
}

void KumirEdit::insertPultLine(const QString &line)
{
	if (!allowRobotToInsertLines) {
		return;
	}
	if (isReadOnly()) {
		return;
	}
	QTextCursor c = textCursor();
	if (c.block().userState() == PROTECTED && !teacherMode) {
		return;
	}
	if (c.hasSelection()) {
		c.clearSelection();
	}
	save(c.blockNumber(), (uint)InsertLine, false);
	LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
	Q_CHECK_PTR(lp);
	lp->lineChanged = true;
	emit yellowIndicator();
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
	if (c.block().text().trimmed().isEmpty()) {
		c.insertText(line);
	} else {
		c.insertText("\n" + line);
	}
	emit linesInserted(textCursor().blockNumber(), 1);
	lp = new LineProp();
	Q_CHECK_PTR(lp);
	lp->lineChanged = true;
	c.block().setUserData(lp);
	c.movePosition(QTextCursor::EndOfBlock);
	normalizeLines(document(), QList<QTextBlock>() << c.block());
	setTextCursor(c);
}


void KumirEdit::doEnter()
{
	QTextBlock from = textCursor().block();
	if (textCursor().hasSelection()) {
		QTextCursor c(document());
		c.setPosition(textCursor().selectionStart());
		int userState = c.block().userState();
		int d1 = textCursor().selectionStart();
		int d2 = textCursor().selectionEnd();
		Q_UNUSED(d1);
		Q_UNUSED(d2);
		QString debug = c.block().text();
		LineProp *lp2 = static_cast<LineProp *>(c.block().userData());
		Q_CHECK_PTR(lp2);
		//      int indent_count = lp2->indent_next + lp2->indent_count;
		save(c.blockNumber(), (uint)Enter, false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		Q_CHECK_PTR(lp);
		lp->lineChanged = true;
		emit yellowIndicator();
		int blockCount = document()->blockCount();
		textCursor().removeSelectedText();
		QString text = textCursor().block().text();
		int commentPos = -1;
		forever {
			commentPos = text.indexOf("|", commentPos + 1);
			if (commentPos == -1) {
				break;
			}
			if (!KumTools::instance()->inLit(text, commentPos)) {
				break;
			}
		}
		emit linesRemoved(c.blockNumber(), blockCount - document()->blockCount());
		blockCount = document()->blockCount();
		QString insTxt = "\n";
		if (commentPos > -1) {
			for (int i = 0; i < commentPos; i++) {
				insTxt += " ";
			}
			insTxt += "| ";
		}
		textCursor().insertText(insTxt);
		if (userState == HIDDEN) {
			textCursor().block().setUserState(HIDDEN);
		}
		emit linesInserted(c.blockNumber(), 1);
		lp = static_cast<LineProp *>(textCursor().block().userData());
		if (lp == NULL) {
			lp = new LineProp();
			textCursor().block().setUserData(lp);
		}
		lp->lineChanged = true;
		emit yellowIndicator();
		//      lp->indent_next = 0;
		//      lp->indent_count = indent_count;

	} else {
		int userState = textCursor().block().userState();
		save(textCursor().blockNumber(), (uint)Enter, false);
		LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
		Q_CHECK_PTR(lp);
		lp->lineChanged = true;
		emit yellowIndicator();
		//      int indent_count = lp->indent_next + lp->indent_count;
		QString text = textCursor().block().text();
		int commentPos = -1;
		forever {
			commentPos = text.indexOf("|", commentPos + 1);
			if (commentPos == -1) {
				break;
			}
			if (!KumTools::instance()->inLit(text, commentPos)) {
				break;
			}
		}
		QString insTxt = "\n";
		if (commentPos > -1 && commentPos < textCursor().columnNumber()) {
			for (int i = 0; i < commentPos; i++) {
				insTxt += " ";
			}
			insTxt += "| ";
		}
		textCursor().insertText(insTxt);
		emit linesInserted(textCursor().blockNumber(), 1);
		lp = new LineProp();
		Q_CHECK_PTR(lp);
		lp->lineChanged = true;
		emit yellowIndicator();
		//      lp->indent_next = 0;
		//      lp->indent_count = indent_count;
		textCursor().block().setUserData(lp);
		if (userState == HIDDEN) {
			textCursor().block().setUserState(HIDDEN);
		}
	}
	normalizeLines(document(), QList<QTextBlock>() << from << textCursor().block());
	applyIndentation(from);
	ensureCursorVisible();
	horizontalScrollBar()->setValue(0);
	//  checkJumpErrorsAvailability();
}


void KumirEdit::initLineProps(QPointer<QTextDocument> doc)
{
	QList<int> a;
	initLineProps(doc, a, a);
}

void KumirEdit::initLineProps(QPointer<QTextDocument> doc, const QList<int> &protectedLines, const QList<int> &hiddenLines)
{
	QTextBlock block = doc->begin();
	while (block.isValid()) {

		if (block.userData() == NULL) {
			block.setUserData(new LineProp());
			block.setUserState(-1);
		}
		if (hiddenLines.contains(block.blockNumber())) {
			block.setUserState(0);
		}
		if (protectedLines.contains(block.blockNumber())) {
			block.setUserState(1);
		}
		block = block.next();
	}
}

// void KumirEdit::setGreeceFlag(bool f)
// {
//  m_greeceFlag = f;
//  if ( f )
//      emit sendMessage(tr("Insert greek symbol"),0);
//  else
//      emit clearMessage();
// }

void KumirEdit::setBreakpoint(int para, bool value)
{
	Q_UNUSED(para);
	Q_UNUSED(value);
	//  QTextBlock block = document()->begin();
	//  int i = 0;
	//  while ( block.isValid() )
	//  {
	//      if ( i == para )
	//      {
	//          LineProp *lp = (LineProp*)block.userData();
	//          Q_CHECK_PTR(lp);
	//          lp->isBreak = value;
	//          break;
	//      }
	//      i++;
	//      block = block.next();
	//  }
	//  update();
}

void KumirEdit::paintWidget(QPainter *painter, const QRect &region)
{
	//  qDebug() << "Error_line: " << m_errorLine;
	//  qDebug() << "Paint: " << region;
	double indentRatio = app()->settings->value("Editor/IndentRatio", 2.0).toDouble();
	int x_offset = 0;
	int y_offset = 0;
	/*if ( app()->settings->value("Appearance/ShowLineNumbers",false).toBool() )*/ {
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
	}
	if (verticalScrollBar()->maximum() > 0) {
		double ratio;
		int s_max = verticalScrollBar()->maximum();
		int s_val = verticalScrollBar()->value();
		int v_height = height();
		double a_height = document()->size().height();
		ratio = (double)s_val / (double)s_max;
		int pixelsHidden = (int)((a_height - v_height) * ratio);
		y_offset = pixelsHidden;
	}
	if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
		painter->setPen(Qt::NoPen);
		if (tabNo == 0) {
			painter->setBrush(QColor("#CFDDCF"));
		} else {
			painter->setBrush(QColor("#EEEEFF"));
		}
		QRect marginRect = QRect(-x_offset, 0, i_lineNumberingMarginWidth, this->height());
		painter->drawRect(marginRect);
		QFont smallFont = this->font();
		smallFont.setPointSize(this->font().pointSize() - 2);
		painter->setFont(smallFont);
	}
	QTextBlock block = document()->begin();
	int i = 0;
	qreal extraHeight = 0;
	if (app()->settings->value("SyntaxHighlighter/OverloadLatinFont", false).toBool()) {
		QString mainFontFamily = app()->settings->value("Appearance/Font", "Courier").toString();
		QString latinFontFamily = app()->settings->value("SyntaxHighlighter/LatinFont", "Courier").toString();
		int fontSize = app()->settings->value("Appearance/FontSize", 12).toInt();
		QFont mainFont = QFont(mainFontFamily, fontSize, QFont::Bold);
		QFont latinFont = QFont(latinFontFamily, fontSize, QFont::Bold);
		QFontMetricsF fmMain = QFontMetricsF(mainFont);
		QFontMetricsF fmLatin = QFontMetricsF(latinFont);
		qreal mainLeading = fmMain.lineSpacing();
		qreal latinLeading = fmLatin.lineSpacing();
		qreal maxLeading = qMax(mainLeading, latinLeading);
		extraHeight = maxLeading - mainLeading;
	}
	QFontMetrics fm(font());
	double baseWidth = indentRatio * fm.width("w");

	while (block.isValid()) {


		QRect blockRect = block.layout()->boundingRect().toRect().translated(block.layout()->position().toPoint());

		int l0, l1, l2, l3;
		l0 = 0;
		l1 = blockRect.y();
		l2 = blockRect.x();
		l3 = blockRect.height();

		QRect drawArea = QRect(l0, l1, l2 + blockRect.width() + 5, l3);
		drawArea.adjust(0, -24, 0, 48);
		//      if (block.blockNumber()==12) {
		//          qDebug() << "blockRect: " << blockRect;
		//          qDebug() << "lRect: " << lRect;
		//          qDebug() << "drawArea: " << drawArea;
		//      }
		if (region.intersects(drawArea.translated(-x_offset, -y_offset)) || m_errorLine > -1 || m_runLine > -1) {
			QTextCursor cursor(block);
			QRect rect = cursorRect(cursor);
			rect.setHeight(rect.height() + (int)extraHeight);
			rect.moveLeft(-x_offset);
			rect.setWidth((int)(/*document()->size().*/width()));
			if (block.userState() == PROTECTED || block.userState() == HIDDEN) {
				painter->setBrush(block.userState() == PROTECTED ? QColor("#CFDDCF") : palette().window());
				painter->setPen(Qt::NoPen);
				QRect protectRect;
				if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
					protectRect = rect.translated(i_lineNumberingMarginWidth, 0);
				} else {
					protectRect = rect;
				}
				if (block.blockNumber() == 0) {
					protectRect.adjust(0, -10, 0, 0);
				}
				painter->drawRect(protectRect);
			}
			painter->setBrush(Qt::green);
			painter->setPen(Qt::NoPen);

			if (i == m_errorLine) {
				painter->setBrush(QColor("tomato"));
			}
			if ((i == m_runLine) || (i == m_errorLine)) {
				painter->drawRect(rect);

			};
			QPen pen;
			pen.setStyle(Qt::SolidLine);
			pen.setWidth(1);
			painter->setPen(pen);
			painter->setBrush(Qt::red);
			rect.setWidth(6);
			int y = rect.y();
			rect.setY(y + (rect.height() - 6) / 2);
			rect.setHeight(6);
			int x = i_lineNumberingMarginWidth - x_offset;
//            x += (int)(1.5*m_leftMargin);
			rect.moveLeft(x);
			LineProp *lp = (LineProp *)block.userData();
			Q_UNUSED(lp);
			i++;
			rect = cursorRect(cursor);
			rect.setHeight(rect.height() + (int)extraHeight);
			int indentCount = 0;
			if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
				QString lineTxt = QString::number(block.blockNumber() + 1);
				int lineWidth = painter->fontMetrics().width(lineTxt);
				QRect lineNumber = QRect(-x_offset + i_lineNumberingMarginWidth - lineWidth - 8,
						rect.y(),
						lineWidth,
						rect.height());
				if (region.intersects(lineNumber)) {
					painter->drawText(lineNumber, Qt::AlignRight | Qt::AlignTop, lineTxt);
				}
				double indentWidth = block.blockFormat().textIndent() - i_lineNumberingMarginWidth;
				indentCount = (int)(indentWidth / baseWidth);
			} else {
				double indentWidth = block.blockFormat().textIndent();
				indentCount = (int)(indentWidth / baseWidth);
			}
			painter->setPen(Qt::NoPen);
			painter->setBrush(QBrush(QColor(Qt::black)));
			for (int j = 0; j < indentCount; j++) {
				int x = (int)(j * baseWidth);
				if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
					x += i_lineNumberingMarginWidth;
				}
				x += DOT_WIDTH - x_offset;
				int y = rect.y() + rect.height() - DOT_WIDTH;
				y -= DOT_WIDTH * 2;
				if (region.intersects(QRect(x - DOT_WIDTH - 1, y - DOT_WIDTH - 1, DOT_WIDTH * 2 + 2, DOT_WIDTH * 2 + 2))) {
					painter->drawEllipse(x, y, DOT_WIDTH, DOT_WIDTH);
				}
			}
		}
		block = block.next();
	}
}


void KumirEdit::paintEvent(QPaintEvent *event)
{
	QPainter painter(viewport());
	paintWidget(&painter, event->rect());
	paintCursor(&painter, event->rect());
	painter.end();
	QTextEdit::paintEvent(event);

}

int KumirEdit::runLine()
{
	return m_runLine;
}

int KumirEdit::errorLine()
{
	return m_errorLine;
}

void KumirEdit::setRunLine(int line)
{
	m_runLine = line;
}

void KumirEdit::setErrorLine(int lineNo)
{
	m_errorLine = lineNo;
};

void KumirEdit::toggleRobotAllowToInsertText()
{
	//allowRobotToInsertLines = !allowRobotToInsertLines;
	allowRobotToInsertLines = app()->mainWindow->actionCatchPultCommands->isChecked();
	//app()->mainWindow->actionCatchPultCommands->setText( allowRobotToInsertLines ? tr("Don't catch commands of Pult") : tr("Catch commands of Pult") );
	//app()->mainWindow->actionCatchPultCommands->setChecked( allowRobotToInsertLines );
}

void KumirEdit::contextMenuEvent(QContextMenuEvent *event)
{
	contextMenuEvent(event->pos(), event->globalPos(), false);
	event->accept();
}

void KumirEdit::createTeacherActions(QMenu *menu)
{
	if (textCursor().hasSelection()) {
		bool hasProtectedLines = false;
		bool allLinesAreProtected = false;
		bool hasUnprotectedLines = false;
		bool hasVisibleLines = false;
		bool hasHiddenLines = false;
		QTextBlock block = document()->findBlock(textCursor().selectionStart());
		QTextBlock end = document()->findBlock(textCursor().selectionEnd());
		do {
			if (block.userState() == PROTECTED) {
				hasProtectedLines = true;
			} else if (block.userState() == HIDDEN) {
				hasHiddenLines = true;
			} else {
				hasUnprotectedLines = true;
				hasVisibleLines = true;
			}
			if (block == end) {
				break;
			}
			block = block.next();
		} while (block != end);
		allLinesAreProtected = hasProtectedLines && !hasUnprotectedLines;
		if (hasProtectedLines) {
			menu->addAction(tr("Unprotect selected lines"), this, SLOT(unprotectLines()));
		}
		if (hasUnprotectedLines) {
			menu->addAction(tr("Protect selected lines"), this, SLOT(protectLines()));
		}
		if (hasVisibleLines) {
			createHideActions(menu, true);
		}
		if (hasHiddenLines) {
			createUnhideActions(menu, true);
		}
	} else {
		if (textCursor().block().userState() == PROTECTED) {
			menu->addAction(tr("Unprotect this line"), this, SLOT(unprotectLines()));
		} else if (textCursor().block().userState() == HIDDEN) {
			menu->addSeparator();
			createUnhideActions(menu, false);
		} else {
			menu->addAction(tr("Protect this line"), this, SLOT(protectLines()));
			menu->addSeparator();
			createHideActions(menu, false);
		}
	}
	menu->addSeparator();
}

void KumirEdit::createHideActions(QMenu *menu, bool hasSelection)
{
	bool prevBlockIsHidden = textCursor().block().blockNumber() == 0 || (textCursor().block().previous().isValid() && textCursor().block().previous().userState() == HIDDEN);
	bool nextBlockIsHidden = (!textCursor().block().next().isValid()) || (textCursor().block().next().isValid() && textCursor().block().next().userState() == HIDDEN);
	if (prevBlockIsHidden || nextBlockIsHidden) {
		menu->addAction(tr("Hide this line"), this, SLOT(hideLine()));
	}
	QTextBlock cur = textCursor().block();
	QTextBlock prev = cur.previous();
	QTextBlock next = cur.next();
	if (prev.isValid() && prev.userState() != HIDDEN && !prevBlockIsHidden) {
		const QString text = hasSelection ? tr("Hide selection and lines above") : tr("Hide this line and lines above");
		menu->addAction(text, this, SLOT(hideProlog()));
	}
	if (next.isValid() && next.userState() != HIDDEN && !nextBlockIsHidden) {
		const QString text = hasSelection ? tr("Hide selection and next lines") : tr("Hide this line and next lines");
		menu->addAction(text, this, SLOT(hideEpilog()));
	}
}

void KumirEdit::createUnhideActions(QMenu *menu, bool hasSelection)
{
	Q_UNUSED(hasSelection);
	bool isProlog = true;
	bool isEpilog = true;
	bool afterCurrentLine = false;
	bool beforeCurrentLine = true;
	for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
		beforeCurrentLine = beforeCurrentLine && block != textCursor().block();
		if (block.previous().isValid()) {
			afterCurrentLine = afterCurrentLine || block.previous() == textCursor().block();
		}
		if (block.userState() != HIDDEN && beforeCurrentLine) {
			isProlog = false;
		}
		if (block.userState() != HIDDEN && afterCurrentLine) {
			isEpilog = false;
		}
	}
	bool prevBlockIsVisible = textCursor().block().previous().isValid() && textCursor().block().previous().userState() != HIDDEN;
	bool nextBlockIsVisible = textCursor().block().next().isValid() && textCursor().block().next().userState() != HIDDEN;
	if (prevBlockIsVisible || nextBlockIsVisible) {
		menu->addAction(tr("Unhide this line"), this, SLOT(unhideLine()));
		if (isProlog) {
			menu->addAction(tr("Unhide this line and lines above"), this, SLOT(unhideProlog()));
		}
		if (isEpilog) {
			menu->addAction(tr("Unhide this line and next lines"), this, SLOT(unhideEpilog()));
		}
	} else {
		menu->addAction(tr("Unhide entire block of lines"), this, SLOT(unhideBlock()));
		if (isProlog && isEpilog) {
			if (isProlog) {
				menu->addAction(tr("Unhide this line and lines above"), this, SLOT(unhideProlog()));
			}
			if (isEpilog) {
				menu->addAction(tr("Unhide this line and next lines"), this, SLOT(unhideEpilog()));
			}
		}
	}
}

void KumirEdit::contextMenuEvent(const QPoint &position, const QPoint &globalPosition, bool shiftPressed)
{
	Q_UNUSED(shiftPressed);
	if (!textCursor().hasSelection()) {
		setTextCursor(cursorForPosition(position));
	}
	QMenu *menu = new QMenu(this);
	//  menu->addAction(tr("Breakpoint"),this,SLOT(toggleBreakpoint()));
	if (teacherMode) {
		createTeacherActions(menu);
	}

	if (textCursor().hasSelection()) {
		if (!isReadOnly()) {
			menu->addAction(tr("Cut"), this, SLOT(cut()));
		}
		menu->addAction(tr("Copy"), this, SLOT(copy()));
		//      if ( shiftPressed )
		//      if ( !m_settingsEditor )
		//          menu->addAction(tr("Copy with format"), this, SLOT(copyWithFormatting()) );
	}
	//  QString toggleRobotText = allowRobotToInsertLines? tr("Deny Robot to insert lines") : tr("Allow Robot to insert lines");
	//  menu->addSeparator();
	//  menu->addAction(toggleRobotText, this, SLOT(toggleRobotAllowToInsertText()) );
	if (!isReadOnly()) {
		menu->addAction(tr("Paste"), this, SLOT(paste()));
	}
	QTextCursor newPos = cursorForPosition(position);
	contextMenuBreakLine = newPos.blockNumber();
	menu->exec(globalPosition);
	delete menu;
}

void KumirEdit::unprotectLines()
{
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	if (textCursor().hasSelection()) {
		QTextBlock block = document()->findBlock(textCursor().selectionStart());
		QTextBlock end = document()->findBlock(textCursor().selectionEnd());
		forever {
			if (block.userState() == PROTECTED) {
				block.setUserState(SIMPLE);
			}
			if (block == end) {
				break;
			}
			block = block.next();
		}
	} else {
		textCursor().block().setUserState(SIMPLE);
		updateRect = QRect(0, cursorRect().y() - 10, viewport()->width(), cursorRect().height() + 10);
		//      qDebug() << "updateRect: " << updateRect;
	}
	viewport()->update(updateRect);
}

void KumirEdit::protectLines()
{
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	if (textCursor().hasSelection()) {
		QTextBlock block = document()->findBlock(textCursor().selectionStart());
		QTextBlock end = document()->findBlock(textCursor().selectionEnd());
		forever {
			if (block.userState() != PROTECTED && block.userState() != HIDDEN) {
				block.setUserState(PROTECTED);
			}
			if (block == end) {
				break;
			}
			block = block.next();
		}
	} else {
		textCursor().block().setUserState(PROTECTED);
		updateRect = QRect(0, cursorRect().y() - 10, viewport()->width(), cursorRect().height() + 10);
		//      qDebug() << "updateRect: " << updateRect;
	}
	viewport()->update(updateRect);
}

void KumirEdit::hideLine()
{
	textCursor().block().setUserState(HIDDEN);
	QRect updateRect = QRect(0, cursorRect().y() - 10, viewport()->width(), cursorRect().height() + 10);
	viewport()->update(updateRect);
}

void KumirEdit::hideProlog()
{
	QTextBlock block = document()->begin();
	QTextBlock end;
	if (textCursor().hasSelection()) {
		int p = textCursor().selectionEnd();
		end = document()->findBlock(p);
	} else {
		end = textCursor().block();
	}
	forever {
		block.setUserState(HIDDEN);
		if (block == end) {
			break;
		}
		block = block.next();
	}
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	viewport()->update(updateRect);
}

void KumirEdit::hideEpilog()
{
	QTextBlock block;
	QTextBlock end = document()->end();
	if (textCursor().hasSelection())  {
		int p = textCursor().selectionStart();
		block = document()->findBlock(p);
	} else {
		block = textCursor().block();
	}

	forever {
		block.setUserState(HIDDEN);
		if (block == end) {
			break;
		}
		block = block.next();
	}
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	viewport()->update(updateRect);
}

void KumirEdit::unhideLine()
{
	textCursor().block().setUserState(SIMPLE);
	QRect updateRect = QRect(0, cursorRect().y() - 10, viewport()->width(), cursorRect().height() + 10);
	viewport()->update(updateRect);
}

void KumirEdit::unhideProlog()
{
	QTextBlock block = document()->begin();
	QTextBlock end = textCursor().block();
	forever {
		block.setUserState(SIMPLE);
		if (block == end) {
			break;
		}
		block = block.next();
	}
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	viewport()->update(updateRect);
}

void KumirEdit::unhideEpilog()
{
	QTextBlock block = textCursor().block();
	QTextBlock end = document()->end();
	forever {
		block.setUserState(SIMPLE);
		if (block == end) {
			break;
		}
		block = block.next();
	}
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	viewport()->update(updateRect);
}

void KumirEdit::unhideBlock()
{
	QTextBlock block = textCursor().block();
	forever {
		if (!block.isValid()) {
			break;
		}
		if (block.userState() == HIDDEN) {
			block.setUserState(SIMPLE);
		} else {
			break;
		}
		block = block.previous();
	}
	block = textCursor().block().next();
	forever {
		if (!block.isValid()) {
			break;
		}
		if (block.userState() == HIDDEN) {
			block.setUserState(SIMPLE);
		} else {
			break;
		}
		block = block.next();
	}
	QRect updateRect = QRect(0, 0, viewport()->width(), viewport()->height());
	viewport()->update(updateRect);
}

QString KumirEdit::toPlainText()
{
	QString prologText;
	QString epilogText;
	if (!hiddenProlog.isNull()) {
		prologText = hiddenProlog->toPlainText();
	}
	if (!hiddenEpilog.isNull()) {
		epilogText = hiddenEpilog->toPlainText();
	}

	QStringList prologLines = prologText.split("\n", QString::KeepEmptyParts);
	QStringList epilogLines = epilogText.split("\n", QString::KeepEmptyParts);

	QStringList allLines;
	if (!prologText.isEmpty()) {
		foreach (QString line, prologLines) {
			allLines << line;
		}
	}
	QString mod;
	for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
		if (block.userState() == PROTECTED) {
			mod = "|@protected";
		} else if (block.userState() == HIDDEN) {
			mod = "|@hidden";
		} else {
			mod = "";
		}
		allLines << block.text() + mod;
	}
	if (!epilogText.isEmpty()) {
		foreach (QString line, epilogLines) {
			allLines << line;
		}
	}
	return allLines.join("\n");
}

void KumirEdit::toggleBreakpoint()
{
	//  QTextBlock block = document()->begin();
	//  int i = 0;
	//  while ( (i<contextMenuBreakLine) && block.isValid() )
	//  {
	//      block = block.next();
	//      i++;
	//  }
	//  LineProp *lp = (LineProp*)block.userData();
	//  if ( lp!=NULL )
	//  {
	//      lp->isBreak = ! lp->isBreak;
	//  }
	//  applyIndentation();
}

void KumirEdit::checkScrollBars()
{
	if (horizontalScrollBar()->isVisible()) {
		setViewportMargins(0, 0, 0, 0);
	} else {
		setViewportMargins(0, 0, 0, horizontalScrollBar()->height());
	}
}

void KumirEdit::resizeEvent(QResizeEvent *event)
{
	QTextEdit::resizeEvent(event);
	checkScrollBars();
}

void KumirEdit::updateLanguage()
{
	if (sh) {
		sh->rehighlight();
	}
}

void KumirEdit::setText(const QString &txt)
{
	m_errorLine = -1;

	QStringList lines = txt.split("\n", QString::KeepEmptyParts);
	if (!hiddenProlog.isNull()) {
		hiddenProlog->deleteLater();
	}
	if (!hiddenEpilog.isNull()) {
		hiddenEpilog->deleteLater();
	}

	QStringList prologLines, visibleLines, epilogLines;

	bool prologMode = true;

	for (int i = 0; i < lines.count(); i++) {
		while (lines[i].startsWith(". ")) {
			lines[i].remove(0, 2);
		}
	}

	if (!teacherMode) {
		for (int i = 0; i < lines.count(); i++) {
			if (lines[i].endsWith("|@hidden")) {
				if (prologMode) {
					prologLines << lines[i];
				} else {
					epilogLines << lines[i];
				}
			} else {
				if (prologMode) {
					prologMode = false;
				}
				if (lines[i].trimmed().isEmpty() && i == lines.count() - 1) {} // последняя пустая строка
				else {
					visibleLines += epilogLines;
					epilogLines.clear();
				}
				visibleLines << lines[i];
			}
		}
		hiddenProlog = new QTextDocument(prologLines.join("\n"), this);
		hiddenEpilog = new QTextDocument(epilogLines.join("\n"), this);
		initLineProps(hiddenProlog);
		initLineProps(hiddenEpilog);
		normalizeLines(hiddenProlog);
		normalizeLines(hiddenEpilog);
	} else {
		visibleLines = lines;
	}

	QList<int> hidden, protcted;

	for (int i = 0; i < visibleLines.count(); i++) {
		if (visibleLines[i].endsWith("|@hidden")) {
			hidden << i;
			visibleLines[i].remove(visibleLines[i].length() - 8, 8);
		} else if (visibleLines[i].endsWith("|@protected")) {
			protcted << i;
			visibleLines[i].remove(visibleLines[i].length() - 11, 11);
		}
	}


	QTextEdit::setPlainText(visibleLines.join("\n"));
	initLineProps(document(), protcted, hidden);

	normalizeLines(document());
	for (int i = 0; i < document()->blockCount(); i++) {
		trim(i);
	}
	checkForCompilationRequired();

	applyIndentation();
	verticalScrollBar()->setValue(0);
	horizontalScrollBar()->setValue(0);
	//  checkJumpErrorsAvailability();
	update();
}

void KumirEdit::updateSHAppearance()
{
	Q_CHECK_PTR(sh);
	sh->updateAppearance();
	sh->rehighlight();
}

bool KumirEdit::canInsertFromMimeData(const QMimeData *source) const
{
	if (source->hasUrls()) {
		foreach (QUrl url, source->urls()) {
			QString path = url.toLocalFile();
			if (path.endsWith(".kum")) {
				return true;
			}
		}
	}
	return false;
}

void KumirEdit::insertFromMimeData(const QMimeData *source)
{
	if (source->hasUrls()) {
		foreach (QUrl url, source->urls()) {
			QString path = url.toLocalFile();
			//          QMessageBox::information(NULL,"insertFromMimeData",path);
			if (path.endsWith(".kum")) {
				emit fileOpenHereRequest(path);
				return;
			}
		}
	}
	QMutexLocker l(&editMutex);
	if (dummyCursor != NULL && dummyCursor->isVisible()) {
		dummyCursor->hide();
		return;
	}

	QStringList lines = source->text().split("\n", QString::KeepEmptyParts);
	for (int i = 0; i < lines.count(); i++) {
		lines[i].replace(QString::fromUtf8("”"), "\"");
		while (lines[i].startsWith(". ") || lines[i].startsWith(QString::fromUtf8("• "))  || lines[i].startsWith(QString::fromUtf8("· "))) {
			lines[i].remove(0, 2);
		}
		if (lines[i] == "." || lines[i] == QString::fromUtf8("•") || lines[i] == QString::fromUtf8("·")) {
			lines[i] = "";
		}
	}
	QString txt = lines.join("\n");
	doInsert(txt);
	//  ensureCursorVisible();
	emit ensureMarginCursorVisible(textCursor().blockNumber());
	checkLineChanged();
}

void KumirEdit::checkLineChanged()
{
	if ((textCursor().blockNumber() != prevCursorBlock) || (prevLinesCount != document()->blockCount())) {
		lineChanged = true;


		//      if ( prevLinesCount != document()->blockCount() )
		//          applyIndentation();
		trim(textCursor().blockNumber());

	}
	checkForCompilationRequired();
	prevCursorBlock = textCursor().blockNumber();
	prevLinesCount = document()->blockCount();
	checkUncommentAvailability();

}

void KumirEdit::trim(int blockNumber)
{
	return;
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	Q_CHECK_PTR(lp);
	int indentSymbols = lp->indentCount * 2;
	QString txt = c.block().text();
	int spacesBefore = 0;
	int spacesAfter = 0;
	QRegExp rxNonSpace("\\S");
	spacesBefore = rxNonSpace.indexIn(txt, indentSymbols);
	spacesAfter = txt.length() - rxNonSpace.lastIndexIn(txt) - 1;
	if (spacesBefore > 0 && !txt.trimmed().startsWith("|")) {
		c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, spacesBefore);
		c.removeSelectedText();
	}
	if (spacesAfter > indentSymbols) {
		c.movePosition(QTextCursor::EndOfBlock);
		c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, spacesAfter);
		c.removeSelectedText();
	}
}

void KumirEdit::verifyTextChanged()
{
	//  if ( previousText != toPlainText() )
	//  {
	//      textChanged = true;
	//      checkTextChanges();
	//      previousText = toPlainText();
	//  }

}

void KumirEdit::checkTextChanges()
{
	//  qDebug("checkTextChanges()");
	//  if ( textChanged && lineChanged )
	//  {
	//      stopListeningEvents();
	// //       qDebug("prevText: %s",previousText.toUtf8().data());
	// //       qDebug("Text: %s",toPlainText().toUtf8().data());
	//      emit requestCompilation(tabNo);
	//      textChanged = false;
	//      lineChanged = false;
	//      startListeningEvents();
	//  }
	//  if ( undoStack.count() != lastCompileTime || forceEmitCompilation ) {
	// //       qDebug("emit request compilation");
	//      lastCompileTime = undoStack.count();
	//      forceEmitCompilation = false;
	//      emit requestCompilation(tabNo);
	//  }
}

bool KumirEdit::isKumirEditor()
{
	return true;
}

void KumirEdit::comment()
{
	if (textCursor().hasSelection()) {
		int from = textCursor().selectionStart();
		int to = textCursor().selectionEnd();
		QTextCursor a(document());
		QTextCursor b(document());
		a.setPosition(from);
		b.setPosition(to);
		from = a.blockNumber();
		to = b.blockNumber();
		if (textCursor().selectedText().startsWith("\n")) {
			from ++;
		}
		if (textCursor().selectedText().endsWith("\n")) {
			to --;
		}
		doComment(from, to);
	} else {
		QTextBlock block = textCursor().block();
		QTextCursor c(block);
		doComment(c.blockNumber(), c.blockNumber());
	}
}

void KumirEdit::uncomment()
{
	//  if ( textCursor().hasSelection() )
	//  {
	//      int from = textCursor().selectionStart();
	//      int to = textCursor().selectionEnd();
	//      QTextCursor a(document());
	//      QTextCursor b(document());
	//      a.setPosition(from);
	//      b.setPosition(to);
	//      QTextBlock begin = a.block();
	//      QTextBlock end = b.block();
	//      QTextBlock curBlock = begin;
	//      while ( (curBlock != end) && (curBlock.isValid()) )
	//      {
	//          QTextCursor c(curBlock);
	//          if ( curBlock.text().startsWith("|") )
	//              c.deleteChar();
	//          while ( curBlock.text().startsWith(" ") )
	//              c.deleteChar();
	//          curBlock = curBlock.next();
	//      }
	//      QTextCursor c(curBlock);
	//      if ( curBlock.text().startsWith("|") )
	//          c.deleteChar();
	//      while ( curBlock.text().startsWith(" ") )
	//          c.deleteChar();
	//  }
	//  else
	//  {
	//      QTextBlock block = textCursor().block();
	//      QTextCursor c(block);
	//      if ( block.text().startsWith("|") )
	//          c.deleteChar();
	//      while ( block.text().startsWith(" ") )
	//          c.deleteChar();
	//  }
	if (textCursor().hasSelection()) {
		int from = textCursor().selectionStart();
		int to = textCursor().selectionEnd();
		QTextCursor a(document());
		QTextCursor b(document());
		a.setPosition(from);
		b.setPosition(to);
		from = a.blockNumber();
		to = b.blockNumber();
		if (textCursor().selectedText().startsWith("\n")) {
			from ++;
		}
		if (textCursor().selectedText().endsWith("\n")) {
			to --;
		}
		doUncomment(from, to);
	} else {
		QTextBlock block = textCursor().block();
		QTextCursor c(block);
		doUncomment(c.blockNumber(), c.blockNumber());
	}
}

bool KumirEdit::isUncommentAvailable()
{
	if (textCursor().hasSelection()) {
		bool result = false;
		int from = textCursor().selectionStart();
		int to = textCursor().selectionEnd();
		QTextCursor a(document());
		QTextCursor b(document());
		a.setPosition(from);
		b.setPosition(to);
		QTextBlock begin = a.block();
		QTextBlock end = b.block();
		QTextBlock curBlock = begin;
		while ((curBlock != end) && curBlock.isValid()) {
			result = result || curBlock.text().trimmed().startsWith("|");
			if (curBlock.userState() == PROTECTED && !teacherMode) {
				return false;
			}
			curBlock = curBlock.next();
		}
		result = result || curBlock.text().trimmed().startsWith("|");
		return result;
	} else {
		if (textCursor().block().userState() == PROTECTED && !teacherMode) {
			return false;
		}
		return textCursor().block().text().trimmed().startsWith("|");
	}
}

void KumirEdit::checkUncommentAvailability()
{
	emit uncommentAvailable(isUncommentAvailable());
}

void KumirEdit::highlightBrackets()
{
	return;
	QTextCursor cursor = QTextCursor(document());
	cursor.select(QTextCursor::Document);
	QTextCharFormat clearFormat;
	clearFormat.setBackground(QColor(Qt::white));
	//  clearFormat.setFontWeight(QFont::Normal);
	cursor.mergeCharFormat(clearFormat);
	cursor = textCursor();
	QTextCharFormat highlightFormat;
	highlightFormat.setBackground(QColor("#d1f6e7"));
	//  highlightFormat.setForeground(QColor(Qt::green));
	//  highlightFormat.setFontWeight(QFont::Bold);
	QString txt = document()->toPlainText();
	int pos = cursor.position();
	if (!cursor.atBlockEnd() && (txt[cursor.position()] == '(')) {
		// ищем закрывающюю ')'
		int openPos = pos;
		int deep = 0;
		bool found = false;
		int closePos = -1;
		while ((pos < txt.length()) && (txt[pos] != '\n')) {
			if (txt[pos] == '(') {
				deep++;
			}
			if (txt[pos] == ')') {
				deep--;
			}
			if (deep == 0) {
				found = true;
				closePos = pos;
				break;
			}
			pos ++;
		}
		if (found) {
			QTextCursor cursor(document());
			cursor.setPosition(openPos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
			cursor.setPosition(closePos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
		}
	}
	pos = cursor.position();
	if ((pos > 0) && (txt[pos] == ')')) {
		// ищем открывающую ')'
		int closePos = pos;
		int deep = 0;
		bool found = false;
		int openPos = -1;
		while (pos >= 0) {
			if (txt[pos] == '(') {
				deep--;
			}
			if (txt[pos] == ')') {
				deep++;
			}
			if (deep == 0) {
				found = true;
				openPos = pos;
				break;
			}
			pos --;
		}
		if (found) {
			QTextCursor cursor(document());
			cursor.setPosition(openPos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
			cursor.setPosition(closePos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
		}
	}
	pos = cursor.position();
	if ((pos < txt.length()) && (txt[pos] == '[')) {
		// ищем закрывающюю ']'
		int openPos = pos;
		int deep = 0;
		bool found = false;
		int closePos = -1;
		while ((pos < txt.length()) && (txt[pos] != '\n')) {
			if (txt[pos] == '[') {
				deep++;
			}
			if (txt[pos] == ']') {
				deep--;
			}
			if (deep == 0) {
				found = true;
				closePos = pos;
				break;
			}
			pos ++;
		}
		if (found) {
			QTextCursor cursor(document());
			cursor.setPosition(openPos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
			cursor.setPosition(closePos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
		}
	}
	pos = cursor.position();
	if ((pos > 0) && (txt[pos] == ']')) {
		// ищем открывающую '['
		int closePos = pos;
		int deep = 0;
		bool found = false;
		int openPos = -1;
		while (pos >= 0) {
			if (txt[pos] == '[') {
				deep--;
			}
			if (txt[pos] == ']') {
				deep++;
			}
			if (deep == 0) {
				found = true;
				openPos = pos;
				break;
			}
			pos --;
		}
		if (found) {
			QTextCursor cursor(document());
			cursor.setPosition(openPos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
			cursor.setPosition(closePos);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.mergeCharFormat(highlightFormat);
		}
	}
}

LineProp *KumirEdit::prop(int lineNo)
{
	K_ASSERT(lineNo >= 0);
	QTextBlock block = document()->begin();
	int i = 0;
	while (block.isValid() && (i < lineNo)) {
		i++;
		block = block.next();
	}
	if (block.isValid()) {
		if (block.userData() != NULL) {
			LineProp *lp = static_cast<LineProp *>(block.userData());
			return lp;
		} else {
			block.setUserData(new LineProp());
			LineProp *lp = static_cast<LineProp *>(block.userData());
			return lp;
		}
	} else {
		return NULL;
	}
}

// void KumirEdit::clearIndents()
// {
//  QTextBlock block = document()->begin();
//  while ( block.isValid() )
//  {
//      LineProp *lp = static_cast<LineProp*>(block.userData());
//      if ( lp == NULL )
//      {
//          lp = new LineProp();
//          lp->indent_count = -1;
//          block.setUserData(lp);
//      }
//      else
//      {
//          lp->indent_count = -1;
//      }
//      block = block.next();
//  }
// }

void KumirEdit::initNewBlock(QTextBlock prevBlock, QTextBlock newBlock)
{
	LineProp *lp = static_cast<LineProp *>(prevBlock.userData());
	Q_UNUSED(lp);
	LineProp *nlp = new LineProp();
	//  nlp->indent_count = lp->indent_count + lp->indent_next;
	//  nlp->indent_next = 0;
	newBlock.setUserData(nlp);
}


void KumirEdit::setTabNo(int i)
{
	tabNo = i;
}

void KumirEdit::setSHAlgNames(const QStringList &names)
{
	sh->lock();
	sh->clearNames();
	foreach (QString name, names) {
		sh->addName(name);
	}
	sh->flushNames();
	sh->unlock();
}

void KumirEdit::playMacro(Macro *m)
{
	QTextBlock bl = textCursor().block();
	BlockState state = BlockState(bl.userState());
	bool deny = state == PROTECTED;
	if (textCursor().hasSelection()) {
		int start = textCursor().selectionStart();
		int end = textCursor().selectionEnd();
		QTextCursor st = QTextCursor(document());
		st.setPosition(start);
		while (st.position() < end) {
			QTextBlock bl = st.block();
			deny = deny || BlockState(bl.userState()) == PROTECTED;
			st.movePosition(QTextCursor::NextBlock);
		}
	}
	if (deny) {
		emit sendMessage(tr("Text is protected in this line"), 1000);
		return;
	}
	//  disconnect ( this, SIGNAL(textChanged()), this, SLOT(checkTextChanges()) );

	TextEdit::playMacro(m);
	//  connect ( this, SIGNAL(textChanged()), this, SLOT(checkTextChanges()) );
	//  checkTextChanges();
	//  for ( QTextBlock bl=document()->begin(); bl != document()->end(); bl=bl.next() ) {
	//      LineProp *lp = static_cast<LineProp*>(bl.userData());
	//      if ( lp != NULL )
	//          qDebug() << bl.text() << " : " << lp->lineChanged;
	//      else
	//          qDebug() << bl.text() << " : NULL! ";
	//  }
	normalizeLines(document());
	emit requestCompilation(tabNo);
}

void KumirEdit::scrollContentsBy(int dx, int dy)
{
	if (scrollRepaintLock) {
		return;
	}
	QTextEdit::scrollContentsBy(dx, dy);
	emit contentsMoving(dx, dy);
	if (dx != 0) {
		//      resize(width()-1,height());
		//      resize(width()+1,height());
		viewport()->update();
	}
	//  if ( dx != 0 ) {
	//      qDebug() << "FORCING EDITOR REPAINT";
	//
	//      disconnect ( SIGNAL( textChanged() ) );
	//      disconnect ( SIGNAL( cursorPositionChanged() ) );
	//      disconnect ( SIGNAL( requestCompilation(int) ) );
	//
	//      scrollRepaintLock = true;
	//
	//      int cursorPos = textCursor().position();
	//
	//      for ( QTextBlock bl = document()->begin(); bl != document()->end(); bl = bl.next() ) {
	//          QString txt = bl.text();
	//          QTextCursor c(bl);
	//          c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	//          c.removeSelectedText();
	//          c.insertText(txt);
	//      }
	//
	//      QTextCursor c = textCursor();
	//      c.setPosition(cursorPos);
	//      setTextCursor(c);
	//
	//      scrollRepaintLock = false;
	//
	//      connect ( this, SIGNAL(textChanged()), this, SLOT(checkScrollBars()) );
	//      connect ( this, SIGNAL(textChanged()), this, SLOT(checkUncommentAvailability()) );
	//      connect ( this, SIGNAL(cursorPositionChanged()), this, SLOT(checkUncommentAvailability()) );
	//      connect ( this, SIGNAL(cursorPositionChanged()), this, SLOT(checkJumpErrorsAvailability()) );
	//      connect ( this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightBrackets()) );
	//      connect ( this, SIGNAL(requestCompilation(int)), app(), SLOT(doCompilation(int)) );
	//  }
}


void KumirEdit::mouseMoveEvent(QMouseEvent *event)
{
	QTextCursor c = cursorForPosition(event->pos());
	bool overHyper = false;
	bool overError = false;
	QString name;
	QString errText;
	if (c.block().userData() != NULL) {
		LineProp *lp = (LineProp *)(c.block().userData());
		foreach (HyperLink h, lp->hyperlinks) {
			if (c.columnNumber() >= h.start && c.columnNumber() <= h.end) {
				overHyper = true;
				name = h.link;
				QTextCursor s = c;
				QTextCursor e = c;
				while (s.columnNumber() > h.start) {
					s.movePosition(QTextCursor::Left);
				}
				while (e.columnNumber() < h.end) {
					e.movePosition(QTextCursor::Right);
				}
				QRect sr = cursorRect(s);
				QRect er = cursorRect(e);
				hyperStart = QPoint(sr.left(), sr.bottom());
				hyperEnd = QPoint(er.right(), er.bottom());
			}
		}
		if (lp->normalize_error > 0) {
			if (c.columnNumber() >= lp->normalizeErrorPosition.x() && c.columnNumber() <= lp->normalizeErrorPosition.y()) {
				overError = true;
				errText = app()->messagesProvider->errorMessage(lp->normalize_error);
			}
		}
		int errBlock = -1;
		if (!overError) {
			int cn, start, end;
			if (c.columnNumber() < lp->errorMask.count()) {
				if (lp->errorMask[c.columnNumber()]) {
					for (int i = 0; i < lp->normalizedLines.count(); i++) {
						cn = c.columnNumber();
						ProgaText pt = lp->normalizedLines[i];
						bool stop = false;
						for (int j = 0; j < pt.P.count(); j++) {
							start = pt.P[j];
							end = pt.P[j] + pt.L[j];
							if (start <= cn && cn < end) {
								errBlock = i;
								overError = true;
								stop = true;
								break;
							}
						}
						if (stop) {
							break;
						}
					}
				}
			}
		}
		if (errBlock > -1) {
			if (lp->normalizedLines[errBlock].Error > 0) {
				errText = app()->messagesProvider->errorMessage(lp->normalizedLines[errBlock].Error);
			} else {
				overError = false;
			}
		}
	}
	QTextCursor lineBegin = c;
	lineBegin.movePosition(QTextCursor::StartOfBlock);
	QTextCursor lineEnd = c;
	lineEnd.movePosition(QTextCursor::EndOfBlock);
	int start = cursorRect(lineBegin).left();
	int end = cursorRect(lineEnd).right();
	if (event->pos().x() < start || event->pos().x() > end) {
		overHyper = false;
		overError = false;
	}
	underlineHyper = overHyper;
	if (!isReadOnly()) {
		if (overHyper) {
			if (!QApplication::overrideCursor()) {
				QApplication::setOverrideCursor(Qt::PointingHandCursor);
			}
			int slashPos = name.lastIndexOf("/");
			if (slashPos != -1) {
				name = name.right(name.length() - slashPos - 1);
			}
			emit sendMessage(tr("Double click to open") + " <font color='blue'>" + name + "</font> " + tr("in new tab"), 0);
			messageEmited = true;
		} else if (overError) {
			errText.replace("&", "&amp;");
			errText.replace("\"", "&quot;");
			errText.replace("<", "&lt;");
			errText.replace(">", "&gt;");
			emit sendMessage("<font color='red'>" + errText + "</font>", 0);
			messageEmited = true;
		} else {
			if (messageEmited) {
				emit sendMessage("", 0);
				messageEmited = false;
			}
			if (QApplication::overrideCursor()) {
				QApplication::restoreOverrideCursor();
			}
		}

	}
	QTextEdit::mouseMoveEvent(event);
}

void KumirEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
	QTextCursor c = cursorForPosition(event->pos());
	bool overHyper = false;
	QString name;
	if (c.block().userData() != NULL) {
		LineProp *lp = (LineProp *)(c.block().userData());
		foreach (HyperLink h, lp->hyperlinks) {
			if (c.columnNumber() >= h.start && c.columnNumber() <= h.end) {
				overHyper = true;
				name = h.link;
				QTextCursor s = c;
				QTextCursor e = c;
				while (s.columnNumber() > h.start) {
					s.movePosition(QTextCursor::Left);
				}
				while (e.columnNumber() < h.end) {
					e.movePosition(QTextCursor::Right);
				}
				QRect sr = cursorRect(s);
				QRect er = cursorRect(e);
				hyperStart = QPoint(sr.left(), sr.bottom());
				hyperEnd = QPoint(er.right(), er.bottom());
			}
		}
	}
	if (overHyper) {
		event->accept();
		qDebug("fileOpenRequest(%s)", name.toUtf8().data());
		emit fileOpenRequest(name);
	} else {
		QTextEdit::mouseDoubleClickEvent(event);
	}
}

void KumirEdit::insertLine(int lineNo, const QString &text)
{
	K_ASSERT(lineNo >= 0);
	doInsertLine(lineNo, text);
}

void KumirEdit::commentLine(int lineNo)
{
	K_ASSERT(lineNo >= 0);
	QTextBlock block = document()->begin();
	for (int i = 0; i < lineNo; i++) {
		block = block.next();
	}
	QString txt = "| " + block.text();
	doChangeLineText(lineNo, txt);
}

void KumirEdit::uncommentLine(int lineNo, int uncommentCount)
{
	K_ASSERT(lineNo >= 0);
	K_ASSERT(uncommentCount >= 0);
	QTextBlock block = document()->begin();
	for (int i = 0; i < lineNo; i++) {
		block = block.next();
	}
	int c = 0;
	QString txt = block.text();
	while (txt.startsWith("|") || txt.startsWith(" ")) {
		if (uncommentCount == c) {
			break;
		}
		if (txt.startsWith("|")) {
			c++;
		};
		txt.remove(0, 1);
	}
	txt = txt.trimmed();
	doChangeLineText(lineNo, txt);
}

void KumirEdit::showHelpMenu()
{

	if (isReadOnly()) {
		return;
	}

	QStringList keyWordList;

	keyWordList.append(trUtf8("алг"));
	keyWordList.append(trUtf8("нач"));
	keyWordList.append(trUtf8("кон"));
	keyWordList.append(trUtf8("для"));
	keyWordList.append(trUtf8("нц"));
	keyWordList.append(trUtf8("кц"));
	keyWordList.append(trUtf8("пока"));
	keyWordList.append(trUtf8("если"));
	keyWordList.append(trUtf8("то"));
	keyWordList.append(trUtf8("иначе"));
	keyWordList.append(trUtf8("все"));
	keyWordList.append(trUtf8("ввод"));
	keyWordList.append(trUtf8("вывод"));
	keyWordList.append(trUtf8("от"));
	keyWordList.append(trUtf8("до"));
	keyWordList.append(trUtf8("выбор"));
	keyWordList.append(trUtf8("при"));
	keyWordList.append(trUtf8("выход"));
	keyWordList.append(trUtf8("арг"));
	keyWordList.append(trUtf8("рез"));
	keyWordList.append(trUtf8("аргрез"));
	keyWordList.append(trUtf8("дано"));
	keyWordList.append(trUtf8("надо"));
	keyWordList.append(trUtf8("раз"));
	keyWordList.append(trUtf8("и"));
	keyWordList.append(trUtf8("или"));
	keyWordList.append(trUtf8("не"));
	keyWordList.append(trUtf8("кц_при"));
	keyWordList.append(trUtf8("утв"));
	keyWordList.append(trUtf8("дано"));
	keyWordList.append(trUtf8("нс"));
	keyWordList.append(trUtf8("исп"));
	keyWordList.append(trUtf8("кон_исп"));
	keyWordList.append(trUtf8("знач"));
	keyWordList.append(trUtf8("шаг"));
	keyWordList.append(trUtf8("использовать"));


	QStringList typeList;

	typeList.append(trUtf8("вещ"));
	typeList.append(trUtf8("цел"));
	typeList.append(trUtf8("лит"));
	typeList.append(trUtf8("сим"));
	typeList.append(trUtf8("лог"));
	typeList.append(trUtf8("вещтаб"));
	typeList.append(trUtf8("целтаб"));
	typeList.append(trUtf8("литтаб"));
	typeList.append(trUtf8("симтаб"));
	typeList.append(trUtf8("логтаб"));


	int p = textCursor().columnNumber();
	QString line = textCursor().block().text().left(p);
	QString rline = textCursor().block().text();
	rline.remove(0, p);

	QRegExp separators = QRegExp("[\\]- \\t/;,.)(\\[+*]");
	QStringList words = line.split(separators, QString::KeepEmptyParts);
	QStringList rwords = rline.split(separators, QString::KeepEmptyParts);
	QString debug = words.last();
	QStringList suggestions;

	if (app()->settings->value("FastHelp/ShowKeywords", false).toBool()) {

		foreach (QString kw, keyWordList) {
			if (kw.startsWith(words.last())) {
				suggestions << kw;
			}
		}

		foreach (QString t, typeList) {
			if (t.startsWith(words.last())) {
				suggestions << t;
			}
		}

	}

	KumModules *modules = app()->compiler->modules();
	for (int i = 0; i < modules->count(); i++) {
		KumSingleModule *mod = modules->module(i);
		if (mod->isEnabled()) {
			for (int j = 0; j < mod->Functions()->size(); j++) {
				QString name = mod->Functions()->nameById(j);
				if (name.startsWith(words.last()) && !name.isEmpty() && !name.startsWith("!") && !name.startsWith("_") && !name.startsWith("@")) {
					suggestions << name;
				}
				if (qApp->arguments().contains("-t") && name.startsWith("@")) {
					suggestions << name;
				}
			}
		}
	}


	if (suggestions.count() > 1) {

		QListWidget *list = toolTipWindow->list;
		list->clear();
		list->addItems(suggestions);
		QRect rect = cursorRect();
		QPoint globalPos = mapToGlobal(rect.bottomRight());
		toolTipWindow->move(globalPos);
		toolTipWindow->cursorPosition = textCursor().position();
		toolTipWindow->btnInfo->setEnabled(false);
		toolTipWindow->btnEnter->setEnabled(false);
		QString curWord = words.last();
		if (rwords.count() > 0) {
			curWord += rwords.first();
		}
		QList<QListWidgetItem *> itms = toolTipWindow->list->findItems(curWord, Qt::MatchExactly);
		if (itms.count() > 0) {
			toolTipWindow->list->setCurrentItem(itms.first());
		}
		toolTipWindow->setVisible(true);
		m_blockKeyboardFlag = true;
		toolTipWindow->list->setFocus();
	} else if (suggestions.count() > 0) {
		QString appendTxt = suggestions[0];
		appendTxt.remove(0, words.last().length());
		int removeCount = 0;
		if (rwords.count() > 0) {
			removeCount = rwords.first().length();
		}
		doFastInsertText(appendTxt, removeCount);
	}

}

void KumirEdit::insertTxtFromPopup(QListWidgetItem *item, const QString &reason)
{
	textCursor().setPosition(toolTipWindow->cursorPosition);
	if (item != NULL) {
		QString appendTxt = item->text();
		toolTipWindow->setVisible(false);
		this->setFocus();
		QRegExp separators = QRegExp("[\\]- \\t/;,.)(\\[+*]");
		QString line = textCursor().block().text().left(textCursor().columnNumber());
		QString rline = textCursor().block().text();
		rline.remove(0, textCursor().columnNumber());
		QStringList words = line.split(separators, QString::KeepEmptyParts);
		QStringList rwords = rline.split(separators, QString::KeepEmptyParts);
		int removeCount = 0;
		if (rwords.count() > 0) {
			removeCount = rwords.first().length();
		}
		appendTxt.remove(0, words.last().length());
		doFastInsertText(appendTxt, removeCount);
		unlockEditor();
	} else {
		toolTipWindow->setVisible(false);
		unlockEditor();
		this->setFocus();
	}
	if (reason == "key") {
		m_blockKeyboardCounter++;
	}
}

// void KumirEdit::gotoPrevError()
// {
//  int line = 0;
//  QTextCursor cur = textCursor();
//  bool first = true;
//  do {
//      int col = cur.columnNumber();
//      LineProp *lp = static_cast<LineProp*>(cur.block().userData());
//      if ( lp==NULL )
//          return;
//      if ( lp->errorPositions.count() > 0 ) {
//          for ( int i=lp->errorPositions.count()-1; i>=0; i-- ){
//              QPoint er = lp->errorPositions[i];
//              if ( first ) {
//                  if ( col > er.y() )
//                  {
//                      cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,col-er.y());
//                      setTextCursor(cur);
//                      ensureCursorVisible();
//                      return;
//                  }
//              }
//              else {
//                  cur.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,col-er.y());
//                  setTextCursor(cur);
//                  ensureCursorVisible();
//                  return;
//              }
//          }
//      }
//      cur.movePosition(QTextCursor::PreviousBlock);
//      cur.movePosition(QTextCursor::EndOfBlock);
//      line = cur.blockNumber();
//      first = false;
//  } while ( line > 0 );
//
//  checkJumpErrorsAvailability();
// }
//
// void KumirEdit::gotoNextError()
// {
//  int line = 0;
//  QTextCursor cur = textCursor();
//  bool first = true;
//  do {
//      int col = cur.columnNumber();
//      LineProp *lp = static_cast<LineProp*>(cur.block().userData());
//      if ( lp==NULL )
//          return;
//      if ( lp->errorPositions.count() > 0 ) {
//          foreach ( QPoint er, lp->errorPositions ) {
//              if ( first ) {
//                  if ( col < er.x() )
//                  {
//                      cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,er.x()-col);
//                      setTextCursor(cur);
//                      ensureCursorVisible();
//                      return;
//                  }
//              }
//              else {
//                  cur.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,er.x());
//                  setTextCursor(cur);
//                  ensureCursorVisible();
//                  return;
//              }
//          }
//      }
//      cur.movePosition(QTextCursor::NextBlock);
//      line = cur.blockNumber();
//      first = false;
//  } while ( line < document()->blockCount() );
//  checkJumpErrorsAvailability();
//
// }
//
// bool KumirEdit::isJumpNextErrorAvailable()
// {
//  bool result = false;
//  QTextBlock curBlock = textCursor().block();
//  LineProp *lp = static_cast<LineProp*>(curBlock.userData());
//  if ( lp==NULL )
//      return false;
//  int line = textCursor().blockNumber();
//  int col = textCursor().columnNumber();
//  if ( lp->errorPositions.count() > 0 ) {
//      foreach ( QPoint er, lp->errorPositions ) {
//          if ( col < er.x() )
//              result = true;
//      }
//  }
//  while ( !result ) {
//      curBlock = curBlock.next();
//      if ( curBlock.isValid() ) {
//          lp = static_cast<LineProp*>(curBlock.userData());
//          if ( lp==NULL )
//              return false;
//          if ( lp->errorPositions.count() > 0 )
//              result = true;
//      }
//      else
//          break;
//  }
//
//  return result;
// }
//
// bool KumirEdit::isJumpPrevErrorAvailable()
// {
//  bool result = false;
//  QTextBlock curBlock = textCursor().block();
//  LineProp *lp = static_cast<LineProp*>(curBlock.userData());
//  if ( lp==NULL )
//      return false;
//  int line = textCursor().blockNumber();
//  int col = textCursor().columnNumber();
//  if ( lp->errorPositions.count() > 0 ) {
//      foreach ( QPoint er, lp->errorPositions ) {
//          if ( col > er.y() )
//              result = true;
//      }
//  }
//  while ( !result ) {
//      curBlock = curBlock.previous();
//      if ( curBlock.isValid() ) {
//          lp = static_cast<LineProp*>(curBlock.userData());
//          if ( lp==NULL )
//              return false;
//          if ( lp->errorPositions.count() > 0 )
//              result = true;
//      }
//      else
//          break;
//  }
//  return result;
// }
//
// void KumirEdit::checkJumpErrorsAvailability()
// {
//  emit jumpPrevErrorAvailable(isJumpPrevErrorAvailable());
//  emit jumpNextErrorAvailable(isJumpNextErrorAvailable());
// }

void KumirEdit::copy()
{
	//  QTextEdit::copy();
	QString plainText = getSelectedPlainText();
	QString html = getSelectedHtml();
	QMimeData *d = new QMimeData();
	d->setText(plainText);
	d->setHtml(html);
	K_ASSERT(d->hasHtml());
	K_ASSERT(d->hasText());
	qDebug() << d->html();
	qDebug() << d->text();
	QApplication::clipboard()->setMimeData(d);
	//  qDebug("copy()");
	//  if ( !textCursor().hasSelection() )
	//      return;
	//  int start = textCursor().selectionStart();
	//  int end = textCursor().selectionEnd();
	//  QTextCursor cur(document());
	//  cur.setPosition(start);
	//  int ls = cur.blockNumber();
	//  int cs = cur.columnNumber();
	//  cur = QTextCursor(document());
	//  cur.setPosition(end);
	//  int le = cur.blockNumber();
	//  int ce = cur.columnNumber();
	//  QTextBlock bl;
	//  QString result;
	//  int i = 0;
	//  for ( bl = document()->begin(); bl.isValid(); bl=bl.next() ) {
	//      if ( i >= ls && i <= le ) {
	//          QString line = bl.text();
	//          LineProp *lp = static_cast<LineProp*>(bl.userData());
	//          if ( lp != NULL ) {
	// //               int indent_count = lp->indent_count;
	//              int indent_count = 0;
	//              for ( int j=0; j<indent_count; j++ )
	//                  line = QString::fromUtf8(". ") + line;
	//          }
	//          result += line;
	//          if ( bl.next().isValid() )
	//              result += "\n";
	//      }
	//      i++;
	//  }
	//  QApplication::clipboard()->setText(result);
}

void KumirEdit::copyWithFormatting()
{
	qDebug("KumirEdit::copyWithFormatting()");
	if (!textCursor().hasSelection()) {
		return;
	}
	//  int start = textCursor().selectionStart();
	//  int end = textCursor().selectionEnd();
	//  QTextCursor cur(document());
	//  cur.setPosition(start);
	//  int ls = cur.blockNumber();
	//  int cs = cur.columnNumber();
	//  Q_UNUSED(cs);
	//  cur = QTextCursor(document());
	//  cur.setPosition(end);
	//  int le = cur.blockNumber();
	//  int ce = cur.columnNumber();
	//  Q_UNUSED(ce);
	//  QTextBlock bl;
	//  QString result;
	//  QStringList marginLines = margin->toPlainText().split("\n",QString::KeepEmptyParts);
	//  int i = 0;
	//  int rang = 0;
	//  for ( bl = document()->begin(); bl.isValid(); bl=bl.next() ) {
	//      if ( i >= ls && i <= le ) {
	//          if ( app()->settings->value("Appearance/ShowLineNumbers",false).toBool() ) {
	//              if ( i+1 < 10 )
	//                  result += " ";
	//              if ( i+1 < 100 )
	//                  result += " ";
	//              result += QString::number(i+1)+ "\t";
	//          }
	//          QString line = bl.text();
	////            if ( i==ls ) {
	////                for ( int j=0; i<cs; j++ )
	////                    line[j] = ' ';
	////            }
	////            if ( i==le ) {
	////                for ( int j=ce; j<line.length(); j++ )
	////                    line[j] = ' ';
	////            }
	//          LineProp *lp = static_cast<LineProp*>(bl.userData());
	//          if ( lp != NULL ) {
	//              rang += lp->indentRang;
	//              for ( int j=0; j<rang; j++ )
	//                  line = QString::fromUtf8(". ") + line;
	//              rang += lp->indentRangNext;
	//          }
	//          result += line;
	//          for ( int j=0; j<50-line.length(); j++ )
	//              result += " ";
	//          result += QString::fromUtf8("||");
	//          result += marginLines[i] + "\n";
	//      }
	//      i++;
	//  }
	//  QApplication::clipboard()->setText(result);
	QString html = getSelectedHtml();
	QMimeData *d = new QMimeData();
	d->setHtml(html);
	K_ASSERT(d->hasHtml());
	qDebug() << d->html();
	QApplication::clipboard()->setMimeData(d);
	//  QApplication::clipboard()->setText(html);
}

QString KumirEdit::getSelectedPlainText() const
{
	QTextCursor cur = textCursor();
	int startPos = cur.selectionStart();
	int endPos = cur.selectionEnd();
	QTextBlock startBlock = document()->findBlock(startPos);
	QTextBlock endBlock = document()->findBlock(endPos);
	QString result = "";
	QString line;
	int startCol = startPos - startBlock.position();
	int endCol = endPos - endBlock.position();
	for (QTextBlock b = startBlock; b != endBlock.next(); b = b.next()) {
		line = b.text();
		if (b == startBlock && startBlock == endBlock) {
			line = line.mid(startCol, endCol - startCol);
		} else if (b == startBlock) {
			line = line.mid(startCol);
		} else if (b == endBlock) {
			line = line.mid(0, endCol);
		}
		result += line;
		if (b != endBlock) {
			result += "\n";
		}
	}
	return result;
}

QString KumirEdit::getSelectedHtml() const
{
	QTextCursor cur = textCursor();
	int startPos = cur.selectionStart();
	int endPos = cur.selectionEnd();
	QTextBlock startBlock = document()->findBlock(startPos);
	QTextBlock endBlock = document()->findBlock(endPos);
	QString line;
	QString htmlLine;
	QString result;
	QString mLine;
	QTextBlock mBlock;
	QStringList programLines;
	QStringList marginLines;
	QFontMetricsF fm(font());
//    qreal baseWidth = fm.width('w');
	QStringList p_style;
	p_style << "font-family:\"Droid Sans Mono\",\"Consolas\",\"Courier New\",\"Courier\",\"Monospace\"";
	p_style << QString("font-size:%1pt").arg(font().pointSize());
	p_style << "margin:0px";
	p_style << "padding:0px";
	result = "<table width='100%'><tr><td width=60%'><p style='" + p_style.join(";") + "'>\n";
	int indent = 0;
	for (QTextBlock b = startBlock; b != endBlock.next(); b = b.next()) {
		line = b.text();
		mBlock = margin->document()->findBlockByNumber(b.blockNumber());
		mLine = mBlock.text();
		if (mLine.trimmed().isEmpty()) {
			mLine = " ";
		}
		LineProp *lp = dynamic_cast<LineProp *>(b.userData());

		htmlLine = sh->highlightBlockAsHtml(line, lp);
		Q_CHECK_PTR(lp);
		indent += lp->indentRang;
		QString dots;
		for (int i = 0; i < indent; i++) {
			dots = ". " + dots;
		}
		indent += lp->indentRangNext;
		if (b.userState() == HIDDEN) {
			htmlLine = "<span style='background-color:#EBE2D2;'>" + htmlLine + "</span>";
		} else if (b.userState() == PROTECTED) {
			htmlLine = "<span style='background-color:#CFDDCF;'>" + htmlLine + "</span>";
		}
		htmlLine = dots + htmlLine;
		programLines << htmlLine;
		marginLines << mLine;
	}
	result += programLines.join("<br>\n");
	result += "</p></td><td>\n<p style='font-family:courier,monospace;font-size:8pt;margin:0px;padding:0px;'>";
	result += marginLines.join("<br>\n");
	result += "</p></td></tr>\n";
	result += "</table>\n";
//    qDebug() << result;
	return result;
}

void KumirEdit::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton && shiftPressed) {
		event->accept();
		contextMenuEvent(event->pos(), event->globalPos(), true);
		shiftPressed = false;
	} else {
		int curBlockNo = textCursor().blockNumber();
		int clickBlockNo = cursorForPosition(event->pos()).blockNumber();
		QTextEdit::mousePressEvent(event);
		if (curBlockNo != clickBlockNo) {
			checkLineChanged();
		}
	}
}

void KumirEdit::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Shift) {
		shiftPressed = false;
	}
	QTextEdit::keyReleaseEvent(event);
}

void KumirEdit::undo()
{
	if (isReadOnly()) {
		return;
	}
//    int linesBefore = toPlainText().count("\n");
	TextEdit::undo();
//    int linesAfter = toPlainText().count("\n");
//    if ( linesBefore != linesAfter )
	{
		emit requestCompilation(tabNo);
	}
}

void KumirEdit::redo()
{
	TextEdit::redo();
	emit requestCompilation(tabNo);
}

void KumirEdit::forcedRepaint()
{
	QPaintEvent *event = new QPaintEvent(QRegion(0, 0, width(), height()));
	paintEvent(event);
}

void KumirEdit::setPostScriptumText(const QString &txt)
{
	postScriptum->setPlainText(txt);
	initLineProps(postScriptum, QList<int>(), QList<int>());
	QList<QTextBlock> blocks;
	QTextBlock bl = postScriptum->begin();
	while (bl.isValid()) {
		blocks << bl;
		bl = bl.next();
	}
	normalizeLines(postScriptum, blocks);
	// Perform checks...
	for (QTextBlock bl = postScriptum->begin(); bl != postScriptum->end(); bl = bl.next()) {
		LineProp *lp = static_cast<LineProp *>(bl.userData());
		Q_CHECK_PTR(lp);
		for (int i = 0; i < lp->normalizedLines.size(); i++) {
			ProgaText pt = lp->normalizedLines[i];
			Q_ASSERT(pt.stroka = -1);
		}
	}
}

void KumirEdit::normalizeLines(QPointer<QTextDocument> doc, QList<QTextBlock> blocks)
{
	if (newUndoStack->inMacro) {
		return;
	}
	int nlc = normLinesTotalCount;
	bool needRehighlight = false;
	int newNLC = 0;
	for (QTextBlock block = doc->begin();
		block != doc->end();
		block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		Q_ASSERT(lp != NULL);
		newNLC += lp->normalizedLines.count();
	}
	bool isMainText = doc != hiddenProlog && doc != hiddenEpilog && doc != postScriptum;
	foreach (QTextBlock block, blocks) {

		LineProp *lp = static_cast<LineProp *>(block.userData());
		K_ASSERT(lp != NULL);
		int nlc_count = 0;
		int diff = 0;
		if (lp != NULL && lp->lineChanged) {
			nlc_count = lp->normalizedLines.count();
			QList<ProgaText> ptl;
			lp->normalize_error = KumTools::instance()->splitUserLine(block.text().mid(lp->indentCount * 2), lp->normalizeErrorPosition, &ptl);
			int pc = 0;
			Q_UNUSED(pc);
			if (doc != document()) {
				// Пролог или элилог -- проставляем номера строк в -1
				for (int i = 0; i < ptl.count(); i++) {
					ptl[i].stroka = -1;
				}
			}


			if (/*lp->errorPositions.count() > 0 ||*/ lp->error_count > 0 || lp->error > 0) {
				lp->errorMask.clear();
				lp->errorMask.fill(block.text().length(), false);
				lp->error_count = lp->error = 0;
				needRehighlight = true;
				if (isMainText) {
					emit clearError(QTextCursor(block).blockNumber());
				}
			}
			diff = ptl.count() - nlc_count;
			//          lp->errorPositions.clear();
			lp->error_count = 0;
			lp->lineChanged = false;
			bool normLinesChanged = false;

			//          QList< QList<uint> > PP, LL;
			//          for ( int i=0; i<ptl.count(); i++ ) {
			//              QList<uint> P, L;
			//              for ( int j=0; j<ptl[i].P.count() ; j++ ) {
			//                  P << ptl[i].P[j];
			//                  L << ptl[i].L[j];
			//              }
			//              PP << P;
			//              LL << L;
			//          }

			if (ptl.count() == lp->normalizedLines.count()) {
				for (int i = 0; i < ptl.count(); i++) {
					lp->normalizedLines[i].L = ptl[i].L;
					lp->normalizedLines[i].P = ptl[i].P;
					if (! ptl[i].equals(lp->normalizedLines[i])) {
						lp->normalizedLines[i] = ptl[i];
						//                      for ( int y=0; y<PP[i].count(); y++ ) {
						//                          lp->normalizedLines[i].P << PP[i][y];
						//                          lp->normalizedLines[i].L << LL[i][y];
						//                      }
						//                      lp->normalizedLines[i].P = ptl[i].P;
						//                      lp->normalizedLines[i].L = ptl[i].L;
						normLinesChanged = true;
					}
				}
			} else {
				lp->normalizedLines = ptl;
				//              for ( int x=0; x<ptl.count(); x++ ) {
				// //                   lp->normalizedLines[x].P = ptl[x].P;
				// //                   lp->normalizedLines[x].L = ptl[x].L;
				// //                   int dbg2 = PP[x].count();
				// //                   for ( int y=0; y<PP[x].count(); y++ ) {
				// //                       lp->normalizedLines[x].P << PP[x][y];
				// //                       lp->normalizedLines[x].L << LL[x][y];
				// //                   }
				//                  int dbg = lp->normalizedLines[x].P.count();
				//
				//              }
				normLinesChanged = true;
			}
			if (normLinesChanged) {
				QPoint indent = KumTools::instance()->getIndentRang(lp->normalizedLines);
				lp->indentRang = indent.x();
				lp->indentRangNext = indent.y();
			}
			needCompilation = needCompilation || normLinesChanged;
			nlc += diff;
		} else if (lp != NULL) {
			nlc += diff;
		}
	}
	needCompilation = needCompilation || (nlc != normLinesTotalCount) || (nlc != newNLC);
	normLinesTotalCount = nlc;
	//  if ( prevLinesCount != document()->blockCount() )
	//      applyIndentation();
	// ===================== ЗАКОММЕНТИРОВАНО, Т.К. CALLGRIND ГОВОРИТ, ЧТО СЪЕДАЕТ МНОГО ПРОЦЕССОРНОГО ВРЕМЕНИ
	if (needRehighlight) {
		sh->rehighlight();
	}
	// ===================== end callcgrind
}

void KumirEdit::normalizeLines(QPointer<QTextDocument> doc)
{
	QList<QTextBlock> blocks;
	for (QTextBlock bl = doc->begin(); bl != doc->end(); bl = bl.next()) {
		blocks << bl;
	}
	normalizeLines(doc, blocks);
}

void KumirEdit::checkForCompilationRequired()
{
	if (newUndoStack->inMacro) {
		return;
	}
	if (needCompilation) {
		emit requestCompilation(tabNo);
	}
	needCompilation = false;
}

QList<ProgaText> KumirEdit::normText(QPointer<QTextDocument> doc)
{
	QList<ProgaText> result;
	if (doc.isNull()) {
		return result;
	}

	int i = 0;
	for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		if (lp == NULL) {
			initLineProps(doc);
			lp = static_cast<LineProp *>(block.userData());
		}
		Q_CHECK_PTR(lp);

		bool good = true;
		for (int j = 0; j < lp->normalizedLines.count(); j++) {
			ProgaText pt = lp->normalizedLines[j];
			good = good && pt.L.count() == pt.P.count() && pt.P.count() == pt.Text.length();
		}
		if (!good) {
			normalizeLines(doc, QList<QTextBlock>() << block);
		}
		for (int j = 0; j < lp->normalizedLines.count(); j++) {
			ProgaText ct = lp->normalizedLines[j];
			ct.Error = lp->normalize_error;
			ct.stroka = doc == hiddenProlog || doc == hiddenEpilog ? -1 : i;
			ct.position = j;
			ct.Base_type = KumTools::instance()->getStringType(ct.Text);
			result << ct;
		}
		i++;
	}
	return result;

}

QList<ProgaText> KumirEdit::completeNormalizedText()
{
	QList<ProgaText> prolog = normText(hiddenProlog);
	QList<ProgaText> main = normText(document());
	QList<ProgaText> epilog = normText(hiddenEpilog);
	QList<ProgaText> ps = normText(postScriptum);

	for (int i = 0; i < ps.size(); i++) {
//        const ProgaText pt = ps[i];
//        Q_ASSERT(pt.stroka==-1);
		ps[i].stroka = -1;
	}


	QList<ProgaText> result = prolog + main + epilog + ps;
	//  qDebug() << "NORMALIZED TEXT: ==========================";
	//  foreach (ProgaText pt, result) {
	//      qDebug() << pt.Text;
	//  }
	//  qDebug() << "END NORMALIZED TEXT: ======================";
	return result;
}

QList< QList< ProgaText > > KumirEdit::normalizedText()
{
	QList < QList <ProgaText> > result;
	QList < ProgaText > curModule;
	int i = 0;
	bool inIsp = false;
	bool ispFound = false;
	bool inInnerIsp = false;
	for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		if (lp == NULL) {
			initLineProps(document());
		}
		Q_CHECK_PTR(lp);
		for (int j = 0; j < lp->normalizedLines.count(); j++) {
			ProgaText ct = lp->normalizedLines[j];
			int pc = lp->normalizedLines[j].P.count();
			Q_UNUSED(pc);
			ct.Error = 0;
			ct.stroka = i;
			ct.position = j;
			if (KumTools::instance()->getStringType(ct.Text) == Kumir::ModuleBegin) {
				//              if ( result.isEmpty() && curModule.isEmpty() ) {
				//                  result << QList<ProgaText>();
				//              }
				if (inIsp) {
					ct.Error = TOS_ISP_IN_ISP;
					//                  ct.err_start = -1;
					//                  ct.err_end = -1;
					inInnerIsp = true;
				} else {
					result << curModule;
					curModule.clear();
				}
				inIsp = true;
				ispFound = true;
			} else if (KumTools::instance()->getStringType(ct.Text) == Kumir::ModuleEnd) {
				inIsp = false;
				if (inInnerIsp) {
					ct.Error = TOS_ISP_IN_ISP;
					//                  ct.err_start = -1;
					//                  ct.err_end = -1;
					inInnerIsp = false;
				}
			} else {
				if (ispFound && !inIsp) {
					ct.Error = TOS_AFTER_ISP;
					//                  ct.err_start = -1;
					//                  ct.err_end = -1;
				}
				if (inInnerIsp) {
					ct.Error = TOS_ISP_IN_ISP;
					//                  ct.err_start = -1;
					//                  ct.err_end = -1;
				}
			}
			if (lp->normalize_error > 0) {
				ct.Error = lp->normalize_error;
			}
			curModule << ct;
		}
		i++;
	}
	if (!curModule.isEmpty()) {
		result << curModule;
	}
	if (result.isEmpty()) {
		result << QList<ProgaText>();
	}
	return result;
}

LineProp *KumirEdit::linePropByLineNumber(QTextDocument *doc, int no)
{
	if (no < 0 || no >= doc->blockCount()) {
		return NULL;
	}
	QTextBlock bl = doc->begin();
	for (int i = 0; i < no; i++) {
		bl = bl.next();
	}
	K_ASSERT(bl.isValid());
	LineProp *lp = static_cast<LineProp *>(bl.userData());
	return lp;
}


void KumirEdit::setErrorsForModules(const QList<int> &ids)
{
	foreach (int id, ids) {
		KumSingleModule *module = app()->compiler->modules()->module(id);
		Q_CHECK_PTR(module);
		for (int i = 0; i < module->Proga_Value()->count(); i++) {
			proga pv = module->Proga_Value()->at(i);
			int line = pv.real_line.lineNo;
			int pos = pv.real_line.termNo;
			if (line == -1 || pos == -1) {
				continue;
			}
			LineProp *lp = linePropByLineNumber(document(), line);
			Q_CHECK_PTR(lp);
			if (lp->normalizedLines[pos].Error == 0) {
				lp->normalizedLines[pos].Error = pv.error;
				lp->normalizedLines[pos].setErrorRange(pv.err_start, pv.err_length);
				//              lp->normalizedLines[pos].P = pv.P;
				//              lp->normalizedLines[pos].L = pv.L;
			}
		}
	}
}

void KumirEdit::clearErrors()
{
	for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		Q_CHECK_PTR(lp);
		lp->errorMask = QVector<bool>(block.text().length(), false);
		lp->indentRang = 0;
		lp->indentRangNext = 0;
		for (int j = 0; j < lp->normalizedLines.count(); j++) {
			lp->normalizedLines[j].Error = 0;
		}
	}
}

#ifdef USE_OLD_TEST_OPER_SKOBKA

void KumirEdit::setNormalizedText(const QList<int> &modulesId)
{
	foreach (int moduleId, modulesId) {
		KumSingleModule *module = app()->compiler->modules()->module(moduleId);
		Q_CHECK_PTR(module);
		for (int i = 0; i < module->progaCount(); i++) {
			ProgaText *pt = module->Proga_Text_Line(i);
			int PC = module->Proga_Text_Line(i)->P.count();
			Q_CHECK_PTR(pt);
			LineProp *lp = linePropByLineNumber(document(), pt->stroka);
			Q_CHECK_PTR(lp);
			int error = pt->Error;
			int pos = pt->position;
			lp->normalizedLines[pos].Error = error;
			//          lp->normalizedLines[pt->position].err_start = pt->err_start;
			//          lp->normalizedLines[pt->position].err_end = pt->err_end;
			//          lp->normalizedLines[pt->position].err_start_incl = pt->err_start_incl;
			//          lp->normalizedLines[pt->position].err_end_incl = pt->err_end_incl;
			K_ASSERT(pt->P.count() > 0);
			lp->normalizedLines[pos].P = pt->P;
			K_ASSERT(lp->normalizedLines[pos].P.count() > 0);
			lp->normalizedLines[pos].setErrorRange(pt->errstart, pt->errlen);
		}
	}
	int i = 0; // for debug
	for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next()) {
		LineProp *lp = static_cast<LineProp *>(block.userData());
		Q_CHECK_PTR(lp);
		lp->error = 0;
		foreach (ProgaText pt, lp->normalizedLines) {
			if (pt.Error > 0 && lp->error == 0) {
				lp->error = pt.Error;
				break;
			}
		}
		//      qDebug() << "Line: " << block.text() << " Error: " << lp->error;
		//      lp->errorPositions = KumTools::instance()->recalculateErrorsPositions(block.text().mid(lp->indentCount*2),lp->normalizedLines);
		lp->errorMask = KumTools::instance()->recalculateErrorPositions(block.text().mid(lp->indentCount * 2), lp->normalizedLines);
		QPoint indRang = KumTools::instance()->getIndentRang(lp->normalizedLines);
		lp->indentRang = indRang.x();
		lp->indentRangNext = indRang.y();
		i++;
	}
	applyIndentation();
	sh->rehighlight();
}

void KumirEdit::setError(int lineNo, int termNo, int error, ErrorPosition position)
{
	LineProp *lp = linePropByLineNumber(document(), lineNo);
	Q_CHECK_PTR(lp);
	lp->normalizedLines[termNo].Error = error;
	//  lp->normalizedLines[termNo].err_start = position.start;
	//  lp->normalizedLines[termNo].err_end = position.end;
	//  lp->normalizedLines[termNo].err_end = position.start + position.len;
	//  lp->normalizedLines[termNo].err_start_incl = position.start_incl;
	//  lp->normalizedLines[termNo].err_end_incl = position.end_incl;
	sh->rehighlight();
}


#else

void KumirEdit::setNormalizedText(const QList<int> &modulesId)
{
	foreach (int moduleId, modulesId) {
		KumSingleModule *module = app()->compiler->modules()->module(moduleId);
		Q_CHECK_PTR(module);
		for (int i = 0; i < module->progaCount(); i++) {
			ProgaText *pt = module->Proga_Text_Line(i);
			int PC = module->Proga_Text_Line(i)->P.count();
			Q_UNUSED(PC);
			Q_CHECK_PTR(pt);
			LineProp *lp = linePropByLineNumber(document(), pt->stroka);
			Q_CHECK_PTR(lp);
			int error = pt->Error;
			int pos = pt->position;
			lp->normalizedLines[pos].Error = error;
			//          lp->normalizedLines[pt->position].err_start = pt->err_start;
			//          lp->normalizedLines[pt->position].err_end = pt->err_end;
			//          lp->normalizedLines[pt->position].err_start_incl = pt->err_start_incl;
			//          lp->normalizedLines[pt->position].err_end_incl = pt->err_end_incl;
			K_ASSERT(pt->P.count() > 0);
			lp->normalizedLines[pos].P = pt->P;
			K_ASSERT(lp->normalizedLines[pos].P.count() > 0);
			lp->normalizedLines[pos].setErrorRange(pt->errstart, pt->errlen);
		}
	}

	// ========= Закомментировано, т.к. этот же код выполняется в Compiler::updateEditor()

	//  int i = 0; // for debug
	//  for ( QTextBlock block=document()->begin(); block!=document()->end(); block = block.next() ) {
	//      LineProp *lp = static_cast<LineProp*>(block.userData());
	//      Q_CHECK_PTR ( lp );
	//      lp->error = 0;
	//      foreach ( ProgaText pt, lp->normalizedLines ) {
	//          if ( pt.Error > 0 && lp->error == 0 ) {
	//              lp->error = pt.Error;
	//              break;
	//          }
	//      }
	// //       qDebug() << "Line: " << block.text() << " Error: " << lp->error;
	// //       lp->errorPositions = KumTools::instance()->recalculateErrorsPositions(block.text().mid(lp->indentCount*2),lp->normalizedLines);
	//      lp->errorMask = KumTools::instance()->recalculateErrorPositions(block.text().mid(lp->indentCount*2),lp->normalizedLines);
	//      QPoint indRang = KumTools::instance()->getIndentRang(lp->normalizedLines);
	//      lp->indentRang = indRang.x();
	//      lp->indentRangNext = indRang.y();
	//      i++;
	//  }
	applyIndentation();
	sh->rehighlight();
}

void KumirEdit::setError(int lineNo, int termNo, int error, ErrorPosition position)
{
	LineProp *lp = linePropByLineNumber(document(), lineNo);
	Q_CHECK_PTR(lp);
	lp->normalizedLines[termNo].Error = error;
	lp->normalizedLines[termNo].errstart = position.start;
	lp->normalizedLines[termNo].errlen = position.len;
	//      lp->normalizedLines[termNo].err_start = position.start;
	//      lp->normalizedLines[termNo].err_end = position.end;
	//  lp->normalizedLines[termNo].err_end = position.start + position.len;
	//  lp->normalizedLines[termNo].err_start_incl = position.start_incl;
	//  lp->normalizedLines[termNo].err_end_incl = position.end_incl;
	//  sh->rehighlight();
}

#endif

void KumirEdit::setErrorsAndRanks(const QList<ProgaText> &pts)
{
	for (int i = 0; i < pts.size(); i++) {
		const ProgaText pt = pts[i];
		if (pt.stroka == -1) {
			continue;
		}
		LineProp *lp = linePropByLineNumber(document(), pt.stroka);
		Q_CHECK_PTR(lp);
		int termNo = pt.position;
		Q_ASSERT(termNo < lp->normalizedLines.size());
		lp->normalizedLines[termNo].Error = pt.Error;
		lp->normalizedLines[termNo].errstart = pt.errstart;
		lp->normalizedLines[termNo].errlen = pt.errlen;
		lp->normalizedLines[termNo].indentRank = pt.indentRank;
		//      lp->normalizedLines[pt.position].P = pt.P;
		//      lp->normalizedLines[pt.position].L = pt.L;
	}

	// ========= Закомментировано, т.к. этот же код выполняется в Compiler::updateEditor()

	//  for ( QTextBlock block=document()->begin(); block != document()->end(); block=block.next() ) {
	//      LineProp *lp = static_cast<LineProp*>(block.userData());
	//      Q_CHECK_PTR ( lp );
	//      lp->errorMask = KumTools::instance()->recalculateErrorPositions(block.text().mid(lp->indentCount*2),lp->normalizedLines);
	//      QPoint indRang = KumTools::instance()->getIndentRang(lp->normalizedLines);
	//      lp->indentRang = indRang.x();
	//      lp->indentRangNext = indRang.y();
	//  }
}


void KumirEdit::doComment(int from, int to)
{
	K_ASSERT(from >= 0);
	K_ASSERT(to >= 0);
	QTextCursor c(document());
	bool hasProtected = false;
	int i = from;
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	while (i <= to) {
		if (c.block().userState() == PROTECTED && !teacherMode) {
			hasProtected = true;
			return;
		}
		c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		i++;
	}
	save(from, (uint)Comment, QChar('\0'), false);
	c = QTextCursor(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	i = from;
	QList<QTextBlock> blocks;
	while (i <= to) {
		const QString txt = "| ";
		c.insertText(txt);
		LineProp *lp = static_cast<LineProp *>(c.block().userData());
		Q_CHECK_PTR(lp);
		lp->lineChanged = true;
		emit yellowIndicator();
		blocks << c.block();
		c.movePosition(QTextCursor::NextBlock);
		i++;
	}
	normalizeLines(document(), blocks);
}

void KumirEdit::doUncomment(int from, int to)
{
	K_ASSERT(from >= 0);
	K_ASSERT(to >= 0);
	QTextCursor c(document());
	bool hasProtected = false;
	int i = from;
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	while (i <= to) {
		if (c.block().userState() == PROTECTED && !teacherMode) {
			hasProtected = true;
			return;
		}
		c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
		i++;
	}
	c = QTextCursor(document());
	save(from, (uint)Uncomment, QChar('\0'), false);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, from);
	i = from;
	QList<QTextBlock> blocks;
	while (i <= to) {
		QString txt = c.block().text();
		uint p = 0;
		uint n = 0;
		bool found = false;
		for (int j = 0; j < txt.length(); j++) {
			if (txt[j] == ' ' || txt[j] == '\t') {
				if (found) {
					n++;
				}
			} else if (txt[j] == '|') {
				if (found) {
					break;
				}
				found = true;
				p = j;
				n = 1;
			} else {
				break;
			}
		}
		if (found) {
			c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, p + n);
			c.removeSelectedText();
			LineProp *lp = static_cast<LineProp *>(c.block().userData());
			Q_CHECK_PTR(lp);
			lp->lineChanged = true;
			emit yellowIndicator();
		}

		blocks << c.block();
		c.movePosition(QTextCursor::NextBlock);
		i++;
	}
	normalizeLines(document(), blocks);
}


void KumirEdit::doInsertFirst(const QString &txt)
{
	emit yellowIndicator();
	TextEdit::doInsertFirst(txt);
	int linesCount = txt.count("\n") + 1;
	QTextBlock b = document()->begin();
	QList<QTextBlock> blocksToNormalize;
	for (int i = 0; i < linesCount; i++) {
		LineProp *lp = new LineProp();
		lp->lineChanged = true;
		b.setUserData(lp);
		blocksToNormalize << b;
		b = b.next();
	}
	normalizeLines(document(), blocksToNormalize);
	applyIndentation(document()->begin());
	checkLineChanged();
}

void KumirEdit::doInsertLine(int line, const QString &txt)
{
	K_ASSERT(line >= 0);
	save(line, (uint)InsertLine, QChar('\0'), false);
	QTextBlock from = textCursor().block();
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
	c.insertText(txt);
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	Q_CHECK_PTR(lp);
	lp->lineChanged = true;
	emit yellowIndicator();
	//  int indent = lp->indent_next + lp->indent_count;
	c.insertText("\n");
	lp = new LineProp();
	//  lp->indent_count = indent;
	lp->lineChanged = true;
	emit yellowIndicator();
	c.block().setUserData(lp);
	normalizeLines(document(), QList<QTextBlock>() << from << c.block());
	applyIndentation(from);
	checkLineChanged();
}

void KumirEdit::doChangeLineText(int line, const QString &txt)
{
	K_ASSERT(line >= 0);
	save(line, (uint)ChangeLineText, QChar('\0'), false);
	QTextCursor c(document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.removeSelectedText();
	c.insertText(txt);
	LineProp *lp = static_cast<LineProp *>(c.block().userData());
	Q_CHECK_PTR(lp);
	lp->lineChanged = true;
	emit yellowIndicator();
	normalizeLines(document(), QList<QTextBlock>() << c.block());
}



void KumirEdit::doFastInsertText(const QString &txt, int removeCount)
{
	K_ASSERT(removeCount >= 0);
	save(textCursor().blockNumber(), (uint)FastInsertText, QChar('\0'), false);
	textCursor().movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, removeCount);
	textCursor().removeSelectedText();
	textCursor().insertText(txt);
	LineProp *lp = static_cast<LineProp *>(textCursor().block().userData());
	Q_CHECK_PTR(lp);
	lp->lineChanged = true;
	emit yellowIndicator();
	normalizeLines(document(), QList<QTextBlock>() << textCursor().block());
}

/**
 * Поддержка пустой строки в конце.
 */
void KumirEdit::keepEmptyLine()
{
	if (lockKeepEmptyLine) {
		return;
	}
	QTextCursor c(document());
	c.movePosition(QTextCursor::End);
	QTextBlock block = c.block();
	if (! block.text().trimmed().isEmpty()) {
		lockKeepEmptyLine = true;
		c.insertText("\n");
		block = c.block();
		block.setUserData(new LineProp());
		c.movePosition(QTextCursor::PreviousCharacter);
		lockKeepEmptyLine = false;
	}


}


void KumirEdit::updateDummyCursor()
{
	if (dummyCursor != NULL) {
		int x_offset = 0;
		int y_offset = 0;

		double indentRatio = app()->settings->value("Editor/IndentRatio", 2.0).toDouble();
		QFontMetricsF fm(font());

		double baseWidth = indentRatio * fm.width("w");
		double leading = fm.lineSpacing();
		Q_UNUSED(leading);

		if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
			x_offset += i_lineNumberingMarginWidth;
		}

		QTextCursor cur = QTextCursor(document()->begin());
		cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, dummyCursor->line);
		QRect curRect = cursorRect(cur);


		//x_offset = y_offset = 0;
		int x1, x2, y1, y2;
		x1 = x_offset + (int)(baseWidth * dummyCursor->indent) + 2;
		y1 = y_offset + curRect.top();
		x2 = x1;
		y2 = y_offset + curRect.bottom();

		QRect cursorRect(x1, y1, 2, y2 - y1);
		update(cursorRect);
	}
}



void KumirEdit::paintCursor(QPainter *painter, const QRect &region)
{
	if (m_errorLine != -1) {
		return;
	}
	if (dummyCursor->state == DummyCursor::Off || isReadOnly() || !hasFocus() || !dummyCursor->isVisible()) {
		return;
	}

	int x_offset = 0;
	int y_offset = 0;

	double indentRatio = app()->settings->value("Editor/IndentRatio", 2.0).toDouble();
	QFontMetricsF fm(font());

	double baseWidth = indentRatio * fm.width("w");
	double leading = fm.lineSpacing();
	Q_UNUSED(leading);

	if (app()->settings->value("Appearance/ShowLineNumbers", true).toBool()) {
		x_offset += i_lineNumberingMarginWidth;
	}

	QTextCursor cur = QTextCursor(document()->begin());
	cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, dummyCursor->line);
	QRect curRect = cursorRect(cur);


	//x_offset = y_offset = 0;
	int x1, x2, y1, y2;
	x1 = x_offset + (int)(baseWidth * dummyCursor->indent) + 2;
	y1 = y_offset + curRect.top();
	x2 = x1;
	y2 = y_offset + curRect.bottom();

	QRect cursorRect(x1, y1, 2, y2 - y1);

	if (cursorRect.intersects(region)) {
		painter->setPen(QColor("black"));
		painter->drawLine(x1, y1, x2, y2);
	}
}

bool KumirEdit::isLineHidden(int lineNo) const
{
	if (lineNo < 0 || lineNo >= document()->blockCount()) {
		return true;
	} else {
		QTextCursor c(document());
		c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNo);
		QTextBlock block = c.block();
		int state = block.userState();
		return state == HIDDEN;
	}
}
