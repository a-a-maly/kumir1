#include "subprocess.h"
#include <QDebug>

SubProcess::SubProcess(QObject *parent,
	const QString &name,
	const QString &execuable,
	const QStringList &envToAdd,
	bool mergeOutStreams) :
	QProcess(parent),
	m_name(name),
	m_execuable(execuable)
{
	QStringList env = environment();
	env.append(envToAdd);
	setEnvironment(env);
	connect(this, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
	connect(this, SIGNAL(started()), this, SLOT(handleProcessStarted()));
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleProcessFinished(int, QProcess::ExitStatus)));
	connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleProcessError(QProcess::ProcessError)));
	if (mergeOutStreams) {
		setProcessChannelMode(QProcess::MergedChannels);
	} else {
		setProcessChannelMode(QProcess::SeparateChannels);
	}
}

void SubProcess::processData(const QByteArray &data)
{
	m_data = data;
	qDebug() << "starting " << m_execuable;
	start(m_execuable);
}

void SubProcess::handleProcessError(QProcess::ProcessError error)
{
	qDebug() << "Process finished with error: " << error;

}

void SubProcess::handleProcessFinished(int code, QProcess::ExitStatus status)
{
	int ret = code;
	QString errorMsg;
	QByteArray toRet;
	if (status == QProcess::CrashExit) {
		ret = -1;
		errorMsg = tr("Segmentation fault");
		toRet = errorMsg.toUtf8();
	} else {
		if (code == 0) {
			toRet = m_reply;
		} else {
			toRet = m_error;
			toRet += " (" + tr("error code") + ": " + QString::number(code) + ")";
		}
	}
	emit processFinished(m_name, toRet, ret);
	deleteLater();
}

void SubProcess::handleReadyRead()
{
	m_reply += readAllStandardOutput();
	m_error += readAllStandardError();
}

void SubProcess::handleProcessStarted()
{
	write(m_data);
	closeWriteChannel();
}
