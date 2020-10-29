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

#include "assistant.h"
//#include <QtWebKit>
#include "application.h"
#include "int_proga.h"
#include "kummodules.h"
#include "kumsinglemodule.h"
#include "mainwindow.h"
#include "syntaxhighlighter.h"
#include "tab.h"
#include "programtab.h"
#include "kumiredit.h"

//static const char* folder_closed_xpm[]={
//	"16 16 9 1",
//	"g c #808080",
//	"b c #c0c000",
//	"e c #c0c0c0",
//	"# c #000000",
//	"c c #ffff00",
//	". c None",
//	"a c #585858",
//	"f c #a0a0a4",
//	"d c #ffffff",
//	"..###...........",
//	".#abc##.........",
//	".#daabc#####....",
//	".#ddeaabbccc#...",
//	".#dedeeabbbba...",
//	".#edeeeeaaaab#..",
//	".#deeeeeeefe#ba.",
//	".#eeeeeeefef#ba.",
//	".#eeeeeefeff#ba.",
//	".#eeeeefefff#ba.",
//	".##geefeffff#ba.",
//	"...##gefffff#ba.",
//	".....##fffff#ba.",
//	".......##fff#b##",
//	".........##f#b##",
//	"...........####."};

//static const char* folder_open_xpm[]={
//	"16 16 11 1",
//	"# c #000000",
//	"g c #c0c0c0",
//	"e c #303030",
//	"a c #ffa858",
//	"b c #808080",
//	"d c #a0a0a4",
//	"f c #585858",
//	"c c #ffdca8",
//	"h c #dcdcdc",
//	"i c #ffffff",
//	". c None",
//	"....###.........",
//	"....#ab##.......",
//	"....#acab####...",
//	"###.#acccccca#..",
//	"#ddefaaaccccca#.",
//	"#bdddbaaaacccab#",
//	".eddddbbaaaacab#",
//	".#bddggdbbaaaab#",
//	"..edgdggggbbaab#",
//	"..#bgggghghdaab#",
//	"...ebhggghicfab#",
//	"....#edhhiiidab#",
//	"......#egiiicfb#",
//	"........#egiibb#",
//	"..........#egib#",
//	"............#ee#"};

//static const char * pix_file []={
//	"16 16 7 1",
//	"# c #000000",
//	"b c #ffffff",
//	"e c #000000",
//	"d c #404000",
//	"c c #c0c000",
//	"a c #ffffc0",
//	". c None",
//	"................",
//	".........#......",
//	"......#.#a##....",
//	".....#b#bbba##..",
//	"....#b#bbbabbb#.",
//	"...#b#bba##bb#..",
//	"..#b#abb#bb##...",
//	".#a#aab#bbbab##.",
//	"#a#aaa#bcbbbbbb#",
//	"#ccdc#bcbbcbbb#.",
//	".##c#bcbbcabb#..",
//	"...#acbacbbbe...",
//	"..#aaaacaba#....",
//	"...##aaaaa#.....",
//	".....##aa#......",
//	".......##......."};

Assistant::Assistant(QWidget *parent)
	: QMainWindow(parent)
	, Ui::Assistant()
{
	setupUi(this);

	QString imagesRoot;
	imagesRoot = ":/icons/";

	icon_folder_enabled_closed = QIcon(imagesRoot + "assistant_folder_enabled_closed.png");
	icon_folder_enabled_open = QIcon(imagesRoot + "assistant_folder_enabled_open.png");
	icon_folder_disabled_closed = QIcon(imagesRoot + "assistant_folder_disabled_closed.png");
	icon_folder_disabled_open = QIcon(imagesRoot + "assistant_folder_disabled_open.png");
	icon_item_enabled = QIcon(imagesRoot + "assistant_item_enabled.png");
	icon_item_disabled = QIcon(imagesRoot + "assistant_item_disabled.png");

	userAlgsIndex->setIconSize(QSize(24, 24));

	connect(userAlgsIndex, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(showHtmlInBrowser(QTreeWidgetItem *, int)));
	connect(userAlgsIndex, SIGNAL(itemExpanded(QTreeWidgetItem *)), this, SLOT(setOpenedIcon(QTreeWidgetItem *)));
	connect(userAlgsIndex, SIGNAL(itemCollapsed(QTreeWidgetItem *)), this, SLOT(setClosedIcon(QTreeWidgetItem *)));
	connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(performSearch(QString)));
	connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(updatePlaceholderText()));
	connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(handleCheckBoxStateClicked(bool)));

	userAlgsIndex->header()->setVisible(false);

