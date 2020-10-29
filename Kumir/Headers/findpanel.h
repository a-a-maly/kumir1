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

#ifndef FINDPANEL_H
#define FINDPANEL_H

#include <QToolBar>
#include <QTextDocument>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class QToolButton;

class FindPanel : public QToolBar
{
	Q_OBJECT

public:
	FindPanel(const QString &title, QWidget *parent = 0);
	~FindPanel();
	QTextDocument::FindFlags getOptions();
	QString getText();
	QString getNewText();
	Qt::CaseSensitivity isCaseSensitivity();

protected:
	QToolButton *closeButton;
	QLineEdit *textToFind;
	QCheckBox *caseSensitive;
	QTextDocument::FindFlags options;
	QAction *actionPrevButton;
	QToolButton *prevButton;
	QToolButton *nextButton;
	void showEvent(QShowEvent *event);
	QLineEdit *textToReplace;
	QToolButton *replaceButton;
	QToolButton *replacePrevAllButton;
	QToolButton *replaceNextAllButton;

protected slots:
	void setButtonsEnable(const QString &);
	void findBack();

signals:
	void findModeChanged();
	void findTextPressed();
	void findNewTextPressed();
	void showed();
	void replacePressed();
	void replacePrevAllPressed();
	void replaceNextAllPressed();
	void keyPressed();
};

#endif
