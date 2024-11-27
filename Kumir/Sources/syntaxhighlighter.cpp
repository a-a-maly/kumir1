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

#include "syntaxhighlighter.h"
#include "kumiredit.h"
#include "application.h"
#include "tools.h"
#include <QSettings>

#define FC_PLAIN char(0x00)
#define FC_STRING char(0x01)
#define FC_VALUE char(0x02)
#define FC_KEYWORD char(0x03)
#define FC_TYPE char(0x04)
#define FC_ALGNAME char(0x05)
#define FC_COMMENT char(0x06)
#define FC_DOCSTRING char(0x07)
#define FC_HYPERLINK char(0x08)
#define FC_HYPERLINK_2 char(0x09)
#define FC_MODULENAME char(0x0A)

#define FC_ERROR_FLAG char(0x10)
#define FC_LATIN_FLAG char(0x20)

SyntaxHighlighter::SyntaxHighlighter(KumirEdit *te)
	: QSyntaxHighlighter((QTextEdit *)te)
{
	ce = te;

	rxInteger = QRegExp("\\b[0-9]+\\b");
	rxIntegerHex = QRegExp("\\b0x[0-9a-z]+\\b");
	rxDouble = QRegExp("\\b([0-9]+\\.)|(\\.[0-9]+)|([0-9]+\\.[0-9]+)\\b");
	rxDoubleExp = QRegExp(QString::fromUtf8("\\b(([0-9]+)|([0-9]+\\.)|(\\.[0-9]+)|([0-9]+\\.[0-9]+))[eEеЕ](([+-]?[0-9]+)|([0-9]+\\.[0-9]*)|([0-9]*\\.[0-9]+))\\b"));


	rxNames = QRegExp("");

	rxComment = QRegExp("\\|.*");
	rxDoc = QRegExp("#.*");

	nameTableModifed = true;

	updateAppearance();

	quotes = QRegExp(QString::fromUtf8("\"|'"));

}

bool SyntaxHighlighter::isAlwaysBold() const
{
	return app()->settings->value("Appearance/FontBold", false).toBool();
}