//	QFont bigFont = userAlgsIndex->font();
//	bigFont.setPointSize(bigFont.pointSize()+1);
//	userAlgsIndex->setFont(bigFont);

//	QFont bigFont1 = browser->font();
//	bigFont1.setPointSize(bigFont.pointSize()+1);
//	browser->setFont(bigFont1);

#ifndef Q_OS_MAC
	const QString cssPath = QApplication::applicationDirPath() + "/Kumir/Help/assistant_stylesheet.css";
#else
	const QString cssPath = QApplication::applicationDirPath() + "/../Resources/assistant_stylesheet.css";
#endif
	QFile f(cssPath);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString style = QString::fromLatin1(f.readAll());
		f.close();

		QPalette p = palette();
		style.replace("%window_background%",
			p.color(QPalette::Window).name());
		style.replace("%highlight_background%",
			p.color(QPalette::Highlight).name());
		style.replace("%highlight_foreground%",
			p.color(QPalette::HighlightedText).name());

		style.replace("%editor_font_family%",
			app()->settings->value("Appearance/Font",
				"monospace").toString());
		style.replace("%editor_font_size%",
			app()->settings->value("Appearance/FontSize", "12").toString() + "pt");
		setStyleSheet(style);

	}
	browser->setHtml(defaultPageHtml());

}

void Assistant::updatePlaceholderText()
{
	QString placeholderText;
	if (checkBox->isChecked()) {
		placeholderText = tr("Type algorhitm name part to search");
	} else {
		placeholderText = tr("Type algorhitm name prefix to search");
	}
#if QT_VERSION >= 0x040700
	lineEdit->setPlaceholderText(placeholderText);
#endif
}

void Assistant::init(KumModules *I)
{
	this->I = I;
}

void Assistant::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
//	browser->clear();
	updateAlgorhitms();
	updatePlaceholderText();
}


Assistant::~Assistant()
{
}

