#ifndef CUSTOMWINDOW_P_H
#define CUSTOMWINDOW_P_H

#include <QWidget>
#include <QAbstractButton>

class CustomWindow;

#define Left 'l'
#define Right 'r'
#define Top 't'
#define Bottom 'b'

class CustomWindowPrivate : public QObject
{
public:
	void init(QWidget *centralWidget,
		const QString &moduleName,
		bool resizableX, bool resizableY);

	QWidget *w_centralWidget;

	QWidget *w_topBorder;
	QWidget *w_bottomBorder;
	QWidget *w_leftBorder;
	QWidget *w_rightBorder;

	QAbstractButton *btn_stayOnTop;
	QAbstractButton *btn_minimize;
	QAbstractButton *btn_close;

	class CustomLabel *lbl_title;

	QList<QPixmap *> px_topBorder;
	QList<QPixmap *> px_bottomBorder;
	QList<QPixmap *> px_leftBorder;
	QList<QPixmap *> px_rightBorder;

	QList<QPixmap *> px_stayOnTopButton;
	QList<QPixmap *> px_minimizeButton;
	QList<QPixmap *> px_closeButton;

	CustomWindow *q;
	QString s_moduleName;

	void timerEvent(QTimerEvent *);
};


class BorderWidget :
	public QWidget
{
public:
	explicit BorderWidget(
		QWidget *parent
		, QWidget *window
		, class CustomWindowPrivate *q
		, char border
		, const QList<QPixmap *> &pixmaps
		, bool resizable
	);
protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

	void mouseDragAction(const QPoint &offset);

	char e_border;
	QList<QPixmap *> px_border;
	bool b_resizable;
	bool b_moving;
	QPoint p_start;
	class CustomWindowPrivate *d;
	QWidget *w_window;
};

class CustomButton : public QAbstractButton
{
public:
	explicit CustomButton(QWidget *parent, bool checkable, const QString &name,
		const QList<QPixmap *> &pixmaps, const QString &toolTip);
	inline bool isHovered() const
	{
		return b_hovered;
	}
	inline bool isPressed() const
	{
		return b_pressed;
	}
protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	bool b_hovered;
	bool b_pressed;
	QList<QPixmap *> px;
};

class CustomLabel : public QWidget
{
public:
	explicit CustomLabel(QWidget *parent, const QString &style);
	void setText(const QString &v);
	inline QString text() const
	{
		return s_text;
	}
protected:
	void paintEvent(QPaintEvent *e);
	QFont f_font;
	QColor col_foreground;
	Qt::AlignmentFlag fl_textAlignment;
	int i_padding;
	QString s_text;

};

#endif // CUSTOMWINDOW_P_H
