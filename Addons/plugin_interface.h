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
#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>
#include <QUrl>


struct Alg {
	QString kumirHeader; // строка описания для КуМир
};

class kumirPluginInterface
{
public:
	virtual void        start() = 0;            // Запуск исполнителя
	virtual QList<Alg>  algList() = 0;          // Список алгоритмов исполнителя
	virtual QString     name() = 0;
	virtual QList<QVariant> algOptResults() = 0; //резы
	virtual QVariant    result() = 0;           // возвращаемое значение
	virtual void        runAlg(QString alg, QList<QVariant> params) = 0; //Запуск алгоритма
	virtual void        showField() = 0;        // Показать поле
	virtual void        showPult() = 0;         // Показать пульт
	virtual void        hidePult() = 0;         // Спрятать пульт
	virtual void        hideField() = 0;        // Спрятать поле
	virtual void        showOpt() = 0;
	virtual bool        hasPult() = 0;          //Есть ли пульт
	virtual bool        hasOpt() = 0;           //Есть ли дополнительное окно
	virtual QString     optText() = 0;          //Текст для дополнительного окна
	virtual void        setMode(int mode) = 0;  //Установить режим
	virtual void reset() = 0;
	virtual QString errorText() const = 0;      //Текст ошибки

	/** Выполняет проверку задания
	  * @param script опциональный ECMAScript-скрипт проверки
	  * @returns оценка в диапазоне [1:10] */
	virtual quint8 check(const QString &script = QString()) = 0;

	/**
	 Установка дополнительных параметров исполнителя.
	 Например: setParameter("environment", QVariant("test.vod"));
	 Установка обстановки test.vod в водолее.
	  */
	virtual void setParameter(const QString &paramName, const QVariant &paramValue) = 0;

	/**
	  Присоединяет слот @param method объекта @param obj к сигналу "Отправить текст"
	  Пример использования:
	    plugin->connectSignalSendText(kumir, SLOT(slotName(QString))
	  */
	virtual void connectSignalSendText(const QObject *obj, const char *method) = 0;

	/**
	  Присоединяет слот @param method объекта @param obj к сигналу "Выполнение команды завершено"
	  Пример использования:
	    plugin->connectSignalSendSync(kumir, SLOT(slotName(bool))
	  */
	virtual void connectSignalSync(const QObject *obj, const char *method) = 0;

	virtual QUrl     pdfUrl() const = 0;           // Помощь
	virtual QUrl     infoXmlUrl() const = 0;       // Ссылка на информацию об исполнителе
	virtual QWidget *mainWidget() = 0;             // Главный виджет исполнителя
	virtual QWidget *pultWidget() = 0;             // Пульт исполнителя
};
Q_DECLARE_INTERFACE(kumirPluginInterface, "kumir.pluginInterface/1.8.0");
#endif