void SyntaxHighlighter::updateAppearance()
{
	Plain.setForeground(Qt::black);
	Plain.setFontWeight(QFont::Normal);
	Plain.setFontUnderline(false);
	Plain.setFontItalic(false);

	Hyper.setForeground(QColor(app()->settings->value("SyntaxHighlighter/QuotedValue/Color", "#0095FF").toString()));
	if (app()->settings->value("SyntaxHighlighter/QuotedValue/Bold", true).toBool()) {
		Hyper.setFontWeight(QFont::Bold);
	} else {
		Hyper.setFontWeight(QFont::Normal);
	}
	Hyper.setFontItalic(app()->settings->value("SyntaxHighlighter/QuotedValue/Italic", false).toBool() && !isOverloadLatinFont());
	Hyper.setFontUnderline(true);

	Keyword.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Keyword/Color", "#000000").toString()));;
	if (app()->settings->value("SyntaxHighlighter/Keyword/Bold", true).toBool()) {
		Keyword.setFontWeight(QFont::Bold);
	} else {
		Keyword.setFontWeight(QFont::Normal);
	}
	Keyword.setFontItalic(app()->settings->value("SyntaxHighlighter/Keyword/Italic", false).toBool() && !isOverloadLatinFont());
	Keyword.setFontUnderline(false);

	Type.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Type/Color", "#C05800").toString()));
	if (app()->settings->value("SyntaxHighlighter/Type/Bold", true).toBool()) {
		Type.setFontWeight(QFont::Bold);
	} else {
		Type.setFontWeight(QFont::Normal);
	}
	Type.setFontUnderline(false);
	Type.setFontItalic(app()->settings->value("SyntaxHighlighter/Type/Italic", false).toBool() && !isOverloadLatinFont());


	Name.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Algorhitm/Color", "#0000C8").toString()));
	if (app()->settings->value("SyntaxHighlighter/Algorhitm/Bold", true).toBool()) {
		Name.setFontWeight(QFont::Bold);
	} else {
		Name.setFontWeight(QFont::Normal);
	}
	Name.setFontUnderline(false);
	Name.setFontItalic(app()->settings->value("SyntaxHighlighter/Algorhitm/Italic", false).toBool() && !isOverloadLatinFont());

	Isp.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Module/Color", "#00AA00").toString()));
	if (app()->settings->value("SyntaxHighlighter/Module/Bold", true).toBool()) {
		Isp.setFontWeight(QFont::Bold);
	} else {
		Isp.setFontWeight(QFont::Normal);
	}
	Isp.setFontUnderline(false);
	Isp.setFontItalic(app()->settings->value("SyntaxHighlighter/Module/Italic", false).toBool() && !isOverloadLatinFont());

	Value.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Value/Color", "#0095FF").toString()));
	if (app()->settings->value("SyntaxHighlighter/Value/Bold", true).toBool()) {
		Value.setFontWeight(QFont::Bold);
	} else {
		Value.setFontWeight(QFont::Normal);
	}
	Value.setFontUnderline(false);
	Value.setFontItalic(app()->settings->value("SyntaxHighlighter/Value/Italic", false).toBool() && !isOverloadLatinFont());

	String.setForeground(QColor(app()->settings->value("SyntaxHighlighter/QuotedValue/Color", "#0095FF").toString()));
	if (app()->settings->value("SyntaxHighlighter/QuotedValue/Bold", true).toBool()) {
		String.setFontWeight(QFont::Bold);
	} else {
		String.setFontWeight(QFont::Normal);
	}
	String.setFontUnderline(false);
	String.setFontItalic(app()->settings->value("SyntaxHighlighter/QuotedValue/Italic", false).toBool() && !isOverloadLatinFont());

	Comment.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Comment/Color", "#888888").toString()));
	if (app()->settings->value("SyntaxHighlighter/Comment/Bold", true).toBool()) {
		Comment.setFontWeight(QFont::Bold);
	} else {
		Comment.setFontWeight(QFont::Normal);
	}
	Comment.setFontUnderline(false);
	Comment.setFontItalic(app()->settings->value("SyntaxHighlighter/Comment/Italic", true).toBool() && !isOverloadLatinFont());

	Hyper1.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Comment/Color", "#888888").toString()));
	if (app()->settings->value("SyntaxHighlighter/Comment/Bold", true).toBool()) {
		Hyper1.setFontWeight(QFont::Bold);
	} else {
		Hyper1.setFontWeight(QFont::Normal);
	}
	Hyper1.setFontUnderline(true);
	Hyper1.setFontItalic(app()->settings->value("SyntaxHighlighter/Comment/Italic", true).toBool() && !isOverloadLatinFont());

	Doc.setForeground(QColor(app()->settings->value("SyntaxHighlighter/Documentation/Color", "#A3ACFF").toString()));
	if (app()->settings->value("SyntaxHighlighter/Documentation/Bold", true).toBool()) {
		Doc.setFontWeight(QFont::Bold);
	} else {
		Doc.setFontWeight(QFont::Normal);
	}
	Doc.setFontUnderline(false);
	Doc.setFontItalic(app()->settings->value("SyntaxHighlighter/Documentation/Italic", true).toBool() && !isOverloadLatinFont());

}

QString SyntaxHighlighter::keywordCSS() const
{
	QString result;
	if (Keyword.fontItalic()) {
		result += "font-style: italic;";
	} else {
		result += "font-style: none;";
	}
	if (Keyword.fontWeight() == QFont::Bold) {
		result += "font-weight: bold;";
	} else {
		result += "font-weight: normal;";
	}
	result += "color: " + Keyword.foreground().color().name() + ";";
	return result;
}

QString SyntaxHighlighter::typeCSS() const
{
	QString result;
	if (Type.fontItalic()) {
		result += "font-style: italic;";
	} else {
		result += "font-style: none;";
	}
	if (Type.fontWeight() == QFont::Bold) {
		result += "font-weight: bold;";
	} else {
		result += "font-weight: normal;";
	}
	result += "color: " + Type.foreground().color().name() + ";";
	return result;
}

QString SyntaxHighlighter::valueCSS() const
{
	QString result;
	if (Value.fontItalic()) {
		result += "font-style: italic;";
	} else {
		result += "font-style: none;";
	}
	if (Value.fontWeight() == QFont::Bold) {
		result += "font-weight: bold;";
	} else {
		result += "font-weight: normal;";
	}
	result += "color: " + Value.foreground().color().name() + ";";
	return result;
}

bool SyntaxHighlighter::isOverloadLatinFont() const
{
	return app()->settings->value(
			"SyntaxHighlighter/OverloadLatinFont",
			true).toBool();
}


void SyntaxHighlighter::clearNames()
{
	prevNames = names;
	names.clear();
}

