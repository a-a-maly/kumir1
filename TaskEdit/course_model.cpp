#include "course_model.h"
#include <QSize>
#include <QColor>
#include <QBrush>
#include <QIcon>
int courseModel::loadCourse(QString file)
{

     courseFileName=file;
    qDebug()<<"Load Course";
    QFile f(courseFileName);
    if  (!f.open(QIODevice::ReadWrite))
    {

    return -1;
    };
if(f.atEnd())return -1;
QString error;
courceXml.setContent(f.readAll(),true,&error);
qDebug()<<"File parce:"<<error;
int count=0;
taskCount=count;
root=courceXml.documentElement();
insertRow(0);
insertColumn(0);
setData(createIndex(0,0),QVariant());
buildCash();
f.close();
return count;
}


int courseModel::rowCount(const QModelIndex &parent) const
{
   // qDebug()<<"RowCount"<< parent;
   if(!parent.isValid())
    {
      // qDebug()<<"NOT VALID"<<" count"<<root.childNodes().length();
       return 1;
   };

   QDomNode node=nodeById(parent.internalId(),root);

   return subTasks(node);

}
QIcon courseModel::iconByMark(int mark,bool isFolder)const
{
    if(isFolder)return QIcon(":/folder.png");
 if((mark>-1)&&(mark<12))return markIcons[mark];

    return QIcon(":/x.png");
}

QVariant courseModel::data(const QModelIndex &index, int role) const
{
   //qDebug()<<"Get data"<<index<<" role"<<role;
    if (!index.isValid())
    return QVariant();
    QDomNode node=nodeById(index.internalId(),root);

    if(role==Qt::DisplayRole)    {





        QString title=node.toElement().attribute("name","");

        return QVariant(title);
            };
     if(role==Qt::SizeHintRole)
    {
         return QVariant(QSize(10,20));
     }
     if(role==Qt::FontRole)
     {
        return QVariant();
     }
     if(role==Qt::TextAlignmentRole)
     {
         return QVariant(Qt::AlignLeft);
     }
     if(role==Qt::ForegroundRole)
     {
         if(isTeacher)QVariant(QBrush(QColor(0,0,0)));
        if(!taskAvailable(node))
            return QVariant(QBrush(QColor(150,150,150)));
       return QVariant(QBrush(QColor(0,0,0)));
     }
     if(role==Qt::CheckStateRole)
     {
        return QVariant();
     }
     if(role==Qt::DecorationRole)
     {
         QDomNode nodeM=nodeById(index.internalId(),root);
       // qDebug()<<"Draw Mark id"<<index.internalId();
         if(nodeM.toElement().attribute("root")=="true")
         {
             qDebug()<<"Folder";
         }
     return iconByMark(taskMark(index.internalId()),nodeM.toElement().attribute("root")=="true");
     //NUZHNO IKONKI ISPOLNITELEY
     }
     if(role==Qt::BackgroundRole)
     {
        return QBrush(QColor(255,255,255));
     }
     qDebug()<<"No" << role<< "role";
     return QVariant();
};
    QVariant courseModel::headerData(int section, Qt::Orientation orientation,
                        int role ) const
    {//qDebug()<<"Get Header";
        return QVariant(courseFileName);};

    QModelIndex courseModel::index(int row, int column, const QModelIndex &parent) const{
      //  qDebug()<<"index"<<" ROW:"<<row<<" column:"<<column;
       if (!hasIndex(row, column, parent))
               return QModelIndex();
     //  qDebug()<<root.attribute("id","");
      return createMyIndex(row,column,parent);


   };
    int courseModel::domRow(QDomNode &child)const //TODO Check
    {
        QDomNode par=child.parentNode();
        QDomNodeList list=par.childNodes();
        for(int i=0;i<list.count();i++)
        {
            if(child==list.at(i))return i;
        }
        return 0;
     };
    QModelIndex courseModel::parent(const QModelIndex &child) const
    {

         if (!child.isValid())
                return QModelIndex();
         if(child.internalId()==0)return QModelIndex();
       QDomNode child_n=nodeById(child.internalId(),root);
      // if(child_n.isNull())return QModelIndex();
       QDomNode par=child_n.parentNode();
       if(par.toElement().attribute("id").toInt()==0) return createIndex(0,0,0);
       return createIndex(domRow(par),0,idByNode(par));
    };
    int courseModel::columnCount(const QModelIndex &parent)const
                   {

                   return 1;} ;



     QDomNode courseModel::nodeByRowColumn(int row,int column,QDomNode *parent) const
     {
      if(!parent)return root;
   qDebug()<<"nodeByRowColumn";
          return parent->childNodes().at(row);

      return root;
     };
     QDomNode courseModel::nodeById( int id,QDomNode parent) const
     {

         if(parent.toElement().attribute("id","")==QString::number(id))return parent;
         if(!parent.hasChildNodes())return QDomNode();
         QDomNode val=cash.value(id);
         if(!val.isNull())return val;

         QDomNodeList childs=parent.childNodes();
         for(int i=0;i<childs.length();i++)
         {
             if(childs.at(i).toElement().attribute("id","")==QString::number(id))
             {
                // if(cash.contains(id)<1)
                     //cash.insert(id,childs.at(i));
                return childs.at(i);
             };

         }
         for(int i=0;i<childs.length();i++)
         {
            // if(childs.at(i).toElement().attribute("id","")==QString::number(id))return childs.at(i);
                if(childs.at(i).hasChildNodes())
                {

                QDomNode toret=nodeById(id,childs.at(i));
                if(!toret.isNull())return toret;
                }
         }
         return QDomNode();
     };


     Qt::ItemFlags courseModel::flags(const QModelIndex &index) const
     {
         if (!index.isValid())
             return 0;
        // qDebug()<<"IS teacher:"<<isTeacher;
         if(isTeacher)return  Qt::ItemIsEnabled |Qt::ItemIsSelectable|Qt::ItemIsDragEnabled;

       if(!taskAvailable(index.internalId()))return Qt::NoItemFlags;
    // qDebug()<<"Flags:"<<index.internalId();
         return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
     }
     QModelIndex courseModel::createMyIndex(int row,int column,QModelIndex parent) const
     {
         if(!parent.isValid())return createIndex(0,0,0);
      int id=parent.internalId();
      if(id<0)return QModelIndex();
      QDomNode par=nodeById(id,root);
      if(par.isNull())return QModelIndex();
      QDomNodeList childs=par.childNodes();
      if(childs.count()<=row)return QModelIndex();
      bool ok;
      QString string_id=childs.at(row).toElement().attribute("id","");
      if(string_id=="")string_id=childs.at(row).toElement().attribute("xml:id","");
      int new_id=string_id.toInt(&ok);
      if(!ok){

          qDebug()<<"Bad ID"<<string_id;
          return createIndex(-10,-10,-10);

            }
      return createIndex(row,column,new_id);
     };
