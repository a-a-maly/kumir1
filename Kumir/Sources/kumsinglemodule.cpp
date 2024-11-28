#include "kumsinglemodule.h"
#include "kum_tables.h"
#include "kum_instrument.h"
#include "kummodules.h"
#include "secondarywindow.h"
#include "application.h"
#include "int_proga.h"
#include <QIcon>
#include <QSettings>

KumSingleModule::KumSingleModule ( QWidget *parent,
								   const QString &Name,
								   IspType Type )
{
	b_protected = false;
	Q_UNUSED(parent);
	b_broken = false;
	m_modules = 0;
	enabled=false;
	compiled=false;
	graphics=false;
	Instrument=NULL;
	tab=NULL;
	values.append ( new symbol_table() );
	functions.free ( 0 );
	proga_value.clear();
	name=Name;
	File="";
        ready=false;
	type=Type;
	hasMenu=false;
    IntErr=0;
    m_initialIP = 0;
    w_window = NULL;
    w_pultWindow = NULL;
}

//void KumSingleModule::localRestoreValues(const QString &name)
//{
//	K_ASSERT ( values.count() >= 2 );
//	symbol_table * to_table = values[values.size()-2];
//	symbol_table * from_table = values[values.size()-1];
//	to_table->localRestore ( name,from_table );
//	delete values.last();
//	values.removeLast();
//	K_ASSERT ( values.count() >0 );
//}

void KumSingleModule::saveValues()
{
    symbol_table * table_copy = values.last()->deepCopy();
    values.push_back(table_copy);
//    qDebug() << "SAVE VALUES: stack size = " << values.size();
}

void KumSingleModule::restoreValues()
{
    K_ASSERT ( values.count() >= 2 );
    symbol_table * to_table = values[values.size()-2];
    symbol_table * from_table = values[values.size()-1];
    K_ASSERT ( from_table->symb_table.size()==to_table->symb_table.size());
    for (int i=0; i<from_table->symb_table.size(); i++) {
        symbol_table::s_table ss = from_table->symb_table[i];
        bool isGlobal = ss.alg=="global";
        bool isRes = ss.is_Res;
        bool isArgRes = ss.is_ArgRes;
        if (isGlobal) {
            to_table->valueCopy(i, ss);
            to_table->symb_table[i].used = ss.used;
        }
        if (isRes || isArgRes) {
            int modId = ss.pointer.mod_id;
            int varId = ss.pointer.perem_id;
            Q_CHECK_PTR(m_modules);
            if (m_modules->module(modId)==this) {
                to_table->valueCopy(varId, from_table->symb_table[varId]);
                to_table->symb_table[varId].used = from_table->symb_table[varId].used;
            }
        }
    }
    delete from_table;
    values.pop_back();
//    qDebug() << "RESTORE VALUES: stack size = " << values.size();
}

void KumSingleModule::setBroken(bool v)
{
    b_broken = v;
}

bool KumSingleModule::isBroken() const
{
    return b_broken;
}

void KumSingleModule::setProtected(bool v)
{
    b_protected = v;
}

bool KumSingleModule::isProtected() const
{
    return b_protected;
}

void KumSingleModule::resetModule()
{
	if(isFriend()) {
		plugin->reset();
		QString ioDir = app()->settings->value("Directories/IO","").toString();
		plugin->setParameter("IODir", ioDir);
		plugin->showField();
	}
}

proga KumSingleModule::Proga_Value_Line(int i)
{
        if ( !ready ) qDebug ( "Proga_Value_Line:Module Not Ready." );
        if ( type!=kumIsp ) qDebug ( "Proga_Value_Line:type!=kumIsp." );
	if ( ( i>proga_value.count() ) )
	{
		int debug=proga_value.count();
                qDebug() << "Proga_Value_Line:ERROR:no such line: " << debug;

	}
	if ( i>=0 )
		return proga_value[i];
	else {
		i = -i;
		qDebug() << "i = " << i;
		qDebug() << "Size = " << proga_value.size();
		int baseStr = i / 10;
		int subStr = (i%10)-1;
		return proga_value[baseStr].generatedLinesAfter[subStr];
	}
}



