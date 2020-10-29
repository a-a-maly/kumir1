#ifndef PULT_WEBKIT_H
#define PULT_WEBKIT_H

#include <QWebView>

class RobotPultWK: public QWebView
{
	Q_OBJECT;
public:
	explicit RobotPultWK(QWidget *parent);
public slots:
	void setHasLink(bool hasLink);
	inline bool isHasLink()
	{
		return b_hasLink;
	}
	void setCommandResult(const QString &text);
signals:
	void goUp();
	void goDown();
	void goLeft();
	void goRight();

	void hasUpWall();
	void hasDownWall();
	void hasLeftWall();
	void hasRightWall();

	void noUpWall();
	void noDownWall();
	void noLeftWall();
	void noRightWall();

	void Color();

	void Clean();
	void Colored();

	void Rad();
	void Temp();
protected slots:
	void setJavaScriptObjects();
	void evaluateCommand(const QString &text);
	void setWindowSize(int w, int h);
	void copyToClipboard(const QString &data);
protected:
	QString s_lastCommand;
	bool b_hasLink;
};

#endif // PULT_WEBKIT_H
