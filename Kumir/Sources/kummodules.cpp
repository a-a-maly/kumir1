#include "kummodules.h"
#include "kumsinglemodule.h"
#include "kum_instrument.h"
#include "network.h"
#include "application.h"
#include "protomodule_interface.h"

KumModules::KumModules(QWidget *parent) :
		QObject(parent)
{
	plugins_ready=false;
}

int KumModules::lastModuleId() const
{
	return Modules.size()-1;
}

int KumModules::count() const
{
	return Modules.size();
}

void KumModules::enableModule(int id)
{
	K_ASSERT(id>=0);
	if ( id>lastModuleId() )
	{
                qDebug ( "KumModules:enable: id>lastModuleId()" );
		return;
	}
	Modules[id]->setEnabled ( true );
}

void KumModules::disableModule(int id)
{
	K_ASSERT(id>=0);
	if ( id>lastModuleId() )
	{
                qDebug ( "KumModules:disbale: id>lastModuleId()" );
		return;
	}
	Modules[id]->setEnabled ( false );
}

KumSingleModule*  KumModules::module ( int id )
{
    KumSingleModule *result = NULL;
    if (id>=0 && id<Modules.size()) {
        result = Modules[id];
    }
    return result;
}

KumSingleModule* KumModules::lastModule()
{
	return Modules.last();
}

void KumModules::FuncByName(const QString &name, int *moduleId, int *id)
{
	for ( int i=0;i<Modules.count();i++ )
	{
		KumSingleModule *module = Modules[i];
		if ( module->isEnabled() )
		{
			*id=module->Functions()->idByName ( name );
			if ( *id>-1 )
			{
				*moduleId=i;
				return;
			};
		};
	};
	*moduleId=-1;
	*id=-1;
}

int KumModules::appendExternalFileModule( const QString &name, const QList<proga> &module_data )
{
	Modules.append( new KumSingleModule (0, name, kumIsp) );
	Modules.last()->setTab(NULL);
	Modules.last()->setProgaValue(module_data);
	Modules.last()->setModules(this);
	return Modules.count()-1;
}

int KumModules::appendUserModule( const QString &name, const QList<proga> &module_data )
{
	Modules.append( new KumSingleModule (0, name, kumIsp) );
	Modules.last()->setTab(NULL);
	Modules.last()->setProgaValue(module_data);
	Modules.last()->setModules(this);
	return Modules.count()-1;
}

int KumModules::appendModuleFromTab ( const QString &name,ProgramTab *tab,QList<ProgaText> pr_text )
{
	Modules.append ( new KumSingleModule ( 0,name,kumIsp ) );
	Modules.last()->setTab ( tab );
	Modules.last()->setProgaText ( pr_text );
	Modules.last()->setModules(this);
	return Modules.count()-1;
};
int KumModules::appendMainModule ( ProgramTab *tab )
{
	Modules.append ( new KumSingleModule ( 0,"main",kumIsp ) );
	Modules.last()->setTab ( tab );
	Modules.last()->setEnabled ( true );
	Modules.last()->setModules(this);
	return Modules.count()-1;
};


QDomDocument KumModules::getXmlInfoDocument(const QUrl &url)
{
	QUrl fullPath;
	QString localHelpBase;
#ifndef Q_OS_MAC
	localHelpBase = QDir::cleanPath(QApplication::applicationDirPath()+"/Kumir/Help/");
#else
        localHelpBase = QApplication::applicationDirPath()+"/../Resources/";

#endif
	if (url.scheme()=="file") {
		fullPath = QUrl::fromLocalFile(QDir::cleanPath(QDir(localHelpBase).absoluteFilePath(url.toLocalFile())));
	}
	else {
		fullPath = url;
	}
	QByteArray data;
	if (fullPath.scheme()=="file") {
		QFile f(fullPath.toLocalFile());
		if (f.open(QIODevice::ReadOnly|QIODevice::Text)) {
			data = f.readAll();
			f.close();
		}
		else
		{
                        qDebug() << "Can't open file: " << fullPath.toString();
		}
	}
	else {
                qDebug() << fullPath.scheme() << " is not supported now to fetch module.info.xml";
	}
	QDomDocument doc;
	if (!data.isEmpty()) {
		int errLine = 0;
		int errCol = 0;
		QString errMsg;
		bool readSuccess = doc.setContent(data, &errMsg, &errLine, &errCol);
		if (!readSuccess) {
                        qDebug() << "Error parsing " << fullPath.toString() << ":";
                        qDebug() << "  " << errLine << ":" << errCol << ": " << errMsg;
		}
	}
	return doc;
}