void SyntaxHighlighter::flushNames()
{
	nameTableModifed = false;
	if (prevNames.count() != names.count()) {
		nameTableModifed = true;
	} else {
		for (int i = 0; i < names.count(); i++) {
			if (names[i] != prevNames[i]) {
				nameTableModifed = true;
				break;
			}
		}
	}
	if (nameTableModifed) {
		QString pattern;
		for (int i = 0; i < names.count() - 1; i++) {
			pattern += "\\b(" + names[i] + ")\\b|";
		}
		if (names.count() > 0) {
			pattern += "\\b(" + names[names.count() - 1] + ")\\b";
		}
		rxNames.setPattern(pattern);
	}

}

void SyntaxHighlighter::addName(QString name)
{
	if (name.trimmed().isEmpty()) {
		return;
	}
	name = name.simplified();
	name.replace(" ", "\\s+");
	names.append(name);
}

int SyntaxHighlighter::FindLit(QString IniString, int startPos, int endPos, int *LitBeg, int *LitEnd)
{
	QRegExp usedQuote;

	int begin, end;

	begin = quotes.indexIn(IniString, startPos);
	if ((begin == -1) || (begin >= endPos)) {
		*LitBeg = -1;
		*LitEnd = -1;
		return 0;
	}
	if (begin >= (endPos - 1)) {
		*LitBeg = begin;
		*LitEnd = -1;
		return 1;
	}

	usedQuote = QRegExp(QString(IniString[begin]));
	end = usedQuote.indexIn(IniString, begin + 1);

	if ((end == -1) || (end > endPos)) {
		*LitBeg = begin;
		*LitEnd = end;
		return 1;
	}

	*LitBeg = begin;
	*LitEnd = end;
	return 0;
}


