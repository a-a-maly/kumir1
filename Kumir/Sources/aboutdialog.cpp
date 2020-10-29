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

#include "aboutdialog.h"
#include "config.h"

AboutDialog::AboutDialog(QWidget *parent, Qt::WindowFlags fl)
	: QDialog(parent, fl), Ui::AboutDialog()
{
	setupUi(this);
	QString label2Text = label_2->text();
	QString day;
	QString month;
	QString year;
	QString version;
#ifndef Q_CC_MSVC

#ifdef BUILD_DAY
	day = QString::number(BUILD_DAY);
#endif

#ifdef BUILD_MONTH
	if (BUILD_MONTH == 1) {
		month = QString::fromUtf8("января");
	} else if (BUILD_MONTH == 2) {
		month = QString::fromUtf8("февраля");
	} else if (BUILD_MONTH == 3) {
		month = QString::fromUtf8("марта");
	} else if (BUILD_MONTH == 4) {
		month = QString::fromUtf8("апреля");
	} else if (BUILD_MONTH == 5) {
		month = QString::fromUtf8("мая");
	} else if (BUILD_MONTH == 6) {
		month = QString::fromUtf8("июня");
	} else if (BUILD_MONTH == 7) {
		month = QString::fromUtf8("июля");
	} else if (BUILD_MONTH == 8) {
		month = QString::fromUtf8("августа");
	} else if (BUILD_MONTH == 9) {
		month = QString::fromUtf8("сентября");
	} else if (BUILD_MONTH == 10) {
		month = QString::fromUtf8("октября");
	} else if (BUILD_MONTH == 11) {
		month = QString::fromUtf8("ноября");
	} else if (BUILD_MONTH == 12) {
		month = QString::fromUtf8("декабря");
	}
#endif

#ifdef BUILD_YEAR
	year = QString::number(BUILD_YEAR);
#endif
	if (!day.isEmpty() && !month.isEmpty() && !year.isEmpty()) {
		label2Text.replace("%date", day + " " + month + " " + year + QString::fromUtf8(" г."));
	} else {
		label2Text.replace("(%date)", "");
	}

#ifdef VERSION_MAJOR
	version += QString::number(VERSION_MAJOR);
#endif
#ifdef VERSION_MINOR
	version += QString(".") + QString::number(VERSION_MINOR);
#endif
#ifdef VERSION_RELEASE
	version += QString(".") + QString::number(VERSION_RELEASE);
#endif
#ifdef REVISION
	version += QString(" (Rev. ") + QString::number(REVISION) + ")";
#endif

#endif
	label2Text.replace("%version", version);
	label_2->setText(label2Text);

	QString s;
	s = QString::fromUtf8("<p><b>КуМир</b> (<b>К</b>омплект <b>У</b>чебных <b>МИР</b>ов) — система программирования, предназначенная для поддержки начальных курсов информатики и программирования в средней и высшей школе.</p>\n");

	s += QString::fromUtf8("<p>Система КуМир поддерживает преподавание по учебникам (общий тираж – около 9 млн. экз.):<p>\n");

	s += QString::fromUtf8("<p><small>\
	1.Основы информатики и вычислительной техники: Пробный учеб. для средних учебных заведений. / А. П. Ершов, А. Г. Кушниренко, Г. В. Лебедев, А. Л. Семенов, А. Х. Шень. – М.: Просвещение, 1988. – 207 с.<br>\
	2.Основы информатики и вычислительной техники : Учеб. для 10-11 кл. общеобразоват. учреждений / А. Г. Кушниренко, Г. В. Лебедев, Р. А. Сворень. – 4-е изд. – М. : Просвещение, 1996. – 223 c.<br>\
	3. Информатика : 7-9 кл.: Учеб. для общеобразоват. учр. / А. Г. Кушниренко, Г. В. Лебедев, Я. Н. Зайдельман. – 4-е изд., стер. – М.: Дрофа, 2003. – 335 c.<br>\
	4. Информационная культура. Кодирование информации. Информационные модели:  9-10 кл.: Учеб. для общеобразоват. учр. / А. Г. Кушниренко, А. Г. Леонов, М. Г. Эпиктетов и др. – 6-е изд., стер. – М.: Дрофа, 2003. – 199 c.<br>\
	5. Информатика : алгоритмика : Учеб. для 6 кл. общеобразоват. учр. / А. К. Звонкин, С. К. Ландо, А. Л. Семенов. – М.: Просвещение, 2006. – 237 c.<br>\
	6. Информатика: алгоритмика : Учеб. для 7 кл. общеобразоват. учр. / С. К. Ландо, А. Л. Семенов, М. Н. Вялый. – М.: Просвещение, 2008. – 207 c.\
	</small></p>\n");

	s += QString::fromUtf8("<p>Программа КуМир разработана в Российской Академии наук в рамках плановых работ и распространяется свободно на условиях лицензии GNU GPL v.2, текст которой включен в поставку КуМир.<br>\n");
	s += QString::fromUtf8("Данная лицензия разрешает Вам или Вашей организации бессрочно использовать КуМир на любом количестве компьютеров в любых целях без оформления каких-либо дополнительных документов.</p>\n");

	s += QString::fromUtf8("<p>Любые вопросы по использованию системы КуМир направляйте в Научно-исследовательский институт системных исследований Российской академии наук (НИИСИ РАН):<br>\n");
	s += QString::fromUtf8("<b>117218, Москва, Нахимовский просп., 36, к.1</b><br>\
телефон: <b>719-76-51</b>, факс: <b>719-76-81</b>, e-mail: <b>mail@niisi.ru</b></p>");

	label->setText(s);

}

AboutDialog::~AboutDialog()
{
}

/*$SPECIALIZATION$*/




