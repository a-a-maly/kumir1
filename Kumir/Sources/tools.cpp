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
/*
=== THIS FILE ENCODED IN UTF-8 LOCALE
*/


#include "tools.h"
//#include "kassert.h"
#include "config.h"
#include "integeroverflowchecker.h"
#include "mainwindow.h"
#include "syntaxhighlighter.h"
#include "programtab.h"
#include "kumiredit.h"

#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>


void addToMap(QHash<QString, QString> *map,
	const QString &value, const int ksymbol)
{
	QStringList variants = value.split("|");
	foreach (QString variant, variants) {
		variant.remove("\\s+");
		variant.remove("\\s*");
		variant.remove("\\s");
		variant.remove("_");
		(*map)[variant] = QString(QChar(ksymbol));
	}
}

void KumTools::initNormalizator(const QString &fileName)
{

	QHash<QString, QString> *map = new QHash<QString, QString>();

	QFile kwdFile(fileName);
	if (kwdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream ts(&kwdFile);
		ts.setCodec("UTF-8");
		const QStringList lines = ts.readAll().split("\n", QString::SkipEmptyParts);
		kwdFile.close();
		foreach (const QString &line, lines) {
			if (line.startsWith("#")) {
				continue;
			}
			const QStringList pair = line.split("=");
			if (pair.size() >= 2) {
				const QString context = pair.at(0).simplified();
				const QString value = pair.at(1).simplified();
				if (context == "begin module") {
					keyWords << value;
					ispKwd = new QString(value);
					addToMap(map, value, KS_ISP);
				} else if (context == "end module") {
					keyWords << value;
					addToMap(map, value, KS_ENDISP);
				} else if (context == "include text") {
					keyWords << value;
					addToMap(map, value, KS_INCLUDE);
				} else if (context == "use module") {
					keyWords << value;
					useKwd = new QString(value);
					addToMap(map, value, KS_USE);
				} else if (context == "algorhitm header") {
					keyWords << value;
					addToMap(map, value, KS_ALG);
				} else if (context == "begin algorhitm implementation") {
					keyWords << value;
					addToMap(map, value, KS_NACH);
				} else if (context == "end algorhitm implementation") {
					keyWords << value;
					addToMap(map, value, KS_KON);
				} else if (context == "algorhitm pre-condition") {
					keyWords << value;
					addToMap(map, value, KS_DANO);
				} else if (context == "algorhitm post-condition") {
					keyWords << value;
					addToMap(map, value, KS_NADO);
				} else if (context == "assertion") {
					keyWords << value;
					addToMap(map, value, KS_UTV);
				} else if (context == "begin loop") {
					keyWords << value;
					addToMap(map, value, KS_NC);
				} else if (context == "end loop") {
					keyWords << value;
					addToMap(map, value, KS_KC);
				} else if (context == "conditional end loop") {
					keyWords << value;
					addToMap(map, value, KS_KC_PRI);
				} else if (context == "'for' loop type") {
					keyWords << value;
					addToMap(map, value, KS_DLYA);
				} else if (context == "'while' loop type") {
					keyWords << value;
					addToMap(map, value, KS_POKA);
				} else if (context == "'times' loop type") {
					keyWords << value;
					addToMap(map, value, KS_RAZ);
				} else if (context == "for loop 'from'") {
					keyWords << value;
					addToMap(map, value, KS_OT);
				} else if (context == "for loop 'to'") {
					keyWords << value;
					addToMap(map, value, KS_DO);
				} else if (context == "for loop 'step'") {
					keyWords << value;
					addToMap(map, value, KS_SHAG);
				} else if (context == "loop break and algorhitm return") {
					keyWords << value;
					addToMap(map, value, KS_EXIT);
				} else if (context == "if") {
					keyWords << value;
					addToMap(map, value, KS_ESLI);
				} else if (context == "then") {
					keyWords << value;
					addToMap(map, value, KS_TO);
				} else if (context == "else") {
					keyWords << value;
					addToMap(map, value, KS_INACHE);
				} else if (context == "end of 'if' or 'switch' statement") {
					keyWords << value;
					addToMap(map, value, KS_VSE);
				} else if (context == "switch") {
					keyWords << value;
					addToMap(map, value, KS_VIBOR);
				} else if (context == "case") {
					keyWords << value;
					addToMap(map, value, KS_PRI);
				} else if (context == "terminal input") {
					keyWords << value;
					addToMap(map, value, KS_VVOD);
				} else if (context == "terminal output") {
					keyWords << value;
					addToMap(map, value, KS_VIVOD);
				} else if (context == "file input") {
					keyWords << value;
					addToMap(map, value, KS_FVVOD);
				} else if (context == "file output") {
					keyWords << value;
					addToMap(map, value, KS_FVIVOD);
				} else if (context == "new line symbol") {
					keyWords << value;
					addToMap(map, value, KS_NS);
				} else if (context == "logical 'not'") {
					keyWords << value;
					addToMap(map, value, KS_NOT);
				} else if (context == "logical 'and'") {
					keyWords << value;
					addToMap(map, value, KS_AND);
				} else if (context == "logical 'or'") {
					keyWords << value;
					addToMap(map, value, KS_OR);
				} else if (context == "'write only' parameter modifier") {
					keyWords << value;
					addToMap(map, value, KS_REZ);
				} else if (context == "'read only' parameter modifier") {
					keyWords << value;
					addToMap(map, value, KS_ARG);
				} else if (context == "'read and write' parameter modifier") {
					keyWords << value;
					addToMap(map, value, KS_ARGREZ);
				} else if (context == "'true' constant value") {
					constNames << value;
					addToMap(map, value, KS_DA);
				} else if (context == "'false' constant value") {
					constNames << value;
					addToMap(map, value, KS_NET);
				} else if (context == "integer type name") {
					typeNames << value;
					addToMap(map, value, KS_CEL);
				} else if (context == "floating point type name") {
					typeNames << value;
					addToMap(map, value, KS_VES);
				} else if (context == "character type name") {
					typeNames << value;
					addToMap(map, value, KS_SIM);
				} else if (context == "string type name") {
					typeNames << value;
					addToMap(map, value, KS_LIT);
				} else if (context == "boolean type name") {
					typeNames << value;
					addToMap(map, value, KS_LOG);
				} else if (context == "integer array type name") {
					typeNames << value;
					addToMap(map, value, KS_CEL_TAB);
				} else if (context == "floating point array type name") {
					typeNames << value;
					addToMap(map, value, KS_VES_TAB);
				} else if (context == "character array type name") {
					typeNames << value;
					addToMap(map, value, KS_SIM_TAB);
				} else if (context == "string array type name") {
					typeNames << value;
					addToMap(map, value, KS_LIT_TAB);
				} else if (context == "boolean array type name") {
					typeNames << value;
					addToMap(map, value, KS_LOG_TAB);
				}

			}
		}
	}



	compounds2 << tr("\\*\\*");
	(*map)["**"] = QString(QChar(KS_STEPEN));

	compounds2 << ">=";
	(*map)[">="] = QString(QChar(KS_BOLSHE_RAVNO));

	compounds2 << "<=";
	(*map)["<="] = QString(QChar(KS_MENSHE_RAVNO));

	compounds2 << "<>";
	(*map)["<>"] = QString(QChar(KS_NE_RAVNO));

	compounds2 << "#";
	(*map)["#"] = QString(QChar(KS_DOC));

	compounds2 << ":=";
	(*map)[":="] = QString(QChar(KS_PRISV));

	compounds2 << "\\~";
	(*map)["~"] = QString(QChar(KS_TILDA));

	compounds2 << "&";
	(*map)["&"] = QString(QChar(KS_PAR));

	compounds2 << "%";
	(*map)["%"] = QString(QChar(KS_PERCENT));

	compounds2 << "\"";
	compounds2 << "'";
	compounds2 << QString::fromUtf8("”");
	compounds2 << "\\|";

	compounds += keyWords;
	compounds += typeNames;
	compounds += constNames;

	keyWords << tr("value");



//    for ( int i=0; i<keyWords.count(); i++ ) {
//        keyWords[i].replace("|","\\b|\\b");
//        keyWords[i] = "\\b"+keyWords[i]+"\\b";

//    }



//    for ( int i=0; i<typeNames.count(); i++ ) {
//        typeNames[i].replace("|","\\b|\\b");
//        typeNames[i] = "\\b"+typeNames[i]+"\\b";
//    }

//    for ( int i=0; i<constNames.count(); i++ ) {
//        constNames[i].replace("|","\\b|\\b");
//        constNames[i] = "\\b"+constNames[i]+"\\b";
//    }



	kwdMap = map;

//    for (int i=0; i<compounds.size(); i++) {
//        compounds[i].replace("|","\\b|\\b");
//    }

//    for (int i=0; i<compounds2.size(); i++) {
//        compounds2[i].replace("|","\\b|\\b");
//    }

	for (int i = 0; i < compounds.size(); i++) {
		compounds[i] = "\\b(" + compounds[i] + ")\\b";
	}

//    for (int i=0; i<compounds2.size(); i++) {
//        compounds2[i] = "\\b("+compounds2[i]+")\\b";
//    }
	QString compounds2Pattern = compounds2.join("|");

	QString compoundsPattern = compounds.join("|");
//    compoundsPattern.replace("|","\\b|\\b");
//    qDebug() << compoundsPattern;
//    qDebug() << compounds2Pattern;

	QString keywordsPattern = "\\b" + keyWords.join("|") + "\\b";
	keywordsPattern.replace("|", "\\b|\\b");

	QString typesPattern = "\\b" + typeNames.join("|") + "\\b";
	typesPattern.replace("|", "\\b|\\b");

	QString constsPattern = "\\b" + constNames.join("|") + "\\b";
	constsPattern.replace("|", "\\b|\\b");

	rxCompound = new QRegExp(compounds2.join("|") + "|" +
		compoundsPattern);
	rxTypes = new QRegExp(typesPattern);
	rxConst = new QRegExp(constsPattern);
	rxKeyWords = new QRegExp(keywordsPattern);

	rxKeyWords->setMinimal(false);
	rxCompound->setMinimal(false);
}

