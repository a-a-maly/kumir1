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

#include "findpanel.h"
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QShowEvent>

FindPanel::FindPanel(const QString &title, QWidget *parent) : QToolBar(title, parent)
{
	options = 0;
	closeButton = new QToolButton(this);
	closeButton->setToolTip(tr("Close"));
	closeButton->setIcon(QIcon(QString::fromUtf8(":/icons/22x22_close_blue.png")));
	addWidget(closeButton);
	addWidget(new QLabel(tr("Find:")));
	textToFind = new QLineEdit(this);
	addWidget(textToFind);
	prevButton = new QToolButton(this);
	prevButton->setToolTip(tr("Find previous"));
	prevButton->setIcon(QIcon(QString::fromUtf8(":/icons/go-previous.png")));
	prevButton->setEnabled(false);
	actionPrevButton = addWidget(prevButton);
	nextButton = new QToolButton(this);
	nextButton->setToolTip(tr("Find next"));
	nextButton->setIcon(QIcon(QString::fromUtf8(":/icons/go-next.png")));
	nextButton->setEnabled(false);
	addWidget(nextButton);
	addSeparator();

	addWidget(new QLabel(tr("Replace with:")));//
	textToReplace = new QLineEdit(this);//
	addWidget(textToReplace);//

	replacePrevAllButton = new QToolButton(this);
	replacePrevAllButton->setToolTip(tr("Replace all previous"));
	replacePrevAllButton->setIcon(QIcon(QString::fromUtf8(":/icons/go-previous.png")));
	replacePrevAllButton->setEnabled(false);
	addWidget(replacePrevAllButton);

	replaceButton = new QToolButton(this);//
	replaceButton->setToolTip(tr("Replace here"));//
	replaceButton->setIcon(QIcon(QString::fromUtf8(":/icons/go-jump-locationbar.png")));//
	replaceButton->setEnabled(false);
	addWidget(replaceButton);//

	replaceNextAllButton = new QToolButton(this);
	replaceNextAllButton->setToolTip(tr("Replace all next"));
	replaceNextAllButton->setIcon(QIcon(QString::fromUtf8(":/icons/go-next.png")));
	replaceNextAllButton->setEnabled(false);
	addWidget(replaceNextAllButton);

	addSeparator();

	caseSensitive = new QCheckBox(tr("Case sensitive"), this);
	addWidget(caseSensitive);

	connect(closeButton, SIGNAL(clicked()), this, SIGNAL(keyPressed()));
	connect(textToFind, SIGNAL(textEdited(QString)), this, SIGNAL(keyPressed()));
	connect(textToReplace, SIGNAL(textEdited(QString)), this, SIGNAL(keyPressed()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(hide()));
	connect(textToFind, SIGNAL(returnPressed()), this, SIGNAL(findTextPressed()));
	connect(textToFind, SIGNAL(textChanged(QString)), this, SLOT(setButtonsEnable(QString)));
	connect(prevButton, SIGNAL(clicked()), this, SLOT(findBack()));
	connect(nextButton, SIGNAL(clicked()), this, SIGNAL(findTextPressed()));
	connect(replaceButton, SIGNAL(clicked()), this, SIGNAL(replacePressed()));  //
	connect(textToReplace, SIGNAL(returnPressed()), this, SIGNAL(replacePressed()));
	connect(replacePrevAllButton, SIGNAL(clicked()), this, SIGNAL(replacePrevAllPressed()));
	connect(replaceNextAllButton, SIGNAL(clicked()), this, SIGNAL(replaceNextAllPressed()));
}

void FindPanel::showEvent(QShowEvent *event)
{
	textToFind->setFocus();
	emit showed();
	event->accept();
}

void FindPanel::findBack()
{
	options = QTextDocument::FindBackward;
	emit findTextPressed();
	options = 0;
}

QTextDocument::FindFlags FindPanel::getOptions()
{
	QTextDocument::FindFlags cs;
	if (caseSensitive->checkState() == Qt::Checked) {
		cs = QTextDocument::FindCaseSensitively;
	} else {
		cs = 0x0;
	}
	return options | cs;
}

Qt::CaseSensitivity FindPanel::isCaseSensitivity()
{
	if (caseSensitive->checkState() == Qt::Checked) {
		return Qt::CaseSensitive;
	} else {
		return Qt::CaseInsensitive;
	}
}

FindPanel::~FindPanel()
{
}

QString FindPanel::getText()
{
	return textToFind->text();
}

void FindPanel::setButtonsEnable(const QString &text)
{
	bool en = ! text.isEmpty();
	prevButton->setEnabled(en);
	nextButton->setEnabled(en);
	replacePrevAllButton->setEnabled(en);
	replaceButton->setEnabled(en);
	replaceNextAllButton->setEnabled(en);
}

QString FindPanel::getNewText()//
{
	return textToReplace->text();
}