QByteArray SyntaxHighlighter::generateFormatString(const QString &text, LineProp *lp)
{
	QRegExp *rxKeyWords = KumTools::instance()->rxKeyWords;
	QRegExp *rxTypes = KumTools::instance()->rxTypes;
	QRegExp *rxConst = KumTools::instance()->rxConst;

	QString txt;
	if (lp != NULL) {
		txt = text.mid(lp->indentCount * 2);
	} else {
		txt = text;
	}
	QMutexLocker l(&namesLock);

	int pos = 0;
	int len = 0;

	QByteArray format;
	// заполнение форматной строки пустыми символами
	format.fill(FC_PLAIN, txt.length());

	//                    Поиск строковых выражений

	bool isQuoted = false;
	bool isDouble = false;
	for (int i = 0; i < txt.length(); i++) {
		// проверка на литеральность текущего символа
		if (!isQuoted) {
			// если до этого не было незакрытых кавычек
			if (txt[i] == '\'') {
				// если встретилась одинарная кавычка
				isQuoted = true;
				isDouble = false;
				// поставить символ в форматной строке
				if (isQuoted) {
					format[i] = FC_STRING;
				}
			}
			if (txt[i] == '"') {
				// если встретилась двойная кавычка
				isQuoted = true;
				isDouble = true;
				// поставить символ в форматной строке
				if (isQuoted) {
					format[i] = FC_STRING;
				}
			}
		} else {
			// если были незакрытые кавычки перед текущим символом

			// поставить символ в форматной строке
			if (isQuoted) {
				format[i] = FC_STRING;
			}

			if (txt[i] == '\'') {
				// если встетилась одинарная кавычка
				// проверка типа предыдущей незакрытой кавычки
				if (!isDouble) {
					// если предыдущая кавычка была одинарной, то закрыть её
					isQuoted = false;
				}
			}
			if (txt[i] == '"') {
				// если встетилась одинарная кавычка
				// проверка типа предыдущей незакрытой кавычки
				if (isDouble) {
					// если предыдущая кавычка была двойной, то закрыть её
					isQuoted = false;
				}
			}
		} // конец проверки на закрытость кавычек
	} // конец цикла по символам исходного текста



	//                   Поиск зарезервированных слов

	if ((!rxKeyWords->isEmpty()) && (rxKeyWords->isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxKeyWords->indexIn(txt, pos);
			if (pos != -1) {
				len = rxKeyWords->matchedLength();
				for (int i = pos; i < pos + len; i++) {
					if (format[i] == FC_PLAIN) {
						format[i] = FC_KEYWORD;
					}
				}
				pos += len;
			}
		}
	}


	//                   Поиск базовых типов

	if ((!rxTypes->isEmpty()) && (rxTypes->isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxTypes->indexIn(txt, pos);
			if (pos != -1) {
				len = rxTypes->matchedLength();
				for (int i = pos; i < pos + len; i++) {
					if (format[i] == FC_PLAIN) {
						format[i] = FC_TYPE;
					}
				}
				pos += len;
			}
		}
	}


	//                   Поиск имен
	if ((!rxNames.isEmpty()) && (rxNames.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxNames.indexIn(txt, pos);

			if (pos != -1) {
				len = rxNames.matchedLength();
				// обход очень странного бага QRegExp...
				if (len <= 0) {
					break;
				}
				// исключение -- "цел" и "лит" являются также именами функций
				QString fragment = txt.mid(pos, len);
				if ((fragment == QString::fromUtf8("цел")) || (fragment == QString::fromUtf8("лит"))) {
					int sk_pos = txt.indexOf("(", pos);
					if (sk_pos != -1) {
						// если найдена скобка после "цел" или "лит", проверяем,
						// есть ли что-нибудь между ними
						QString between = txt.mid(pos + 3, sk_pos - pos - 3);
						if (between.trimmed().length() == 0) {
							// если ничего нет кроме пробелов,
							// принудительно считаем "цел" или "лит" именем функции
							for (int i = pos; i < pos + len; i++)
								if (format[i] != FC_STRING) {
									format[i] = FC_ALGNAME;
								}
						}
					}
					// смотрим, что перед "цел" или "лит"
					QString before = txt.mid(0, pos);

					bool isFunction = true;

					if (before.startsWith(QString::fromUtf8("алг "))) {
						isFunction = false;
					}

					if (before.startsWith(QString::fromUtf8("нач "))) {
						isFunction = false;
					}

					if (before.startsWith(QString::fromUtf8("арг "))) {
						isFunction = false;
					}

					if (before.startsWith(QString::fromUtf8("аргрез "))) {
						isFunction = false;
					}

					if (before.startsWith(QString::fromUtf8("рез "))) {
						isFunction = false;
					}

					if (before.length() == 0) {
						isFunction = false;
					}

					if (before.trimmed().endsWith(";") || before.trimmed().endsWith(",")) {
						isFunction = false;
					}

					if (isFunction) {
						for (int i = pos; i < pos + len; i++)
							if (format[i] != FC_STRING) {
								format[i] = FC_ALGNAME;
							}
					}

				} // конец проверок на функцию "цел"
				else {
					for (int i = pos; i < pos + len; i++) {
						if (format[i] == FC_PLAIN) {
							format[i] = FC_ALGNAME;
						}
					}
				}
				pos += len;
			}
		}
	}


	//                   Поиск значений
	//           а) числовые значения
	if ((!rxDoubleExp.isEmpty()) && (rxDoubleExp.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxDoubleExp.indexIn(txt, pos);
			if (pos != -1) {
				bool skip = format[pos] == FC_VALUE;
				len = rxDoubleExp.matchedLength();
				if (pos > 0 && (txt[pos - 1] == '.' || txt[pos - 1].isLetterOrNumber())) {
					skip = true;
				}
				if (pos + len < txt.length() && (txt[pos + len] == '.' || txt[pos + len].isLetterOrNumber())) {
					skip = true;
				}
				if (!skip) {
					for (int i = pos; i < pos + len; i++) {
						if (format[i] == FC_PLAIN) {
							format[i] = FC_VALUE;
						}
					}
				}
				pos += len;
			}
		}
	}

	if ((!rxDouble.isEmpty()) && (rxDouble.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxDouble.indexIn(txt, pos);
			if (pos != -1) {
				bool skip = format[pos] == FC_VALUE;
				len = rxDouble.matchedLength();
				if (pos > 0 && (txt[pos - 1] == '.' || txt[pos - 1].isLetterOrNumber())) {
					skip = true;
				}
				if (pos + len < txt.length() && (txt[pos + len] == '.' || txt[pos + len].isLetterOrNumber())) {
					skip = true;
				}
				if (!skip) {
					for (int i = pos; i < pos + len; i++) {
						if (format[i] == FC_PLAIN) {
							format[i] = FC_VALUE;
						}
					}
				}
				pos += len;
			}
		}
	}

	if ((!rxIntegerHex.isEmpty()) && (rxIntegerHex.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxIntegerHex.indexIn(txt, pos);
			if (pos != -1) {
				bool skip = format[pos] == FC_VALUE;
				len = rxIntegerHex.matchedLength();
				if (pos > 0 && (txt[pos - 1] == '.' || txt[pos - 1].isLetterOrNumber())) {
					skip = true;
				}
				if (pos + len < txt.length() && (txt[pos + len] == '.' || txt[pos + len].isLetterOrNumber())) {
					skip = true;
				}
				if (!skip) {
					for (int i = pos; i < pos + len; i++) {
						if (format[i] == FC_PLAIN) {
							format[i] = FC_VALUE;
						}
					}
				}
				pos += len;
			}
		}
	}

	if ((!rxInteger.isEmpty()) && (rxInteger.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxInteger.indexIn(txt, pos);
			if (pos != -1) {
				bool skip = format[pos] == FC_VALUE;
				len = rxInteger.matchedLength();
				if (pos > 0 && (txt[pos - 1] == '.' || txt[pos - 1].isLetterOrNumber())) {
					skip = true;
				}
				if (pos + len < txt.length() && (txt[pos + len] == '.' || txt[pos + len].isLetterOrNumber())) {
					skip = true;
				}
				if (!skip) {
					for (int i = pos; i < pos + len; i++) {
						if (format[i] == FC_PLAIN) {
							format[i] = FC_VALUE;
						}
					}
				}
				pos += len;
			}
		}
	}

	//          б) предопределённые константы
	if ((!rxConst->isEmpty()) && (rxConst->isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxConst->indexIn(txt, pos);
			if (pos != -1) {
				len = rxConst->matchedLength();
				for (int i = pos; i < pos + len; i++) {
					if (format[i] == FC_PLAIN) {
						format[i] = FC_VALUE;
					}
				}
				pos += len;
			}
		}
	}

	//                  Исполнители



	QString ispKwd = *(KumTools::instance()->ispKwd);
	QString useKwd = *(KumTools::instance()->useKwd);
	QString t = txt.trimmed();
	if (t.startsWith(ispKwd + " ") || t.startsWith(useKwd + " ")) {
		int pos = txt.indexOf(ispKwd + " ");
		if (pos != -1) {
			int start = pos + ispKwd.length() + 1;
			int end = txt.length();
			for (int i = start; i < end; i++) {
				format[i] = FC_MODULENAME;
			}
		} else {
			pos = txt.indexOf(useKwd + " ");
			if (pos != -1) {
				int start = pos + useKwd.length() + 1;
				int end = txt.length();
				for (int i = start; i < end; i++) {
					format[i] = FC_MODULENAME;
				}
			}
		}
	}


	//                  Обработка комментариев и строк документации



	if ((!rxDoc.isEmpty()) && (rxDoc.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxDoc.indexIn(txt, pos);
			if (pos != -1) {
				if ((format[pos] == FC_STRING)) {
					pos++;
				} else {
					len = rxDoc.matchedLength();
					for (int i = pos; i < pos + len; i++) {
						format[i] = FC_DOCSTRING;
					}
					pos += len;
				}
			}
		}
	}
	if ((!rxComment.isEmpty()) && (rxComment.isValid())) {
		pos = 0;
		while (pos >= 0) {
			pos = rxComment.indexIn(txt, pos);
			if (pos != -1) {
				if ((format[pos] == FC_STRING) || (format[pos] == FC_DOCSTRING)) {
					pos++;
				} else {
					len = rxComment.matchedLength();
					for (int i = pos; i < pos + len; i++) {
						format[i] = FC_COMMENT;
					}
					pos += len;
				}
			}
		}
	}



	//                  Выделение ошибок
	if (lp != NULL && !lp->lineChanged) {
		// Выделяем ошибку нормализации только когда она учтена компилятором
		if (lp->normalize_error > 0 && lp->error_count > 0) {
			int start = qMax(lp->normalizeErrorPosition.x(), 0);
			int end = qMin(lp->normalizeErrorPosition.y(), txt.length() - 1);
			for (int i = start; i <= end; i++) {
				if (txt[i] != ' ' && format[i] != FC_COMMENT) {
					format[i] = format[i] | FC_ERROR_FLAG;
				}
			}
		}
		for (int i = 0; i < lp->errorMask.count(); i++) {
			if (lp->errorMask[i]) {
				format[i] = format[i] | FC_ERROR_FLAG;
			}
		}
	}

	//                  Выделение гиперссылок
	if ((lp != NULL) && (lp->error == 0)) {
		foreach (HyperLink h, lp->hyperlinks) {
			for (int i = h.start; i < h.end; i++) {
				if (format[i] == FC_COMMENT) {
					format[i] = FC_HYPERLINK;
				} else {
					format[i] = FC_HYPERLINK_2;
				}
			}
		}
	}

	//                  Выделение символов латиницы

	for (int i = 0; i < txt.length(); i++) {
		if (txt[i].isLetter() && txt[i].toLatin1() != 0) {
			format[i] = format[i] | FC_LATIN_FLAG;
		}
	}
	return format;
}