void KumTools::init()
{
	KSIMS = new QRegExp("[\\x3E8-\\x3EF]+|[\\x1100-\\x1200]+");
	Breaks = QString::fromUtf8("[+|№|\\^|$|;|=|\\?|*|~|`|\\-|\\\\|/|<|>|\\(|\\)|\\[|\\]|\\,|\\:|") + QString(QChar(KS_DOC)) + "|" +      //QString(QChar(KS_NOT))+
		QString(QChar(UNARY_MINUS)) + "|" +
		QString(QChar(KS_STEPEN)) + "|" + QString(QChar(KS_NE_RAVNO)) + "|"
		+ QString(QChar(KS_OR)) + "|" + QString(QChar(KS_AND)) + "|"
		+ QString(QChar(KS_BOLSHE_RAVNO)) + "|" + QString(QChar(KS_MENSHE_RAVNO)) + "|"
		+ QString(QChar(UNARY_PLUS)) + "|" + QString(QChar(FOR_CIRCLE)) + "|"
		+ QString(QChar(RAZ_CIRCLE)) + "|" + QString(QChar(WHILE_CIRCLE)) + "|"
		+ QString(QChar(KS_STEPEN)) + "|" + QString(QChar(KS_BOLSHE_RAVNO)) + "|"
		+ QString(QChar(KS_MENSHE_RAVNO)) + "|" + QString(QChar(KS_NE_RAVNO)) + "|"
		+ "|" + QString(QChar(KS_OR)) + "|" + QString(QChar(KS_AND)) + "|"
		+ QString(QChar(KS_PRISV)) + "|" + QString(QChar(KS_CEL)) + "|"
		+ QString(QChar(KS_CEL_TAB)) + "|" + QString(QChar(KS_VES)) + "|"
		+ QString(QChar(KS_VES_TAB)) + "|" + QString(QChar(KS_LIT)) + "|"
		+ QString(QChar(KS_LIT_TAB)) + "|" + QString(QChar(KS_SIM)) + "|"
		+ QString(QChar(KS_SIM_TAB)) + "|" + QString(QChar(KS_LOG)) + "|"
		+ QString(QChar(KS_LOG_TAB)) + "|" + QString(QChar(KS_VSE)) + "|"
		+ QString(QChar(KS_ESLI)) + "|" + QString(QChar(KS_TO)) + "|"
		+ QString(QChar(KS_INACHE)) + "|" + QString(QChar(KS_NACH)) + "|"
		+ QString(QChar(KS_KON)) + "|"
		+ QString(QChar(KS_SHAG)) + "|"
		+ QString(QChar(KS_NC)) + "|" + QString(QChar(KS_KC)) + "|"
		+ QString(QChar(KS_DLYA)) + "|" + QString(QChar(KS_RAZ)) + "|"
		+ QString(QChar(KS_POKA)) + "|" + QString(QChar(KS_MALG)) + "|"
		+ QString(QChar(KS_ALG)) + "|" + QString(QChar(KS_DO)) + "|"
		+ QString(QChar(KS_OT)) + "|" + QString(QChar(KS_PRI)) + "|"
		+ QString(QChar(KS_ARG)) + "|" + QString(QChar(KS_REZ)) + "|" + QString(QChar(KS_ARGREZ)) + "|"
		+ QString(QChar(KS_VVOD)) + "|" + QString(QChar(KS_VIVOD)) + "|"
		+ QString(QChar(KS_UTV)) + "|" + QString(QChar(KS_DANO)) + "|"
		+ QString(QChar(KS_NADO)) + "|" + QString(QChar(KS_KC_PRI)) + "|"
		+ QString(QChar(KS_EXP_PLUS)) + "|" + QString(QChar(KS_EXP_MINUS)) + "|"
		+ QString(QChar(KS_NS)) + "|" + QString(QChar(KS_DOC)) + "|" + QString(QChar(KS_NOT))
		+ "|" + QString(QChar(KS_AT)) + "|" + QString(QChar(KS_TILDA)) + "|"
		+ "|" + QString(QChar(KS_PERCENT)) + "|" + QString(QChar(KS_DOLLAR)) + "|"
		+ "|" + QString(QChar(KS_PAR)) + "|" + QString(QChar(KS_DOC)) + "|"
		+ "|" + QString(QChar(KS_VIBOR)) /*+"|\\^"*/ + "]|(\\^\\d+)";

	brks = QRegExp(Breaks);
	rxLitBreaks = QRegExp(QString::fromUtf8(
				"\\b(цел[\\s]+таб|цел|целтаб)\\b|\\b(вещ[\\s]+таб|вещ|вещтаб)\\b|\\b(лит[\\s]+таб|лит[\\s]+|литтаб)\\b|\\b(сим[\\s]+таб|сим|симтаб)\\b|\\b(лог[\\s]+таб|лог|логтаб)\\b|<=|>=|\\$|:=|\\\\|#|\\*\\*|\\*|:|&|\\?|~|%|\\^|`|;|=|�� |№|<|≤|≥|>|<>|\\[|\\]|\\(|\\)|,|-|/|\\+|\\bвсе\\b|\\bалг\\b|\\bесли\\b|\\bто\\b|\\bиначе\\b|\\bнач\\b|\\bкон\\b|\\bшаг\\b|\\bнц\\b|\\bкц\\b|\\bдля\\b|\\bраз\\b|\\bпока\\b|\\bдо\\b|\\bот\\b|\\bарг\\s+рез\\b|\\bарг\\b|\\bрез\\b|\\bаргрез\\b|\\bввод\\b|\\bвывод\\b|\\bутв\\b|\\bдано\\b|\\bнадо\\b|\\bкц_при\\b|\\bвыбор\\b|\\bи\\b|\\bили\\b|\\bпри\\b|\\bнс\\b|\\bиспользовать\\b|\\bисп\\b|\\bкон_исп\\b"
			));

	rx_NOT = QRegExp("\\b" + trUtf8("не") + "\\b");
	rx_OR = QRegExp("\\b" + trUtf8("или") + "\\b");
	rx_AND = QRegExp("\\b" + trUtf8("и") + "\\b");
	rx_CEL_TAB = QRegExp("\\b" + trUtf8("цел таб") + "\\b");
	rx_LIT_TAB = QRegExp("\\b" + trUtf8("лит таб") + "\\b");
	rx_VES_TAB = QRegExp("\\b" + trUtf8("вещ таб") + "\\b");
	rx_SIM_TAB = QRegExp("\\b" + trUtf8("сим таб") + "\\b");
	rx_LOG_TAB = QRegExp("\\b" + trUtf8("лог таб") + "\\b");
	rx_CEL = QRegExp("\\b" + trUtf8("цел") + "\\b");
	rx_LIT = QRegExp("\\b" + trUtf8("лит") + "\\b");
	rx_VES = QRegExp("\\b" + trUtf8("вещ") + "\\b");
	rx_SIM = QRegExp("\\b" + trUtf8("сим") + "\\b");
	rx_LOG = QRegExp("\\b" + trUtf8("лог") + "\\b");
	rx_ESLI = QRegExp("\\b" + trUtf8("если") + "\\b");
	rx_TO = QRegExp("\\b" + trUtf8("то") + "\\b");
	rx_INACHE = QRegExp("\\b" + trUtf8("иначе") + "\\b");
	rx_VSE = QRegExp("\\b" + trUtf8("все") + "\\b");
	rx_VIBOR = QRegExp("\\b" + trUtf8("выбор") + "\\b");
	rx_PRI = QRegExp("\\b" + trUtf8("при") + "\\b");
	rx_ALG = QRegExp("\\b" + trUtf8("алг") + "\\b");
	rx_ARG = QRegExp("\\b" + trUtf8("арг") + "\\b");
	rx_NACH = QRegExp("\\b" + trUtf8("нач") + "\\b");
	rx_KON = QRegExp("\\b" + trUtf8("кон") + "\\b");
	rx_NC = QRegExp("\\b" + trUtf8("нц") + "\\b");
	rx_KC = QRegExp("\\b" + trUtf8("кц") + "\\b");
	rx_DLYA = QRegExp("\\b" + trUtf8("для") + "\\b");
	rx_RAZ = QRegExp("\\b" + trUtf8("раз") + "\\b");
	rx_POKA = QRegExp("\\b" + trUtf8("пока") + "\\b");
	rx_DANO = QRegExp("\\b" + trUtf8("дано") + "\\b");
	rx_NADO = QRegExp("\\b" + trUtf8("надо") + "\\b");
	rx_EXIT = QRegExp("\\b" + trUtf8("выход") + "\\b");
	rx_NS = QRegExp("\\b" + trUtf8("нс") + "\\b");
	rx_ISP = QRegExp("\\b" + trUtf8("исп") + "\\b");
	rx_ENDISP = QRegExp("\\b" + trUtf8("кон_исп") + "\\b");
	rx_OT = QRegExp("\\b" + trUtf8("от") + "\\b");
	rx_DO = QRegExp("\\b" + trUtf8("до") + "\\b");
	rx_SHAG = QRegExp("\\b" + trUtf8("шаг") + "\\b");
	rx_REZ = QRegExp("\\b" + trUtf8("рез") + "\\b");
	rx_ARGREZ = QRegExp("\\b" + trUtf8("аргрез") + "\\b");
	rx_VIVOD = QRegExp("\\b" + trUtf8("вывод") + "\\b");
	rx_DA = QRegExp("\\b" + trUtf8("да") + "\\b");
	rx_NET = QRegExp("\\b" + trUtf8("нет") + "\\b");
	rx_UTV = QRegExp("\\b" + trUtf8("утв") + "\\b");

	st_CEL = QString(QChar(KS_CEL)) + " ";
	st_LIT = QString(QChar(KS_LIT)) + " ";
	st_SIM = QString(QChar(KS_SIM)) + " ";
	st_VES = QString(QChar(KS_VES)) + " ";
	st_LOG = QString(QChar(KS_LOG)) + " ";

	kwdMap = NULL;
	rxCompound = NULL;

	m_urlFetcher = new UrlFetcher(this);
}

KumTools::KumTools()
{
	init();
	m_allowHiddenNames = false;
}

int KumTools::getKSCount(int pos, QString stroka)
{
	QString ksims = "[+|,|=|~|**|*|\\-|/|<|>|\\(|\\)|\\[|\\]|" +
		QString(QChar(KS_STEPEN)) + "|" + QString(QChar(KS_NE_RAVNO)) + "|"
		+ QString(QChar(KS_OR)) + "|" + QString(QChar(KS_AND)) + "|" + QString(QChar(KS_NOT)) + "|" + QString(QChar(KS_BOLSHE_RAVNO)) + "|"
		+ QString(QChar(KS_MENSHE_RAVNO)) + "|" + QString(QChar(KS_PRISV)) + "]";

	QRegExp operators;
	operators = QRegExp(ksims);
	int i = 0;
	int pos_t = 0;
	int count = 0;
	if (pos == -1) {
		pos = stroka.length();
	}
	while (i < pos) {
		pos_t = operators.indexIn(stroka, pos_t + 1);
		if (pos_t == -1) {
			return count;
		}
		count++;
		i = pos_t;

	};
	return count;


};


int KumTools::getPosByKS(int ks_id, QString stroka)
{
	QString ksims = "[+|,|=|~|*|\\-|/|<|>|\\(|\\)|\\[|\\]|" +
		QString(QChar(KS_STEPEN)) + "|" + QString(QChar(KS_NE_RAVNO)) + "|"
		+ QString(QChar(KS_OR)) + "|" + QString(QChar(KS_AND)) + "|" + QString(QChar(KS_NOT)) + "|" + QString(QChar(KS_BOLSHE_RAVNO)) + "|"
		+ QString(QChar(KS_MENSHE_RAVNO)) + "|" + QString(QChar(KS_PRISV)) + "]";

	QRegExp operators;
	operators = QRegExp(ksims);
	int i = 0;
	int pos = 0;
	while (i < ks_id) {
		pos = operators.indexIn(stroka, pos + 1);
		if (pos == -1) {
			return -1;
		}
		i++;

	};
	return pos;
};


/**
 * Замена ключевых слов на Ксимволы в Строке
 * @param string Строка
 * @return Код ошибки
 */
QString KumTools::zamena(QString string)
{


	// int comment=string.indexOf("|");
	// string=string.left(comment);

	string.replace("**", QChar(KS_STEPEN));
	string.replace(">=", QChar(KS_BOLSHE_RAVNO));
	string.replace("<=", QChar(KS_MENSHE_RAVNO));
	string.replace("<>", QChar(KS_NE_RAVNO));
	string.replace("#", QChar(KS_DOC));

	// Если строка документации,
	// то выход, иначе в тексте строки
	// документации появятся ксимволы
	if (string.startsWith(QChar(KS_DOC))) {
		return string;
	}



	QString t_string = "";
	string.replace(rx_NOT, QChar(KS_NOT));

	t_string = "\\b" + trUtf8("да") + "\\b";
	QRegExp rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_DA));

	t_string = "\\b" + trUtf8("использовать") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_USE));

	t_string = "\\b" + trUtf8("нет") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_NET));

	t_string = "\\b" + trUtf8("вещтаб") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_VES_TAB));

	t_string = "\\b" + trUtf8("целтаб") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_CEL_TAB));

	t_string = "\\b" + trUtf8("логтаб") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_LOG_TAB));

	t_string = "\\b" + trUtf8("симтаб") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_SIM_TAB));

	t_string = "\\b" + trUtf8("литтаб") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_LIT_TAB));


	t_string = "\\b" + trUtf8("ввод") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_VVOD));

	t_string = "\\b" + trUtf8("ф_ввод") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_FVVOD));

	t_string = "\\b" + trUtf8("ф_вывод") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_FVIVOD));

	t_string = "\\b" + trUtf8("кц_при") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_KC_PRI));

	t_string = "\\b" + trUtf8("нс") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_NS));


	string.replace(":=", QChar(KS_PRISV));

	string.replace(rx_OR, QChar(KS_OR));

	string.replace(rx_AND, QChar(KS_AND));

	string.replace(rx_CEL_TAB, QChar(KS_CEL_TAB));

	string.replace(rx_LIT_TAB, QChar(KS_LIT_TAB));

	string.replace(rx_VES_TAB, QChar(KS_VES_TAB));

	string.replace(rx_SIM_TAB, QChar(KS_SIM_TAB));

	string.replace(rx_LOG_TAB, QChar(KS_LOG_TAB));

	string.replace(rx_DANO, QChar(KS_DANO));

	string.replace(rx_NADO, QChar(KS_NADO));

	int pos1 = 0;
	int len = 0;
	int pos2 = 0;
	//   QRegExp rx_CEL_Func = QRegExp(trUtf8("цел\\s*\\("));
	//   while ( pos1 < string.length() )
	//   {
	//     pos1 = rx_CEL.indexIn(string,pos1);
	//     if ( pos1 == -1 )
	//       break;
	//     pos2 = rx_CEL_Func.indexIn(string,pos1);
	//     len = rx_CEL.matchedLength();
	//     if ( pos1 != pos2 )
	//     {
	//       string.replace(pos1,len,st_CEL);
	//     }
	//     pos1 += len;
	//   }

	string.replace(rx_CEL, st_CEL);

	pos1 = 0;
	len = 0;
	pos2 = 0;
	QRegExp rx_LIT_Func = QRegExp(trUtf8("лит\\s*\\("));
	int length = string.length();
	while (pos1 < length) {
		pos1 = rx_LIT.indexIn(string, pos1);
		if (pos1 == -1) {
			break;
		}
		pos2 = rx_LIT_Func.indexIn(string, pos1);
		len = rx_LIT.matchedLength();
		if (pos1 != pos2) {
			string.replace(pos1, len, st_LIT);
		}
		pos1 += len;
	}

	//   string.replace(rx_LIT,st_LIT);
	t_string = "\\b" + trUtf8("арг рез") + "\\b";
	rx = QRegExp(t_string);
	string.replace(rx, QChar(KS_ARGREZ));

	string.replace(rx_VES, st_VES);

	string.replace(rx_SIM, st_SIM);

	string.replace(rx_LOG, st_LOG);

	string.replace(rx_ESLI, QChar(KS_ESLI));

	string.replace(rx_TO, QChar(KS_TO));

	string.replace(rx_INACHE, QChar(KS_INACHE));

	string.replace(rx_VSE, QChar(KS_VSE));

	string.replace(rx_VIBOR, QChar(KS_VIBOR));

	string.replace(rx_PRI, QChar(KS_PRI));


	string.replace(rx_ALG, QChar(KS_ALG));

	string.replace(rx_ARG, QChar(KS_ARG));

	string.replace(rx_NACH, QChar(KS_NACH));

	string.replace(rx_KON, QChar(KS_KON));

	string.replace(rx_NC, QChar(KS_NC));

	string.replace(rx_KC, QChar(KS_KC));

	string.replace(rx_DLYA, QChar(KS_DLYA));

	string.replace(rx_RAZ, QChar(KS_RAZ));

	string.replace(rx_POKA, QChar(KS_POKA));

	string.replace(rx_EXIT, QChar(KS_EXIT));

	string.replace(rx_ISP, QChar(KS_ISP));
	string.replace(rx_ENDISP, QChar(KS_ENDISP));

	string.replace(rx_OT, QChar(KS_OT));


	//string.replace("@",QChar(KS_AT));


	string.replace("~", QChar(KS_TILDA));

	//string.replace("$",QChar(KS_DOLLAR));
	string.replace("&", QChar(KS_PAR));

	string.replace("%", QChar(KS_PERCENT));

	string.replace(rx_DO, QChar(KS_DO));

	string.replace(rx_SHAG, QChar(KS_SHAG));

	string.replace(rx_REZ, QChar(KS_REZ));

	string.replace(rx_ARGREZ, QChar(KS_ARGREZ));

	string.replace(rx_VIVOD, QChar(KS_VIVOD));

	string.replace(rx_DA, QChar(KS_DA));

	string.replace(rx_NET, QChar(KS_NET));

	string.replace(rx_UTV, QChar(KS_UTV));

	string.replace("; ", ";");


	//Замена русских букв на латинские схожие по начертанию



	return string;
};

int KumTools::ks_count(QString string, int begin, int end)
{
	string = string.mid(begin, end - begin);
	QRegExp break_rx = QRegExp(QString("[+|*|\\-|/|~|)|(|=|,|" + QString(QChar(KS_ESLI))
				+ "|>|<|" + QString(QChar(KS_TO)) + "|" + QString(QChar(KS_AND)) + "]"));

	return string.count(break_rx);

};


