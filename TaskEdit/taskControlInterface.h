#ifndef TASKCONTROLINTERFACE_H
#define TASKCONTROLINTERFACE_H

#include <QStringList>
#include <QMap>
#include <QRect>

class KumZadanie
{
public:

	KumZadanie()
	{
		isps.clear();
	}

	QString Isp(int no) const
	{
		if (isps.count() <= no) {
			return "";
		}
		return isps[no];
	}

	QString field(QString ispName, int fieldNo)
	{
		QList<QString> ispFields = fields.values(ispName);
		if (fieldNo >= ispFields.count()) {
			return "";
		}
		return ispFields.at(fieldNo);
	}

	int fieldsCount(QString ispName)
	{
		QList<QString> ispFields = fields.values(ispName);

		return ispFields.count();
	}

	QString name;

	//исполнители используемые в задании
	QStringList isps;

	//Обстановки для каждого исполнителя fields[исп,обст]
	QMap<QString, QString> fields;

	QStringList scripts;
};

class CSInterface;
class taskControlInterface
{
public:
	virtual void start(QString csName) = 0;
	virtual void setCSmode(int mode) = 0;
	virtual void setCSinterface(CSInterface *interface) = 0;
	virtual void setWindowGeometry(QRect retc) = 0;
	virtual void show() = 0;
	virtual void checkFinished(int mark) = 0;
	virtual KumZadanie *Task() = 0;
	virtual QString Isp(int no) = 0;
	virtual QString ispName() = 0;
};

Q_DECLARE_INTERFACE(taskControlInterface, "kumir.taskControlInterface/1.7.1");
#endif // TASKCONTROLINTERFACE_H
