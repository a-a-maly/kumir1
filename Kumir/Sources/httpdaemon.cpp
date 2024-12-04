//
// C++ Implementation: httpdaemon
//
// Description:
//
//
// Author: suboch <suboch@suboch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//

#include "httpdaemon.h"
#include "application.h"
#include "compiler.h"
#include "kummodules.h"
#include "kumsinglemodule.h"
#include <QTcpSocket>

const quint16 HttpDaemon::defaultPort = 4243;

HttpDaemon::HttpDaemon(quint16 port, QObject *parent) : QTcpServer(parent)
{
	if (listen(QHostAddress::Any, port)) {
		disabled = false;
	} else {
		disabled = true;
	}
}

bool HttpDaemon::isEnabled()
{
	return (!disabled);
}

void HttpDaemon::incomingConnection(int socket)
{
	if (disabled) {
		return;
	}

	// When a new client connects, the server constructs a QTcpSocket and all
	// communication with the client is done over this QTcpSocket. QTcpSocket
	// works asynchronously, this means that all the communication is done
	// in the two slots readClient() and discardClient().
	QTcpSocket *s = new QTcpSocket(this);
	connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
	s->setSocketDescriptor(socket);
}

void HttpDaemon::pause()
{
	disabled = true;
}

void HttpDaemon::resume()
{
	disabled = false;
}

void HttpDaemon::readClient()
{
	if (disabled) {
		return;
	}

	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	QTcpSocket *socket = (QTcpSocket *)sender();
	if (socket->canReadLine()) {
		QTextStream os(socket);
		os.setCodec("UTF-8");
		QString headerString;
		while (socket->canReadLine()) {
			QString t(socket->readLine());
			headerString += t;
			if (t == "\r\n") {
				break;
			}
		}
#if QT_VERSION >= 0x050000
		os << "HTTP/1.0 501 Not implemented\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
#else
		QString dir(app()->settings->value("HyperText/Path", defaultPath()).toString());
		if (!(dir.endsWith('\\') || dir.endsWith('/'))) {
			dir += QDir::separator();
		}

		QHttpRequestHeader header(headerString);
		if (header.method() == "GET") {
			QFile htmlFile(dir + (header.path() == "/" ? "index.html" : header.path().mid(1)));
			//qDebug() << htmlFile.fileName();
			if (header.value("Host").startsWith("localhost") && htmlFile.exists()) {
				os << "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
				htmlFile.open(QIODevice::ReadOnly);
				QTextStream ts(&htmlFile);
				ts.setCodec("UTF-8");
				os << ts.readAll();
				htmlFile.close();
			} else {
				QHttpResponseHeader rh(404, "Not Found");
				rh.setContentType("text/html");
				rh.setValue("charset", "utf-8");
				os << rh.toString();
				os << "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY><H1>Not Found</H1></BODY></HTML>";
			}
		} else if (header.method() == "POST") {
			qDebug() << header.toString();
			if (header.path().endsWith("/browser2kumir")) {
				QString pathReq(header.path().mid(1, header.path().length() - 14)); // length("browser2kumir") = 13
				qDebug() << "pathReq=" << pathReq;
				if (socket->bytesAvailable() < header.contentLength()) {
					os << QHttpResponseHeader(100, "Continue").toString();
					socket->waitForReadyRead(500);
				}
				if (socket->bytesAvailable() >= header.contentLength()) {
					int code = 200;
					QString responseBody;
					QStringList data(QString::fromUtf8(socket->readAll()).split("\r\n"));
					qDebug() << data;
					if (data.size() >= 2 && data[0] == QString::fromUtf8("Текст программы")) {
						emit setProgramText(data[1]);
					} else if (data.size() >= 2 && data[0] == QString::fromUtf8("setProgramFile")) {
						emit setProgramFile(dir + pathReq + data[1]);
					} else if (data.size() >= 1 && data[0] == QString::fromUtf8("Выполнить")) {
						emit runProgram();
					} else if (data.size() >= 1 && data[0] == QString::fromUtf8("Большой Шаг")) {
						emit doSTEP();
					} else if (data.size() >= 1 && data[0] == QString::fromUtf8("Малый Шаг")) {
						emit doSmallStep();
					} else if (data.size() >= 1 && data[0] == QString::fromUtf8("Stop")) {
						emit stop();
					} else if (data.size() >= 2 && data[0] == QString::fromUtf8("Выполнить алгоритм")) {
						emit runAlg(data[1], -1);
					} else if (data.size() >= 4 && data[0] == QString::fromUtf8("Значение")) {
						KumModules *modules = &app()->compiler->Modules;
						QString value(tr("n/a"));
						for (int i = 0; i < modules->lastModuleId() + 1; ++i) {
							if (modules->module(i)->Name() == data[1]) {
								symbol_table *symbols = modules->module(i)->Values();
								for (int j = 0; j < symbols->symb_table.size(); ++j) {
									if (symbols->symb_table[j].alg == data[2] && symbols->symb_table[j].name == data[3]) {
										bool initialized = symbols->symb_table[j].used;
										if (symbols->isRes(j) || symbols->isArgRes(j)) {
											PeremPrt link = symbols->pointTo(j);
											if (!link.mod_id == -1 && !link.perem_id == -1) {
												value = modules->module(link.mod_id)->Values()->symb_table[link.perem_id].value.getValueAsString();
												initialized = modules->module(link.mod_id)->Values()->symb_table[link.perem_id].used;
											}
										} else {
											if (initialized) {
												value = symbols->symb_table[j].value.getValueAsString();
											}
										}
									}
								}
							}
						}
						responseBody = value;
					} else if (data.size() >= 1 && data[0] == QString::fromUtf8("checkConnect")) {
						responseBody = QString::fromUtf8("OK. Соединение с Кумиром установлено, порт ") + QString::number(this->serverPort()) + '.';
					} else if (data.size() >= 2 && data[0] == QString::fromUtf8("setRobotField")) {
						emit setRobotEnvironment(dir + pathReq + data[1]);
					} else {
						code = 501;
						QHttpResponseHeader rh(501, "Not Implemented");
						rh.setContentType("text/html");
						rh.setValue("charset", "utf-8");
						os << rh.toString();
						os << QString::fromUtf8("Кумир не смог распознать команду");
					}
					if (code == 200) {
						QHttpResponseHeader rh(200, "OK");
						rh.setContentType("text/html");
						rh.setValue("charset", "utf-8");
						os << rh.toString();
						if (!responseBody.isEmpty()) {
							os << responseBody;
						}
					}
				} else {
					QHttpResponseHeader rh(400, "Bad Request");
					os << rh.toString();
					os << "No enough data in request body";
				}
			} else {
				QHttpResponseHeader rh(404, "Not Found");
				os << rh.toString();
			}
		} else {
			QHttpResponseHeader rh(400, "Bad Request");
			os << rh.toString();
			os << "Bad method";
		}
#endif
		socket->close();
		if (socket->state() == QTcpSocket::UnconnectedState) {
			delete socket;
		}
	}
}

void HttpDaemon::discardClient()
{
	QTcpSocket *socket = (QTcpSocket *)sender();
	socket->deleteLater();
}

QString HttpDaemon::defaultPath()
{
	return QApplication::applicationDirPath() + "/Kumir/HyperText/";
}