QList<SyntaxHighlighter::Block> SyntaxHighlighter::splitFormatString(const QByteArray &in)
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

QString SyntaxHighlighter::highlightBlockAsHtml(const QString &text, LineProp *lp)
{
	QString result = "";
	QByteArray format = generateFormatString(text, lp);
	QList<Block> formats = splitFormatString(format);
	foreach (const Block &b, formats) {
		QTextCharFormat f;
		QString plainText = text.mid(b.pos, b.len);
		char baseval = b.val & 0x0F;
		switch (baseval) {
		case FC_STRING:
			f = String;
			break;
		case FC_VALUE:
			f = Value;
			break;
		case FC_KEYWORD:
			f = Keyword;
			break;
		case FC_TYPE:
			f = Type;
			break;
		case FC_ALGNAME:
			f = Name;
			break;
		case FC_COMMENT:
			f = Comment;
			break;
		case FC_DOCSTRING:
			f = Doc;
			break;
		case FC_HYPERLINK:
			f = Hyper;
			break;
		case FC_HYPERLINK_2:
			f = Hyper1;
			break;
		case FC_MODULENAME:
			f = Isp;
			break;
		default:
			f = Plain;
			break;
		}
		if (b.val & FC_ERROR_FLAG) {
			f.setUnderlineColor(Qt::red);
			f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
		}
		if (isOverloadLatinFont() && b.val & FC_LATIN_FLAG) {
			f.setFontItalic(true);
		}
		if (isAlwaysBold()) {
			f.setFontWeight(QFont::Bold);
		}
		QString s_color = f.foreground().color().name();
		QString s_font_weight = f.fontWeight() == QFont::Bold ? "bold" : "normal";
		QString s_font_style = f.fontItalic() ? "italic" : "normal";
		QString s_text_decoration = "none";
		if (f.underlineStyle() == QTextCharFormat::SpellCheckUnderline) {
			s_color = "red";
			s_text_decoration = "underline";
		}
		QString style = QString("style=\"color: %1; font-weight: %2; font-style: %3; text-decoration: %4\"")
			.arg(s_color)
			.arg(s_font_weight)
			.arg(s_font_style)
			.arg(s_text_decoration);
		result += "<span " + style + ">" + plainText + "</span>";
	}

	return result;
}


