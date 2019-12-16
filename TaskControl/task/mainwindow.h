/****************************************************************************
**
** Copyright (C) 2004-2010 NIISI RAS. All rights reserved.
**
** This file is part of the KuMir.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "../course_model.h"
#include "../taskControlInterface.h"
#include "editdialog.h"
#include "newkursdialog.h"
class CSInterface;
namespace Ui {
    class MainWindowTask;
}

class MainWindowTask : public QMainWindow {
    Q_OBJECT
public:
    MainWindowTask(QWidget *parent = 0);
    ~MainWindowTask();
    void setInterface(CSInterface * csInterface){interface=csInterface;};
    void setCS(QString cs){CS=cs;};
    KumZadanie task;
    void setup();
     QString getFileName(QString fileName);
     void setTeacher(bool mode);

public slots:
    void aboutToQuit ();

    void loadCourse();
    void returnTested();
    void saveCourse();
    void saveCourseFile();
    void saveBaseKurs();
    void showText(const QModelIndex & index );
    void loadHtml(QString fileName);
    void startTask();
    void checkTask();
    void lockCheck();
    void setMark(int mark);
    void resetTask();
    void Close();
    void customContextMenuRequested(QPoint  pos);
    void addTask();
    void addDeepTask();
    void deleteTask();
    void saveKurs();//TEACHER
    void saveKursAs();//TEACHER
    void editTask();//Teacher
    void setEditTaskEnabled(bool flag);
    void moveUp();
    void moveDown();
     void newKurs();
     void endRootEdit();
     void cancelRootEdit();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    void markProgChange();
    void createMoveMenu();
    void setUpDown(QModelIndex index);
    QString loadScript(QString file_name);
    QString loadTestAlg(QString file_name);
    void loadCourseData(const QString filename);
    void loadMarks(const QString fileName);

    QString curDir;
    courseModel* course;
    QModelIndex curTaskIdx;
    CSInterface * interface;
    QString CS;
    bool onTask;
    courseChanges changes;
    QString cursFile;
    QList<int> progChange;
    QFile cursWorkFile;
    QMenu customMenu;
    bool isTeacher;
    EditDialog* editDialog;
    newKursDialog* newDialog;
    QSettings* settings;
     QLineEdit *editRoot;
     QFileInfo baseKursFile; //4 mode
       Ui::MainWindowTask *ui;

};

#endif // MAINWINDOW_H