/**
 * Нормализация строки
 * @param src IN: строка исходного Кумир-текста
 * @param P OUT: вектор начальных позиций нормализованных символов в исходной строке
 * @param L OUT: вектор длин нормализованных символов в исходной строке
 * @param error OUT: код ошибки при нормализации
 * @param err_start OUT: позиция начала ошибки в исходной строке
 * @param err_len OUT: длина ошибочной области в исходной строке
 * @return нормализованная строка
 */
QString KumTools::normalize(const QString &src, QList<int> &P, QList<int> &L, int &error, int &err_start, int &err_len)
{
	// различные проверки

	Q_CHECK_PTR(rxCompound);
	Q_CHECK_PTR(kwdMap);
	K_ASSERT(rxCompound->isValid());
	K_ASSERT(!rxCompound->pattern().isEmpty());


	/*
	    Имеется регулярное выражение (rxCompound, инициализируется в initNormalizator),
	    описывающее все составные ксимволы, а также символы «кавычка» и «комментарий».

	    В цикле по длине строки выполняется поиск позиций подстрок в соответствии с
	    данным регулярным выражением, при этом каждый i-ый поиск даёт два числа:
	    позиция найденного вхождения  p_i и его длина l_i.

	    При этом строится нормализованная строка и соответствующие вектора [P] и [L]
	    в соответствии с таблицей замен (kwdMap, инициализируется в initNormalizator).

	    Если найденное вхождение находится в литеральной константе, то выполняется сдвиг
	    кода символа на STR_HIDE_OFFSET.

	    Если найден символ комментария, то цикл останавливается. Если позиция p_i
	    отличается от p_{i-1} больше чем на 1, то нормализованная строка а также вектора
	    заполняются символами исходной строки начиная с позиции p_{i-1} + l_{i-1} до позиции p_{i-1},
	    после чего добавляется соответствующий ксимвол.
	    */
	// знач
	QString result = "";
	// флаг "внутри литеральной константы"
	bool inLit = false;
	// символ используемой кавычки в литеральной константе
	QChar litSymb;
	// текущая и предыдущая позиции найденного вхождения
	int cur, prev;
	prev = -1;
	// флаг "пропускать пробелы"
	bool skipSpaces = true;
	// инициализация векторов (пустые)
	P = QList<int>();
	L = QList<int>();
	// инициализация переменных
	error = 0;
	err_start = 0;
	err_len = 0;
	// если строка пустая, то выходим
	if (src.isEmpty()) {
		return result;
	}
	// бексонечный цикл (останавливается изнутри оператором break)
	forever {
		// поиск следующего вхождения составного ксимвола, кавычки или комментария
		cur = rxCompound->indexIn(src, prev + 1);
		if (cur != -1) {
			// если такой найден...
			// проверяем, нет ли между этим и предыдущим ксимволом чего-нибудь
			if (cur - prev > 1) {
				// здесь обрабатываются все символы между текущим
				// и предыдущим ксимволом

				if (inLit) {
					// случай, когда находимся в литеральной константе

					// добавляем все символы из исходной строки, находящиеся
					// в области [prev+1; cur)

					// при этом заменяем коды их символов
					for (int j = prev + 1; j < cur; j++) {
						result += QString(QChar(src[j].unicode() + STR_HIDE_OFFSET));
						P << j;
						L << 1;
					}
				} else {
					// случай, когда вне литеральной константы

					// всё то же самое, только коды символов оставляем незатронутыми,
					// при этом нормализуем пробелы
					for (int j = prev + 1; j < cur; j++) {
						if (src[j] == ' ' || src[j] == '\t') {
							if (!skipSpaces) {
								result += QString(src[j]);
								P << j;
								L << 1;
							}
							skipSpaces = true;
						} else {
							result += QString(src[j]);
							P << j;
							L << 1;
							if (src[j] == ';') {
								skipSpaces = true;
							} else {
								skipSpaces = false;
							}
						}
					}
				}
			}

			// Теперь разбираемся с текущим ксимволом.
			// Проверяем, не находится ли он в литеральной константе...
			if (inLit) {
				// если найденное вхождение находится в литеральной константе

				// сначала проверяем, не является ли данный символ закрывающей кавычкой
				QString symb = rxCompound->cap(0);
				if (symb == "\"" || symb == "'" || symb == trUtf8("”")) {
					if (symb[0] == litSymb) {
						// найденный символ является закрывающей кавычкой

						// вне зависимости от исходной кавычки,
						// в нормализованной строке кавычка -- это "
						result += "\"";
						P << cur;
						L << 1;
						// сбрасываем флаг "внутри литеральной константы"
						inLit = false;
					} else {
						// если найденная кавычка не является закрывающей,
						// то добавляем её со сдвигом кода символа
						// и флаг "внутри литеральной константы" НЕ сбрасываем
						result += QString(QChar(src[cur].unicode() + STR_HIDE_OFFSET));
						P << cur;
						L << 1;
					}
				} else {
					// если найденный символ не является никакой кавычкой

					// поскольку мы находимся внутри литеральной константы,
					// то нормализовать ключевые слова и пр. не нужно

					// добавляем их со сдвигом кода символа
					// (при этом учитываем, что длина найденного вхождения
					// может быть больше одного символа)
					for (int j = cur; j < cur + rxCompound->matchedLength(); j++) {
						result += QString(QChar(src[j].unicode() + STR_HIDE_OFFSET));
						P << j;
						L << 1;
					}
				}
			} else {
				// случай, когда найденное вхождение ВНЕ литеральной константы

				// проверяем, не является ли найденный символ признаком комментария
				QString symb = rxCompound->cap(0);
				if (symb == "|") {
					// если комментарий, то завершаем нормализацию этой строки
					break;
				}

				else if (symb == "#") {
					result += QChar(KS_DOC) + src.mid(cur + 1);
					P << cur;
					L << 1;
					for (int j = cur + 1; j < src.length(); j++) {
						P << j;
						L << 1;
					}
					break;
				}

				// проверяем на открывающую кавычку
				else if (symb == "\"" || symb == "'" || symb == trUtf8("”")) {
					// запоминаем, какая кавычка у нас является открывающей
					litSymb = symb[0];
					// добавляем в нормализованную строку "
					result += "\"";
					P << cur;
					L << 1;
					// устанавливаем флаг "внутри литеральной константы"
					inLit = true;
				}

//                else if (symb.isEmpty()) {
//                    cur++;
//                }

				// самый общий случай -- не кавычка и не комментарий
				else {
					// выполняем поиск в хеш-таблице замен и вставляем нужный символ
					symb.remove(' ');
					symb.remove('_');
					const QString ksymbol = (*kwdMap)[symb];
					K_ASSERT(ksymbol.length() > 0);
					result += ksymbol;
					// запоминаем позицию найденного вхождения...
					P << cur;
					// ... и его длину в ИСХОДНОЙ строке
					L << rxCompound->matchedLength();
				}
			}

			// в завершении всего, запоминаем позицию, откуда искать на следующей итерации цикла

			// запоминаем предыдущую позицию, как указывающую на ПОСЛЕДНИЙ символ текущего
			// найденного вхождения
			prev = cur + (rxCompound->matchedLength() - 1);
		}


		else {
			// здесь обрабавывается остаток строки, когда составные символы уже не находятся

			// проверка на литеральную константу
			if (inLit) {
				// добавляем остаток строки со сдвигом кодов символов
				for (int j = prev + 1; j < src.length(); j++) {
					result += QString(QChar(src[j].unicode() + STR_HIDE_OFFSET));
					P << j;
					L << 1;
				}

				// поскольку литеральная константа в конце -- это явная ошибка
				// (закрывающая кавычка уже точно не найдётся), то нужно отругаться

				// ищем с конца открывающую кавычку и отмечаем её ошибкой
				// "Непарная кавычка"
				err_start = 0;
				err_len = 1;
				error = BAD_KAVICHKA;
				for (int j = src.length() - 1; j >= 0; j--) {
					if (src[j] == litSymb) {
						err_start = j;
						break;
					}
				}
			} else {

				// остаток строки ВНЕ литеральной константы

				// добавляем "как есть" в нормализованную строку
				// (с нормализацией пробелов)
				for (int j = prev + 1; j < src.length(); j++) {
					if (src[j] == ' ' || src[j] == '\t') {
						if (!skipSpaces) {
							QString sp = QString(src[j]);
							if (sp == "\t") {
								sp = " ";
							}
							result += QString(src[j]);
							P << j;
							L << 1;
						}
						skipSpaces = true;
					} else {
						result += QString(src[j]);
						P << j;
						L << 1;
						if (src[j] == ';') {
							skipSpaces = true;
						} else {
							skipSpaces = false;
						}
					}
				}
			}

			// поскольку больше искать нечего, прерываем основной цикл
			break;
		}
		skipSpaces = false;
	}



	// зачистка от начальных и конечных пробелов
	while (result.startsWith(" ") || result.startsWith("\t")) {
		result.remove(0, 1);
		P.pop_front();
		L.pop_front();
	}

	while (result.endsWith(" ") || result.endsWith("\t")) {
		result.chop(1);
		P.pop_back();
		L.pop_back();
	}

	// возвращаем результат
//    qDebug() << result;
	return result;
}


QString KumTools::normalizeString(QString string, int *err, int *err_start, int *err_end)
{



	int cav_pos;


	QString t_string = trUtf8("\'|\"|”");
	QRegExp rx = QRegExp(t_string);
	cav_pos = rx.indexIn(string, 0);


	if (cav_pos == -1) {
		int comment_pos = string.indexOf('|', 0);
		if (comment_pos > -1) {
			string = string.left(comment_pos);
		}
		string = string.simplified();
		string = zamena(string);
		return string;
	};

	QChar cav;
	cav = string[cav_pos];


	int cav_pos1 = 0;
	int cav_pos2 = 0;
	bool first = true;
	while (cav_pos2 > -1) {
		if (first) {
			cav_pos2 = -1;
		}
		first = false;//TODO RAZOBRATSYA
		t_string = "\'|\"|”";
		rx = QRegExp(t_string);
		cav_pos = rx.indexIn(string, cav_pos2 + 1);
		if (cav_pos == -1) {

			int comment_pos = string.indexOf('|', 0);
			if (comment_pos > -1) {
				string = string.left(comment_pos);
			}
			break;
		};
		cav = string[cav_pos];
		int comment_pos;
		if (cav_pos2 > 0) {
			comment_pos = string.indexOf('|', cav_pos2);
		} else {
			comment_pos = string.indexOf('|', 0);
		}
		cav_pos1 = string.indexOf(cav, cav_pos2 + 1);
		cav_pos2 = string.indexOf(cav, cav_pos1 + 1);

		if (comment_pos > cav_pos2) {
			if (cav_pos2 > -1) {
				for (int i = cav_pos1 + 1; i < cav_pos2; i++) {
					string[i] = QChar(string[i].unicode() + STR_HIDE_OFFSET);
				};

				string[cav_pos1] = '\"';
				string[cav_pos2] = '\"';

				int test_cav = rx.indexIn(string, cav_pos2 + 1);
				if ((test_cav < comment_pos) && (test_cav > -1)) {
					continue;
				} else {
					string = string.left(comment_pos);
					break;
				};
			} else {
				string = string.left(comment_pos);
				if (cav_pos1 < comment_pos) {
					*err_start = cav_pos1;
					*err_end = cav_pos1 + 1;
					*err = BAD_KAVICHKA;
					string = zamena(string);
					return string;
				};
				break;
			};
		};

		if ((comment_pos < cav_pos1) && (comment_pos > -1)) {
			string = string.left(comment_pos);
			break;
		};


		if (cav_pos2 == -1) {
			if (comment_pos > cav_pos1) {
				string = string.left(comment_pos);
				string = zamena(string);
				return string;
			};
			*err_start = cav_pos;
			*err_end = cav_pos + 1;
			*err = BAD_KAVICHKA;
			string = zamena(string);
			return string;
		};
		if ((cav_pos1 == -1) || (cav_pos2 == -1)) {
			break;
		}
		for (int i = cav_pos1 + 1; i < cav_pos2; i++) {
			string[i] = QChar(string[i].unicode() + STR_HIDE_OFFSET);
		};
		string[cav_pos1] = '\"';
		string[cav_pos2] = '\"';
	}
	;//while

	string = string.simplified();


	string = zamena(string);  //замена ксимволов на коды


	return string;
};

/**
 * Определение типа строки
 * @param string Строка
 * @return Тип
 */
Kumir::InstructionType KumTools::getStringType(const QString &string)
{
	uint first_b = string[0].unicode();
	if (string.startsWith(trUtf8("|"))) {
		return Kumir::Comment;
	};
	if (first_b == KS_INCLUDE) {
		return Kumir::Include;
	}
	if (first_b == KS_ALG) {
		return Kumir::AlgDecl;
	}
	if (first_b == KS_NACH) {
		return Kumir::AlgBegin;
	}
	if (first_b == KS_KON) {
		return Kumir::AlgEnd;
	}
	if (first_b == KS_CEL) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_CEL_TAB) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_LIT) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_LIT_TAB) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_SIM) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_SIM_TAB) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_LOG_TAB) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_LOG) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_VES) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_VES_TAB) {
		return Kumir::VarDecl;
	}
	if (first_b == KS_NC) {
		return Kumir::LoopBegin;
	}
	if (first_b == KS_KC) {
		return Kumir::LoopEnd;
	}
	if (first_b == KS_ESLI) {
		return Kumir::If;
	}
	if (first_b == KS_VIBOR) {
		return Kumir::Switch;
	}
	if (first_b == KS_PRI) {
		return Kumir::Case;
	}
	if (first_b == KS_TO) {
		return Kumir::Then;
	}
	if (first_b == KS_VSE) {
		return Kumir::Fin;
	}
	if (first_b == KS_VIVOD) {
		return Kumir::Output;
	}
	if (first_b == KS_FVIVOD) {
		return Kumir::FileOutput;
	}
	if (first_b == KS_FVVOD) {
		return Kumir::FileInput;
	}
	if (first_b == KS_INACHE) {
		return Kumir::Else;
	}
	if (first_b == KS_UTV) {
		return Kumir::Assert;
	}
	if (first_b == KS_DANO) {
		return Kumir::Pre;
	}
	if (first_b == KS_NADO) {
		return Kumir::Post;
	}
	if (first_b == KS_DOC) {
		return Kumir::Doc;
	}
	if (first_b == KS_VVOD) {
		return Kumir::Input;
	}
	if (first_b == KS_KC_PRI) {
		return Kumir::LoopEndCond;
	}
	if (first_b == KS_EXIT) {
		return Kumir::Exit;
	}
	if (first_b == KS_ISP) {
		return Kumir::ModuleBegin;
	}
	if (first_b == KS_ENDISP) {
		return Kumir::ModuleEnd;
	}
	if (first_b == KS_USE) {
		return Kumir::UseModule;
	}
	if (string.count(QChar(KS_PRISV)) >= 1) {
		return Kumir::Assign;
	}
	if (string.isEmpty()) {
		return Kumir::Empty;
	}
	if (string.isNull()) {
		return Kumir::Empty;
	}
	return Kumir::CallAlg;
}