void KumSingleModule::tabClosed() //Slot
{
	if ( type!=kumIsp )
	{
                qDebug ( "KumSingleModule:: type!=kumIsp, but tabClosed() signal." );
		return;
	};
	ready=false;
	tab=NULL;
};

void KumSingleModule::showWindow()
{
	if(!isFriend()){
        qDebug()<<"KumSingleModule::showWindow !isFriend()";
        return;};
    //	plugin->showField();
	if (w_window) {
		w_window->show();
	}

};
void KumSingleModule::showPult()
{
	if(!isFriend()){
        qDebug()<<"KumSingleModule::showPult !isFriend()";
        return;};
    //	plugin->showPult();
    if (w_pultWindow) {
        w_pultWindow->show();
    }

};
void KumSingleModule::showOpt()
{
	if(!isFriend()){
        qDebug()<<"KumSingleModule::showPult !isFriend()";
        return;};
	plugin->showOpt();

};
void KumSingleModule::hideWindow()
{
	if(!isFriend()){
        qDebug()<<"KumSingleModule::hideWindow()!isFriend()";
        return;};
    //	plugin->hideField();
	if (w_window) {
		w_window->hide();
	}

};
void KumSingleModule::hidePult()
{
	if(!isFriend()){
        qDebug()<<"KumSingleModule::hidePult !isFriend()";
        return;};
    //	plugin->hidePult();
    if (w_pultWindow) {
        w_pultWindow->hide();
    }

};

void KumSingleModule::runFunc(QString name,QList<KumValueStackElem>* arguments,int* err)
{
    if(!isFriend())
	{
        KumInstrument* instrum=instrument();
        instrum->runFunc(name,arguments,err);
	}else
	{
        m_pluginCallName = name;
        int f_id = Functions()->idByName(name);
        int argsCount = Functions()->argCountById(f_id);
        l_resLinks.clear();
        QList<QVariant> args;
        QList<QVariant> variants = convertKVSE2QVariant(arguments);
        for (int i=0; i<argsCount; i++) {
            if (Functions()->isRes(f_id,i) || Functions()->isArgRes(f_id, i)) {
                int mod_id = variants.first().toInt();
                variants.pop_front();
                int var_id = variants.first().toInt();
                variants.pop_front();
                l_resLinks << QPair<int,int>(mod_id, var_id);
            }
            else {
                args << variants.first();
                variants.pop_front();
            }
        }

        plugin->runAlg(name,args);

        //emit sync();
	};
};
QList<QVariant> KumSingleModule::convertKVSE2QVariant(QList<KumValueStackElem>* KVSE)
{
    QList<QVariant> temp;
    for(int i=0;i<KVSE->count();i++)
	{
        temp<<KVSE->value(i).asVariant();
	};
    return temp;
};
void KumSingleModule::connectSig()
{
    if(!isFriend())
	{
        connect(instrument(),SIGNAL(sync()),this,
                SLOT(syncSlot()));
	}else
	{
        plugin->connectSignalSync(this,SLOT(syncSlot()));
	};
};
void KumSingleModule::disconectSig()
{if(!isFriend())
	{
        disconnect(instrument(),SIGNAL(sync()),this,
                   SLOT(syncSlot()));
	}else
	{

	};
};

void KumSingleModule::setModules(KumModules *modules)
{
	m_modules = modules;
	Q_CHECK_PTR(m_modules);
}

