#include "testwidget.h"
#include <QDebug>
#include <QPainter>
#include <QClipboard>
#include <QApplication>
#include <QMouseEvent>


loggerButton::loggerButton(QWidget* parent): QWidget(parent)
{
	isUpArrow = false;
	posX = 1;
	posY = 1;
	buttonImageUp.load(":/img/butt.png");
	buttonImageDown.load(":/img/buttd.png");
	downFlag = false;
	Parent = parent;
	int mid = buttonImageUp.width() / 2;
	upArrow.append(QLine(mid, 10, mid - 11, 15));
	upArrow.append(QLine(mid, 10, mid + 11, 15));
	downArrow.append(QLine(mid, 15, mid - 11, 10));
	downArrow.append(QLine(mid, 15, mid + 11, 10));
}

void loggerButton::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	if (!downFlag) {
		painter.drawImage(QPoint(posX, posY), buttonImageUp);
		QPen blackPen(QColor(50, 50, 50));
		blackPen.setWidth(3);
		painter.setPen(blackPen);
		if (isUpArrow) {
			painter.drawLines(upArrow);
		} else {
			painter.drawLines(downArrow);
		}
	} else    {
		painter.drawImage(QPoint(posX, posY), buttonImageDown);
		QPen whitePen(QColor(170, 170, 170));
		whitePen.setWidth(3);
		painter.setPen(whitePen);
		if (isUpArrow) {
			painter.drawLines(upArrow);
		} else {
			painter.drawLines(downArrow);
		}
	}
}


void loggerButton::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	qWarning("MousePress");
	emit pressed();
	downFlag = true;
	repaint();
}

void loggerButton::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	downFlag = false;
	repaint();
}


pultLogger::pultLogger(QWidget *parent): QWidget(parent)
{
	qDebug() << "TEST LOGGER TEST LOGGER CONSTRUTOR";
	mainFrame = new QFrame(parent);
	mainFrame->setLineWidth(2);
	mainFrame->setFrameShadow(QFrame::Sunken);
	mainFrame->setFrameShape(QFrame::Panel);
	mainFrame->setPalette(QPalette(QColor(50, 50, 50), QColor(100, 100, 100)));
	mainFrame->setBackgroundRole(QPalette::Window);
	mainFrame->setAutoFillBackground(true);
	mainFrame->show();
	mainFrame->setParent(this);
	dummyFrame = new QFrame(mainFrame);
	dummyFrame->setLineWidth(0);
	dummyFrame->setFrameShadow(QFrame::Sunken);
	dummyFrame->setFrameShape(QFrame::Panel);
	dummyFrame->setPalette(QPalette(QColor(50, 50, 50), QColor(100, 100, 100)));
	dummyFrame->setBackgroundRole(QPalette::Window);
	dummyFrame->setAutoFillBackground(true);
	dummyFrame->move(2, 2);
	dummyFrame->show();
	W = 150;
	H = 160;
	respFrame = new QFrame(mainFrame);
	respFrame->setLineWidth(0);
	respFrame->setFrameShadow(QFrame::Sunken);
	respFrame->setFrameShape(QFrame::Panel);
	respFrame->setPalette(QPalette(QColor(50, 50, 50), QColor(120, 110, 110)));
	respFrame->setBackgroundRole(QPalette::Window);
	respFrame->setAutoFillBackground(true);
	respFrame->move(W - RESP_PANEL, 2);
	respFrame->show();
}

pultLogger::~pultLogger()
{
}

void pultLogger::Move(uint x, uint y)
{
	mainFrame->move(x, y + 26);
	qWarning("WidgetMove");
	move(x, y);
}

void pultLogger::setSizes(uint w, uint h)
{
	mainFrame->resize(w, h);
	dummyFrame->resize(w - RESP_PANEL, h);
	respFrame->resize(RESP_PANEL, h);
	respFrame->move(w - RESP_PANEL, 2);
	qDebug() << "dummy geom" << dummyFrame->geometry() << " w" << w << " h" << h;
	resize(w, h);
	W = w;
	H = h;
	buttonSize = w;
}

void pultLogger::appendText(QString kumCommand, QString text, QString replay)
{
	while (pos > H - H / 50) {
		downBtnPressed();
	}
	lines.append(logLine(kumCommand, text, replay, dummyFrame, respFrame, pos));
	pos = pos + TEXT_STEP;
}


void pultLogger::upBtnPressed()
{

	if (lines.count() == 0) {
		return;
	}
	if (lines.first().pos() > 2) {
		return;
	}

	for (int i = 0; i < lines.count(); i++) {
		lines[i].moveDown();
	}
	pos = pos + SCROLL_STEP;
	qWarning("TEXT MOVE");
}

void pultLogger::downBtnPressed()
{
	if (pos < 14) {
		return;
	}
	for (int i = 0; i < lines.count(); i++) {
		lines[i].moveUp();
	}
	pos = pos - SCROLL_STEP;
}

void pultLogger::ClearLog()
{
	for (int i = 0; i < lines.count(); i++) {
		lines[i].removeLabels();
	}
	lines.clear();
	pos = 4;

}

void pultLogger::CopyLog()
{
	if (lines.count() == 0) {
		return;
	}
	QClipboard * cp = QApplication::clipboard();
	QString text;
	for (int i = 0; i < lines.count(); i++)
		if (!lines[i].KumCommand().isEmpty()) {
			text += lines[i].KumCommand() + "\n";
		}
	cp->setText(text);
}



Logger::Logger(QWidget *parent) : QWidget(parent)
{
}

void Logger::mousePressEvent(QMouseEvent *e)
{
	qDebug() << "Logger mouse press event received";
	e->accept();
}

void Logger::mouseReleaseEvent(QMouseEvent *e)
{
	qDebug() << "Logger mouse release event received";
	e->accept();
}

void Logger::mouseMoveEvent(QMouseEvent *e)
{
	qDebug() << "Logger mouse move event received";
	e->accept();
}
