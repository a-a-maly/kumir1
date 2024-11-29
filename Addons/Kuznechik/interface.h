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

#include "../plugin_interface.h"
class KumKuznec;
class KNPServer;
class GrasshopperPult;


class KuznecStarter: public QObject, public kumirPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(kumirPluginInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_PLUGIN_METADATA(IID "kumir1.Kuznec")
#endif

public:
	void start(); //Запуск исполнителя
	QList<Alg> algList(); //Список алгоритмов исполнителя

	QString name()
	{
		return trUtf8("Кузнечик");
	}

	QList<QVariant> algOptResults(); //резы
	QVariant result();

	void runAlg(QString alg, QList<QVariant> params);

	QString errorText() const
	{
		return errortext;
	}

	void showField();
	void hideField();
	void showPult();
	void hidePult();
	void reset();

	void showOpt() {};

	bool hasPult()
	{
		return true;
	}

	bool hasOpt()
	{
		return false;
	}

	QString optText()
	{
		return "";
	}

	void setMode(int mode);

	quint8 check(const QString &)
	{
		return 10;
	}

	void setParameter(const QString &paramName, const QVariant &paramValue);

	/**
	  Присоединяет слот @param method объекта @param obj к сигналу "Отправить текст"
	  Пример использования:
	    plugin->connectSignalSendText(kumir, SLOT(slotName(QString))
	  */
	void connectSignalSendText(const QObject *obj, const char *method);

	/**
	  Присоединяет слот @param method объекта @param obj к сигналу "Выполнение команды завершено"
	  Пример использования:
	    plugin->connectSignalSendSync(kumir, SLOT(slotName(bool))
	  */
	void connectSignalSync(const QObject *obj, const char *method);

	void connectSignal(const QObject *obj, const char *method)
	{
		Q_UNUSED(obj);
		Q_UNUSED(method);
		return;
	}

	QWidget *mainWidget();
	QWidget *pultWidget();

	virtual QUrl pdfUrl() const; //Помощь
	virtual QUrl infoXmlUrl() const; // Ссылка на информацию об исполнителе

signals:
	void sync();
	void sendText(QString text);

public slots:
	void sendText2Kumir(QString text);

private:
	void openServerPort(int port);

	int mode;
	KumKuznec *mw;
	GrasshopperPult *t_pult;
	KNPServer *server;
	QString errortext;
};

