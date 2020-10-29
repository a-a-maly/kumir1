#ifndef SECONDARYWINDOW_WIN32_H
#define SECONDARYWINDOW_WIN32_H
#include <QWidget>
#include <windows.h>

class SecondaryWindow : public QWidget
{
	Q_OBJECT;
public:
	explicit SecondaryWindow(QWidget *innerWidget, const QString &moduleName);
	void setStayOnTop(bool v);
	bool isStayOnTop() const;
signals:
	void shown();
	void closed();
protected:
	bool winEvent(MSG *message, long *result);
	void timerEvent(QTimerEvent *e);
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);
	QWidget *w_innerWidget;
	QString s_moduleName;
	HMENU h_windowMenu;
	UINT ID_STAYONTOP;
	HBITMAP h_image0;
	HBITMAP h_image1;
	QString s_stayOnTop;
	QString s_notStayOnTop;
	bool b_created;
};
#endif // SECONDARYWINDOW_WIN32_H
