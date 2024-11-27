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
#include "tools.h"
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QValidator>

class KumIOArea;
class QAction;

class InputHighlighter : public QSyntaxHighlighter
{
	struct Block {
		inline Block(int a, int b, char c)
		{
			pos = a;
			len = b;
			val = c;
		}
		int pos;
		int len;
		char val;
	};
public:
	InputHighlighter(KumIOArea *editor);
protected:
	QByteArray generateFormatString(const QString &text, struct InputUserData *d);
	static QList<Block> splitFormatString(const QByteArray &in);
	void highlightBlock(const QString &text);
	KumIOArea *editor;
	QRegExp rx_latinSymbols;
	QTextCharFormat cf_plain;
	QTextCharFormat cf_input;
};

struct InputUserData
	: public QTextBlockUserData {
	InputUserData()
	{
		highlightable = false;
	}
	bool highlightable;
};

class KumIOArea : public QTextEdit
{
	Q_OBJECT
	friend class InputHighlighter;
public:
	KumIOArea(QWidget *parent = 0);
	QAction *saveAll;
	QAction *saveLast;
	QAction *saveSelected;
	void getLastOutput(QStringList &list);

public slots:
	void input(QString format);
	void reset(const QString &fileName, bool isTestingMode = false);
	void stop(const QString &reason, const QString &color);

	void output(const QString &text, bool isErrorMessage = false);
	void clear();
	void scrollUp();
	void scrollDown();
	void saveAllOutput();
	void saveLastOutput();
	void saveSelectedOutput();
	void scanKey(bool ignoreShiftControlAltMeta);


protected slots:
	void forceRepaint();


protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void tryFinishInput();
	void finishInput();
	void insertFromMimeData(const QMimeData *source);
	virtual void resizeEvent(QResizeEvent *e);
	void updateLastBlockBottomMargin();


	bool validateInput();

	int inputStartPosition;
	bool waitForInput;
	bool waitForKey;
	bool ignoreModifiers;

	QString tmpInput;
	QValidator::State inputReady;

	QValidator *integerValidator;
	QValidator *doubleValidator;
	QValidator *booleanValidator;
	QValidator *characterValidator;
	QValidator *stringValidator;

	QList<int> runSeparators;
	QList<InputTerm> words;
	QList<PeremType> types;

	QStringList formats;
	QTextFrame *inputFrame;
	int inputGarbageStart;
	QSyntaxHighlighter *sh;
	QString runningFilename;

signals:
	void inputFinished(QStringList texts);
	void outputAvailable(bool);
	void needRepaint();
	void sendMessage(const QString &txt, int timeout);
	void scanKeyFinished(uint code, bool shiftPressed, bool controlPressed, bool altPressed, bool metaPressed);
};
