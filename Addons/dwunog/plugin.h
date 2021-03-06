#ifndef PLUGIN_H
#define PLUGIN_H

#include "dwunogworld.h"
#include "pult.h"
#include "plugin_interface.h"
#include <QObject>
#include <QList>
#include <QVariant>
class Pult;


class Plugin:
        public QObject,
        public kumirPluginInterface
{

    Q_OBJECT
    Q_INTERFACES(kumirPluginInterface)

    public:

        Plugin();
        ~Plugin();

        void start(); //Запуск исполнителя

        void showField();
        void hideField();

        bool hasPult();
        void showPult();
        void hidePult();

        bool hasOpt(); //options
        void showOpt();
        void hideOpt();

        void reset();
        void setMode(int mode);  //Установить режим

        QString name();
        QString help();
        QString optText();  //Текст для дополнительного окна
        QString errorText() const;
        QWidget *mainWidget();

        QVariant        result();       //возвращаемое значение
        QList<Alg>      algList();      //Список алгоритмов исполнителя
        QList<QVariant> algOptResults();//резы

        void runAlg(QString alg,QList<QVariant> params); //Запуск алгоритма
        void setParameter(const QString &paramName, const QVariant &paramValue);
        void connectSignalSync(const QObject *obj, const char *method);
        void connectSignalSendText(const QObject *obj, const char *method);

    public slots:
        void emitSync();

    signals:
        void sendText(QString);
        void sync();

    private:
        Pult *pult;
        DwunogWorld *world;

};

#endif // PLUGIN_H
