#ifndef PAINTERTOOLS_H
#define PAINTERTOOLS_H

#include <QString>
#include <QColor>

class PainterTools
{
public:
	static QColor parceColor(const QString &css);
	static QString CSS_RGB_toString(const QString &cssRgb);
};

#endif // PAINTERTOOLS_H
