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
#ifndef KUMUNDOSTACK_H
#define KUMUNDOSTACK_H

//#include "lineprop.h"
#include <QStack>

/**
    @author Victor Yacovlev <V.Yacovlev@gmail.com>
*/

enum EditorCommand {
	Undo,
	Redo,
	MacroCMD,
	Charect,
	Enter,
	Delete,
	Cut,
	Backspace,
	Insert,
	Comment,
	Uncomment,
	SetText,
	InsertLine,
	ChangeLineText,
	FastInsertText
};

struct KumUndoElement {
	QList<LineProp> savedLineProps;
	QString savedText;
	EditorCommand command;
	int fromLine;
	int absoluteCursorPosition;
	bool cursorHasSelection;
	int cursorSelectionStart;
	int cursorSelectionEnd;
	QList<int> savedBlockStates;
};

struct KumUndoStack {
	QStack<KumUndoElement> stack; // стек
	EditorCommand lastCommand; // последняя команда или None
	QChar lastCommandCharect; // последний добавленный/удалённый символ (для Charect, Delete или Backspace), или \0
	bool inMacro; // выполняется макрос
};

struct KumRedoStack {
	QStack<KumUndoElement> stack;
};

#endif