int findMinSkobka(QString stroka)
{
	int test = stroka.indexOf('(');
	if (test < 0) {
		return -1;
	}
	int skobk_count = 0, skobk_pos = -1, min_skobk = -1;
	skobk_count = 0;
	for (int i = 0; i <= stroka.length(); i++) {
		if (stroka[i] == QChar('(')) {
			skobk_count++;
		}
		if (stroka[i] == QChar(')')) {
			skobk_count--;
		}
		if (min_skobk < skobk_count) {
			min_skobk = skobk_count;
			skobk_pos = i;
		};
	};

	return skobk_pos;

};





int KumTools::test_skobk(QString stroka)
{
	int count = 0;
	bool kavichka = false;
	for (int i = 0; i <= stroka.length(); i++) {
		if (stroka[i] == QChar('\"')) {
			kavichka = !kavichka;
		}
		if ((stroka[i] == QChar('(')) && (!kavichka)) {
			count++;
		}
		if ((stroka[i] == QChar(')')) && (!kavichka)) {
			count--;
		}
		if (count < 0) {
			return -1;
		}
	}
	if (!kavichka) {
		return count;
	} else {
		return -1;
	}

}

int KumTools::test_name(QString name, int &pos, int &len)
{

	pos = 0;
	len = 0;
	if (name.isEmpty()) {
		return 0;
	}

	/*
	    TN_BAD_NAME_1 "Плохой символ в имени"
	    TN_BAD_NAME_1A "Плохой символ в имени"
	    TN_BAD_NAME_2 "Имя не может нач. с цифры"
	    TN_BAD_NAME_3 "Ключевое слово в имени"
	    TN_BAD_NAME_4 "Непарная \""
	    */

	QString pattern = QString::fromUtf8("[+\\-=:*&?/><#%()\\^$.,");
	if (!m_allowHiddenNames) {
		pattern += "!";
	}
	pattern += QString::fromUtf8("|№\\[\\]{}~`\\\\]");

	QRegExp rxSym = QRegExp(pattern);

	K_ASSERT(rxSym.isValid());

	int st = 0;
	while ((st < name.length()) && (name[st] == ' ' || name[st] == '\t')) {
		st ++ ;
	}

	if (name[st].isDigit()) {
		pos = st;
		len = 0;
		for (int i = st; i < name.length(); i++) {
			if (name[i].isDigit()) {
				len++;
			} else {
				break;
			}
		}
		return TN_BAD_NAME_2;
	}

	if (name.count("\"") % 2) {
		pos = name.indexOf("\"");
		len = 1;
		return TN_BAD_NAME_4;
	}

	QRegExp rxKwd = QRegExp(trUtf8("\\bзнач\\b|\\bтаб\\b"));

	int ps; // позиция первого найденного неправильного символа
	int pks; // позиция первого найденного ксимвола

	ps = rxSym.indexIn(name);
	QString found = rxSym.cap();
	QRegExp KS = QRegExp("[\\x3E8-\\x3EF]+|[\\x1100-\\x1200]+");
	pks = KS.indexIn(name);

	int error = 0;

	if (ps != -1) {
		error = TN_BAD_NAME_1;
		pos = ps;
		len = rxSym.matchedLength();
	}

	ps = name.indexOf(QChar(KS_BOLSHE_RAVNO));
	if (ps != -1) {
		error = TN_BAD_NAME_1;
		pos = ps;
		len = 1;
	}

	ps = name.indexOf(QChar(KS_MENSHE_RAVNO));
	if (ps != -1) {
		error = TN_BAD_NAME_1;
		pos = ps;
		len = 1;
	}

	if (pks != -1) {
		QChar debug = name[pks];
		ushort debug_code = debug.unicode();
		Q_UNUSED(debug_code);
		if (error > 0) {
			if (pks < ps) {
				error = TN_BAD_NAME_3;
				pos = pks;
				len = KS.matchedLength();
			}
		} else {
			error = TN_BAD_NAME_3;
			pos = pks;
			len = KS.matchedLength();
		}
	}




	pks = rxKwd.indexIn(name);

	if (pks != -1) {
		QChar debug = name[pks];
		ushort debug_code = debug.unicode();
		Q_UNUSED(debug_code);
		if (error > 0) {
			if (pks < ps) {
				error = TN_BAD_NAME_3;
				pos = pks;
				len = rxKwd.matchedLength();
			}
		} else {
			error = TN_BAD_NAME_3;
			pos = pks;
			len = rxKwd.matchedLength();
		}
	}


	if (error == TN_BAD_NAME_3) {
		uint code = KS.cap() [0].unicode();
		if (code >= 4503 && code <= 4508) {
			// Нормализованный обычный символ
			error = TN_BAD_NAME_1A;
		}
	}

	return error;

	if (name.contains("\"") && !name.startsWith("\"") && (name.count("\"") % 2) == 0) {
		pos = name.indexOf("\"");
		len = 1;
		return TN_BAD_NAME_5;
	}
	return error;
}

QString KumTools::SplitPrisvBlock(
	const QStringList p_List,
	QStringList *p_Result,
	int p_Num = -1,
	bool p_FuncParam = false
) {
	if (p_Num == -1) {
		p_Result->clear();
	}
	QString l_MainString = p_Num == -1 ? p_List.last() : p_List[p_Num];
	l_MainString = l_MainString.simplified();
	if (l_MainString[l_MainString.length() - 1] == QChar(0)) {
		l_MainString = l_MainString.left(l_MainString.length() - 1);
	}
	QString l_NotFunctSyms = "+-*/><&1234567890^()=\"\',";
	for (int i = 1000; i < 1007; i++) {
		l_NotFunctSyms += QChar(i);
	}
	l_NotFunctSyms += QChar(KS_PRISV);

	QString l_NewMainString = "";
	if ((l_MainString.indexOf(',') == -1) && (l_MainString.indexOf('&') == -1) && p_FuncParam) {
		p_Result->append(l_MainString);
		l_NewMainString += '&' + QString::number(p_Result->count() - 1);
		return l_NewMainString;
	}
	bool l_IsFunction = false;
	for (int i = 0 ; i < l_MainString.length(); i++) {
		if (l_MainString[i] == '&') {
			i++;
			QString l_NumString = "";
			while (l_MainString[i].isNumber() && i < l_MainString.length()) {
				l_NumString += l_MainString[i++];
			}
			i--;
			QString l_StringText = SplitPrisvBlock(p_List, p_Result, l_NumString.toInt(), l_IsFunction);
			if (l_IsFunction) {
				l_NewMainString += l_StringText;
			} else if (p_FuncParam) {
				if (l_StringText.contains('&') != 1) {
					p_Result->append(l_StringText);
					l_NewMainString += '&' + QString::number(p_Result->count() - 1);
				} else {
					l_NewMainString += l_StringText;
				}
			} else {
				l_NewMainString += '(' + l_StringText + ')';
			}
		} else {
			if (!p_FuncParam || l_MainString[i] != ',') {
				l_NewMainString += l_MainString[i];
			}
		}

		if (l_MainString[i] == '\"' || l_MainString[i] == '\'') {
			QChar l_QuoteChar = l_MainString[i];
			for (++i; l_MainString[i] != l_QuoteChar && i < l_MainString.length(); i++) {
				l_NewMainString += l_MainString[i];
			}
			l_NewMainString += l_QuoteChar;
		} else {
			l_IsFunction = l_NotFunctSyms.indexOf(l_MainString[i]) == -1;
			if (l_IsFunction) {
				for (++i; l_MainString[i] != '&' && i < l_MainString.length(); i++) {
					l_NewMainString += l_MainString[i];
				}
				i--;
			}
		}
	}
	if (p_FuncParam && l_MainString.indexOf(',') == -1) {
		p_Result->append(l_NewMainString);
		return '&' + QString::number(p_Result->count() - 1);
	}
	if (p_Num == -1) {
		p_Result->append(l_NewMainString);
	}

	return l_NewMainString;

}

QStringList KumTools::splitElse(QString normal_else_string)
{
	QStringList temp_List;
	temp_List.clear();
	if (normal_else_string.length() > 1) {
		temp_List.append(QString(normal_else_string[0]));
		int vse_pos = normal_else_string.indexOf(QChar(KS_VSE));
		if (vse_pos > -1) {
			temp_List.append(normal_else_string.mid(2, vse_pos - 2));
			//                  outPutWindow->append("Hvost:"+normal_else_string.mid(1,vse_pos-1));
			temp_List.append(QString(QChar(KS_VSE)));
			return temp_List;
		} else {
			temp_List.append(normal_else_string.mid(2, 9999));
		}
		return temp_List;
	} else {
		temp_List.append(normal_else_string);
		return temp_List;
	};

	return temp_List;

}


QStringList KumTools::splitTo(QString normal_to_string)
{
	QStringList temp_List;
	temp_List.clear();
	QString normal_else_string;
	if (normal_to_string.length() > 1) {
		temp_List.append(QString(normal_to_string[0]));
		int else_pos = normal_to_string.indexOf(QChar(KS_INACHE));
		//int vse_pos=normal_to_string.indexOf(QChar(KS_VSE));
		if (else_pos > -1) {
			temp_List.append(normal_to_string.mid(2, else_pos - 2));
			normal_else_string = normal_to_string.mid(else_pos, 9999);
			//                  outPutWindow->append("ElseString:"+normal_else_string);
			temp_List += splitElse(normal_else_string);
			return temp_List;
		}

		int vse_pos = normal_to_string.indexOf(QChar(KS_VSE));
		if (vse_pos > -1) {
			temp_List << normal_to_string.mid(2, vse_pos - 2).split(";", QString::SkipEmptyParts);
			return temp_List;
		} else {
			temp_List.append(normal_to_string.mid(2, 9999));
		}
		return temp_List;



	} else {
		temp_List.append(normal_to_string);
		return temp_List;
	};

	return temp_List;

}


//int KumTools::splitUserStr ( QString stroka,QList<ProgaText>* result,int str )
//{
//    int err=0;
//    int err_start = 0;
//    int err_end = 0;
//    ProgaText temp_proga;
//    QString norma_string=normalizeString ( stroka,&err,&err_start,&err_end );
//    int e1, e2, e3;
//    QList<int> P;
//    QList<int> L;
//    QString n2 = normalize(stroka, P, L, e1, e2, e3);
//    K_ASSERT( e1>=0 );
//    K_ASSERT( e2>=0 );
//    K_ASSERT( e3>=0 );
//    K_ASSERT ( norma_string == n2 );
//    if ( err>0 ) return err;
//    temp_proga.stroka=str;
//    temp_proga.x_offset=0;
//    Kumir::InstructionType RealLineType=getStringType ( norma_string );
//    QStringList List;
//    List.clear();
//    switch ( RealLineType )
//    {

//    case Kumir::AlgDecl:
//        temp_proga.Text=norma_string.trimmed();
//        result->append ( temp_proga );
//        return 0;
//        break;

//    case Kumir::AlgBegin:
//        {
//            int ns_len = norma_string.length();
//            if ( ns_len>1 )
//            {
//                List.append ( QString ( QChar ( KS_NACH ) ) );
//                QStringList tmp_lst = norma_string.mid ( 2 ).split ( ";",QString::SkipEmptyParts );
//                for ( int i=0;i<tmp_lst.count();i++ ) List.append ( tmp_lst[i].trimmed() );
//            }
//            else
//            {
//                temp_proga.Text=norma_string;
//                result->append ( temp_proga );
//                return 0;
//            };
//        }
//        break;


//    case Kumir::LoopBegin:
//        {
//            int raz_pos=norma_string.indexOf ( QChar ( KS_RAZ ) );
//            int ns_len = norma_string.length();
//            if ( raz_pos<ns_len-1 )
//            {
//                List.append ( norma_string.mid ( 0,raz_pos+1 ) );
//                QString comm_line=norma_string.mid ( raz_pos+1,9999 );
//                QString append_string="";
//                if ( norma_string[norma_string.length()-1]==QChar ( KS_KC ) )
//                {
//                    comm_line=comm_line.mid ( 0,comm_line.length()-1 );
//                    append_string=QString ( QChar ( KS_KC ) );
//                };

//                List += comm_line.split ( ";",QString::SkipEmptyParts );
//                if ( !append_string.isEmpty() ) List.append ( append_string );
//            }
//            else
//                List.append ( norma_string.trimmed() );
//        }
//        break; //exit

//    case Kumir::LoopEnd:

//        temp_proga.Text=norma_string;
//        result->append ( temp_proga );
//        return 0;

//        break;

//    case Kumir::Else:

//        List+=splitElse ( norma_string );

//        break; //else



//    case Kumir::Exit:
//        {
//            temp_proga.Text=norma_string;
//            result->append ( temp_proga );
//            return 0;
//            break; //exit
//        }

//    case Kumir::If:
//        {
//            int to_pos=norma_string.indexOf ( QChar ( KS_TO ) );
//            if ( to_pos>-1 )
//            {
//                List.append ( norma_string.mid ( 0,to_pos ) );
//                QString to_line=norma_string.mid ( to_pos,9999 );
//                //        outPutWindow->append("toLine:"+to_line);
//                List+=splitTo ( to_line );
//            }
//            else
//                List.append ( norma_string );
//        }

//        break;

//        /*    case VIBOR_BEGINS:
//                        if(!build)
//                        out=parce_vibor(instr,table,symbols,proga_str,&err_start,&err_end);
//                        else
//                        out=0;
//                        break; */

//    case Kumir::Case:
//        {
//            int dvoet_pos=norma_string.indexOf ( ':' );
//            int ns_len = norma_string.length();
//            if ( dvoet_pos<ns_len-1 )
//            {
//                List.append ( norma_string.mid ( 0,dvoet_pos+1 ) );
//                QString comm_line=norma_string.mid ( dvoet_pos+1,9999 );
//                comm_line=comm_line.trimmed();
//                List << comm_line.split ( ";",QString::SkipEmptyParts );
//            }
//            else
//                List.append ( norma_string.trimmed() );
//        }
//        break;

