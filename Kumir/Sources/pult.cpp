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
#include "pult.h"
#include <QPainter>
#include <QDebug>
#include <QClipboard>


linkLight::linkLight(QWidget *parent): QWidget(parent)
{
	posX = 1;
	posY = 1;
	onLine = true;
	text = trUtf8("СВЯЗЬ");
};

void linkLight::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	QRectF rectangle(posX, posY, 10.0, 10.0);

	QPen pen(Qt::green);  // creates a default pen

	QBrush brush(Qt::green);
	if (!onLine) {
		brush.setColor(QColor(20, 60, 20));
	}
	brush.setStyle(Qt::SolidPattern);
	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawEllipse(rectangle);

	brush.setColor(QColor(10, 10, 10));
	painter.setPen(QPen(QColor(10, 10, 10)));
	painter.setBrush(brush);
	QFont font("Arial");
	font.setBold(true);
	painter.setFont(font);
	int cur_pos = 0;
	for (int i = 0; i < text.length(); i++) {
		painter.drawText(posX, posY + TEXT_STEP * i + 26, QString(text[i]));
		cur_pos = posY + TEXT_STEP * i + 26;
	};
	QRectF rectangle2(posX, cur_pos + 7, 10, 10);
	brush.setColor(QColor(30, 0, 0));
	if (!onLine) {
		brush.setColor(QColor(250, 50, 50));
	}
	pen.setColor(Qt::red);

	brush.setStyle(Qt::SolidPattern);
	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawEllipse(rectangle2);


};



MainButton::MainButton(QWidget *parent): QWidget(parent)
{
	direction = UP;
	posX = 1;
	posY = 1;
	buttonImageUp.load(":/icons/71_71grMet.png");
	buttonImageDown.load(":/icons/71_71grMet_d.png");
	downFlag = false;
	Parent = parent;
	int mid = buttonImageUp.width() / 2;
	upArrow.append(QLine(mid, 30, mid - 15, 40));
	upArrow.append(QLine(mid, 30, mid + 15, 40));
	downArrow.append(QLine(mid, 40, mid - 15, 30));
	downArrow.append(QLine(mid, 40, mid + 15, 30));
	leftArrow.append(QLine(30, mid, 40, mid - 15));
	leftArrow.append(QLine(30, mid, 40, mid + 15));
	rightArrow.append(QLine(40, mid, 30, mid - 15));
	rightArrow.append(QLine(40, mid, 30, mid + 15));
	text = "";
	checked = false;
	checkable = false;
	mouseOver = false;
	icon = false;
	resize(71, 71);
};

void MainButton::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	if (!downFlag) {
		painter.drawImage(QPoint(posX, posY), buttonImageUp);
		if (icon) {
			painter.drawImage(10, 10, buttonIcon);
		}
		QPen blackPen(QColor(40, 40, 40));
		blackPen.setWidth(3);
		painter.setPen(blackPen);
		drawAddons(&painter);
		if (mouseOver) {
			QLinearGradient grad(1, 1, 5, 65);
			grad.setColorAt(0.7, QColor(200, 190, 222));
			grad.setColorAt(0.3, QColor(191, 208, 208));

			QBrush solidBrush(grad);
			painter.setBrush(solidBrush);
			painter.setOpacity(0.1);
			painter.drawRect(3, 3, 65, 65);
		};

	} else    {
		painter.drawImage(QPoint(posX, posY), buttonImageDown);
		if (icon) {
			//buttonIcon.invertPixels();
			painter.drawImage(12, 12, buttonIcon);
			//buttonIcon.invertPixels();
		};
		QPen whitePen(QColor(170, 170, 170));
		whitePen.setWidth(3);
		painter.setPen(whitePen);
		drawAddons(&painter);

	};
};

void MainButton::drawAddons(QPainter *painter)
{
	if (direction == UP) {
		painter->drawLines(upArrow);
	}
	if (direction == DOWN) {
		painter->drawLines(downArrow);
	}
	if (direction == LEFT) {
		painter->drawLines(leftArrow);
	}
	if (direction == RIGHT) {
		painter->drawLines(rightArrow);
	}
	if (direction == TEXT) {
		if (!downFlag) {
			painter->setPen(QColor(10, 10, 10));
		}
		QFont font("FreeSans");
		font.setBold(true);
		painter->setFont(font);
		QStringList textLines = text.split("|");
		int start_pos = 42 - 7 * textLines.count();
		for (int i = 0; i < textLines.count(); i++) {
			painter->drawText(7, start_pos + TEXT_STEP * i, textLines[i]);
		}
	};

};
bool MainButton::loadIcon(QString iconFile)
{
	icon = true;
	return buttonIcon.load(iconFile);
};
void MainButton::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	Q_UNUSED(event);
	qDebug("MousePress");
	emit pressed();
	if (checkable) {
		checked = !checked;
	}
	downFlag = true;
	repaint();
};
void MainButton::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	if (checkable) {
		if (!checked) {
			downFlag = false;
		}
	} else {
		downFlag = false;
	}
	if (mouseOver) {
		emit clicked();
	}
	repaint();
};