QString courseModel::getTaskText(QModelIndex index)
{
    if(!index.isValid())return "INDEX NOT VALID";
    QDomNode node=nodeById(index.internalId(),root);
    QDomElement titleEl=node.firstChildElement("DESC");
    if(titleEl.isNull())return "";

    return titleEl.text();
};
QString courseModel::getTaskCheck(QModelIndex index)
{
    if(!index.isValid())return "INDEX NOT VALID";
    QDomNode node=nodeById(index.internalId(),root);
    QDomElement titleEl=node.firstChildElement("CHECK");
    if(titleEl.isNull())return "";

    return titleEl.text();
};
QString courseModel::csName(int index)
{

    QDomNode node=nodeById(index,root);
    QDomElement csEl=node.firstChildElement("CS");
    if(csEl.isNull())return "NO CS";

    return csEl.text();
};
QString courseModel::progFile(int index)
{

    QDomNode node=nodeById(index,root);
    QDomElement csEl=node.firstChildElement("PROGRAM");
    if(csEl.isNull())return "";

    return csEl.text();
};
QStringList courseModel::Modules(int index)
{

    QDomNode node=nodeById(index,root);
    QDomNodeList ispsNodes=node.toElement().elementsByTagName("ISP");
    //QDomElement csEl=node.firstChildElement("ISP");


   QStringList modules;
   for(int i=0;i<ispsNodes.count();i++)
   {
       QDomElement curEl=ispsNodes.at(i).toElement();
       if((curEl.attribute("ispname")!="")&&(curEl.parentNode()==node))
       {
                        modules<<curEl.attribute("ispname");

     qDebug()<<"M"<<curEl.toElement().attribute("ispname");};
    };
    return modules;
};
 void courseModel::removeModule(int id,QString modName)
 {

     QDomNode node=nodeById(id,root);

     QDomElement csEl=node.firstChildElement("ISP");
     qDebug()<<"csEl isNull:"<<csEl.isNull();


    while(!csEl.isNull())
    {
        if(csEl.attribute("ispname")==modName)
        {
          csEl.parentNode().removeChild(csEl);
          return;
        };

      csEl=csEl.nextSiblingElement("ISP");
    };



 };


 void courseModel::addModule(QModelIndex index,QString isp)
 {

      QDomNode node=nodeById(index.internalId(),root);
      QDomText text=courceXml.createTextNode(isp);
      qDebug()<<"Append ISP"<<isp;

      QDomElement ispEl=courceXml.createElement("ISP");
      ispEl.setAttribute("ispname",isp);
      QDomElement csEl=node.firstChildElement("ISP");
      ispEl.appendChild(text);
           if(csEl.isNull())
      {

      node.toElement().appendChild(ispEl);
      }else node.insertAfter(ispEl,csEl);


 };


