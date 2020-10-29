//
// C++ Implementation: semaphorewidget
//
// Description:
//
//
// Author: Виктор Яковлев <V.Yacovlev@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "semaphorewidget.h"
#include <QPainter>

SemaphoreWidget::SemaphoreWidget(QWidget *parent) : QWidget(parent)
{
	setFixedSize(QSize(8, 24));
	fastRepaint = true;
	mode = 2; // green
}

void SemaphoreWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	// draw red
	painter.setPen(QPen(QColor("red")));
	painter.setBrush(mode == 0 ? QColor("red") : QColor("lightgray"));
	painter.drawEllipse(0, 1, 6, 6);
	// draw yellow
	painter.setPen(QPen(QColor("goldenrod")));
	painter.setBrush(mode == 1 ? QColor("goldenrod") : QColor("lightgray"));
	painter.drawEllipse(0, 9, 6, 6);
	// draw green
	painter.setPen(QPen(QColor("darkgreen")));
	painter.setBrush(mode == 2 ? QColor("green") : QColor("lightgray"));
	painter.drawEllipse(0, 17, 6, 6);
	painter.end();
}

void SemaphoreWidget::setModeRed()
{
	uint pmode = mode;
//  qDebug() << "MODE: RED";
	mode = 0;
	if (pmode != mode) {
		if (fastRepaint) {
			repaint();
		} else {
			update();
		}
	}
}

void SemaphoreWidget::setModeGreen()
{
	uint pmode = mode;
//  qDebug() << "MODE: GREEN";
	mode = 2;
	if (pmode != mode) {
		update();
	}
}

void SemaphoreWidget::setModeYellow()
{
	uint pmode = mode;
//  qDebug() << "MODE: YELLOW";
	mode = 1;
	if (pmode != mode) {
		update();
	}
}

SemaphoreWidget::~SemaphoreWidget()
{
}