void Assistant::updateAlgorhitms()
{

	QMap<QString, bool> map;
	for (int i = 0; i < userAlgsIndex->topLevelItemCount(); ++i) {
		map[userAlgsIndex->topLevelItem(i)->text(0)] = userAlgsIndex->topLevelItem(i)->isExpanded();
	}
	userAlgsIndex->clear();

	for (int i = 0; i < I->count(); i++) {
		QTreeWidgetItem *item;
		QString name(I->module(i)->Name());
		QString visName = I->module(i)->moduleInfoName();
		if (visName.isEmpty()) {
			visName = name;
		}
//		QString description = I->module(i)->moduleDescrption();
		if (name == "@main") {
			visName = trUtf8("Алгоритмы пользователя");
		}
		if (!userAlgsIndex->findItems(visName, Qt::MatchExactly).isEmpty()) {
			item = userAlgsIndex->findItems(visName, Qt::MatchExactly).first();
		} else {
			item = new QTreeWidgetItem(userAlgsIndex, QStringList(visName));
			item->setData(0, Qt::UserRole, name);
			if (I->module(i)->isEnabled() || name == "@main") {
				item->setIcon(0, icon_folder_enabled_closed);
			} else {
				item->setIcon(0, icon_folder_disabled_closed);
			}
		}
		function_table *F = I->module(i)->Functions();
		for (int j = 0; j < F->size(); j++) {
			const QString nam = F->nameById(j);
			bool hide = nam.startsWith("!") || nam.startsWith("_");
			if (hide) {
				continue;
			}
			bool hideNotTeacher = nam.startsWith("@") && !qApp->arguments().contains("-t");
			if (hideNotTeacher) {
				continue;
			}
			{
				QString fname(F->nameById(j));
				if (fname.isEmpty() && name == "@main") {
					fname = trUtf8("Основной алгоритм");
				}
				QTreeWidgetItem *f = new QTreeWidgetItem(item, QStringList(fname));
				QString leafText = getHtmlCode(i, j);
				f->setData(0, Qt::UserRole, leafText);
//				f->setIcon(0,QIcon(QPixmap(pix_file)));
				f->setIcon(0, I->module(i)->isEnabled() ? icon_item_enabled : icon_item_disabled);
			}
		}
	}
	setEnableForUserAlgs();

	userAlgsIndex->sortItems(0, Qt::AscendingOrder);
	sortModules();
//	QList<QTreeWidgetItem*> list = userAlgsIndex->findItems(trUtf8("Алгоритмы пользователя"), Qt::MatchExactly);
//	if (!list.isEmpty())
//	{
//		int ind = userAlgsIndex->indexOfTopLevelItem(list.first());
//		if (ind!=0)
//			userAlgsIndex->insertTopLevelItem(0, userAlgsIndex->takeTopLevelItem(ind));
//	}
//	list = userAlgsIndex->findItems(trUtf8("Стандартные алгоритмы"), Qt::MatchExactly);
//	if (!list.isEmpty())
//	{
//		int ind = userAlgsIndex->indexOfTopLevelItem(list.first());
//		if (ind!=1)
//			userAlgsIndex->insertTopLevelItem(1, userAlgsIndex->takeTopLevelItem(ind));
//	}

//	for(int i=0; i<userAlgsIndex->topLevelItem(0)->childCount(); ++i)
//	{
//		QTreeWidgetItem* item = userAlgsIndex->topLevelItem(0)->child(i);
//		if (item->text(0)==trUtf8("Основной алгоритм"))
//		{
//			if (i!=0)
//				userAlgsIndex->topLevelItem(0)->insertChild(0, userAlgsIndex->topLevelItem(0)->takeChild(userAlgsIndex->topLevelItem(0)->indexOfChild(item)));
//			break;
//		}
//	}

//	for(int i=0; i<userAlgsIndex->topLevelItemCount(); ++i)
//		if (map.contains(userAlgsIndex->topLevelItem(i)->text(0)))
//			userAlgsIndex->topLevelItem(i)->setExpanded(map[userAlgsIndex->topLevelItem(i)->text(0)]);
}

bool Assistant::isInternalFunction(const QString &name)
{
	int moduleId, id;
	getFuncByName(name, moduleId, id);
	return moduleId != -1 && id != -1;
}