void courseModel::setIsps(QModelIndex index,QStringList isp)
{
    QDomNode node=nodeById(index.internalId(),root);
    QDomElement csEl=node.firstChildElement("ISP");
    while (!csEl.isNull ())
    {
    node.removeChild ( csEl );
    csEl=node.firstChildElement("ISP");
        };

    for(int i=0;i<isp.count();i++)
       {
        if(isp.at(i)=="")continue;
       QDomText text=courceXml.createTextNode(isp.at(i));
       qDebug()<<"Append ISP"<<isp.at(i);
       QDomElement ispEl=courceXml.createElement("ISP");
       ispEl.setAttribute("ispname",isp.at(i));
       node.toElement().appendChild(ispEl);



       }

};
   void courseModel::setIspEnvs(QModelIndex index,QString isp,QStringList Envs)
   {
       QDomNode node=nodeById(index.internalId(),root);
       QDomElement csEl=node.firstChildElement("ISP");
       while(!csEl.isNull())
       {
         //  qDebug()<<"ISP NAME"<<csEl.attribute("ispname")<<" isp:"<<isp;
         //modules<<csEl.attribute(ispName);
         if(csEl.attribute("ispname")==isp)
         {
             QDomNodeList childList=csEl.childNodes();
           for(int j=0;j<childList.count();j++)
             {
               if(childList.at(j).nodeName()=="ENV")csEl.removeChild(childList.at(j));
             }

             for(int i=0;i<Envs.count();i++)
                {
                QDomText text=courceXml.createTextNode(Envs.at(i));
                qDebug()<<"Append Env"<<Envs.at(i);
                QDomElement envEl=courceXml.createElement("ENV");
                csEl.toElement().appendChild(envEl);
                envEl.appendChild(text);


                }

         }
         csEl=csEl.nextSiblingElement("ISP");
       };

   };
QStringList courseModel::Fields(int index,QString isp)
{

    QDomNode node=nodeById(index,root);
    QDomElement csEl=node.firstChildElement("ISP");


   QStringList fields;
   while(!csEl.isNull())
   {
      // qDebug()<<"ISP NAME"<<csEl.attribute("ispname")<<" isp:"<<isp;
     //modules<<csEl.attribute(ispName);
     if(csEl.attribute("ispname")==isp)
     {
        QDomElement fieldEl=csEl.firstChildElement("ENV");
        while(!fieldEl.isNull())
        {
        //qDebug()<<"fiield:"<<fieldEl.text();
        fields.append(fieldEl.text());
        fieldEl=fieldEl.nextSiblingElement("ENV");
         }
      //  qDebug()<<"Return fiield:"<<fields;
        return fields;
     }
     csEl=csEl.nextSiblingElement("ISP");
   };
    return fields;
};
QString courseModel::Script(int index,QString isp)
{

    QDomNode node=nodeById(index,root);
    QDomElement csEl=node.firstChildElement("ISP");



   while(!csEl.isNull())
   {

     if(csEl.attribute("ispname","")==isp)
     {
        QDomElement fieldEl=csEl.firstChildElement("CONTROL");
        qDebug()<<"Script name from XML:"<<fieldEl.text();
        return fieldEl.text();
     }
     csEl=csEl.nextSiblingElement("ISP");
   };
    return "";
};
 QDomElement courseModel::ispNodeByName(QModelIndex task_index,QString ispName)
 {
     QDomNode node=nodeById(task_index.internalId(),root);
     QDomElement csEl=node.firstChildElement("ISP");
     while(!csEl.isNull())
     {
     if(csEl.attribute("ispname")==ispName)
       {
         return csEl;

       }
       csEl=csEl.nextSiblingElement("ISP");
     };

 };
 void courseModel::removeEvn(QDomElement ispElement,QString fieldStr)
 {
     QDomElement fieldEl=ispElement.firstChildElement("ENV");
     while(!fieldEl.isNull())
     {
   if(fieldEl.text()==fieldStr)
      {
       ispElement.removeChild(fieldEl);
       return;
       };
     fieldEl=fieldEl.nextSiblingElement("ENV");
      }
 }

void courseModel::addEnv(QDomElement ispElement,QString fieldStr)
{
    QDomElement csEl=ispElement.firstChildElement("ENV");
    if(csEl.isNull())
    {
        QDomText text=courceXml.createTextNode(fieldStr);

        QDomElement envEl=courceXml.createElement("ENV");
        ispElement.appendChild(envEl);
        envEl.appendChild(text);
    }else
    {
        QDomText text=courceXml.createTextNode(fieldStr);

        QDomElement envEl=courceXml.createElement("ENV");
        ispElement.insertAfter(envEl,csEl);
        envEl.appendChild(text);
    }
};
