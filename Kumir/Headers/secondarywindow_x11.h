#ifndef SECONDARYWINDOW_X11_H
#define SECONDARYWINDOW_X11_H

#include <QWidget>

class SecondaryWindow: public QWidget
{
	Q_OBJECT;
public:
	explicit SecondaryWindow(QWidget *innerWidget, const QString &windowName);
signals:
	void shown();
	void closed();
protected:
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);
	void timerEvent(QTimerEvent *e);
private:
	QString s_windowName;
	QWidget *w_innerWidget;
};

#endif // SECONDARYWINDOW_X11_H
