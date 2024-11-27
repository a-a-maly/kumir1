#ifndef KUM_SINGLE_MODULE_H
#define KUM_SINGLE_MODULE_H

#include "kum_tables.h"
#include "Addons/plugin_interface.h"
#include <QObject>
#include <QDomDocument>
class QProcess;


class KumInstrument;
class ProgramTab;
class KumModules;

class KumSingleModule: public QObject
{
	Q_OBJECT
public:
	explicit KumSingleModule(
		QWidget *parent = 0,
		const QString &Name = "No Name",
		IspType Type = KumSystem
	);
	void setModules(KumModules *modules);
	void setBroken(bool v);
	bool isBroken() const;
	bool isProtected() const;
	void setProtected(bool v);
	inline QString moduleInfoName() const
	{
		return s_moduleInfoName;
	}
	inline QString moduleDescrption() const
	{
		return s_moduleDescription;
	}

	inline void setModuleName(const QString &v)
	{
		name = v;
	}
	KumInstrument *Instrument; //Сылка на обьект
	void setTab(ProgramTab *Tab);

	void setInstrument(KumInstrument *instrument);
	void setDocumentationXML(const QDomDocument &doc);
	void updateDocumentation();
	KumInstrument *instrument();
	inline bool isEnabled()
	{
		return enabled;
	}
public:
	inline void setEnabled(bool flag)
	{
		enabled = flag;
	}

	inline bool Graphics()
	{
		if (!ready) {
			qDebug("Module:Graphics:not ready!");
		}
		return graphics;
	}
	/**
	    * Установить тип исполнителя
	    * @param mType Тип исполнителя
	    */
	void setType(IspType mType);

	symbol_table *Values();

	inline function_table *Functions()
	{
		return &functions;
	}
	QList<ProgaText> *Proga_Text();

	QList<proga> *Proga_Value();


	inline void setProgaValue(const QList<proga> &v)
	{
		proga_value = v;
	}

	inline void setProgaValueAt(int i, const proga &v)
	{
		K_ASSERT(i < proga_value.count());
		if (i < proga_value.count()) {
			proga_value[i] = v;
		}
	}

	inline int bytecodeSize() const
	{
		return proga_value.count();
	}

	inline void appendBytecode(const QList<proga> &v)
	{
		proga_value += v;
	}

	inline void setInitialIP(int ip)
	{
		m_initialIP = ip;
	}

	inline int initialIP() const
	{
		return m_initialIP;
	}

	inline void removeDummyLines()
	{
		for (int i = 0; i < proga_value.count(); i++) {
			if (proga_value[i].real_line.lineNo == -2) {
				proga_value.removeAt(i);
				i--;
			}
		}
	}

	ProgramTab *Tab()
	{
		if (!ready) {
			qDebug("Proga_Text:Module Not Ready.");
		}
		if (type != kumIsp) {
			qDebug("Proga_Text:type!=kumIsp.");
		}
		return tab;
	};
	int progaCount()
	{
		return proga_text.count();
	}
	ProgaText *Proga_Text_Line(int i)
	{
		if (!ready) {
			qDebug("Proga_Text_Line:Module Not Ready.");
		}
		if (type != kumIsp) {
			qDebug("Proga_Text_Line:type!=kumIsp.");
		}
		if ((i < 0) || (i > proga_text.count())) {
			qDebug("Proga_Text_Line:ERROR:no such line!");

		};
		return &proga_text[i];
	};

	proga  Proga_Value_Line(int i);

	/**Устанавливает начало и конец исполнителя
	    * в исходном тексте - вкладки или файла
	    * @param start первая строка исполнителя
	    * @param end Последняя строка исполнителея
	    */
	void setRange(int start, int end)
	{
		if (type != kumIsp) {
			qDebug("KumSingleModule:: setRange type!=kumIsp");
		}


		if (start < end) {
			qDebug("KumSingleModule::start < end");
		}

		from = start;
		to = end;
	};

