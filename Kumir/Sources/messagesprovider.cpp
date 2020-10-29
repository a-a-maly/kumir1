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
#include "messagesprovider.h"
#include "application.h"
#include "error.h"
#include "config.h"

MessagesProvider::MessagesProvider(QObject *parent) : QObject(parent)
{

}


void MessagesProvider::init(const QString &languageFileName)
{
	map.clear();
	QString msgFilePath;
	QString appDirPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
	msgFilePath = QDir::cleanPath(appDirPath + "/../Resources/" + languageFileName);
#else
	msgFilePath = appDirPath + "/Kumir/Languages/" + languageFileName;
#endif
	QFile f(msgFilePath);
	if (!f.open(QIODevice::ReadOnly)) {
		qDebug("Warning: error messages file \"%s\" not found! Error messages will be empty.", languageFileName.toUtf8().data());
		return;
	}
	QTextStream ts(&f);
	ts.setCodec("UTF-8");
	QString data = ts.readAll();
	f.close();
	QStringList lines = data.split("\n");
	foreach (QString line, lines) {
		if (!line.startsWith("#")) {
			uint code;
			QString msg;
			QStringList pair = line.split(QRegExp("\\s+"));
			if (pair.count() >= 1) {
				bool ok;
				code = pair[0].toUInt(&ok);
				if (pair.count() >= 2) {
					for (int i = 1; i < pair.count(); i++) {
						msg += pair[i].simplified() + " ";
					}
				}
				if (ok) {
					map[code] = msg;
				}
			}
		}
	}
}


QString MessagesProvider::errorMessage(uint code)
{
	if (code == ISP_INT_ERROR) {
		return app()->ispErrorText();
	};
	if (map.keys().contains(code)) {
		QString result = map[code];
#ifdef SHOW_ERROR_CODES
		{
			result += " (" + QString::number(code) + ")";
		}
#endif
		return result;
	} else {
		return QString(tr("Error")) + " " + QString::number(code);
	}
}

MessagesProvider::~MessagesProvider()
{
}


