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
#ifndef LINEPROP_H
#define LINEPROP_H

#include "int_proga.h"
#include <QTextBlockUserData>

enum BlockState { SIMPLE = -1, HIDDEN = 0, PROTECTED = 1 };

struct HyperLink {
	HyperLink(int start, int end, const QString &link)
	{
		this->start = start;
		this->end = end;
		this->link = link;
	}
	HyperLink()
	{
		start = -1;
		end = -1;
		link = "";
	}
	int start;
	int end;
	QString link;
};

/**
 * Структура данных нагрузки
 */
struct LineProp : QTextBlockUserData {
	/**
	 *  Конструктор. Инициализирует пустую нагрузку
	 */
	LineProp()
	{
		str_type = Kumir::Empty;

//      editable = true;
//      visible = true;
//      allow_insert_after = true;
//      allow_insert_before = true;
//      hiddenText = "";

		error = 0;
		error_count = 0;
		normalize_error = 0;
//      isBreak = false;

		lineChanged = true;
		indentRang = 0;
		indentRangNext = 0;
		indentCount = 0;
	}
	/** Тип строки */
	Kumir::InstructionType str_type;
	/** Отступ строки */
//  int indent_count;
	/** Отступ следующей строки. Используется для вставки новой строки без перекомпиляции  */
//  int indent_next;
	/** Флаг редактируемости */
//  bool editable;
	/** Флаг видимости */
//  bool visible;
	/** Флаг возможности вставлять строку после */
//  bool allow_insert_after;
	/** Флаг возможности вставлять строку перед */
//  bool allow_insert_before;
	/** Скрытый текст */
//  QString hiddenText;

	int error;
	int error_count;

//  bool isBreak;
	int indentRang;
	int indentRangNext;

	int indentCount;
	int normalize_error;

	QList<HyperLink> hyperlinks;
	QList<ProgaText> normalizedLines;
//  QList<QPoint> errorPositions;
	QVector<bool> errorMask;
	QPoint normalizeErrorPosition;

	bool lineChanged;
};

#endif