QString Assistant::getSyntaxFromTable(int modId, int funcID, QString &description) const
{

	function_table *F = I->module(modId)->Functions();
	const QString name = F->nameById(funcID);

	int argsCount = F->argCountById(funcID);
	PeremType retType = F->typeById(funcID);
	QString syntax = trUtf8("алг ");

	QMap<int, QString> argMap;
	QRegExp rxArg("<arg id=\"(\\d+)\">(.*)</arg>");
	rxArg.setMinimal(true);
	int pos = 0;
	while ((pos != -1) && (pos < description.length())) {
		pos = rxArg.indexIn(description, pos);
		if (pos != -1) {
			QStringList result = rxArg.capturedTexts();
			if (result.count() == 3) {
				int argNo = result[1].toInt();
				argMap[argNo] = result[2];
			}
			pos++;
		}
	}

	if (retType == integer) {
		syntax += trUtf8("цел ");
	}
	if (retType == real) {
		syntax += trUtf8("вещ ");
	}
	if (retType == charect) {
		syntax += trUtf8("сим ");
	}
	if (retType == kumBoolean) {
		syntax += trUtf8("лог ");
	}
	if (retType == kumString) {
		syntax += trUtf8("лит ");
	}
	if (retType == mass_integer) {
		syntax += trUtf8("цел таб ");
	}
	if (retType == mass_real) {
		syntax += trUtf8("вещ таб ");
	}
	if (retType == mass_charect) {
		syntax += trUtf8("сим таб ");
	}
	if (retType == mass_bool) {
		syntax += trUtf8("лог таб ");
	}
	if (retType == mass_string) {
		syntax += trUtf8("лит таб ");
	}

	syntax += name;

	if (argsCount > 0) {
		syntax += "(";
	}

	for (int i = 0; i < argsCount; i++) {
		if (F->isRes(funcID, i)) {
			syntax += trUtf8("рез ");
		} else if (F->isArgRes(funcID, i)) {
			syntax += trUtf8("арг рез ");
		}
		PeremType argType = F->argTypeById(funcID, i);
		if (argType == integer) {
			syntax += trUtf8("цел ");
		}
		if (argType == real) {
			syntax += trUtf8("вещ ");
		}
		if (argType == charect) {
			syntax += trUtf8("сим ");
		}
		if (argType == kumBoolean) {
			syntax += trUtf8("лог ");
		}
		if (argType == kumString) {
			syntax += trUtf8("лит ");
		}
		if (argType == mass_integer) {
			syntax += trUtf8("цел таб ");
		}
		if (argType == mass_real) {
			syntax += trUtf8("вещ таб ");
		}
		if (argType == mass_charect) {
			syntax += trUtf8("сим таб ");
		}
		if (argType == mass_bool) {
			syntax += trUtf8("лог таб ");
		}
		if (argType == mass_string) {
			syntax += trUtf8("лит таб ");
		}

		if (argMap.contains(i + 1)) {
			syntax += argMap[i + 1];
		} else {
			int varId = F->argPeremId(funcID, i);
			QString varName;
			if (varId > -1 && varId < 512) {
				varName = I->module(modId)->Values()->symb_table[varId].name;
			}
			syntax += varName;
		}

		if (i < (argsCount - 1)) {
			syntax += ", ";
		}
	}

	if (argsCount > 0) {
		syntax += ")";
	}

	ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
	Q_CHECK_PTR(pt);
	KumirEdit *editor = pt->editor();
	Q_CHECK_PTR(editor);
	SyntaxHighlighter *sh = editor->syntaxHighlighter();
	Q_CHECK_PTR(sh);

	return sh->highlightBlockAsHtml(syntax, NULL);
}

void Assistant::showHtmlInBrowser(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);
	bool isTopLevel = false;
	for (int i = 0; i < userAlgsIndex->topLevelItemCount(); i++) {
		if (item == userAlgsIndex->topLevelItem(i)) {
			isTopLevel = true;
			break;
		}
	}

	if (!isTopLevel) {
//		browser->setHtml(getHtmlCode(item->data(0)));
		browser->setHtml(item->data(0, Qt::UserRole).toString());
	} else {
//		browser->setHtml(defaultPageHtml());
		browser->setHtml(modulePageHtml(item->data(0, Qt::UserRole).toString()));
	}
//	if ( item->text(0)==trUtf8("Основной алгоритм") && item->text(0)==trUtf8("Основной исполнитель") )
//	{
//		browser->clear();
//	}
//	else
//	{
//		browser->setHtml(getHtmlCode(item->text(0)));
//		browser->setDisabled(item->isDisabled());
//	}
}

void Assistant::setStyleSheet(const QString &styleSheet)
{
	s_styleSheet = styleSheet;
}

QString Assistant::styleSheet() const
{
	return s_styleSheet;
}

QString Assistant::defaultPageHtml() const
{
	QString data;
	data += "<html><head><style type='text/css'>";
	data += styleSheet();
	data += "</style></head><body>\n";
	data += "<information>\n";
	data += "<p>" + tr("Click on function name at left to dysplay corresponding information.") + "</p>\n";
	data += "</information>\n";
	data += "</body></html>\n";
	return data;
}

