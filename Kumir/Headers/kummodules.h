#ifndef KUMMODULES_H
#define KUMMODULES_H

#include "kum_tables.h"
#include <QDomDocument>
#include <QDir>
class QTcpSocket;

class KumModules : public QObject

{
	Q_OBJECT
public:
	QString xmlData();
	explicit KumModules ( QWidget *parent = 0 );
	int lastModuleId() const;
	int count() const;
	void enableModule ( int id );
	void disableModule ( int id );

	KumSingleModule* module ( int id );

	KumSingleModule* lastModule ();
	int appendModuleFromTab ( const QString &name,ProgramTab *tab,QList<ProgaText> proga_text );
	int appendMainModule ( ProgramTab *tab );
	int appendExternalFileModule( const QString &name, const QList<proga> &module_data );
	int appendUserModule(const QString &name, const QList<proga> &moduleData);
        int appendInternalModule ( const QString &name, class KumInstrument* instrument, const QUrl &docFile );
        int appendFriendModule ( const QString &, class kumirPluginInterface  *);
	int appendNetworkModule ( const QString &name, KNPConnection * connection );
	int idByName ( QString name );
	int idByPort ( uint port );


	bool  isGraphics ( uint id );

	KumInstrument* Instrument ( uint id );

	void FuncByName ( const QString &name, int * moduleId,int *id );


	void ExtFuncByName ( QString name,int my_module_id,int * ext_moduleId,int *id );

	int FuncPerm ( int func_id,int moduleId );

	QString FuncName ( int func_id,int moduleId );
	PeremType FuncType ( int func_id,int moduleId );
	KNPConnection * connection(int module_id);

	int idByKNPConnection(KNPConnection * connection);

	int idByTcpSokcet(QTcpSocket * connection);
	void lockGUI();

	void unLockGUI();

	void showGraphicsInstr();
	void removeModule ( int module_id );

	void Debug ();
	void setUsed(PeremPrt ptr,bool flag);
	bool isUsed(PeremPrt ptr);
	void tempDisable();
	void tempEnable();
	void setFriendSocket(QString name,KNPConnection *soc);

	void setIntegerValue(PeremPrt ptr,int value);
	void setFloatValue(PeremPrt ptr,double value);
	void setBoolValue(PeremPrt ptr,bool value);
	void setCharValue(PeremPrt ptr,QChar value);
	void setStringValue(PeremPrt ptr,QString value);
	bool testNameConflict(int module_id);
	QString getStringValue(PeremPrt ptr);
	int scanPlugins(QDir plugin_dir,QString mask);//Поиск и подключение плагинов
	inline bool pluginsReady(){return plugins_ready;}

public slots:
	void runPluginFunc(int module_id,QString alg,QList<QVariant> args);
	void resetPlugin(int module_id);
signals:
	void sync();
private:
	static QDomDocument getXmlInfoDocument(const QUrl &url);
	QList<KumSingleModule*> Modules;
	symbol_table funcGlobPerem;
	QList<int> tempDisabled;
	bool plugins_ready;

};

#endif // KUMMODULES_H