int KumModules::appendInternalModule ( const QString &name,KumInstrument * instrument, const QUrl &docUrl )
{
	QDomDocument doc = getXmlInfoDocument(docUrl);
	Modules.append ( new KumSingleModule ( 0,name,internal ) );
	if (!doc.isNull()) {
		Modules.last()->setDocumentationXML(doc);
	}
	Modules.last()->setModules(this);
	Modules.last()->setInstrument ( instrument );
	Modules.last()->setEnabled ( true );
	return Modules.count()-1;
};

int KumModules::appendNetworkModule ( const QString &name,KNPConnection * connection )
{
	if(idByName(name)>0)return -1;
	Modules.append ( new KumSingleModule ( 0,name,network ) );

	//Modules.last()->setInstrument ( instrument );
	Modules.last()->setEnabled ( false );
	Modules.last()->addr=connection->Addr;
	Modules.last()->port=connection->Port;
	Modules.last()->SetConnection(connection);
	return Modules.count()-1;
};

int KumModules::appendFriendModule ( const QString &name,kumirPluginInterface  * Interface )
{
	if(idByName(name)>0)return -1;
	Modules.append ( new KumSingleModule ( 0,name,friendIsp ) );

	//Modules.last()->setInstrument ( instrument );
	Modules.last()->setModules(this);
	Modules.last()->setEnabled ( false);
	Modules.last()->setPlugin(Interface);
	Modules.last()->setGraphics(true);

	QUrl infoXmlUrl = Interface->infoXmlUrl();
	if (infoXmlUrl.isValid() && !infoXmlUrl.isEmpty()) {
		QDomDocument doc = getXmlInfoDocument(infoXmlUrl);
		if (!doc.isNull()) {
			Modules.last()->setDocumentationXML(doc);
		}
	}

	return lastModuleId();
};

int KumModules::idByName ( QString name )
{
	for ( int i=0;i<Modules.count();i++ )
	{
		QString curName = Modules[i]->name;
		if ( curName==name ) return i;
	};
	return -1;
};
void KumModules::lockGUI()
{
	for ( int i=2;i<Modules.count();i++ )
	{
		if((Modules[i]->isEnabled())&&(Modules[i]->isInternal()))Modules[i]->instrument()->lockControls();
		if(Modules[i]->Name()=="Robot" ||Modules[i]->Name()=="Draw" )Modules[i]->instrument()->reset();
	};
};
bool KumModules::testNameConflict(int module_id)
{
    for(int i=0;i<Modules[module_id]->Functions()->size();i++)
	{
        QString name=Modules[module_id]->Functions()->nameById(i);
        int mod_id,perem_id;
        ExtFuncByName(name,module_id,&mod_id,&perem_id);
        if((mod_id>0)&&(mod_id!=module_id))return false;
	};
    return true;
};
int KumModules::idByTcpSokcet(QTcpSocket * connection)
{
    for(int i=0;i<Modules.count();i++)
        if((module(i)->isFriend())&&(Modules[i]->Connection()->Socket()==connection))return i;
    return -1;
};
void KumModules::unLockGUI()
{
	for ( int i=2;i<Modules.count();i++ )
	{
		if((Modules[i]->isEnabled())&&(Modules[i]->isInternal()))Modules[i]->instrument()->unLockControls();
	};
};

void KumModules::showGraphicsInstr()
{
	for ( int i=0;i<Modules.count();i++ )
	{
		KumSingleModule *module = Modules[i];
		const QString moduleName = module->name; // for debug
		Q_UNUSED(moduleName);
		if (module->isEnabled())
		{
			module->showWindow();
		}
	}
}

