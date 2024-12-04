#include "programacceptor.h"

ProgramAcceptor::ProgramAcceptor(QObject *parent, const QString &name, const QString &execuable) :
	QProcess(parent),
	m_name(name),
	m_execuable(execuable)
{
}

void ProgramAcceptor::processData(const QByteArray &data)
{
	m_data = data;
	connect(this, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
	connect(this, SIGNAL(started()), this, SLOT(handleProcessStarted()));
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleProcessFinished(int, QProcess::ExitStatus)));
	connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleProcessError(QProcess::ProcessError)));
	qDebug() << "starting " << m_execuable;
	start(m_execuable);
}

void ProgramAcceptor::handleProcessError(QProcess::ProcessError error)
{
	Q_UNUSED(error);
}

void ProgramAcceptor::handleProcessFinished(int code, QProcess::ExitStatus status)
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

void ProgramAcceptor::handleReadyRead()
{
	m_reply += readAllStandardOutput();
	m_error += readAllStandardError();
}

void ProgramAcceptor::handleProcessStarted()
{
	write(m_data);
	closeWriteChannel();
}
