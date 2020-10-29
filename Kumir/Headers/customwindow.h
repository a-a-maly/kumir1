#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QWidget>

class CustomWindow :
	public QWidget
{
	Q_OBJECT;
	Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle);
	Q_PROPERTY(bool stayOnTop READ isStayOnTop WRITE setStayOnTop);
public:
	explicit CustomWindow(QWidget *centralComponent,
		const QString &moduleName,
		bool resizableX, bool resizableY);
public slots:
	void setWindowTitle(const QString &);
	void show();
	void close();
	void setVisible(bool visible);
	void showMinimized();
	QString windowTitle();
	void setStayOnTop(bool v);
	bool isStayOnTop();
protected:
	void closeEvent(QCloseEvent *);
private:
	class CustomWindowPrivate *d;
};


#endif // CUSTOMWINDOW_H
