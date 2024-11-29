#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#define SCROLL_STEP 10
#define RESP_PANEL 50
#define LOGGER_BUTTONS 140
#define TEXT_STEP 14

#include <QWidget>
#include <QLabel>
class QLabel;
class QFrame;

class  loggerButton : public QWidget
{
	Q_OBJECT
public:
	loggerButton(QWidget* parent = 0);
	~loggerButton() {};
	void upArrowType(bool b)
	{
		isUpArrow = b;
	}

signals:
	void pressed();

protected:
	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

private:
	int posX, posY;
	bool isUpArrow;
	QImage buttonImageUp, buttonImageDown;
	bool downFlag;
	QWidget *Parent;
	QVector<QLine> upArrow, downArrow;
};


class logLine
{
public:
	logLine(
        QString KumCommand, QString LogCommand,
		QString React, QFrame* frame, QFrame* respFrame, uint pos
    ) {
		kumCommand = KumCommand;
		logCommand = LogCommand;
		react = React;
		textLabel = new QLabel(frame);

		textLabel->setText(logCommand);
		textLabel->move(4, pos);
		textLabel->resize(120, 20);
		textLabel->show();
		textLabel->setStyleSheet("background-color:rgba(0,0,0,0);");
		respLabel = new QLabel(respFrame);
		respLabel->setText(React);
		respLabel->move(4, pos);
		respLabel->resize(RESP_PANEL, 20);
		respLabel->show();
	}

	void moveUp()
	{
		textLabel->move(textLabel->x(), textLabel->y() - SCROLL_STEP);
		respLabel->move(respLabel->x(), respLabel->y() - SCROLL_STEP);
	}

	void moveDown()
	{
		textLabel->move(textLabel->x(), textLabel->y() + SCROLL_STEP);
		respLabel->move(respLabel->x(), respLabel->y() + SCROLL_STEP);
	}

	int pos()
	{
		return textLabel->y();
	}

	void removeLabels()
	{
		if (textLabel) {
			delete textLabel;
		}
		if (respLabel) {
			delete respLabel;
		}
	}

	QString KumCommand()
	{
		return kumCommand;
	}

private:
	QString kumCommand;
	QString logCommand;
	QString react;
	QLabel *textLabel;
	QLabel *respLabel;
};

class pultLogger : public QWidget
{
	Q_OBJECT
public:
	explicit pultLogger(QWidget* parent = 0);
	~pultLogger();
	void setSizes(uint w, uint h);
	void Move(uint x, uint y);

	void Show()
	{
	}

	void appendText(QString kumCommand, QString text, QString replay);

	QString log()
	{
		QString toret;
		for (int i = 0; i < lines.count(); i++) {
			toret += lines[i].KumCommand();
		}
		return toret;
	}

public slots:
	void upBtnPressed();
	void downBtnPressed();
	void ClearLog();
	void CopyLog();

private:
	QFrame *mainFrame;
	QFrame *dummyFrame;
	QFrame *respFrame;
	int W, H;
	int pos;
	QList<logLine> lines;
	int buttonSize;

};

class Logger : public QWidget
{
	Q_OBJECT
public:
	explicit Logger(QWidget *parent = 0);

signals:

protected:
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);

};

#endif // TESTWIDGET_H