QString Assistant::modulePageHtml(const QString &moduleName) const
{
	QString data;
	data += "<html><head><style type='text/css'>";
	data += styleSheet();
	data += "</style></head><body>\n";
	int id = I->idByName(moduleName);
	if (id != -1) {
		KumSingleModule *module = I->module(id);
		if (!module->isEnabled()) {
			data += "<section>\n";
			ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
			Q_CHECK_PTR(pt);
			KumirEdit *editor = pt->editor();
			Q_CHECK_PTR(editor);
			SyntaxHighlighter *sh = editor->syntaxHighlighter();
			Q_CHECK_PTR(sh);
			const QString visibleName = module->moduleInfoName().isEmpty() ? module->name : module->moduleInfoName();
			const QString useLine = QString::fromUtf8("использовать ") + visibleName;
			data += "<header>" + tr("This module is not enabled") + "</header>\n";
			data += "<description><p>" +
				tr("Algorhitms of this module are not available. To use them write the first line of your program: ") +
				"<example>\n" +
				sh->highlightBlockAsHtml(useLine, NULL) +
				"</example>\n" +
				"</p></description>\n";

			data += "</section>\n";
		}
		if (!module->moduleDescrption().isEmpty()) {
			data += "<section>\n";
			data += "<header>" + tr("Description") + "</header>\n";
			data += "<description>\n";
			data += module->moduleDescrption();
			data += "</description>\n";
			data += "</section>\n";
		}
		if (module->name == "@main") {
			data += mainModuleUsage();
		}

	}
	data += "</body></html>\n";
	return data;

}

QString Assistant::mainModuleUsage() const
{
	QString data;
	data += "<section>\n";
	data += "<header>" + tr("Description") + "</header>\n";
	data += "<description>\n";
	data += "<p>" + tr("Algorhitms in user's program.") + "</p>\n";
	data += "<p>" + tr("Use documentation lines to supply your algorhitms these texts.") + "</p>\n";
	data += "<p>" + tr("Documentation lines starts with symbol # (hash symbol) and must be placed between algorhitm header and algorhitm body.") + "</p>\n";
	data += "</description>\n";
	data += "</section>\n";
	data += "<section>\n";
	data += "<header>" + tr("Example") + "</header>\n";
	data += "<example>\n";
	const QStringList example =
		QString::fromUtf8(
			"алг цел сумма чисел(цел N, целтаб T[1:N])\n"
			". # Возвращает сумму N целых чисел\n"
			". # из массива T\n"
			"нач\n"
			". цел i\n"
			". знач:=0\n"
			". нц для i от 1 до N\n"
			". . знач:=знач+T[i]\n"
			". кц\n"
			"кон\n"
		).split("\n");
	ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
	Q_CHECK_PTR(pt);
	KumirEdit *editor = pt->editor();
	Q_CHECK_PTR(editor);
	SyntaxHighlighter *sh = editor->syntaxHighlighter();
	Q_CHECK_PTR(sh);
	foreach (const QString &ex, example) {
		data += sh->highlightBlockAsHtml(ex, NULL) + "<br/>\n";
	}

	data += "</example>\n";
	data += "</section>\n";
	return data;
}

QString Assistant::getHtmlCode(int modId, int funcId) const
{
	QString data;
	data += "<html><head><style type='text/css'>";
	data += styleSheet();
	data += "</style></head><body>\n";
	QString description;
	QString syntax;

	description = I->module(modId)->Functions()->docString(funcId);
	syntax = getSyntaxFromTable(modId, funcId, description);

	QRegExp rxExample("<example>(.*)</example>");
	QString example;
	rxExample.setMinimal(true);
	int examplePos = rxExample.indexIn(description);
	int exampleLen = rxExample.matchedLength();
	if (examplePos != -1) {

		ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
		Q_CHECK_PTR(pt);
		KumirEdit *editor = pt->editor();
		Q_CHECK_PTR(editor);
		SyntaxHighlighter *sh = editor->syntaxHighlighter();
		Q_CHECK_PTR(sh);
		QString ex = rxExample.cap(1);
		QStringList exLines = ex.split("\n");
		for (int i = 0; i < exLines.count(); i++) {
			QString exLine = exLines.at(i).trimmed();
			if (exLine.isEmpty()) {
				if (i == 0 || i == exLines.size() - 1) {
					continue;
				}
			}
			QString htmlLine = sh->highlightBlockAsHtml(exLine, NULL);
			htmlLine.replace(". ", ".&nbsp;");
			example += htmlLine + "<br/>";
		}

		description.remove(examplePos, exampleLen);
	}
	description.remove("<description>");
	description.remove("</description>");
	description = description.simplified();
	data += trUtf8("<section><header>Синтаксис</header>\n<syntax>");
	data += syntax + "\n</syntax>\n</section>\n";
	if (!description.isEmpty()) {
		data += trUtf8("<section><header>Описание</header>\n<description>\n");
		data += description + "\n</description>\n</section>\n";
	}

	if (!example.isEmpty()) {
		data += trUtf8("<section><header>Пример</header>\n<example>");
		data += example + "\n</example>\n</section>\n";
	}
	data += "</body></html>\n";
	return data;
}

