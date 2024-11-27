#include "secondarywindow.h"

#ifdef Q_OS_UNIX

#include "application.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QTimerEvent>

SecondaryWindow::SecondaryWindow(QWidget *innerWidget, const QString &windowName)
	: QWidget(0)
	, s_windowName(windowName)
	, w_innerWidget(innerWidget)
{
	innerWidget->setParent(this);
	QVBoxLayout *l = new QVBoxLayout;
	setLayout(l);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(innerWidget);
	bool onTop = app()->settings->value("Windows/" + s_windowName + "/StayOnTop", false).toBool();
	setWindowFlags(onTop ? Qt::WindowStaysOnTopHint | Qt::Window : Qt::Window);
	QRect r = app()->settings->value("Windows/" + s_windowName + "/Geometry",
			QRect(0, 0, 0, 0)).toRect();
	QSize minimumWindowSize = innerWidget->minimumSize();
	r.setWidth(qMax(r.width(), minimumWindowSize.width()));
	r.setHeight(qMax(r.height(), minimumWindowSize.height()));

	bool isFixedW = innerWidget->sizePolicy().horizontalPolicy() == QSizePolicy::Fixed;
	bool isFixedH = innerWidget->sizePolicy().verticalPolicy() == QSizePolicy::Fixed;

	if (isFixedW) {
		setFixedWidth(r.width());
	}
	if (isFixedH) {
		setFixedHeight(r.height());
	}

	resize(r.size());
	move(r.topLeft());

	startTimer(500);
}


void SecondaryWindow::closeEvent(QCloseEvent *e)
{
	QRect r(x(), y(), width(), height());
	if (r.width() > 0 && r.height() > 0) {
		app()->settings->setValue("Windows/" + s_windowName + "/Geometry", r);
	}
	app()->settings->setValue("Windows/" + s_windowName + "/StayOnTop",
		windowFlags().testFlag(Qt::WindowStaysOnTopHint));
	QWidget::closeEvent(e);
	emit closed();
}

void SecondaryWindow::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
	emit shown();
}

void SecondaryWindow::timerEvent(QTimerEvent *e)
{
	e->accept();
	QString newTitle = w_innerWidget->windowTitle();
	QString oldTitle = windowTitle();
	if (newTitle != oldTitle) {
		setWindowTitle(newTitle);
	}
	QSize newSize(w_innerWidget->size());
	if (size() != newSize) {
		resize(newSize);
	}
}

#endif

