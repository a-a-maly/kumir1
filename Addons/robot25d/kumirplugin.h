#ifndef KUMIRPLUGIN_H
#define KUMIRPLUGIN_H

#include "../plugin_interface.h"
#include <QVariant>

class Robot25DWindow;

class Robot25DPlugin : public QObject, public kumirPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(kumirPluginInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_PLUGIN_METADATA(IID "kumir1.Robot25D")
#endif
public:
	Robot25DPlugin();

	void start(); //Запуск исполнителя
	QList<Alg> algList(); //Список алгоритмов исполнителя
	QString name();
	QList<QVariant> algOptResults(); //резы
	QVariant result();//возвращаемое значение
	void        runAlg(QString alg, QList<QVariant> params); //Запуск алгоритма
	void        showField(); //Показать поле
	void        showPult();//Показать пульт
	void        hidePult();
	void        hideField();
	void        showOpt();
	bool        hasPult(); //Есть ли пульт
	bool        hasOpt(); //Есть ли дополнительное окно
	QString     optText();//Текст для дополнительного окна
	void          setMode(int mode); //Установить режим
	void reset();
	QString errorText() const; //Текст ошибки


	/** Выполняет проверку задания
	  * @param script опциональный ECMAScript-скрипт проверки
	  * @returns оценка в диапазоне [1:10] */
	quint8 check(const QString &script = QString());


	/**
	 Установка дополнительных параметров исполнителя.
	 Например: setParameter("environment", QVariant("test.vod"));
	 Установка обстановки test.vod в водолее.
	  */
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

	QUrl     pdfUrl() const ; //Помощь
	QUrl     infoXmlUrl() const; // Ссылка на информацию об исполнителе
	QWidget *mainWidget();//Главный виджет исполнителя
	QWidget *pultWidget();//Пульт исполнителя
private slots:
	void handleEvaluationFinished();
signals:
	void sync();
	void error(const QString &);
private:
	QVariant v_result;
	QString s_error;
	Robot25DWindow *m_window;
};

#endif // KUMIRPLUGIN_H