//    case Kumir::Then:
//        {
//            List+=splitTo ( norma_string );
//        }
//        break;

//    case Kumir::Fin:

//        temp_proga.Text=norma_string;
//        result->append ( temp_proga );
//        return 0;
//        break;
//        /*
//                                case UTV:
//                                        if(!build)
//                                        out=parce_utv(instr,table,symbols,proga_str,&err_start,&err_end);
//                                        else
//                                        out=0;
//                                        break;
//                                case KC_PRI:
//                                        if(!build)
//                                        out=parce_kc_pri(instr,table,symbols,proga_str,&err_start,&err_end);
//                                        else
//                                        out=0;
//                                        break;

//                                case DANO:
//                                        if(!build)
//                                        out=parce_dano(instr,table,symbols,proga_str,&err_start,&err_end);
//                                        else
//                                        out=0;
//                                        break;
//                                case NADO:
//                                        if(!build)
//                                        out=parce_nado(instr,table,symbols,proga_str,&err_start,&err_end);
//                                        else
//                                        out=0;
//                                        break;

//                                case VVOD:
//                                        if(!build)
//                                        out=parce_vvod(instr,table,symbols,proga_str,&err_start,&err_end);
//                                        else
//                                        out=0;
//                                        break;

//                                case COMMENT:
//                                        out=0;
//                                        Str_Data.line=instr;
//                                        break; */


//    default:
//        List = norma_string.split ( ":",QString::SkipEmptyParts );
//    }
//    //switch




//    for ( int i=0;i<List.count();i++ )
//    {

//        temp_proga.Text=List[i];
//        if ( i>0 )
//        {
//            //int zdvig= FindNomerNorm(List[i-1],List[i-1].length()-1,1,&null);
//            int zdvig;
//            zdvig = List[i-1].count ( QRegExp ( Breaks ) );
//            if ( zdvig<0 ) zdvig=0;
//            temp_proga.x_offset=temp_proga.x_offset+zdvig; //считаем в ксимволах
//            //       temp_proga.x_offset=temp_proga.x_offset+1;
//        }

//        result->append ( temp_proga );
//    }
//    if ( result->count() > 1 )
//    {
//        for ( int i=0;i<result->count()-1;i++ )
//        {
//            int debug = result->count();
//            int y_offset = ( *result ) [i+1].x_offset;
//            ( *result ) [i].y_offset = y_offset;
//            Q_UNUSED(debug);
//        }
//    }
//    if ( List.count() > 1 )
//        return RealLineType;
//    else
//        return EMPTY;
//}

int KumTools::splitUserIsp(QStringList *stroki, QList<ProgaText> *result, QString file)
{
	int error = 0;
	int err_start = 0;
	int err_end = 0;

	for (int i = 0; i < stroki->count(); i++) {
		QString stroka = stroki->at(i);
		QString norma_string = normalizeString(stroka, &error, &err_start, &err_end); //Нормализация строки
		QStringList FStroki = norma_string.split(";", QString::KeepEmptyParts);
		QList< QList<int> > PP, LL;
		error = splitFStringList(FStroki, result, i, 0, 0, file, PP, LL, stroka); //� азбиение строки.

		//  if(error>0)return error;
	};
	int debug = result->count();
	Q_UNUSED(debug);
	return 0;
}

void KumTools::trim(QString &s, QList<int> &P, QList<int> &L)
{
	while (s.startsWith(" ")) {
		s.remove(0, 1);
		P.pop_front();
		L.pop_front();
	}

	while (s.endsWith(" ")) {
		s.chop(1);
		P.pop_back();
		L.pop_back();
	}
}

//QString KumTools::naznachitKostil(QString setLine,int *err)
//{
//    QStringList zptSpl=setLine.split(',');
//    if(zptSpl.count()!=2)
//    {
//        *err=BAD_ASSIGN;
//        return setLine;
//    };

//    if(!zptSpl[1].endsWith(')'))
//    {
//       *err= TRASH_AFTER_ASSIGN;
//       return setLine;
//    };

//    if(zptSpl[0].endsWith(QString('('+QChar ( KS_VIVOD ))))
//    {
//        QString fileName=zptSpl[1].left(zptSpl[1].length()-1);
//       return  trUtf8("НАЗНАЧИТЬ ВЫВОД(")+fileName+")";
//    }
//    if(zptSpl[0].endsWith(QString('('+QChar ( KS_VVOD ))))
//    {
//        QString fileName=zptSpl[1].left(zptSpl[1].length()-1);
//       return  trUtf8("НАЗНАЧИТЬ ВВОД(")+fileName+")";
//    }
//    *err=BAD_ASSIGN;
//    return setLine;
//}


/**
 * � азбиение списка Ф-строк. Вызывается рекурсивно.
 * @param FSpisok Список ф-строк
 * @param result � езультат разбиения не отчищается, а добавляется.
 * @param realLine Номер строки в реальном редакторе
 * @return
 */
int KumTools::splitFStringList(QStringList FSpisok, QList<ProgaText> *result, int realLine, int Last_F_ID, int  offset, QString file, const QList<QList<int> > &PPs, const QList<QList<int> > &LLs, const QString &sourceText)
{
	int curLine = Last_F_ID;

	QList< QList<int> > PP = PPs;
	QList< QList<int> > LL = LLs;

	while ((FSpisok[curLine]).isEmpty()) {
		if (curLine == (FSpisok.count() - 1)) {
			return 0;
		}
		curLine++;
	}

	int RealLineType = getStringType(FSpisok[curLine]);

	ProgaText temp_proga;//Временные результаты разбора.
	temp_proga.stroka = realLine;
	temp_proga.x_offset = offset;
	temp_proga.Error = 0;
	temp_proga.File = file;
	temp_proga.source = sourceText.trimmed();
	if (temp_proga.source.endsWith("|@protected")) {
		temp_proga.source = temp_proga.source.left(temp_proga.source.length() - 11);
	}
	if (temp_proga.source.endsWith("|@hidden")) {
		temp_proga.source = temp_proga.source.left(temp_proga.source.length() - 8);
	}
	QString tmp;
	int to_pos, pri_pos, raz_pos;
	int zdvig;
	switch (RealLineType) {
	//Неразделяемые строки
	case Kumir::AlgDecl:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;
		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;



	case Kumir::Pre:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;



	case Kumir::Post:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;


	case Kumir::ModuleBegin:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;



	case Kumir::ModuleEnd:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;


	case Kumir::Fin:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);


		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;

	case Kumir::LoopEndCond:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);


		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;

	case Kumir::LoopEnd:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);


		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;


	// include
	case Kumir::Include: {
		temp_proga.Text = FSpisok[curLine];

		QList<ProgaText> include;
		int local_error = includeTextOrBytecode(FSpisok[curLine], include);
		if (local_error) {
			return local_error;
		}
		result->append(include);

		zdvig = FSpisok[curLine].count(brks);
		temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;
		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;
	}



	case Kumir::AlgEnd:
		temp_proga.Text = FSpisok[curLine];
		result->append(temp_proga);
		result->last().P = PP[curLine];
		result->last().L = LL[curLine];
		trim(result->last().Text, result->last().P, result->last().L);

		FSpisok[curLine] = "";
		PP[curLine].clear();
		LL[curLine].clear();
		break;


	//НАЧ
	case Kumir::AlgBegin:
		temp_proga.Text = QString(QChar(KS_NACH));
		result->append(temp_proga);
		result->last().P.clear();
		result->last().P << PP[curLine].first();
		result->last().L.clear();
		result->last().L << LL[curLine].first();
		PP[curLine].pop_front();
		LL[curLine].pop_front();

		tmp = FSpisok[curLine].mid(1, 9999);
		temp_proga.x_offset++;
		trim(tmp, PP[curLine], LL[curLine]);
		FSpisok[curLine] = tmp;
		break;


	//при
	case Kumir::Case:
		pri_pos = FSpisok[curLine].indexOf(":");
		if (pri_pos > -1) {
			temp_proga.Text = FSpisok[curLine].mid(0, pri_pos + 1);
			result->append(temp_proga);
			result->last().P.clear();
			result->last().L.clear();
			for (int j = 0; j < result->last().Text.length(); j++) {
				result->last().P << PP[curLine].first();
				result->last().L << LL[curLine].first();
				PP[curLine].pop_front();
				LL[curLine].pop_front();
			}

			//         zdvig = FSpisok[curLine].count(brks);
			zdvig = temp_proga.Text.count(brks);
			temp_proga.x_offset = temp_proga.x_offset + zdvig;
			QString pri_line = FSpisok[curLine].mid(pri_pos + 1, 9999);
			trim(pri_line, PP[curLine], LL[curLine]);
			FSpisok[curLine] = pri_line;
		} else {
			temp_proga.Text = FSpisok[curLine];
			result->append(temp_proga);
			result->last().P = PP[curLine];
			result->last().L = LL[curLine];
			trim(result->last().Text, result->last().P, result->last().L);

			FSpisok[curLine] = "";
			PP[curLine].clear();
			LL[curLine].clear();
		};
		break;


	case Kumir::Then:
		temp_proga.Text = QString(QChar(KS_TO));
		result->append(temp_proga);
		result->last().P.clear();
		result->last().P << PP[curLine].first();
		PP[curLine].pop_front();
		result->last().L.clear();
		result->last().L << LL[curLine].first();
		LL[curLine].pop_front();

		temp_proga.x_offset++;
		tmp = FSpisok[curLine].mid(1, 9999);
		trim(tmp, PP[curLine], LL[curLine]);
		FSpisok[curLine] = tmp;
		break;

	case Kumir::Else:
		temp_proga.Text = QString(QChar(KS_INACHE));
		result->append(temp_proga);
		result->last().P.clear();
		result->last().P << PP[curLine].first();
		PP[curLine].pop_front();
		result->last().L.clear();
		result->last().L << LL[curLine].first();
		LL[curLine].pop_front();

		temp_proga.x_offset++;
		tmp = FSpisok[curLine].mid(1, 9999);
		trim(tmp, PP[curLine], LL[curLine]);
		FSpisok[curLine] = tmp;
		break;


	case Kumir::Switch:
		temp_proga.Text = QString(QChar(KS_VIBOR));
		result->append(temp_proga);
		result->last().P.clear();
		result->last().P << PP[curLine].first();
		PP[curLine].pop_front();
		result->last().L.clear();
		result->last().L << LL[curLine].first();
		LL[curLine].pop_front();

		temp_proga.x_offset++;
		tmp = FSpisok[curLine].mid(1, 9999);
		trim(tmp, PP[curLine], LL[curLine]);
		FSpisok[curLine] = tmp;
		break;

	case Kumir::If:
		to_pos = FSpisok[curLine].indexOf(QChar(KS_TO));     //Ищем то
		if (to_pos > -1) {
			temp_proga.Text = FSpisok[curLine].mid(0, to_pos);
			result->append(temp_proga);
			result->last().P.clear();
			result->last().L.clear();
			for (int j = 0; j < result->last().Text.length(); j++) {
				result->last().P << PP[curLine].first();
				result->last().L << LL[curLine].first();
				PP[curLine].pop_front();
				LL[curLine].pop_front();
			}


			zdvig = temp_proga.Text.count(brks);
			temp_proga.x_offset = temp_proga.x_offset + zdvig;

			temp_proga.Text = QString(QChar(KS_TO));
			result->append(temp_proga);
			result->last().P.clear();
			result->last().L.clear();
			result->last().P << PP[curLine].first();
			result->last().L << LL[curLine].first();
			PP[curLine].pop_front();
			LL[curLine].pop_front();

			temp_proga.x_offset++;
			QString to_line = FSpisok[curLine].mid(to_pos + 1, 9999);
			trim(to_line, PP[curLine], LL[curLine]);
			FSpisok[curLine] = to_line;
		} else {
			temp_proga.Text = FSpisok[curLine];
			result->append(temp_proga);
			result->last().P = PP[curLine];
			result->last().L = LL[curLine];
			trim(result->last().Text, result->last().P, result->last().L);


			zdvig = temp_proga.Text.count(brks);
			temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;

			FSpisok[curLine] = "";
			PP[curLine].clear();
			LL[curLine].clear();
		};
		break;

	case Kumir::LoopBegin:
		raz_pos = FSpisok[curLine].indexOf(QChar(KS_RAZ));     //Ищем раз
		if (raz_pos > -1) {
			temp_proga.Text = FSpisok[curLine].mid(0, raz_pos + 1);
			result->append(temp_proga);
			for (int j = 0; j < result->last().Text.length(); j++) {
				result->last().P << PP[curLine].first();
				result->last().L << LL[curLine].first();
				PP[curLine].pop_front();
				LL[curLine].pop_front();
			}


			zdvig = temp_proga.Text.count(brks);
			temp_proga.x_offset = temp_proga.x_offset + zdvig;

			QString to_line = FSpisok[curLine].mid(raz_pos + 1, 9999);
			trim(to_line, PP[curLine], LL[curLine]);
			FSpisok[curLine] = to_line;
		} else {
			if ((FSpisok[curLine].indexOf(QChar(KS_DLYA)) > -1) || (FSpisok[curLine].indexOf(QChar(KS_POKA)) > -1)) {
				temp_proga.Text = FSpisok[curLine];
				result->append(temp_proga);
				result->last().P = PP[curLine];
				result->last().L = LL[curLine];
				trim(result->last().Text, result->last().P, result->last().L);

				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = zdvig + 1;
				FSpisok[curLine] = "";
				PP[curLine].clear();
				LL[curLine].clear();
			} else {
				temp_proga.Text = QString(QChar(KS_NC));
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				result->last().P << PP[curLine].first();
				result->last().L << LL[curLine].first();
				PP[curLine].pop_front();
				LL[curLine].pop_front();

				temp_proga.x_offset++;
				tmp = FSpisok[curLine].mid(1, 9999);
				trim(tmp, PP[curLine], LL[curLine]);
				FSpisok[curLine] = tmp;
			};
		};
		break;




	default: //Простая строка или ошибка.
		tmp = FSpisok[curLine];
		bool flag = false; //� азделители
		for (int i = 0; i < tmp.length(); i++) {
			if (flag) {
				break;    //Что-то уже нашли
			}


			switch (tmp[i].unicode()) {
			case KS_INACHE:
				flag = true;
				temp_proga.Text = FSpisok[curLine].mid(0, i);
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				for (int j = 0; j < result->last().Text.length(); j++) {
					result->last().P << PP[curLine].first();
					result->last().L << LL[curLine].first();
					PP[curLine].pop_front();
					LL[curLine].pop_front();
				}

				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = temp_proga.x_offset + zdvig;
				FSpisok[curLine] = FSpisok[curLine].mid(i, 9999);
				break;

			case KS_VSE:
				flag = true;
				temp_proga.Text = FSpisok[curLine].mid(0, i);
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				for (int j = 0; j < result->last().Text.length(); j++) {
					result->last().P << PP[curLine].first();
					result->last().L << LL[curLine].first();
					PP[curLine].pop_front();
					LL[curLine].pop_front();
				}


				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = temp_proga.x_offset + zdvig;


				FSpisok[curLine] = FSpisok[curLine].mid(i, 9999);
				break;

			case KS_KC:
				flag = true;
				temp_proga.Text = FSpisok[curLine].mid(0, i);
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				for (int j = 0; j < result->last().Text.length(); j++) {
					result->last().P << PP[curLine].first();
					result->last().L << LL[curLine].first();
					PP[curLine].pop_front();
					LL[curLine].pop_front();
				}

				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = temp_proga.x_offset + zdvig;


				FSpisok[curLine] = FSpisok[curLine].mid(i, 9999);
				break;

			case KS_PRI:
				flag = true;
				temp_proga.Text = FSpisok[curLine].mid(0, i);
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				for (int j = 0; j < result->last().Text.length(); j++) {
					result->last().P << PP[curLine].first();
					result->last().L << LL[curLine].first();
					PP[curLine].pop_front();
					LL[curLine].pop_front();
				}


				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = temp_proga.x_offset + zdvig;


				FSpisok[curLine] = FSpisok[curLine].mid(i, 9999);
				break;

			case KS_KC_PRI:
				flag = true;
				temp_proga.Text = FSpisok[curLine].mid(0, i);
				result->append(temp_proga);
				result->last().P.clear();
				result->last().L.clear();
				for (int j = 0; j < result->last().Text.length(); j++) {
					result->last().P << PP[curLine].first();
					result->last().L << LL[curLine].first();
					PP[curLine].pop_front();
					LL[curLine].pop_front();
				}


				zdvig = temp_proga.Text.count(brks);
				temp_proga.x_offset = temp_proga.x_offset + zdvig;

				FSpisok[curLine] = FSpisok[curLine].mid(i, 9999);
				break;

			};
		};
		if (!flag) {
//                        qDebug()<<"TMP"<<tmp;
			zdvig = FSpisok[curLine].count(brks);
			temp_proga.x_offset = temp_proga.x_offset + zdvig + 1;
//                      int P_off=0;
//                      int L_off=0;

//                        if(tmp.startsWith(trUtf8("назначить(")))
//                        {
//                            tmp=naznachitKostil(tmp,&temp_proga.Error);//Заплата для переназначения ввода вывода :(
//                            FSpisok[curLine]=tmp;
//                        //  temp_proga.x_offset=temp_proga.x_offset+2;
//                            int P_off=0;
//                            int L_off=1;temp_proga.stroka=-1;
//                            if(temp_proga.Error)return temp_proga.Error;
//                        };

			temp_proga.Text = FSpisok[curLine];

			result->append(temp_proga);
			result->last().P = PP[curLine];
			result->last().L = LL[curLine];


			FSpisok[curLine] = "";
			PP[curLine].clear();
			LL[curLine].clear();
		};
		break;

	};
	int err = 0;
	int fCount = FSpisok.count();

	// различные проверки, -- убрать или закомментировать после тестирования,
	// т.к. сильно снижает скорость работы

	for (int i = 0; i < result->count(); i++) {
		int a, b, c;
		a = result->at(i).Text.length();
		b = result->at(i).P.count();
		c = result->at(i).L.count();
		K_ASSERT(a == b);
		K_ASSERT(b == c);
	}

	for (int i = 0; i < FSpisok.count(); i++) {
		K_ASSERT(FSpisok[i].length() == PP[i].count());
		K_ASSERT(PP[i].count() == LL[i].count());
	}

	if (curLine < fCount) {
		err = splitFStringList(FSpisok, result, realLine, curLine, temp_proga.x_offset, file, PP, LL, sourceText);
	}
	return err;
}

