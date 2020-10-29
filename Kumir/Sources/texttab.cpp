//
// C++ Implementation:
//
// Description:
//
//
// Author: ?????? ??????? <V.Yacovlev@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "texttab.h"
#include "application.h"
#include "textedit.h"
#include "mainwindow.h"
#include <QPrinter>

TextTab::TextTab ( QWidget* parent, Qt::WindowFlags fl ) :
	QWidget ( parent, fl )
{
	m_editor = new TextEdit(this);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(m_editor);
	setLayout(layout);
	QString editorsFont = app()->settings->value("Appearance/Font","Courier").toString();
	QFont font(editorsFont);
	int fontSize = app()->settings->value("Appearance/FontSize",12).toInt();
	bool boldFont = app()->settings->value("Appearance/FontBold",false).toBool();
	font.setBold(boldFont);
	font.setPointSize(fontSize);
	setFont(font);
	connect ( m_editor, SIGNAL(altLeftPressed()), app()->mainWindow, SLOT(switchPreviousTab()) );
	connect ( m_editor, SIGNAL(altRightPressed()), app()->mainWindow, SLOT(switchNextTab()) );
	connect ( m_editor, SIGNAL(copyAvailable(bool)), this, SLOT(emitCopyAvailable(bool)) );
	connect ( m_editor, SIGNAL(undoAvailable(bool)), this, SLOT(emitUndoAvailable(bool)) );
	connect ( m_editor, SIGNAL(redoAvailable(bool)), this, SLOT(emitRedoAvailable(bool)) );
	module = false;
	m_editor->setFocus();
}

void TextTab::emitCopyAvailable(bool v)
{
	emit copyAvailable(v);
}

void TextTab::emitUndoAvailable(bool v)
{
	emit undoAvailable(v);
}

void TextTab::emitRedoAvailable(bool v)
{
	emit redoAvailable(v);
}

bool TextTab::isCopyAvailable()
{
	return m_editor->textCursor().hasSelection();
}

bool TextTab::isUndoAvailable()
{
	return m_editor->isUndoAvailable();
}

bool TextTab::isRedoAvailable()
{
	return m_editor->isRedoAvailable();
}


TextTab::~TextTab()
{
	
}

QFont TextTab::font()
{
	return m_editor->font();
}

void TextTab::setFont(const QFont & font)
{
	m_editor->setFont(font);
	QTextCharFormat f;
	f.setFont(font);
	QTextCursor c(m_editor->document());
	c.select(QTextCursor::Document);
	c.setCharFormat(f);
}

QString TextTab::editorText(bool withIndentation)
{
	Q_UNUSED(withIndentation);
	return m_editor->toPlainText();
}

QString TextTab::marginText()
{
	return "";
}

void TextTab::setEditorText(const QString & txt)
{
	m_editor->setPlainText(txt);
	m_editor->applyIndentation();
}


bool TextTab::isProgram()
{
	return false;
}

void TextTab::disconnectEditActions()
{
	disconnect(SIGNAL(copyAvailable(bool)));
	disconnect(SIGNAL(undoAvailable(bool)));
	disconnect(SIGNAL(redoAvailable(bool)));
}

void TextTab::editCut()
{
	m_editor->cut();
}

void TextTab::editCopy()
{
	m_editor->copy();
}

void TextTab::editPaste()
{
	m_editor->paste();
}

void TextTab::editUndo()
{
	m_editor->undo();
}

void TextTab::editRedo()
{
	m_editor->redo();
}

TextEdit* TextTab::editor()
{
	return m_editor;
}

void TextTab::paste()
{
	m_editor->paste();
}

void TextTab::print(QPrinter *printer)
{
	QPainter painter;
	painter.begin(printer);
	QFont fnt = editor()->font();
	fnt.setPointSize(10);
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
	painter.drawRect(0,0,30,printer->pageRect().height());
	painter.drawRect(printer->pageRect().width()-30,0,30,printer->pageRect().height());
	painter.drawRect(0,0,printer->pageRect().width(),1.5*lineHeight);
	painter.drawRect(0,printer->pageRect().height()-1.5*lineHeight,printer->pageRect().width(),1.5*lineHeight);
	painter.setPen(Qt::black);
	painter.drawText(QRect(0,0,printer->pageRect().width(),1.5*lineHeight),
									 Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
					getFileName());
	painter.drawText(QRect(0,printer->pageRect().height()-1.5*lineHeight,printer->pageRect().width(),1.5*lineHeight),
									 Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
					tr("Page ")+QString::number(pageNo));
	QTextBlock ed = editor()->document()->begin();
	
	while ( ed.isValid() ) {
		if ( (lineNo+2)*lineHeight >= printer->pageRect().height() ) {
			printer->newPage();
			pageNo++;
			painter.setBrush(QColor("lightgray"));
			painter.setPen(Qt::NoPen);
			painter.drawRect(0,0,30,printer->pageRect().height());
			painter.drawRect(printer->pageRect().width()-30,0,30,printer->pageRect().height());
			painter.drawRect(0,0,printer->pageRect().width(),1.5*lineHeight);
			painter.drawRect(0,printer->pageRect().height()-1.5*lineHeight,printer->pageRect().width(),1.5*lineHeight);
			painter.setPen(Qt::black);
			painter.drawText(QRect(0,0,printer->pageRect().width(),1.5*lineHeight),
											 Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
						getFileName());
			painter.drawText(QRect(0,printer->pageRect().height()-1.5*lineHeight,printer->pageRect().width(),1.5*lineHeight),
											 Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
						tr("Page ")+QString::number(pageNo));
			lineNo = 3;
		}
		painter.setFont(fnt);
		QRect lineNumberRect = QRect(0,(lineNo-1)*lineHeight,30,lineHeight);
		painter.drawText(lineNumberRect,Qt::AlignCenter,QString::number(realLineNo));
		qreal shift = 5.0;
		QRect textRect = QRect(30+shift,(lineNo-1)*lineHeight,printer->pageRect().width()-shift-35.0,lineHeight);
		QString line = ed.text();
		QFontMetrics test(fnt);
		while ( test.width(line) > textRect.width() ) {
			line.chop(1);
		}
		painter.drawText(textRect,Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, line);
		ed = ed.next();
		lineNo++;
		realLineNo++;
	}
	painter.end();
}