void KumModules::setIntegerValue(PeremPrt ptr,int value)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.setIntegerValue(value);
};
void KumModules::setFloatValue(PeremPrt ptr,double value)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.setFloatValue(value);
};
void KumModules::setBoolValue(PeremPrt ptr,bool value)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.setBoolValue(value);
};
void KumModules::setCharValue(PeremPrt ptr,QChar value)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.setCharectValue(value);
};
void KumModules::setStringValue(PeremPrt ptr,QString value)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.setStringValue(value);
};

QString KumModules::getStringValue(PeremPrt ptr)
{
    K_ASSERT(Modules[ptr.mod_id]->isEnabled());
    return Modules[ptr.mod_id]->Values()->symb_table[ptr.perem_id].value.getValueAsString();
};


void KumModules::runPluginFunc(int module_id,QString alg,QList<QVariant> args)
{
    if(!module(module_id)->isFriend())
	{
        emit sync();
        qDebug()<<"KumModules::runPluginFunc module "<<module_id<<"is not a plugin!!!";
        return;
	};

    module(module_id)->Plugin()->runAlg(alg,args);
    QVariant znach=module(module_id)->Plugin()->result();
    int func_id=module(module_id)->Functions()->idByName(alg);
    if(module(module_id)->Functions()->typeById(func_id)!=none)
	{
        int perem_id=module(module_id)->Values()->inTable(alg,"GLOBAL");
        Q_UNUSED(perem_id);
		qDebug()<<"Todo!!"<<" Set Value!";

	};
    emit sync();
};
void KumModules::resetPlugin(int module_id)
{
    module(module_id)->resetModule();
};
int KumModules::scanPlugins(QDir plugin_dir,QString mask)
{
    int count=0;
    QStringList pluginFiles=plugin_dir.entryList(QStringList() << mask);
    // QMessageBox::warning(NULL,tr("List count: "),QString::number(pluginFiles.count()));
    foreach (QString fileName, pluginFiles) {
        qDebug()<<fileName;
        //QMessageBox::warning(NULL,tr("Before Load "),fileName);
        QPluginLoader loader(plugin_dir.absoluteFilePath(fileName),this);
        QObject *plugin = loader.instance();
        if (plugin) {
            kumirPluginInterface *modulePlugin =
                    qobject_cast<kumirPluginInterface*>(plugin);
            ProtoModuleInterface *protoPlugin =
                    qobject_cast<ProtoModuleInterface*>(plugin);
            if (modulePlugin) {
                QStringList algHeaders;
                modulePlugin->start();
                appendFriendModule(modulePlugin->name(), modulePlugin);
                for(int i=0;i<modulePlugin->algList().count();i++)
                {
                    algHeaders << modulePlugin->algList()[i].kumirHeader;
                }
                app()->compileAlgList(lastModuleId(),algHeaders);
                module(lastModuleId())->updateDocumentation();
                count++;
            }
            else if (protoPlugin) {
                protoPlugin->setRootDirectory(QApplication::applicationDirPath()+"/Addons/");
                QStringList modulesList = protoPlugin->virtualModulesList();//
                foreach (const QString &moduleName, modulesList)
                {
                    QObject *curObj = protoPlugin->createModule(moduleName);
                    kumirPluginInterface *curModule =
                            qobject_cast<kumirPluginInterface*>(curObj);
                    if (curModule) {
                        curModule->start();
                        appendFriendModule(moduleName, curModule);
                        QStringList algHeaders;
                        for (int i=0; i<curModule->algList().size(); i++) {
                            algHeaders << curModule->algList()[i].kumirHeader;
                        }
                        app()->compileAlgList(lastModuleId(),algHeaders);
                        count++;
                    }
                }

            }
        }
        else {
            qDebug() << "Error loading plugin: " << fileName;
            qDebug() << " --- " << loader.errorString();
        }
    }
	//QMessageBox::warning(NULL,tr("Plugin count: "),QString::number(count));
    plugins_ready=true;
    return count;
};



