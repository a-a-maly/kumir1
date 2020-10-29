#ifndef LATINHIGHLIGHTER_H
#define LATINHIGHLIGHTER_H

#include <QSyntaxHighlighter>
class QTextEdit;

class LatinHighlighter : public QSyntaxHighlighter
{
public:
	explicit LatinHighlighter(QTextEdit *editor);
protected:
	void highlightBlock(const QString &text);

	QRegExp rxLatinSymbols;
};

#endif // LATINHIGHLIGHTER_H