//int KumTools::splitUserFile ( QFile &file, QList< QList<ProgaText> > *result )
//{
//    QList<ProgaText> all;
//    if ( file.open(QIODevice::ReadOnly|QIODevice::Text) ) {
//        QTextStream ts ( &file );
//        ts.setCodec("UTF-16LE");


//        QStringList lines = ts.readAll().split("\n",QString::KeepEmptyParts);
//        for ( int i=0; i<lines.count(); i++ ) {
//            QPoint errorPosition;
//            splitUserLine(lines[i],errorPosition,&all);
//        }
//        for ( int i=0; i<all.count(); i++ ) {
//            all[i].File = QFileInfo(file).absoluteFilePath();
//        }
//        file.close();
//        bool inIsp = false;
//        bool ispFound = false;
//        bool inInnerIsp = false;
//        result->clear();
//        QList<ProgaText> curModule;
//        for ( int i=0; i<all.count(); i++ ) {
//            if ( getStringType(all[i].Text) == ISP ) {
//                if ( inIsp ) {
//                    all[i].Error = TOS_ISP_IN_ISP;
//                    all[i].errstart = 0;
//                    all[i].errlen = all[i].text().length();
//                    inInnerIsp = true;
//                }
//                else {
//                    if ( !curModule.isEmpty() )
//                        result->append(curModule);
//                    curModule.clear();
//                }
//                inIsp = true;
//                ispFound = true;
//            }
//            else if ( getStringType(all[i].Text) == ISP_END ) {
//                inIsp = false;
//                if ( inInnerIsp ) {
//                    all[i].Error = TOS_ISP_IN_ISP;
//                    all[i].errstart = 0;
//                    all[i].errlen = all[i].text().length();
//                    inInnerIsp = false;
//                }
//            }
//            else {
//                if ( ispFound && !inIsp ) {
//                    all[i].Error = TOS_AFTER_ISP;
//                    all[i].errstart = 0;
//                    all[i].errlen = all[i].text().length();
//                }
//                if ( inInnerIsp ) {
//                    all[i].Error = TOS_ISP_IN_ISP;
//                    all[i].errstart = 0;
//                    all[i].errlen = all[i].text().length();
//                }
//            }
//            curModule << all[i];
//        }
//        if ( !curModule.isEmpty() )
//            result->append(curModule);
//        return 0;
//    }
//    else {
//        return -1;
//    }
//}

QStringList splitStringList(
	const QString &src, // IN: исходная строка
	const QList<int> &P, // IN: вектор P
	const QList<int> &L, // IN: вектор L
	QList< QList<int> > &PP, // OUT: список векторов P
	QList< QList<int> > &LL // OUT: список векторов L
)
{
	QStringList result;
	QString cur = "";
	QList<int> CP;
	QList<int> CL;
	PP.clear();
	LL.clear();
	for (int i = 0; i < src.length(); i++) {
		if (src[i] == ';' && !cur.startsWith(QChar(KS_INCLUDE))) {
			result << cur;
			cur = "";
			PP << CP;
			CP.clear();
			LL << CL;
			CL.clear();
		} else {
			cur += src[i];
			CP << P[i];
			CL << L[i];
		}
	}
	result << cur;
	PP << CP;
	LL << CL;
	return result;
}

int KumTools::splitUserLine(const QString &text, QPoint &errorPosition, QList<ProgaText> *result)
{
	int err = 0;
	int err_start = -1;
	int err_end = -1;
	int other_err = 0;
	QString norma_string;
//    if ( !NEW_NORMALIZATION )
//        norma_string=normalizeString ( text,&err,&err_start,&err_end );
	QString n2;
	int e1, e2, e3;
	QList<int> P, L;
	n2 = normalize(text, P, L, e1, e2, e3);
	K_ASSERT(e1 >= 0);
	K_ASSERT(e2 >= 0);
	K_ASSERT(e3 >= 0);
	//  qDebug() << "n2: -" << n2.toUtf8();
	//  qDebug() << "P: --" << P;
	//  qDebug() << "L: --" << L;
	K_ASSERT((n2.length() == P.count()) && (P.count() == L.count()));

//    if ( NEW_NORMALIZATION ) {
	norma_string = n2;
	err = e1;
	err_start = e2;
	err_end = e2 + e3 - 1;
//    }

	if (err > 0) {
		errorPosition = QPoint(err_start, err_end);
	}
	QStringList FStroki;
	QList< QList<int> > PP, LL;
//    if ( !NEW_NORMALIZATION )
//        FStroki=norma_string.split ( ";",QString::KeepEmptyParts ); //Построение списка Фстрок.
//    else
	FStroki = splitStringList(norma_string, P, L, PP, LL);

	other_err = splitFStringList(FStroki, result, 0, 0, 0, "\'USER", PP, LL, text); //� азбиение строки.
	if (other_err > 0 && err == 0) {
		err = other_err;
		errorPosition = QPoint(0, text.length() - 1);
	}
	//  int idbg = result->count();
	//  qDebug() << "RESULT.count(): " << idbg;
	//  for ( int i=0; i<idbg; i++ ) {
	//      qDebug("RESULT[%i] = %s",i,(*result)[i].Text.toUtf8().data());
	//  }
	return err;
}

/**
 * � азбиение строк пользовательского текста. В соответствии с описанием от 15.07.07
 * @param textEdit � едактор текста.
 * @param result � езультат разбиения.
 * @param BASE С какой строки начинать.
 * @return Код ошибки
 */
int KumTools::splitUserTextV2(QString text, QList<LineProp> *textProp, QList<ProgaText> *result, int BASE)
{
	int err = 0;
	int err_start = -1;
	int err_end = -1;
	QStringList lines = text.split("\n", QString::KeepEmptyParts);
	for (int i = BASE; i < lines.count(); i++) {
		QString txt = lines[i];
		//      ( *textProp ) [i].error_l_start = -1;
		//      ( *textProp ) [i].error_l_end = -1;
		QString norma_string = normalizeString(txt, &err, &err_start, &err_end); //Нормализация строки
		(*textProp) [i].error = err;   //Запись ошибки
		//      ( *textProp ) [i].error_l_start = err_start;
		//      ( *textProp ) [i].error_l_end = err_end;
		QList< QList<int> > PP, LL;
		QStringList FStroki = norma_string.split(";");  //Построение списка Фстрок.
		err = splitFStringList(FStroki, result, i, 0, 0, "\'USER", PP, LL, txt); //� азбиение строки.
		//      if ( ( *textProp ) [i].error == 0 )
		//          ( *textProp ) [i].error = err; //Запись ошибки
		err_start = -1;
		err_end = -1;

	};
	int debug = result->count();
	Q_UNUSED(debug);
	return 0;
}





/**
 * Проверяет, находится ли позиция pos внутри литеральной константы
 * @param s исходная ненормализованная строка
 * @param pos позиция
 * @return true, если внутри литеральной константы или false, если нет
 */

bool KumTools::inLit(QString s, int pos)
{
	bool isDoubleQuote = false;
	bool inL = false;
	int cur_pos = -1;
	int len = s.length();
	int pos_DQ, pos_SQ;
	while (cur_pos < len) {
		pos_DQ = s.indexOf("\"", cur_pos + 1);
		pos_SQ = s.indexOf("'", cur_pos + 1);
		if ((pos_DQ == -1) && (pos_SQ == -1)) {
			return inL;
		}
		if (
			(pos_DQ == -1)
			||
			((pos_SQ < pos_DQ) && (pos_SQ != -1))
		) { // next is '
			cur_pos = pos_SQ;
			if (pos < cur_pos) {
				return inL;
			}
			if (!inL) { // begin lit
				inL = true;
				isDoubleQuote = false;
			} else {
				if (!isDoubleQuote) {
					inL = false;
				}
			}
		} else { // next is "
			cur_pos = pos_DQ;
			if (pos < cur_pos) {
				return inL;
			}
			if (!inL) {
				isDoubleQuote = true;
				inL = true;
			} else {
				if (isDoubleQuote) {
					inL = false;
				}
			}
		}
	}
	return false;
}

// void KumTools::recalculateErrorsPositions_findNextXymbol(const QString &text, const int start, int &pos, int &len)
// {
//  rxLitBreaks.setMinimal(false);
//  K_ASSERT ( rxLitBreaks.isValid() );
//  pos = rxLitBreaks.indexIn(text,start);
//  while ( inLit(text,pos) ) {
//      pos++;
//      pos = rxLitBreaks.indexIn(text,pos);
//      if ( pos == -1 ) {
//          break;
//      }
//  }
//  len = rxLitBreaks.matchedLength();
// }

QVector<bool> KumTools::recalculateErrorPositions(const QString &text, const QList<ProgaText> &ptl)
{
	int resultSize = text.length();
	QVector<bool> result = QVector<bool>(resultSize, false);
	foreach (ProgaText pt, ptl) {
		if (pt.Error > 0) {
			int pc = pt.P.count();
			int lc = pt.L.count();
			Q_UNUSED(pc);
			Q_UNUSED(lc);
			int tl = pt.Text.length();
			Q_UNUSED(tl);
			K_ASSERT(pt.P.count() == pt.L.count());
			K_ASSERT(pt.P.count() == pt.Text.length());
			for (int i = pt.errstart; i < pt.errstart + pt.errlen; i++) {
				int start = pt.P[i];
				int len = pt.L[i];
				for (int j = start; j < start + len; j++) {
					result[j] = true;
				}
			}
		}
	}
	int start = result.indexOf(true);
	if (start != -1) {
		for (int i = start; i < text.length(); i++) {
			if (text[i] == ' ' || text[i] == '\t') {
				result[i] = false;
			} else {
				break;
			}
		}
	}
	int end = result.lastIndexOf(true);
	if (end != -1) {
		for (int i = end; i >= 0; i--) {
			if (text[i] == ' ' || text[i] == '\t') {
				result[i] = false;
			} else {
				break;
			}
		}
	}
	return result;
}



