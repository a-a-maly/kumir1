#ifndef PAINTERPLUGIN_H
#define PAINTERPLUGIN_H

#include "plugin_interface.h"


class PainterPlugin : public QObject, public kumirPluginInterface
{
	Q_OBJECT;
	Q_INTERFACES(kumirPluginInterface);
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_PLUGIN_METADATA(IID "kumir1.Painter")
	#endif
public:
	PainterPlugin(QObject *parent = 0);
	void start() { }
	QList<Alg> algList();
	QString name();
	QList<QVariant> algOptResults();
	QVariant result();
	void runAlg(QString alg, QList<QVariant> params);
	void showField();
	void showPult() { }
	void hidePult() { }
	void hideField();
	void showOpt() { }
	bool hasPult()
	{
		return false;
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
	void reset();
	QString errorText() const;
	quint8 check(const QString &script);
	void setParameter(const QString &paramName, const QVariant &paramValue);
	void connectSignalSendText(const QObject *obj, const char *method);
	void connectSignalSync(const QObject *obj, const char *method);
	QUrl pdfUrl() const;
	QUrl infoXmlUrl() const;
	QWidget *mainWidget();
	QWidget *pultWidget()
	{
		return 0;
	}
private slots:
	void handleResetRequest();
	void handleNewImageRequest(int w, int h, const QString &bg, bool useTemplate, const QString &fileName);
	void handleLoadImageRequest(const QString &fileName);
signals:
	void sendText(const QString &text);
	void sync();
private:
	struct PainterPluginPrivate *d;
};

#endif // PAINTERPLUGIN_H
