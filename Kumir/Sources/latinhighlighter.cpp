#include "latinhighlighter.h"
#include "application.h"
#include <QTextEdit>

LatinHighlighter::LatinHighlighter(QTextEdit *editor) :
	QSyntaxHighlighter(editor)
{
	rxLatinSymbols = QRegExp("[a-zA-Z]+");
}

void LatinHighlighter::highlightBlock(const QString &text)
{
	if (app()->settings->value(
			"SyntaxHighlighter/OverloadLatinFont",
			true).toBool()) {
		QTextCharFormat f;
		f.setFontItalic(true);
		int pos = 0;
		int len = 0;
		while (pos >= 0) {
			pos = rxLatinSymbols.indexIn(text, pos);
			if (pos != -1) {
				len = rxLatinSymbols.matchedLength();
				setFormat(pos, len, f);
				pos += len;
			}
		}
	}
}