bool KumTools::allowIndent(const int str_type, const int err_code)
{
	Q_UNUSED(str_type);
	if (err_code == 0) {
		return true;
	} else if ((err_code >= 1101 && err_code <= 1199) || (err_code >= 11100 && err_code <= 11105) || (err_code >= 12001 && err_code <= 12030)) {
		return false;
	} else {
		return true;
	}
}

QPoint KumTools::getIndentRang(const ProgaText &pt)
{
	QPoint result;
	Kumir::InstructionType st = KumTools::getStringType(pt.Text);
	if (st == Kumir::AlgBegin) { // 13, нач
		result = QPoint(0, 1);
	} else if (st == Kumir::AlgEnd) { // 14, кон
		result = QPoint(-1, 0);
	} else if (st == Kumir::LoopBegin) { // 15, нц
		result = QPoint(0, 1);
	} else if (st == Kumir::LoopEnd) { // 16, кц
		result = QPoint(-1, 0);
	} else if (st == Kumir::LoopEndCond) { // 33, кц_при
		result = QPoint(-1, 0);
	} else if (st == Kumir::Else) { // 17, иначе
		result = QPoint(-1, 1);
	} else if (st == Kumir::If) { // 20, если
		result = QPoint(0, 2);
	} else if (st == Kumir::Then) { // 21, то
		result = QPoint(-1, 1);
	} else if (st == Kumir::Fin) { // 22, все
		result = QPoint(-2, 0);
	} else if (st == Kumir::Switch) { // 23, выбор
		result = QPoint(0, 2);
	} else if (st == Kumir::Case) { // 24, при
		result = QPoint(-1, 1);
	} else if (st == Kumir::Pre) { // 30, дано
		result = QPoint(1, -1);
	} else if (st == Kumir::Post) { // 31, надо
		result = QPoint(1, -1);
	} else if (st == Kumir::ModuleBegin) { // 34, исп
		result = QPoint(0, 1);
	} else if (st == Kumir::ModuleEnd) { // 35, кон_исп
		result = QPoint(-1, 0);
	} else if (st == Kumir::Doc) { // 98, #
		result = QPoint(1, -1);
	} else {
		result = QPoint(0, 0);
	}
	int error = pt.Error;
	if (allowIndent(st, error)) {
		return result;
	} else {
		return QPoint(0, 0);
	}
}

QPoint KumTools::getIndentRang(const QList<ProgaText> &ptl)
{
	if (ptl.isEmpty()) {
		return QPoint(0, 0);
	}

	int start = 0;
	int end = 0;

	for (int i = 0; i < ptl.count(); i++) {
		QPoint ir = ptl[i].indentRank;
		if (i == 0) {
			start = ir.x();
			end = ir.y();
		} else {
			end += ir.x() + ir.y();
		}
	}
	return QPoint(start, end);
}

KumTools *KumTools::m_instance = NULL;

KumTools *KumTools::instance()
{
	if (m_instance == NULL) {
		m_instance = new KumTools();
	}
	return m_instance;
}

bool KumTools::isBooleanConstant(const QString &text) const
{
	return constNames.contains(text);
}

void KumTools::setExtraCharset(const QString &charsetSpec)
{
	m_extraCharRanges.clear();
	m_extraChars.clear();
	bool ok = false;
	bool ok1 = false;
	bool ok2 = false;
	ushort unicode;
	ushort start;
	ushort end;
	QStringList specs = charsetSpec.split(",");
	foreach (const QString &spec, specs) {
		if (spec.contains("-")) {
			QStringList p = spec.split("-");
			if (p.count() > 1) {
				start = p[0].simplified().toUShort(&ok1, 16);
				end = p[1].simplified().toUShort(&ok2, 16);
				if (ok1 && ok2) {
					m_extraCharRanges << QPair<ushort, ushort>(start, end);
				}
			}
		} else {
			unicode = spec.simplified().toUShort(&ok, 16);
			if (ok) {
				m_extraChars << unicode;
			}
		}
	}
}

bool KumTools::isValidChar(const QChar &ch) const
{
	if (ch.unicode() < 128) {
		return true;
	}
	for (int i = 0; i < m_extraChars.count(); i++) {
		if (ch.unicode() == m_extraChars[i]) {
			return true;
		}
	}
	for (int i = 0; i < m_extraCharRanges.count(); i++) {
		QPair<ushort, ushort> range = m_extraCharRanges[i];
		if (ch.unicode() >= range.first && ch.unicode() <= range.second) {
			return true;
		}
	}
	return false;
}

QStringList KumTools::getKeyWords()
{
	return keyWords;
}

QStringList KumTools::getConstNames()
{
	return constNames;
}

QStringList KumTools::getTypeNames()
{
	return typeNames;
}

int KumTools::includeTextOrBytecode(const QString &src, QList<ProgaText> &data)
{
	QStringList lexems = src.split(QRegExp("\\s+"));
	if (lexems.size() < 2) {
		return INCLUDE_NO_NAME;
	} else if (lexems.size() > 2) {
		return INCLUDE_EXTRA_LEXEM;
	}
	const QString lexem2 = lexems[1].trimmed();
	bool dQuoted = lexem2.startsWith("\"") && lexem2.endsWith("\"");
	bool sQuoted = lexem2.startsWith("'") && lexem2.endsWith("'");
//	bool tQuoted = lexem2.startsWith("<") && lexem2.endsWith(">");

	if (!dQuoted && !sQuoted/* && !tQuoted*/) {
		return INCLUDE_INVALID_LEXEM;
	}

	const QString linkData = lexem2.mid(1, lexem2.length() - 2).trimmed();
	QString link;
	foreach (const QChar &c, linkData) {
		link += QChar(c.unicode() - STR_HIDE_OFFSET);
	}
	QStringList lines;
	if (link.toLower().startsWith("http://")) {
		return INCLUDE_HTTP_NOT_SUPPORTED;
		const QUrl url(link);
		if (!url.isValid() || url.host().isEmpty()) {
			return INCLUDE_INVALID_URL;
		} else {
			QByteArray bData;
			int error = m_urlFetcher->GET(url, bData);
			if (error) {
				return error;
			}
			QTextStream ts(&bData);
			ts.setCodec("UTF16-LE");
			lines = ts.readAll().split("\n", QString::KeepEmptyParts);
		}
	} else {
		const QString fileName = link.trimmed();
		QFileInfo fi(fileName);
		QFile f;
		if (fi.isAbsolute()) {
			f.setFileName(fi.absoluteFilePath());
		} else {
			const QString rootDir = app()->settings->value("Directories/StdLib",
					QApplication::applicationDirPath() + "/Kumir/Include/").toString();
			f.setFileName(rootDir + "/" + fileName);
		}
		if (!f.exists()) {
			return INCLUDE_FILE_NOT_EXISTS;
		}
		if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return INCLUDE_FILE_READ_ERROR;
		}
		QTextStream ts(&f);
		ts.setCodec("UTF-16LE");
		lines = ts.readAll().split("\n", QString::KeepEmptyParts);
		f.close();
	}
	int error = 0;
	foreach (const QString &line, lines) {
		QPoint p;
		error |= splitUserLine(line, p, &data);
	}
	if (error) {
		data.clear();
		return INCLUDE_CONTAINS_NORM_ERRORS;
	} else {
		for (int i = 0; i < data.size(); i++) {
			data[i].setError(0);
			data[i].stroka = -1;
		}
	}
	return 0;
}

UrlFetcher::UrlFetcher(QObject *parent)
	: QThread(parent)
	, m_buffer(0)
	, m_accessManager(0)
	, i_result(0)
{

}

int UrlFetcher::GET(const QUrl &url, QByteArray &result)
{
	m_url = url;
	m_buffer = &result;
	start();
	wait();
	return i_result;
}


void UrlFetcher::run()
{
	i_result = 0;
	QNetworkRequest request(m_url);
	m_accessManager = new QNetworkAccessManager(0);
	QNetworkReply *reply = m_accessManager->get(request);
	while (!reply->isFinished()) {
		msleep(500);
	}
	if (reply->error() == QNetworkReply::NoError) {
		m_buffer->append(reply->readAll());
		i_result = 0;
	} else if (reply->error() == QNetworkReply::HostNotFoundError) {
		i_result = INCLUDE_NSLOOKUP_ERROR;
	} else if (reply->error() == QNetworkReply::ContentAccessDenied) {
		i_result = INCLUDE_HTTP_401;
	} else if (reply->error() == QNetworkReply::ContentNotFoundError) {
		i_result = INCLUDE_HTTP_404;
	} else {
		i_result = INCLUDE_CONNECTION_ERRROR;
	}
	m_accessManager->deleteLater();
}

QStringList KumTools::finishInput()
{
	QStringList ret;
	foreach (InputTerm word, words) {
		if (word.pType == kumString) {
			if (word.text.startsWith("\"") && word.text.endsWith("\"")) {
				word.text = word.text.mid(1, word.text.length() - 2);
			}
			if (word.text.startsWith("'") && word.text.endsWith("'")) {
				word.text = word.text.mid(1, word.text.length() - 2);
			}
			word.text.replace(QString(QChar(1000)), ",");
			word.text.replace(QString(QChar(1001)), " ");
			word.text.replace(QString(QChar(1002)), "\t");
			word.text.replace(QString(QChar(1003)), "\"");
			word.text.replace(QString(QChar(1004)), "'");
			ret << word.text;
		} else if (word.pType == charect)  {
			QString QUOTE = QString(QChar(1001)) + QString(QChar(1001));
			if (word.text.length() == 3) {
				word.text.remove(0, 1);
				word.text.remove(1, 1);
				word.text.replace(QString(QChar(1000)), ",");
				word.text.replace(QString(QChar(1001)), " ");
				word.text.replace(QString(QChar(1002)), "\t");
				word.text.replace(QString(QChar(1003)), "\"");
				word.text.replace(QString(QChar(1004)), "'");
			}
			if (word.text.length() > 1) {
				word.text = word.text.trimmed();
			}
			if (word.text.isEmpty()) {
				word.text = " ";
			}
			ret << word.text;
		} else if (word.pType == integer) {
			if (word.text.trimmed().startsWith("$")) {
				word.text.remove(0, 1);
				bool ok;
				int value = word.text.toInt(&ok, 16);
				K_ASSERT(ok);
				ret << QString::number(value);
			} else {
				ret << word.text.trimmed();
			}
		} else if (word.pType == real) {
			word.text.replace(QString::fromUtf8("Е"), "E");
			word.text.replace(QString::fromUtf8("е"), "e");
			ret << word.text.trimmed();
		} else {
			ret << word.text.trimmed();
		}
	}
	return ret;
}




bool KumTools::tryFinishInput(const QString &txt, QStringList &result, QString &msg)
{
	msg = "";
	bool ok = validateInput(txt);

	if (ok) {
		result = finishInput();
	} else {
		foreach (InputTerm word, words) {
			if (!word.valid && word.start > -1) {
				msg = QApplication::translate("KumIOArea", "INPUT ");
				QStringList m;
				foreach (InputTerm word, words) {
					QString t;
					if (word.pType == integer) {
						t = QApplication::translate("KumIOArea", "int");
					} else if (word.pType == real) {
						t = QApplication::translate("KumIOArea", "float");
					} else if (word.pType == charect) {
						t = QApplication::translate("KumIOArea", "char");
					} else if (word.pType == kumString) {
						t = QApplication::translate("KumIOArea", "string");
					} else if (word.pType == kumBoolean) {
						t = QApplication::translate("KumIOArea", "bool");
					}
					if (word.valid) {
						m << t;
					} else if (word.text.length() > 0) {
						m << t;
					} else {
						m << t;
					}

				}
				for (int i = words.count(); i < types.count() ; i++) {
					if (types[i] == integer) {
						m << QApplication::translate("KumIOArea", "int");
					}
					if (types[i] == real) {
						m << QApplication::translate("KumIOArea", "float");
					}
					if (types[i] == charect) {
						m << QApplication::translate("KumIOArea", "char");
					}
					if (types[i] == kumString) {
						m << QApplication::translate("KumIOArea", "string");
					}
					if (types[i] == kumBoolean) {
						m << QApplication::translate("KumIOArea", "bool");
					}
				}
				msg += m.join(", ") + ".";
				if (word.text.length() > 0) {
					if (word.error == notInteger) {
						msg += QApplication::translate("KumIOArea", "It is not integer.");
					} else if (word.error == notReal) {
						msg += QApplication::translate("KumIOArea", "It is not real.");
					} else if (word.error == notBool) {
						msg += QApplication::translate("KumIOArea", "It is not kumBoolean.");
					} else if (word.error == unpairedQuote) {
						msg += QApplication::translate("KumIOArea", "Unpaired quote.");
					} else if (word.error == longCharect) {
						msg += QApplication::translate("KumIOArea", "More then one character.");
					} else if (word.error == quoteMisplace) {
						msg += QApplication::translate("KumIOArea", "Quotes misplaced.");
					} else if (word.error == differentQuotes) {
						msg += QApplication::translate("KumIOArea", "Different quotes.");
					}
				} else {
					if (word.pType == integer) {
						msg += QApplication::translate("KumIOArea", "Integer not entered.");
					}
					if (word.pType == real) {
						msg += QApplication::translate("KumIOArea", "Real not entered.");
					}
					if (word.pType == kumBoolean) {
						msg += QApplication::translate("KumIOArea", "Boolean not entered.");
					}
					if (word.pType == charect) {
						msg += QApplication::translate("KumIOArea", "Charect not entered.");
					}
				}
				return ok;
			} else if (!word.valid) {
			}
		}
		if (words.count() < types.count()) {
			msg = tr("INPUT ");
			QStringList m;
			foreach (InputTerm word, words) {
				QString t;
				if (word.pType == integer) {
					t = QApplication::translate("KumIOArea", "int");
				} else if (word.pType == real) {
					t = QApplication::translate("KumIOArea", "float");
				} else if (word.pType == charect) {
					t = QApplication::translate("KumIOArea", "char");
				} else if (word.pType == kumString) {
					t = QApplication::translate("KumIOArea", "string");
				} else if (word.pType == kumBoolean) {
					t = QApplication::translate("KumIOArea", "bool");
				}
				if (word.valid) {
					m << t;
				} else if (word.text.length() > 0) {
					m << t;
				} else {
					m << t;
				}
			}
			for (int i = words.count(); i < types.count(); i++) {
				QString t;
				PeremType pType = types[i];
				if (pType == integer) {
					t = QApplication::translate("KumIOArea", "int");
				} else if (pType == real) {
					t = QApplication::translate("KumIOArea", "float");
				} else if (pType == charect) {
					t = QApplication::translate("KumIOArea", "char");
				} else if (pType == kumString) {
					t = QApplication::translate("KumIOArea", "string");
				} else if (pType == kumBoolean) {
					t = QApplication::translate("KumIOArea", "bool");
				}
				m << t;
			}
			msg += m.join(", ") + ".";
			PeremType pType = types[words.count()];
			if (pType == integer) {
				msg += QApplication::translate("KumIOArea", "Integer not entered.");
			}
			if (pType == real) {
				msg += QApplication::translate("KumIOArea", "Real not entered.");
			}
			if (pType == kumBoolean) {
				msg += QApplication::translate("KumIOArea", "Boolean not entered.");
			}
			if (pType == charect) {
				msg += QApplication::translate("KumIOArea", "Charect not entered.");
			}
			if (pType == kumString) {
				msg += QApplication::translate("KumIOArea", "String not entered.");
			}
		}
		if (inputGarbageStart > -1) {
			msg = QApplication::translate("KumIOArea", "INPUT ");
			QStringList m;
			foreach (InputTerm word, words) {
				QString t;
				if (word.pType == integer) {
					t = QApplication::translate("KumIOArea", "int");
				} else if (word.pType == real) {
					t = QApplication::translate("KumIOArea", "float");
				} else if (word.pType == charect) {
					t = QApplication::translate("KumIOArea", "char");
				} else if (word.pType == kumString) {
					t = QApplication::translate("KumIOArea", "string");
				} else if (word.pType == kumBoolean) {
					t = QApplication::translate("KumIOArea", "bool");
				}
				if (word.valid) {
					m << t;
				} else if (word.text.length() > 0) {
					m << t;
				} else {
					m << t;
				}

			}
			msg += m.join(", ") + ".";
			msg += QApplication::translate("KumIOArea", "Garbage after input string.");
		}
	}
	return ok;
}