void Assistant::setEnableForUserAlgs()
{
	for (int i = 0; i < userAlgsIndex->topLevelItemCount(); ++i) {
		QTreeWidgetItem *item = userAlgsIndex->topLevelItem(i);
		const QString modName = item->data(0, Qt::UserRole).toString();
		int id = I->idByName(modName);
		if (id != -1) {
			bool enabled = I->module(id)->isEnabled() || modName == "@main";
//			item->setDisabled(!enabled);
			QFont fnt = item->font(0);
			fnt.setBold(enabled);
			item->setFont(0, fnt);
		}
	}
}

void Assistant::setOpenedIcon(QTreeWidgetItem *item)
{
	if (item->childCount() > 0) {
		const QString moduleName = item->data(0, Qt::UserRole).toString();
		KumSingleModule *module = I->module(I->idByName(moduleName));
		item->setIcon(0, module->isEnabled() ? icon_folder_enabled_open : icon_folder_disabled_open);
	}
}

void Assistant::setClosedIcon(QTreeWidgetItem *item)
{

	if (item->childCount() > 0) {
		const QString moduleName = item->data(0, Qt::UserRole).toString();
		KumSingleModule *module = I->module(I->idByName(moduleName));
		item->setIcon(0, module->isEnabled() ? icon_folder_enabled_closed : icon_folder_disabled_closed);
	}
}

void Assistant::getFuncByName(QString name, int &moduleId, int &id)
{
	moduleId = -1;
	id = -1;
	for (int i = 0; i <= I->lastModuleId(); ++i) {
		id = I->module(i)->Functions()->idByName(name);
		if (id > -1) {
			moduleId = i;
			break;
		}
	}
}

void Assistant::performSearch(const QString &line)
{
	for (int i = 0; i < userAlgsIndex->topLevelItemCount(); i++) {
		QTreeWidgetItem *node = userAlgsIndex->topLevelItem(i);
		bool hasVisible = false;
		for (int j = 0; j < node->childCount(); j++) {
			QTreeWidgetItem *leaf = node->child(j);
			const QString text = leaf->text(0);
			bool visible = line.simplified().isEmpty();
			if (!visible) {
				if (checkBox->isChecked()) {
					visible = text.contains(line.simplified());
				} else {
					visible = text.startsWith(line.simplified());
				}
			}
			leaf->setHidden(!visible);
			hasVisible = hasVisible || visible;
		}
		node->setExpanded(hasVisible && !line.isEmpty());
	}
}

void Assistant::handleCheckBoxStateClicked(bool state)
{
	Q_UNUSED(state);
	if (!lineEdit->text().simplified().isEmpty()) {
		performSearch(lineEdit->text());
	}
}

void Assistant::sortModules()
{
	QList<QTreeWidgetItem *> alwaysEnabledModules;
	QList<QTreeWidgetItem *> enabledModules;
	QList<QTreeWidgetItem *> disabledModules;
	int items = userAlgsIndex->topLevelItemCount();
	for (int i = 0; i < items; i++) {
		QTreeWidgetItem *item = userAlgsIndex->takeTopLevelItem(0);
		const QString moduleName = item->data(0, Qt::UserRole).toString();
		KumSingleModule *module = I->module(I->idByName(moduleName));
		if (moduleName == "@main") {
			alwaysEnabledModules << item;
		} else if (moduleName == "system") {
			alwaysEnabledModules << item;
		} else if (moduleName == "internal") {
			alwaysEnabledModules << item;
		} else if (module->isEnabled()) {
			enabledModules << item;
		} else {
			disabledModules << item;
		}
	}
	K_ASSERT(userAlgsIndex->topLevelItemCount() == 0);
	const QList <QTreeWidgetItem *> allItems = alwaysEnabledModules + enabledModules + disabledModules;
	for (int i = 0; i < allItems.size(); i++) {
		userAlgsIndex->insertTopLevelItem(i, allItems[i]);
	}



}