void MainButton::enterEvent(QEvent *event)
{
	Q_UNUSED(event);
	if (!mouseOver) {
		mouseOver = true;
		repaint();
	};

	qDebug("mouseOnWidget");
};
void MainButton::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	if (mouseOver) {
		mouseOver = false;
		repaint();
		qDebug("mouseOffWidget");
	};
};





loggerButton::loggerButton(QWidget *parent): QWidget(parent)
{
	isUpArrow = false;
	posX = 1;
	posY = 1;
	buttonImageUp.load(":/icons/button_proto.png");
	buttonImageDown.load(":/icons/button_proto_down.png");
	downFlag = false;
	Parent = parent;
	int mid = buttonImageUp.width() / 2;
	upArrow.append(QLine(mid, 10, mid - 11, 15));
	upArrow.append(QLine(mid, 10, mid + 11, 15));
	downArrow.append(QLine(mid, 15, mid - 11, 10));
	downArrow.append(QLine(mid, 15, mid + 11, 10));
};

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
	};
};


void loggerButton::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	qDebug("MousePress");
	emit pressed();
	downFlag = true;
	repaint();
};
void loggerButton::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	downFlag = false;
	repaint();
};





pultLogger::pultLogger(QWidget *parent): QWidget(parent)
{
	mainFrame = new QFrame(parent);
	mainFrame->setLineWidth(2);
	mainFrame->setFrameShadow(QFrame::Sunken);
	mainFrame->setFrameShape(QFrame::Panel);
	mainFrame->setPalette(QPalette(QColor(50, 50, 50), QColor(100, 100, 100)));
	mainFrame->setBackgroundRole(QPalette::Window);
	mainFrame->setAutoFillBackground(true);

	dummyFrame = new QFrame(mainFrame);
	dummyFrame->setLineWidth(0);
	dummyFrame->setFrameShadow(QFrame::Sunken);
	dummyFrame->setFrameShape(QFrame::Panel);
	dummyFrame->setPalette(QPalette(QColor(50, 50, 50), QColor(100, 100, 100)));
	dummyFrame->setBackgroundRole(QPalette::Window);
	dummyFrame->setAutoFillBackground(true);
	dummyFrame->move(2, 2);
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


	downButton = new loggerButton(parent);
	downButton->move(0, H - 24);
	downButton->resize(140, 24);
	downButton->show();
	upButton = new loggerButton(parent);
	upButton->move(0, 8);
	upButton->resize(140, 26);
	upButton->upArrowType(true);
	upButton->show();
	pos = 4;
	connect(downButton, SIGNAL(pressed()), this, SLOT(downBtnPressed()));
	connect(upButton, SIGNAL(pressed()), this, SLOT(upBtnPressed()));
//buttonSize=50;

};
pultLogger::~pultLogger()
{
//delete mainFrame;
};

void pultLogger::Move(uint x, uint y)
{
	mainFrame->move(x, y + 26);
	//dummyFrame->move( x+1, y+27);
	int offset = (W - LOGGER_BUTTONS) / 2;
	downButton->move(x + offset, y + H - 24);
	upButton->move(x + offset, y);
	//testLabel->move(4,30);
	qDebug() << "upBtn geom " << upButton->geometry();
	qDebug("WidgetMove");
	move(x, y);
};

void pultLogger::setSizes(uint w, uint h)
{
	mainFrame->resize(w, h - 50);
	dummyFrame->resize(w - RESP_PANEL, h);
	respFrame->resize(RESP_PANEL, h);
	respFrame->move(w - RESP_PANEL, 2);
	qDebug() << "dummy geom" << dummyFrame->geometry();
	resize(w, h);
	W = w;
	H = h;
	buttonSize = w;
};

void pultLogger::appendText(QString kumCommand, QString text, QString replay)
{
	while (pos > H - 68) {
		downBtnPressed();
	}
	lines.append(logLine(kumCommand, text, replay, dummyFrame, respFrame, pos));
	pos = pos + TEXT_STEP;
};


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
	qDebug("TEXT MOVE");
};
void pultLogger::downBtnPressed()
{
	if (pos < 14) {
		return;
	}
	for (int i = 0; i < lines.count(); i++) {
		lines[i].moveUp();
	}
	pos = pos - SCROLL_STEP;
};

void pultLogger::ClearLog()
{
	for (int i = 0; i < lines.count(); i++) {
		lines[i].removeLabels();
	}
	lines.clear();
	pos = 4;
};
void pultLogger::CopyLog()
{
	if (lines.count() == 0) {
		return;
	}
	QClipboard *cp = QApplication::clipboard();
	QString text;
	for (int i = 0; i < lines.count(); i++)
		if (!lines[i].KumCommand().isEmpty()) {
			text += lines[i].KumCommand() + "\n";
		}
	cp->setText(text);
}