int findTermStart2(int termNo, QString line)
{
	int c = 0;
	bool found = false;
	int p = -1;
	for (int i = 0; i < line.length(); i++) {
		if (line[i] == ',') {
			c++;
		}
		if (c == termNo) {
			p = i;
			if (c > 0) {
				p++;
			}
			found = true;
			break;
		}
	}
	if (found && p >= line.length()) {
		p = -1;
	}
	return p;
}

int prepareString2(QString &s)
{
	bool inQuote = false;
	QChar quote;
	int quotePos = -1;
	for (int i = 0; i < s.length(); i++) {
		if (!inQuote) {
			if (s[i] == '"' || s[i] == '\'') {
				inQuote = true;
				quote = s[i];
				quotePos = i;
			}
		} else {
			if (s[i] == quote) {
				inQuote = false;
			}
			if (s[i] == '"' && s[i] != quote) {
				s[i] = QChar(1003);
			}
			if (s[i] == '\'' && s[i] != quote) {
				s[i] = QChar(1004);
			}
			if (s[i] == ' ') {
				s[i] = QChar(1001);
			}
			if (s[i] == '\t') {
				s[i] = QChar(1002);
			}
			if (s[i] == ',') {
				s[i] = QChar(1000);
			}
		}
	}
	if (inQuote) {
		for (int i = quotePos + 1; i < s.length(); i++) {
			if (s[i] == QChar(1003)) {
				s[i] = '"';
			}
			if (s[i] == QChar(1004)) {
				s[i] = '\'';
			}
			if (s[i] == QChar(1001)) {
				s[i] = ' ';
			}
			if (s[i] == QChar(1002)) {
				s[i] = '\t';
			}
			if (s[i] == QChar(1000)) {
				s[i] = ',';
			}
		}
		return quotePos;
	} else {
		return -1;
	}
}

void split2(QString S, QRegExp R, QList<InputTerm> &T)
{
	int cur;
	int prev;
	cur = 0;
	QRegExp nonSpace("\\S");
	prev = nonSpace.indexIn(S) > -1 ? nonSpace.indexIn(S) : 0;
	forever {
		InputTerm t;
		bool stop = false;
		cur = R.indexIn(S, prev);
		if (cur > -1) {
			t.text = S.mid(prev, cur - prev);
			t.allow_left = !(t.text.startsWith("\"") || t.text.startsWith("'"));
			t.allow_right = !(t.text.endsWith("\"") || t.text.endsWith("'"));
			t.allow_right = t.allow_right && (! R.cap().contains(","));
			t.start = prev;
			prev = cur + R.matchedLength();
		} else {
			t.text = S.mid(prev);
			if (t.text.trimmed().isEmpty()) {
				break;
			}
			t.allow_left = !(t.text.startsWith("\"") || t.text.startsWith("'"));
			t.allow_right = false;
			t.start = prev;
			stop = true;
		}
		T << t;
		if (stop) {
			break;
		}
	}
}

void validate2(QList<PeremType> L, QList<InputTerm> S, QList<InputTerm> &T, bool &R, int &G)
{
	int i;
	InputTerm t;
	PeremType l;
	i = 0;
	foreach (l, L) {
		if (i >= S.size()) {
			break;
		}
		t = InputTerm();
		if (l == kumString) {
			t = S[i];
			while (t.allow_right && i < (S.size() - 1) && S[i].allow_left) {
				i = i + 1;
				t = t + S[i];
			}
			if (t.text.contains("\"")) {
				bool valid = t.text.startsWith("\"") && t.text.endsWith("\"") && t.text.count("\"") == 2;
				if (valid) {
					t.error = OK;
				} else {
					if (t.text.count("\"") != 2) {
						t.error = unpairedQuote;
					} else {
						t.error = quoteMisplace;
					}
				}
				t.valid = valid;

			} else if (t.text.contains("'")) {
				bool valid = t.text.startsWith("'") && t.text.endsWith("'") && t.text.count("'") == 2;
				if (valid) {
					t.error = OK;
				} else {
					if (t.text.count("'") != 2) {
						t.error = unpairedQuote;
					} else {
						t.error = quoteMisplace;
					}
				}
				t.valid = valid;
			} else {
				t.valid = true;
			}
		}
		if (l == charect) {
			t = S[i];
			if (t.text.length() == 1) {
				t.valid = true;
				t.error = OK;
			} else if (S[i].text.length() == 3 && (S[i].text[0] == '"' || S[i].text[0] == '\'') && S[i].text[0] == S[i].text[2]) {
				t.error = OK;
				t.valid = true;
			} else {
				bool hasQuote = S[i].text.contains("'") || S[i].text.contains("\"");
				if (S[i].text.length() > 3 || !hasQuote) {
					t.error = longCharect;
				} else if (S[i].text[0] != S[i].text[2]) {
					t.error = differentQuotes;
				} else {
					t.error = unpairedQuote;
				}
				t.valid = false;
			}
		}
		if (l == integer) {
			t = S[i];
			bool ok;
			t.text.toInt(&ok);
			if (!ok && t.text.trimmed().startsWith("$")) {
				QString h = t.text;
				h.remove(0, 1);
				h.toInt(&ok, 16);
			}
			t.valid = ok && IntegerOverflowChecker::checkFromString(t.text);
		}
		if (l == real) {
			t = S[i];
			bool ok;
			t.text.replace(QString::fromUtf8("Е"), "E");
			t.text.replace(QString::fromUtf8("е"), "e");
			t.text.toDouble(&ok);
			t.valid = ok && t.text.toLower() != "nan" && t.text.toLower() != "inf";
			if (t.valid) {
				t.error = OK;
			} else {
				t.error = notReal;
			}
		}
		if (l == kumBoolean) {
			t = S[i];
			bool ok;
			ok = t.text.toLower() == QApplication::translate("KumIOArea", "yes");
			ok = ok || t.text.toLower() == QApplication::translate("KumIOArea", "no");
			t.valid = ok;
			if (ok) {
				t.error = OK;
			} else {
				t.error = notBool;
			}
		}
		t.pType = l;
		T << t;
		i = i + 1;
	}
	if (i < S.size()) {
		R = false;
		G = S[i].start;
	} else if (T.size() < L.size()) {
		R = false;
		G = -1;
	} else {
		R = true;
		G = -1;
		foreach (t, T) {
			R = R && t.valid;
		}
	}
}

bool KumTools::validateInput(QString txt)
{

	prepareString2(txt);
	QList<InputTerm> terms;
	QRegExp rxDelim = QRegExp("(\\s*,\\s*)|(\\s+)");
	split2(txt, rxDelim, terms);
	bool correct;
	words.clear();

	validate2(types, terms, words, correct, inputGarbageStart);
	if (types.count() == 1 && types.first() == kumString) {
		InputTerm term;
		term.pType = kumString;
		term.text = "";
		term.start = 0;
		term.valid = true;
		terms << term;
		words << term;
		correct = true;
	}
	if (txt.trimmed().endsWith(",") && correct && (types.last() != kumString || words.last().text.endsWith("\"") || words.last().text.endsWith("'"))) {
		inputGarbageStart = txt.lastIndexOf(",");
		correct = false;
	} else if (txt.trimmed().endsWith(",") && correct && types.last() == kumString && !(words.last().text.endsWith("\"") || words.last().text.endsWith("'"))) {
		words.last().text += ",";
	}

	QString msg = tr("INPUT ");
	QStringList m;
	foreach (PeremType pType, types) {
		QString t;
		if (pType == integer) {
			t = tr("int");
		} else if (pType == real) {
			t = tr("float");
		} else if (pType == charect) {
			t = tr("char");
		} else if (pType == kumString) {
			t = tr("string");
		} else if (pType == kumBoolean) {
			t = tr("bool");
		}
		m << t;
	}
	msg += m.join(", ");
	return correct;
}

void KumTools::input(const QString &format)
{
	words.clear();
	types.clear();
	formats = format.split("%");
	foreach (QString fmt, formats) {
		if (fmt.isEmpty()) {
			continue;
		}
		PeremType pType;
		if (fmt == "s") {
			pType = kumString;
		}
		if (fmt == "i") {
			pType = integer;
		}
		if (fmt == "c") {
			pType = charect;
		}
		if (fmt == "f") {
			pType = real;
		}
		if (fmt == "b") {
			pType = kumBoolean;
		}
		types << pType;
	}
	inputGarbageStart = -1;
}

bool KumTools::parceInputString(const QString &format
	, const QString &txt
	, QStringList &result
	, QString &error)
{
	input(format);
	return tryFinishInput(txt, result, error);
}

QString KumTools::getKeyWordsHTMLList() const
{
	ProgramTab *pt = dynamic_cast<ProgramTab *>(app()->mainWindow->tab(0));
	SyntaxHighlighter *sh = pt->editor()->syntaxHighlighter();
	Q_CHECK_PTR(sh);
	const QString keyWordStyle = sh->keywordCSS();
	const QString typeStyle = sh->typeCSS();
	const QString valueStyle = sh->valueCSS();
	QPalette p = pt->palette();
	const QString tableHeadingBg = p.color(QPalette::Highlight).name();
	const QString tableHeadingFg = p.color(QPalette::HighlightedText).name();
	const QString fontName = pt->editor()->font().family();
	const QString fontSize = QString::number(pt->editor()->font().pointSize()) + "pt";
	QString css;
	css += "body { background-color: " + p.color(QPalette::Window).name() + "; }\n";
	css += "table { width: 100%; text-align: center; font-family: " + fontName + "; font-size: " + fontSize + "; " +
		"margin-bottom: 10px; margin-top: 10px; margin-left: 0px; margin-right: 0px; spacing: 0px; padding: 0px; border: 1px solid black; }\n";
	css += ".heading { background-color: " + tableHeadingBg + "; " +
		"color: " + tableHeadingFg + "; padding: 3px; }\n";
	css += ".odd { background-color: #ffffff; padding: 3px;} \n";
	css += ".even { background-color: " +
		p.color(QPalette::Highlight).lighter(400).name() +
		"; padding: 3px;} \n";
	css += ".keyword { " + keyWordStyle + " }\n";
	css += ".type { " + typeStyle + " }\n";
	css += ".value { " + valueStyle + " }\n";
	QString head = "<head><style type='text/css'>\n" + css + "</style></head>\n";
	QString body = "<body>\n";
	body += "<table>\n";
	body += "<tr class='heading'><td>" + tr("Language lexems") + "</td></tr>\n";

	int oddEvenCounter;


	oddEvenCounter = 0;

	foreach (QString k, keyWords) {
		QStringList variants = k.split("|");
		QStringList vs;
		const QString trClass = oddEvenCounter % 2 ? "odd" : "even";
		oddEvenCounter++;
		foreach (QString v, variants) {
			const QString vv = "<span class='keyword'>" +
				v.replace("\\s*", "").replace("\\s+", " ").simplified() +
				"</span>";

			vs << vv;
		}
		body += "<tr class='" + trClass + "'><td>" + vs.join(tr(" or ")) + "</td></tr>\n";
	}

	body += "</table>\n";

	body += "<table>\n";
	body += "<tr class='heading'><td>" + tr("Type names") + "</td></tr>\n";

	oddEvenCounter = 0;

	foreach (QString k, typeNames) {
		QStringList variants = k.split("|");
		QStringList vs;
		const QString trClass = oddEvenCounter % 2 ? "odd" : "even";
		oddEvenCounter++;
		foreach (QString v, variants) {
			const QString vv = "<span class='type'>" +
				v.replace("\\s*", "").replace("\\s+", " ").simplified() +
				"</span>";

			vs << vv;
		}
		body += "<tr class='" + trClass + "'><td>" + vs.join(tr(" or ")) + "</td></tr>\n";
	}

	body += "</table>\n";

	body += "<table>\n";
	body += "<tr class='heading'><td>" + tr("Predefined constants") + "</td></tr>\n";

	oddEvenCounter = 0;

	foreach (QString k, constNames) {
		QStringList variants = k.split("|");
		QStringList vs;
		const QString trClass = oddEvenCounter % 2 ? "odd" : "even";
		oddEvenCounter++;
		foreach (QString v, variants) {
			const QString vv = "<span class='value'>" +
				v.replace("\\s*", "").replace("\\s+", " ").simplified() +
				"</span>";

			vs << vv;
		}
		body += "<tr class='" + trClass + "'><td>" + vs.join(tr(" or ")) + "</td></tr>\n";
	}

	body += "</table>\n";
	body += "</body>\n";
	const QString html = "<html>" + head + body + "</html>\n";
	return html;
}
