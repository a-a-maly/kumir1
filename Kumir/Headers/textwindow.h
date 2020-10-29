//
// C++ Interface:
//
// Description:
//
//
// Author: ?????? ??????? <V.Yacovlev@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include <QMainWindow>
#include "ui_textwindow1.h"

class TextWindow : public QMainWindow, public Ui::TextWindow1
{
	Q_OBJECT

public:
	Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
	Q_PROPERTY(QString textEncoding READ textEncoding WRITE setTextEncoding)
	TextWindow(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	~TextWindow();
	void setFileName(const QString &fileName);
	inline QString fileName()
	{
		return m_fileName;
	};

	inline void setTextEncoding(const QString &enc)
	{
		m_textEncoding = enc;
	}
	inline QString textEncoding()
	{
		return m_textEncoding;
	}

public slots:
	bool save();
	bool saveAs();
	void open(QString filename = "");

protected:
	QString m_fileName;
	QString m_fileDir;
	QString m_textEncoding;

	bool saveFileTo(const QString &path, const QString &enc);
	void enterEvent(QEvent *event) ;

	void closeEvent(QCloseEvent *event);

	QString fakeLabel();

protected slots:
	void checkChanges();

};

#endif