RoboPult::RoboPult(QWidget *parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), Ui::RoboPult()
{

	setupUi(this);
	Logger = new pultLogger(this);
	Logger->setSizes(164, 150);
	Logger->Move(40, 5);
	greenLight = new linkLight(this);
	greenLight->move(15, 29);
	greenLight->resize(12, 104);

	UpB->hide();
	buttUp = new MainButton(this);
	buttUp->move(90, 170);

	DownB->hide();
	buttDown = new MainButton(this);
	buttDown->move(DownB->pos());
	buttDown->setDirection(DOWN);

	LeftB->hide();
	buttLeft = new MainButton(this);
	buttLeft->move(LeftB->pos());
	buttLeft->setDirection(LEFT);
	RightB->hide();
	buttRight = new MainButton(this);
	buttRight->move(RightB->pos());
	buttRight->setDirection(RIGHT);

	StenaB->hide();
	askStena = new MainButton(this);
	askStena->move(StenaB->pos());
	askStena->setCheckable(true);
	askStena->setText(trUtf8("  "));
	askStena->loadIcon(":/icons/stena.png");
	askStena->setCheckable(true);

	SvobodnoB->hide();
	askFree = new MainButton(this);
	askFree->move(SvobodnoB->pos());
	askFree->setCheckable(true);
	askFree->setText(trUtf8(" "));
	askFree->loadIcon(":/icons/svobodno.png");
	askFree->setCheckable(true);

	RadB->hide();
	buttRad = new MainButton(this);
	buttRad->move(RadB->pos());

	buttRad->setText(trUtf8(" "));
	if (!buttRad->loadIcon(":/icons/radiation.png")) {
		qDebug("Image not loaded!");
	}
	TempB->hide();
	buttTemp = new MainButton(this);
	buttTemp->move(TempB->pos());

	buttTemp->setText(" ");
	buttTemp->loadIcon(":/icons/tC.png");

	CenterB->setIcon(QIcon(":/icons/robo_field.png"));
	connect(buttUp, SIGNAL(clicked()), this, SLOT(Up()));
	connect(buttDown, SIGNAL(clicked()), this, SLOT(Down()));
	connect(buttLeft, SIGNAL(clicked()), this, SLOT(Left()));
	connect(buttRight, SIGNAL(clicked()), this, SLOT(Right()));

	connect(buttRad, SIGNAL(clicked()), this, SLOT(RadS()));
	connect(buttTemp, SIGNAL(clicked()), this, SLOT(TempS()));

	connect(askStena, SIGNAL(clicked()), this, SLOT(SwStena()));
	connect(askFree, SIGNAL(clicked()), this, SLOT(SwSvobodno()));
	connect(CenterB, SIGNAL(clicked()), this, SLOT(CenterButton()));

	connect(ClearLog, SIGNAL(clicked()), Logger, SLOT(ClearLog()));

//	connect(CopyLog,SIGNAL(clicked()),Logger,SLOT(CopyLog()));

	link = true;
};

void RoboPult::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.save();
	p.setPen(Qt::NoPen);
	p.setBrush(QColor("#8C877E"));
	p.drawRect(0, 0, width(), height());
	p.restore();
	QWidget::paintEvent(event);
}

void RoboPult::noLink()
{
	link = false;
	greenLight->setLink(link);
	greenLight->repaint();
	qDebug("NoLINK");
};
void RoboPult::LinkOK()
{
	link = true;
	greenLight->setLink(link);
	greenLight->repaint();
};

void RoboPult::Up()
{
	if (askStena->isChecked()) {
		emit hasUpWall();
		askStena->setChecked(false);
		qDebug("Up Stena");;
		return;
	};
	if (askFree->isChecked()) {
		emit noUpWall();
		askFree->setChecked(false);
		return;
	};
	emit goUp();
};
void RoboPult::Down()
{
	if (askStena->isChecked()) {
		emit hasDownWall();
		askStena->setChecked(false);
		return;
	};
	if (askFree->isChecked()) {
		emit noDownWall();
		askFree->setChecked(false);
		return;
	};
	emit goDown();
};
void RoboPult::Left()
{
	if (askStena->isChecked()) {
		emit hasLeftWall();
		askStena->setChecked(false);
		return;
	};
	if (askFree->isChecked()) {
		emit noLeftWall();
		askFree->setChecked(false);
		return;
	};
	emit goLeft();
};
void RoboPult::Right()
{
	if (askStena->isChecked()) {
		emit hasRightWall();
		askStena->setChecked(false);
		return;
	};
	if (askFree->isChecked()) {
		emit noRightWall();
		askFree->setChecked(false);
		return;
	};
	emit goRight();
};

void RoboPult::SwStena()
{
	if (askFree->isChecked()) {
		askFree->setChecked(false);
	}
};
void RoboPult::SwSvobodno()
{
	if (askStena->isChecked()) {
		askStena->setChecked(false);
	}
};
void RoboPult::TempS()
{
	askStena->setChecked(false);
	askFree->setChecked(false);
	emit Temp();
};
void RoboPult::RadS()
{
	askStena->setChecked(false);
	askFree->setChecked(false);
	emit Rad();
};

void RoboPult::CenterButton()
{
	if (askStena->isChecked()) {
		emit Colored();
		askStena->setChecked(false);
		return;
	};
	if (askFree->isChecked()) {
		emit Clean();
		askFree->setChecked(false);
		return;
	};
	emit Color();
};