void KumSingleModule::syncSlot()
{
    if(isFriend())
    {

        if(plugin->errorText()!="")
        {
            qDebug()<<plugin->errorText();
            emit IspError(plugin->errorText());
            emit sync();
        }

        else emit {
            QVariant returnValue = plugin->result();
            QList<QVariant> optResults = plugin->algOptResults();
            symbol_table* table = Values();

            K_ASSERT ( !m_pluginCallName.isEmpty() );
            K_ASSERT(l_resLinks.size()==optResults.size());
            int id = table->inTable(m_pluginCallName,"global");
            QList<QPair<int,int> > links = l_resLinks;
            for (int i=0; i<l_resLinks.size(); i++) {
                QVariant val = optResults[i];
                int m_id = l_resLinks[i].second;
                int v_id = l_resLinks[i].first;
                Q_CHECK_PTR(m_modules);
                symbol_table * tab = m_modules->module(m_id)->Values();
                Q_CHECK_PTR(tab);
                PeremType pType = tab->getTypeByID(v_id);
                tab->setUsed(v_id, true);
                if (pType==integer) {
                    tab->symb_table[v_id].value.setIntegerValue(val.toInt());
                }
                else if (pType==real) {
                    tab->symb_table[v_id].value.setFloatValue(val.toReal());
                }
                else if (pType==kumString) {
                    tab->symb_table[v_id].value.setStringValue(val.toString());
                }
                else if (pType==charect) {
                    tab->symb_table[v_id].value.setCharectValue(val.toChar());
                }
                else if (pType==kumBoolean) {
                    tab->symb_table[v_id].value.setBoolValue(val.toBool());
                }

            }
            //            K_ASSERT ( id > -1 );
            if ( id > -1 ) {
                table->setUsed(id, true);
                PeremType ptype = table->symb_table[id].type;
                K_ASSERT ( (ptype==kumBoolean)||(ptype==integer)||(ptype==real)||(ptype==kumString)||(ptype==charect) );
                if (ptype==kumBoolean) {
                    K_ASSERT ( returnValue.canConvert(QVariant::Bool) );
                    bool bvalue = returnValue.toBool();
                    table->symb_table[id].value.setBoolValue(bvalue);
                }
                else if (ptype==integer) {
                    K_ASSERT ( returnValue.canConvert(QVariant::Int) );
                    int ivalue = returnValue.toInt();
                    table->symb_table[id].value.setIntegerValue(ivalue);
                }
                else if (ptype==real) {
                    K_ASSERT ( returnValue.canConvert(QVariant::Double) );
                    double dvalue = returnValue.toDouble();
                    table->symb_table[id].value.setFloatValue(dvalue);
                }
                else if (ptype==kumString) {
                    K_ASSERT ( returnValue.canConvert(QVariant::String) );
                    QString svalue = returnValue.toString();
                    table->symb_table[id].value.setStringValue(svalue);
                }
                else if (ptype==charect) {
                    K_ASSERT ( returnValue.canConvert(QVariant::Char) );
                    QChar cvalue = returnValue.toChar();
                    table->symb_table[id].value.setCharectValue(cvalue);
                }
            }
            sync();
        }
        return;
    };
    emit sync();
}


void KumSingleModule::SwitchIspMode(int mode)
{
    if(!isFriend())
	{
        instrument()->SwitchMode(mode);
	}else
	{
        plugin->setMode(mode);
        emit sync();
	};

}
void KumSingleModule::setTab(ProgramTab *Tab)
{
        if ( ready ) qDebug ( "setTab: tab set 2 times." );
	tab=Tab;
	if ( type==kumIsp ) ready = true;
}

void KumSingleModule::setInstrument(KumInstrument *instrument)
{
        if ( ready ) qDebug ( "setInstrument():set 2 times." );
	Instrument=instrument;
	if ( type==internal ) ready = true;
}

KumInstrument* KumSingleModule::instrument()
{
        if ( !ready ) qDebug ( "Module:instrument:not ready!" );
	if ( type!=internal ) return  NULL;
	return Instrument;
}