	void Clean()
	{
		foreach (symbol_table *st, values) {
			delete st;
		}
		values.clear();//TODO Check me
		values.append(new symbol_table());
		functions.free(0);
		proga_text.clear();
		proga_value.clear();
	};
	QString file()
	{
		return File;
	};
public:
	bool isInternal()
	{
		if (type == internal) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isKumIsp()
	{
		if (type == kumIsp) {
			return true;
		} else {
			return false;
		}
	};



public:
	bool isNetwork()
	{
		if (type == network) {
			return true;
		} else {
			return false;
		}
	};

public:
	bool isSystem()
	{
		if (type == KumSystem) {
			return true;
		} else {
			return false;
		}
	};
public:
	bool isFriend()
	{
		if (type == friendIsp) {
			return true;
		} else {
			return false;
		}
	};

	QString Name()
	{
		return name;
	};
	void setGraphics(bool flag)
	{
		graphics = flag;
	};
	void saveValues();

//	void localRestoreValues ( const QString & name );
	void restoreValues();

	void setProgaText(QList<ProgaText> Proga_text)
	{
		proga_text = Proga_text;
	};
	void setFile(QString fileName)
	{
		File = fileName;
	};
	int IntErr;
	void setError(int err)
	{
		IntErr = err;
	};
	QString name;
	void setConnection(KNPConnection *isp)
	{
		connection = isp;
		ready = true;
	};
	void quitModule()
	{

		if (isFriend()) {
			plugin->hidePult();
			plugin->hideField();

		};
	};
	bool isReady()
	{
		return ready;
	};
	KNPConnection *Connection()
	{
		return connection;
	};
	void SetConnection(KNPConnection *KNP_isp)
	{
		connection = KNP_isp;
	};
	QString addr;
	quint16 port;
	bool b_protected;
	QString iconFile;
	void setProcess(QProcess *Process)
	{
		if (!isFriend()) {
			return;
		}
		friendProcess = Process;
	};
	QProcess *Process()
	{
		if (!isFriend()) {
			return NULL;
		}
		return friendProcess;
	};
	bool menu()
	{
		return hasMenu;
	};
	void setMenu()
	{
		hasMenu = true;
	};
	void startProcess(int kumPort)
	{
		Q_UNUSED(kumPort);
		qDebug() << "Dummy func called";
		return;
		;
	};
	void setBinFile(QString name)
	{
		bin_file = name;
	};
	void setPlugin(kumirPluginInterface *Plugin);
	kumirPluginInterface *Plugin()
	{
		return plugin;
	};
	QList<QVariant> convertKVSE2QVariant(QList<KumValueStackElem> *KVSE);
	void connectSig();
	void disconectSig();
	void resetModule();

public slots:
	void tabClosed();

	void showWindow();
	void showPult();
	void showOpt();
	void hideWindow();
	void hidePult();
	void runFunc(QString name, QList<KumValueStackElem> *arguments, int *err);
	void syncSlot();
	void SwitchIspMode(int mode);
signals:
	void sync();
	void IspError(QString text);
private:
	QMap<QString, QString> m_docStrings;
	QString s_moduleInfoName;
	QString s_moduleDescription;
	bool b_broken;
	QString File;
	IspType type;
	bool enabled;
	bool compiled;
	QList< QPair<int, int> > l_resLinks;
	int m_initialIP;
	bool graphics;
	bool ready;
	QList<symbol_table *> values;
	function_table functions;
	ProgramTab *tab; //вкладка
	QList<proga> proga_value;
	QList<ProgaText> proga_text;
	int from, to;
	KNPConnection *connection;
	QString bin_file;

	KumModules *m_modules;
	QProcess *friendProcess;
	kumirPluginInterface *plugin;
	bool hasMenu;
	QString m_pluginCallName;

	QWidget *w_window;
	QWidget *w_pultWindow;
};


#endif
