/***************************************************************************
 *   Copyright (C) 2008 by Nikolay Suboch                                  *
 *   suboch@inbox.ru                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "pwrulesnormalizer.h"
#include <QIODevice>
#include <QBuffer>
#include <QFile>
#include <QDebug>

bool PWRulesNormalizer::isTerminal(const QString &str)
{
	return QRegExp(QString::fromUtf8("[a-z|а-я][a-z|а-я|_]*")).exactMatch(str);
}

bool PWRulesNormalizer::isNeterminal(const QString &str)
{
	return QRegExp(QString::fromUtf8("[А-Я|A-Z][А-Я|A-Z|1-9|_*]*")).exactMatch(str);
}

bool PWRulesNormalizer::isNagruzka(const QString &str)
{
	return str.startsWith("{") && str.endsWith("}");
}

int PWRulesNormalizer::process(const QStringList &files, QIODevice *out, QList<int> priors)
{
	if (priors.isEmpty()) {
		for (int i = 0; i < files.size(); ++i) {
			priors.append(1);
		}
	}

	int iskNet = 0;

	if (files.isEmpty()) {
		qDebug() << QString::fromUtf8("Нет .rules-файлов");
		return 2;
	}

	QBuffer filIn1;
	filIn1.open(QIODevice::WriteOnly);

	QTextStream sIn1(&filIn1);
	sIn1.setCodec("UTF-8");


	int fileNum = -1;
	foreach (QString ifn, files) {

		++fileNum;
		QFile filIn(ifn);

		filIn.open(QIODevice::ReadOnly);

		QTextStream sIn(&filIn);
		sIn.setCodec("UTF-8");


		for (; !sIn.atEnd() ;) {
			QString str(sIn.readLine().trimmed());
			int openFig = 0;
			int i = 0;
			do {
				for (; i < str.length(); ++i) {
					if (str.at(i) == '{') {
						++openFig;
					}
					if (str.at(i) == '}') {
						--openFig;
					}
				}
				if (openFig == 0) {
					if (!str.startsWith("#") && !str.isEmpty()) {
						sIn1 << str << " [" << priors[fileNum] << "]\n";
					}
					break;
				} else {
					if (!sIn.atEnd()) {
						str += sIn.readLine();
					} else {
						sIn1 << "Syntax error in {}:\n";
						sIn1 << str << endl;
						return 3;
					}
				}
			} while (true);
		}
		filIn.close();
	}

	filIn1.close();

//  filIn.setFileName(ifn+".1");
	QBuffer filIn;
	filIn.setData(filIn1.data());
	filIn.open(QIODevice::ReadOnly);
//  sIn.setDevice(&filIn);
	QTextStream sIn(&filIn);
	sIn.setCodec("UTF-8");
	//QFile filOut("output.txt");
	//filOut.open(QIODevice::WriteOnly);
	if (!out->open(QIODevice::WriteOnly)) {
		return 0;
	}
	//QTextStream sOut1(&filOut);
	QTextStream sOut(out);
	sOut.setCodec("UTF-8");

	QStringList pravila1;
	for (; !sIn.atEnd() ;) {
		QString str(sIn.readLine().trimmed());
		int i = str.indexOf('#');
		if (i != -1) {
			str.truncate(i);
		}
		str = str.trimmed();
		if (!str.isEmpty()) {
			pravila1 << str;
		}
	}
//  foreach ( QString s, pravila1 )
//      qDebug() << "pravila1-- " << s;
//  qDebug() << "Before sort";
//  for ( int i=0; i<pravila1.count(); i++ )
//      qDebug() << "pravila1["<<i<<"]: "<<pravila1[i];
	pravila1.sort();
//  qDebug() << "After sort";
//  for ( int i=0; i<pravila1.count(); i++ )
//      qDebug() << "pravila1["<<i<<"]: "<<pravila1[i];

//  foreach ( QString s, pravila1 )
//      qDebug() << "pravila1:: " << s;

	QStringList nagruzki1;
	QList<int> priorities;
	for (int i = 0; i < pravila1.size(); ++i) {
		QString str = pravila1[i];
		int d = str.indexOf(':');
		int d1 = str.lastIndexOf(':');
		if (d != d1) {
			qDebug() << QString::fromUtf8("Два двоеточия в строке: ") << str << endl;
			return 1;
		}
		int e = str.lastIndexOf('[');
//		int e1 = str.lastIndexOf('[');
//      if (e!=e1)
//      {
//          qDebug()<<QString::fromUtf8("Два приоритета в строке: ")<<str<<endl;
//          return 1;
//      }
		if (e == -1) {
			qDebug() << QString::fromUtf8("Не задан приоритет строки: ") << str << endl;
			return 1;
		}
		if (d != -1) {
			nagruzki1 << str.mid(d + 1, e - d - 1);
			pravila1[i].truncate(d);
		} else {
			nagruzki1 << QString();
			pravila1[i].truncate(e);
		}
		QString p(str.mid(e).trimmed());
		if (!p.startsWith("[") && !p.endsWith("]")) {
			qDebug() << QString::fromUtf8("Ошибка задания приоритета в строке: ") << str << endl;
			return 1;
		}
		priorities << p.mid(1, p.length() - 2).toInt();
	}

	QList<QStringList> pravila;
	foreach (QString s, pravila1) {
//      qDebug() << "plavila1: " << s;
		pravila << s.split(QRegExp("\\s+"), QString::SkipEmptyParts);
		//qDebug()<<pravila.last()[0]<<isNeterminal(pravila.last()[0]);
//      qDebug() << "plavila: " << pravila.last();
		if (!isNeterminal(pravila.last()[0]) || pravila.last()[1] != "->" || pravila.last().count() < 3) {
			qDebug() << QString::fromUtf8("Ошибка в левой части: ") << pravila.last().join(" ") << endl;
			return 1;
		}
	}

	for (int i = 0; i < nagruzki1.size(); ++i) {
		if (nagruzki1[i].isEmpty()) {
			for (int j = 0; j < pravila[i].size(); ++j) {
				if (isTerminal(pravila[i][j])) {
					nagruzki1[i] += "{}";
				}
			}
		}
	}

	QList<QStringList> nagruzki;
	int ind = 0;
	foreach (QString s, nagruzki1) {
		QStringList rez;
		int countNagr = 0;
		int i, j;
		for (i = 0; i < s.length() - 1;) {
			j = i + 1;
			int openFig = 0;
			if (s.at(i) == '{') {
				openFig = 1;
				for (; j < s.length(); ++j) {
					if (s.at(j) == '{') {
						++openFig;
					}
					if (s.at(j) == '}') {
						--openFig;
					}
					if (openFig == 0) {
						break;
					}
				}
				rez << s.mid(i, j - i + 1);
				++countNagr;
			}
			i = j;
		}
		int countTerm = 0;
		foreach (QString str, pravila[ind]) {
			if (isTerminal(str)) {
				++countTerm;
			}
		}
		if (countTerm != countNagr) {
			qDebug() << QString::fromUtf8("Не совпадает число терминалов и нагрузок: ") << pravila[ind].join(" ") << " : " << s;
			return 1;
		}
		nagruzki << rez;
		++ind;
	}
	int c;

	QStringList sorts;

	for (bool flag = true ; flag ;) {
		QString s;
		for (int i = 0; i < pravila.size(); ++i) {
			s += pravila[i][0];
			s += " ";
			s += pravila[i][2];
			s += " ; ";
		}
		if (sorts.indexOf(s) != -1) {
			qDebug() << QString::fromUtf8("Данная грамматика является леворекурсивной\n");
			return 1;
		} else {
			sorts << s;
		}
		flag = false;
		for (int i = 1; i < pravila.size(); ++i) {
			for (int j = 0; j < i; ++j) {
				if (isNeterminal(pravila[i][2]) && pravila[i][2] == pravila[j][0]) {
					QString s1(pravila[j][0]);
					QString s2(pravila[i][0]);

					int k1, k2;
					for (int k = pravila.size() - 1; k >= 0; --k) {
						if (pravila[k][0] == s1) {
							k1 = k;
						}
						if (pravila[k][0] == s2) {
							k2 = k;
						}
					}
					for (int shift = 0 ; pravila[k2 + shift][0] == s2 ; ++shift) {
						pravila.move(k2 + shift, k1 + shift);
						nagruzki.move(k2 + shift, k1 + shift);
						priorities.move(k2 + shift, k1 + shift);
					}
					flag = true;
				}
			}
		}
	}

	for (int i = pravila.size() - 1; i >= 0; --i) {
		QList<QStringList> add;
		QList<QStringList> addNagr;
		QList<int> addPriors;
		for (int j = i + 1; j < pravila.size(); ++j) {
			if (pravila[j][0] == pravila[i][2] && (isTerminal(pravila[j][2]) || isNeterminal(pravila[j][2]))) {
				QStringList sl;
				sl << pravila[i][0] << "->" << pravila[j].mid(2, pravila[j].size() - 2) << pravila[i].mid(3, pravila[i].size() - 3);
				add << sl;
				sl.clear();
				sl << nagruzki[j] << nagruzki[i];
				addNagr << sl;
				int a = priorities[i];
				int b = priorities[j];
				addPriors << (a > b ? a : b);
			}
		}
		for (int j = 0; j < add.size(); ++j) {
			pravila.insert(i + j + 1, add[j]);
			nagruzki.insert(i + j + 1, addNagr[j]);
			priorities.insert(i + j + 1, addPriors[j]);
		}
		if (add.size() > 0) {
			pravila.removeAt(i);
			nagruzki.removeAt(i);
			priorities.removeAt(i);
		}
	}

	for (int i = pravila.size() - 1; i >= 0; --i) {
		QList<int> nirps;
		for (int j = 2; j < pravila[i].size(); ++j)
			if (isTerminal(pravila[i][j])) {
				nirps << j;
			}
		if (nirps.size() > 1) {
			nirps << pravila[i].size();
			QList<QStringList> add;
			QList<QStringList> addNagr;
			QList<int> addPriors;
			bool flag = false;
			for (int j = 0; j < nirps.size() - 1; ++j) {
				QStringList sl;
				QStringList slNagr;
				if (!flag) {
					sl << pravila[i][0];
					flag = true;
				} else {
					sl << QString::number(iskNet);
				}
				sl << "->" << pravila[i].mid(nirps[j], nirps[j + 1] - nirps[j]);
				slNagr << nagruzki[i][j];
				if (j < nirps.size() - 2) {
					++iskNet;
					sl << QString::number(iskNet);
				}
				add << sl;
				addNagr << slNagr;
				addPriors << priorities[i];
			}
			for (int j = 0; j < add.size(); ++j) {
				pravila.insert(i + j + 1, add[j]);
				nagruzki.insert(i + j + 1, addNagr[j]);
				priorities.insert(i + j + 1, addPriors[j]);
			}
			if (add.size() > 0) {
				pravila.removeAt(i);
				nagruzki.removeAt(i);
				priorities.removeAt(i);
			}
		}
	}

	/*sOut << "Posle razbitija:\n";
	c = 0;
	foreach(QStringList sl, pravila)
	{
	foreach(QString s, sl)
	{
	sOut << s << " ";
	}
	if (!nagruzki[c].isEmpty())
	sOut << ":";
	foreach(QString s, nagruzki[c])
	{
	sOut << " " << s;
	}
	sOut << endl;
	++c;
	}
	sOut << endl;*/

	for (c = 0; c < pravila.size(); ++c) {
		QStringList sl(pravila[c]);
		QString pravilo(sl.join(" "));
		QString nagruzka(nagruzki[c].join(" "));
		int priority = priorities[c];
		for (int j = c + 1; j < pravila.size();) {
			QString pravilo1(pravila[j].join(" "));
			QString nagruzka1(nagruzki[j].join(" "));
			int priority1 = priorities[j];
			if (pravilo == pravilo1) {
				if (nagruzka == nagruzka1 && priority == priority1) {
					//qDebug() << "Dublicates:";
					//qDebug() << pravilo << " : " << nagruzka;
					//qDebug() << pravilo1 << " : " << nagruzka1;
					pravila.removeAt(j);
					nagruzki.removeAt(j);
					priorities.removeAt(j);
				} else {
					qDebug() << "Dva pravila s raznoj nagruzkoj ili raznymi prioritetami:";
					qDebug() << pravilo << " : " << nagruzka << " [" << priority << "]";
					qDebug() << pravilo1 << " : " << nagruzka1 << " [" << priority1 << "]";
					++j;
				}
			} else {
				++j;
			}
		}
	}

	//sOut1 << "Posle udalenija dublej:\n";
	c = 0;
	foreach (QStringList sl, pravila) {
		foreach (QString s, sl) {
			sOut << s << " ";
			//sOut1 << s << " ";
		}
		if (!nagruzki[c].isEmpty()) {
			sOut << ":";
			//sOut1 << ":";
		}
		foreach (QString s, nagruzki[c]) {
			sOut << " " << s;
			//sOut1 << " " << s;
		}
		sOut << " [" << priorities[c] << "]\n";
		//sOut1 << " [" << priorities[c] << "]\n";
		++c;
	}
	sOut << endl;
	//sOut1 << endl;

	//filOut.close();
	out->close();

	//qDebug() << QString::fromUtf8("Сканирование правил завершено успешно.");
	return 0;
}