void SyntaxHighlighter::highlightBlock(const QString &text)
{
	LineProp *lp = static_cast<LineProp *>(currentBlockUserData());
	QByteArray format = generateFormatString(text, lp);
	QList<Block> formats = splitFormatString(format);
	foreach (const Block &b, formats) {
		QTextCharFormat f;
		char baseval = b.val & 0x0F;
		switch (baseval) {
		case FC_STRING:
			f = String;
			break;
		case FC_VALUE:
			f = Value;
			break;
		case FC_KEYWORD:
			f = Keyword;
			break;
		case FC_TYPE:
			f = Type;
			break;
		case FC_ALGNAME:
			f = Name;
			break;
		case FC_COMMENT:
			f = Comment;
			break;
		case FC_DOCSTRING:
			f = Doc;
			break;
		case FC_HYPERLINK:
			f = Hyper;
			break;
		case FC_HYPERLINK_2:
			f = Hyper1;
			break;
		case FC_MODULENAME:
			f = Isp;
			break;
		default:
			f = Plain;
			break;
		}
		if (b.val & FC_ERROR_FLAG) {
			f.setUnderlineColor(Qt::red);
			f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
		}
		if (isOverloadLatinFont() && b.val & FC_LATIN_FLAG) {
			f.setFontItalic(true);
		}
		if (isAlwaysBold()) {
			f.setFontWeight(QFont::Bold);
		}
		setFormat(b.pos, b.len, f);
	}
}

void SyntaxHighlighter::lock()
{
	namesLock.lock();
}

void SyntaxHighlighter::unlock()
{
	namesLock.unlock();
}