QString xmlToText(const QDomNode &node)
{
	QString buffer;
	QString result;
	const QDomNodeList childs = node.childNodes();
	for (int i=0; i<childs.count(); i++) {
		const QDomNode child = childs.at(i);
		if (child.isText()) {
			buffer = child.toText().data();
			result += buffer;
		}
		else if (child.isElement()) {
			result += "<"+child.toElement().tagName();
			QDomNamedNodeMap attributes = child.toElement().attributes();
			for (int j=0; j<attributes.count(); j++) {
				QDomAttr attr = attributes.item(j).toAttr();
				result += " "+attr.name()+"=\""+attr.value()+"\"";
			}
			result += ">";
			result += xmlToText(child);
			result += "</"+child.toElement().tagName()+">";
		}
	}
	return result;
}

void KumSingleModule::setDocumentationXML(const QDomDocument &doc)
{
	const QDomNode root = doc.documentElement();
	const QDomNodeList childs = root.childNodes();
	QString buffer;
	QString desc;
	QString name;
	QString algName;
	QString algDesc;
	if (root.toElement().hasAttribute("name")) {
		name = root.toElement().attribute("name");
	}
	for (int i=0; i<childs.count(); i++) {
		const QDomNode child = childs.at(i);
		if (child.isText()) {
			buffer = child.toText().data().simplified();
			desc += " "+buffer;
		}
		else if (child.isElement() && child.toElement().tagName()=="alg") {
			if (child.toElement().hasAttribute("name")) {
				algName = child.toElement().attribute("name");
				algDesc = xmlToText(child);
				m_docStrings[algName] = algDesc;
			}
		}
	}
	if (!name.isEmpty()) {
		s_moduleInfoName = name;
	}
	desc = desc.simplified();
	if (!desc.isEmpty()) {
		s_moduleDescription = desc;
	}
}

void KumSingleModule::updateDocumentation()
{
	for (int i=0; i<functions.size(); i++) {
		const QString fname = functions.nameById(i);
		if (m_docStrings.contains(fname)) {
			const QString doc = m_docStrings[fname];
			functions.setDocString(i, doc);
		}
	}
}

void KumSingleModule::setType(IspType mType)
{
	if ( mType==kumIsp ) //Текстовый
	{
		Instrument =NULL;
	}
	type=mType;
	tab=NULL;
}

symbol_table* KumSingleModule::Values()
{
	if(values.count()==0){
                qDebug ( "NO Values in module!!!." );
		return NULL;
	}
	return values.last();
}

QList<ProgaText> * KumSingleModule::Proga_Text()
{
        if ( !ready ) qDebug ( "Proga_Text:Module Not Ready." );
	if ( type!=kumIsp )
                qDebug ( "Proga_Text:type!=kumIsp." );
	return &proga_text;
}

QList<proga> * KumSingleModule::Proga_Value()
{
        if ( !ready ) qDebug ( "Proga_Text:Module Not Ready." );
        if ( type!=kumIsp ) qDebug ( "Proga_Text:type!=kumIsp." );
	return &proga_value;
}



void KumSingleModule::setPlugin(kumirPluginInterface *Plugin)
{
	Q_CHECK_PTR(Plugin);
	QWidget *pluginWidget = Plugin->mainWidget();
	Q_CHECK_PTR(pluginWidget);
	bool resizableX = false;
	bool resizableY = false;
	resizableX = pluginWidget->property("resizableX").toBool();
	resizableY = pluginWidget->property("resizableY").toBool();
    w_window = new SecondaryWindow(Plugin->mainWidget(), name);
	w_window->setWindowIcon(QIcon(QString(":/icons/module_window.png")));
	plugin=Plugin;
	app()->secondaryWindows << w_window;
	if (plugin->hasPult()) {
		w_pultWindow = new SecondaryWindow(Plugin->pultWidget(), name+tr(" - Pult"));
		w_pultWindow->setWindowIcon(QIcon(":/icons/pult_window.png"));
		app()->secondaryWindows << w_pultWindow;
	}
	ready=true;
}