int KumModules::idByPort(uint port)
{
	for ( int i=0;i<Modules.count();i++ )
	{
		if(!Modules[i]->isNetwork())continue;
		if(Modules[i]->port==port)return i;
	}
	return -1;
}

bool KumModules::isGraphics(uint id)
{
	return Modules[id]->Graphics();
}

KumInstrument* KumModules::Instrument(uint id)
{
	return Modules[id]->instrument();
}

void KumModules::ExtFuncByName(QString name, int my_module_id, int *ext_moduleId, int *id)
{
	for ( int i=0;i<Modules.count();i++ )
	{
		if(i==my_module_id)continue;
		if (( Modules[i]->isEnabled() ))
		{
			*id=Modules[i]->Functions()->idByName ( name );
			if ( *id>-1 )
			{
				*ext_moduleId=i;
				return;
			};
		};
	};
	*ext_moduleId=-1;
	*id=-1;
}

int KumModules::FuncPerm(int func_id, int moduleId)
{
	if ( !Modules[moduleId]->isEnabled() )
	{
                qDebug ( "Module %i disabled!",moduleId );
		return -1;
	};
	QString name=Modules[moduleId]->Functions()->nameById(func_id);
	return Modules[moduleId]->Values()->inTable ( name,"global" );
}

QString KumModules::FuncName(int func_id, int moduleId)
{
	if ( !Modules[moduleId]->isEnabled() )
	{
                qDebug ( "Module %i disabled!",moduleId );
		return "";
	};
	return Modules[moduleId]->Functions()->nameById(func_id);
}

PeremType KumModules::FuncType(int func_id, int moduleId)
{
	if ( !Modules[moduleId]->isEnabled() )
	{
                qDebug ( "Module %i disabled!",moduleId );
		return none;
	};
	return Modules[moduleId]->Functions()->typeById(func_id);
}

KNPConnection* KumModules::connection(int module_id)
{
	K_ASSERT(module_id>=0);
	if(Modules.count()<module_id+1)
	{
                qDebug("connection: no moudule!");
		return NULL;
	};
	return Modules[module_id]->Connection();
}

int KumModules::idByKNPConnection(KNPConnection *connection)
{

		for(int i=0;i<Modules.count();i++)
			if(Modules[i]->Connection()==connection)return i;
		return -1;
}

void KumModules::removeModule(int module_id)
{
	if ( module_id>=Modules.count() )
	{
                qDebug ( "KumModules::remove No such module!" );
		return;
	};
	Modules.removeAt ( module_id );
}

void KumModules::Debug()
{
	for(int i=0;i<Modules.count();i++)
	{
		if(Modules[i]->isEnabled())
                        qDebug()<<"Module "<< i <<" "<< Modules[i]->name<<" enabled";
		else
                        qDebug()<<"Module "<< i <<" "<< Modules[i]->name<<" disabled";
	}
}

void KumModules::setUsed(PeremPrt ptr, bool flag)
{

	K_ASSERT(Modules[ptr.mod_id]->isEnabled());
	Modules[ptr.mod_id]->Values()->setUsed(ptr.perem_id,flag);
}

bool KumModules::isUsed(PeremPrt ptr)
{
	K_ASSERT(Modules[ptr.mod_id]->isEnabled());
	return Modules[ptr.mod_id]->Values()->isUsed(ptr.perem_id);
}

void KumModules::tempDisable()
{
	tempDisabled.clear();
	for(int i=2;i<=lastModuleId();i++)
	{
		if(Modules[i]->isEnabled())tempDisabled.append(i);
		disableModule(i);
	}
}

void KumModules::tempEnable()
{
	for(int i=0;i<tempDisabled.count();i++)enableModule(tempDisabled[i]);
}

void KumModules::setFriendSocket(QString name, KNPConnection *soc)
{
	int mod_id=idByName(name);
	if(mod_id>0)
	{
		module(mod_id)->setConnection(soc);

	}
}


