#ifndef CSINTERFACE_H
#define CSINTERFACE_H

#include <QVariant>
#include <QStringList>

class CSInterface
{
public:
	virtual QString getText() = 0;
	virtual bool startNewTask(QStringList isps) = 0;
	virtual bool setPreProgram(QVariant param) = 0;
	virtual bool setTesting(QString param) = 0;
	virtual void startProgram(QVariant param) = 0;
	virtual void setParam(QString paramname, QString value) = 0;
	virtual int  stepsCount() = 0;
	virtual QVariant runInternalFunc(QString name, QVariant param) = 0;
};

#endif // CSINTERFACE_H

