/****************************************************************************
**
** Copyright (C) 2004-2008 NIISI RAS. All rights reserved.
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

#include "editField.h"
#include "application.h"
#include "compiler.h"
#include "kumsinglemodule.h"
#include "kumrobot.h"
#include "printDialog.h"
//#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSpinBox>
#include <QDialogButtonBox>

editField::editField(QWidget *parent, Qt::WindowFlags fl) :
	QMainWindow(parent, fl), Ui::editField()
{

	setupUi(this);
	//setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint);
	field = new RoboField(0, 0);
	View->setScene(field);
	menuH = this->menubar->height();

	field->setEditMode(true);
	field->createField(10, 15);
	field->setRoboPos(0, 0);
	field->createRobot();

	field->drawField(FIELD_SIZE_SMALL);
	int err = field->loadFromFile(app()->settings->value("Robot/StartField/File", "").toString());
	field->drawField(FIELD_SIZE_SMALL);
	setSizes();
	CurrentFileName = app()->settings->value("Robot/StartField/File", "").toString();
	statusbar->setVisible(false);
	QFileInfo fi(CurrentFileName);

	curDir = fi.filePath();

	menubar->addAction(actionHelp);

	connect(actionOpen, SIGNAL(triggered()), this, SLOT(loadFromFile()));
	connect(actionHelp, SIGNAL(triggered()), this, SLOT(Help()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(Save()));
	connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(SaveAs()));
	connect(actionSave_as_start, SIGNAL(triggered()), this, SLOT(SaveAsStart()));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(Close()));
	connect(actionNew_field, SIGNAL(triggered()), this, SLOT(New()));
	connect(actionPrint_to_file, SIGNAL(triggered()), this, SLOT(PrintPDF()));



	connect(field, SIGNAL(was_edit()), this, SLOT(fieldChange()));
	//QFileInfo fi=QFileInfo(CurrentFileName);
	fOpenErr = false;
	if (err == 0) {
		setWindowTitle(QString::fromUtf8("Обстановка - ") + fi.fileName());
	} else {
		setWindowTitle(QString::fromUtf8("Обстановка - нет файла"));
		field->dropWasEdit();
		fOpenErr = true;

	}


	//Диалог нвое поле
	QPalette PallGrey, BackPall;
	BackPall.setBrush(QPalette::Window, QBrush(QColor("lightgrey")));

	PallGrey.setBrush(QPalette::Window, QBrush(QColor(140, 160, 140)));
	NewWindow = new QDialog(this);
	QGridLayout *nwl = new QGridLayout;
	NewWindow->setLayout(nwl);
	NewWindow->setWindowModality(Qt::WindowModal);
	NewWindow->setPalette(BackPall);
	NewWindow->setWindowTitle(QString::fromUtf8("Новая обстановка"));
	QLabel *lKol = new QLabel(QString::fromUtf8("Количество:"), NewWindow, 0);
	nwl->addWidget(lKol, 0, 0, 1, 2, Qt::AlignCenter);

	QLabel *lXSize = new QLabel(QString::fromUtf8("Столбцов"), NewWindow, 0);
	nwl->addWidget(lXSize, 1, 1, 1, 1, Qt::AlignCenter);


	QLabel *lYSize = new QLabel(QString::fromUtf8("Строк"), NewWindow, 0);
	nwl->addWidget(lYSize, 1, 0, 1, 1, Qt::AlignCenter);

	NewWindow->setFixedSize(250, 150);
	eXSizeEdit = new QSpinBox(NewWindow);
	eXSizeEdit->setRange(1, MAX_COLUMNS);
	//eTemp->show();
	eYSizeEdit = new QSpinBox(NewWindow);


	eYSizeEdit->setRange(1, MAX_ROWS);
	nwl->addWidget(eYSizeEdit, 2, 0, 1, 1, Qt::AlignCenter);
	nwl->addWidget(eXSizeEdit, 2, 1, 1, 1, Qt::AlignCenter);


	QDialogButtonBox *buttonBox = new QDialogButtonBox(NewWindow);

	btnOK1 = buttonBox->addButton(QDialogButtonBox::Ok);
	btnCancel1 = buttonBox->addButton(QDialogButtonBox::Cancel);

	nwl->addWidget(buttonBox, 3, 0, 1, 2, Qt::AlignRight);

	connect(btnOK1, SIGNAL(clicked()), this, SLOT(btnOK1Click()));
	connect(btnCancel1, SIGNAL(clicked()), NewWindow, SLOT(close()));
	connect(field, SIGNAL(was_edit()), this, SLOT(fieldChange()));

	prepareNewWindow();
	getLastList();
	rescentMenu = new QMenu();
	rescentMenu->setTitle(trUtf8("Недавние обстановки..."));
	menuField->insertMenu(actionSave, rescentMenu);
	pDialog = new PrintDialog(this);

	setSizes();
	append2LastFiles(app()->settings->value("Robot/StartField/File", "").toString());
	createRescentMenu();


};



editField::~editField()
{

}


void editField::prepareNewWindow()
{

	QPoint pos = this->pos();
	pos.setX(pos.x() + width() / 2 - 100);
	pos.setY(pos.y() + height() / 2 - 40);
	NewWindow->move(pos);
	eXSizeEdit->setValue(field->columns());
	eYSizeEdit->setValue(field->rows());

};
void editField::Help()
{
	QMessageBox::information(this, trUtf8("Помощь"),
		trUtf8("<B><CENTER>Редактирование обстановки.</CENTER></B>\n""Поставить/убрать стену – щелкнуть по границе между клетками.\n"
			"Закрасить/сделать чистой клетку – щелкнуть по клетке.\n<tr></tr>""Поставить/убрать точку – щелкнуть по клетке при нажатой клавише Ctrl.\n""Установить температуру, радиацию, метки – щелкнуть по клетке правой кнопкой\n<tr></tr>""Переместить Робота – тащить мышью."),
		QMessageBox::Ok);
};
void editField::btnOK1Click() //Create new field
{
	field->destroyField();
	field->destroyRobot();
	field->createField(eYSizeEdit->value(), eXSizeEdit->value());
	field->setRoboPos(0, 0);
	field->createRobot();
	setSizes();
	field->drawField(FIELD_SIZE_SMALL);

	setWindowTitle(QString::fromUtf8("Обстановка - нет файла"));
	CurrentFileName = "";
	NewWindow->close();
};


void editField::setSizes()
{
	int sizeAdd = 0;
	if (field->columns() < 3) {
		sizeAdd = FIELD_SIZE_SMALL;
	}

	View->setSceneRect(-FIELD_SIZE_SMALL / 2 + FIELD_SIZE_SMALL / 7 - sizeAdd, -FIELD_SIZE_SMALL / 2 - sizeAdd,
		FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + 2 * sizeAdd, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + 2 * sizeAdd);
	View->setMinimumSize(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + 2 * sizeAdd + 4, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + 2 * sizeAdd + 4);
	View->setMaximumSize(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + 2 * sizeAdd + 4, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + 2 * sizeAdd + 4);
	//   View->setMinimumSize(FIELD_SIZE_SMALL*field->columns()+FIELD_SIZE_SMALL+2*sizeAdd,FIELD_SIZE_SMALL*field->rows()+FIELD_SIZE_SMALL+FIELD_SIZE_SMALL/4+2*sizeAdd);
	//  View->setMaximumSize(FIELD_SIZE_SMALL*field->columns()+FIELD_SIZE_SMALL+2*sizeAdd,FIELD_SIZE_SMALL*field->rows()+FIELD_SIZE_SMALL+FIELD_SIZE_SMALL/4+2*sizeAdd);
	View->update();
	int sizeC = field->columns();

	qDebug() << "SceneRect:" << View->sceneRect();
	qDebug() << "SceneGeom:" << View->frameGeometry();
	qDebug() << "SceneSize:" << View->size();
	int debug1 = FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL;
	int debug2 = menubar->height();
	qDebug() << "Debug1:" << debug1 << " Debug2:" << debug2 << " Summ" << debug1 + debug2;
	this->setMaximumSize(FIELD_SIZE_SMALL * sizeC + FIELD_SIZE_SMALL + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + menuH + 2 * sizeAdd);

	this->setMinimumSize(FIELD_SIZE_SMALL * sizeC + FIELD_SIZE_SMALL + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + menuH + 2 * sizeAdd);
	resize(FIELD_SIZE_SMALL * sizeC + FIELD_SIZE_SMALL + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + menuH + 2 * sizeAdd);
	updateGeometry();
	qDebug() << "WindowGeom:" << this->rect();
};
void editField::markModeChange()
{
	//  field->setMarkMode(Color->isChecked ());
	qDebug("modeChange");
};
void editField::loadFromFile()
{


	if (field->WasEdit()) {
		if (QMessageBox::question(this, "", QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			Save();
		};

	}
	fOpenErr = false;



//        QFileDialog dialog(this,QString::fromUtf8 ("Открыть файл"),curDir, "(*.fil)");

//	if(!dialog.exec())return;
//	QString  RobotFile=dialog.selectedFiles().first();
//	QDir dir=dialog.directory();
	//QString  RobotFile=dialog.selectedFiles().first();


	QString RobotFile = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл"), curDir, "(*.fil)");



	QFileInfo info(RobotFile);
	QDir dir = info.absoluteDir();



	curDir = dir.path();
	if (RobotFile.isEmpty()) {
		return;
	}
	CurrentFileName = RobotFile;
	// field->clear();
	int err = field->loadFromFile(RobotFile);
	if (err == 0) {

		field->drawField(FIELD_SIZE_SMALL);
		setSizes();
		QFileInfo fi(CurrentFileName);

		QString name = fi.fileName();
		setWindowTitle(QString::fromUtf8("Обстановка - ") + name);
		this->setWindowModified(true);
	} else {
		QMessageBox::information(0, "", QString::fromUtf8("Ошибка открытия файла! ") + CurrentFileName, 0, 0, 0);
		return;
	};
	if (lastFiles.indexOf(CurrentFileName) == -1) {
		lastFiles.prepend(CurrentFileName);
	}
	createRescentMenu();

};
void editField::New()
{
	if (field->WasEdit()) {
		if (QMessageBox::question(this, "", QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			Save();
		};

	}
	fOpenErr = false;
	QPoint pos = this->pos();
	pos.setX(pos.x() + width() / 2 - 100);
	pos.setY(pos.y() + height() / 2 - 40);
	NewWindow->move(pos);
	NewWindow->show();

};
void editField::Close()
{
	if (field->WasEdit()) {
		if (QMessageBox::question(this, QString::fromUtf8("Обстановка"), QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			Save();
		};
		this->setWindowModified(false);
		field->dropWasEdit();
	}
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->restoreRobotStartField();
	emit editFinished();
	close();
};


void editField::Save()
{

	if ((CurrentFileName == "") || (fOpenErr)) {
		SaveAs();
		return;
	};
	field->saveToFile(CurrentFileName);
	this->setWindowModified(false);
	if (lastFiles.indexOf(CurrentFileName) == -1) {
		lastFiles.prepend(CurrentFileName);
	}
	// app()->settings->setValue("Robot/StartField/File",CurrentFileName);
};


void editField::SaveAs()
{


//    QFileDialog dialog(this,QString::fromUtf8 ("Сохранить файл"),CurrentFileName, "(*.fil)");
//    dialog.setAcceptMode(QFileDialog::AcceptSave);
//    if(!dialog.exec())return;

	QString RobotFile = QFileDialog::getSaveFileName(this, QString::fromUtf8("Сохранить файл"), curDir, "(*.fil)");


	//QString   RobotFile=dialog.selectedFiles().first();
	QFileInfo info(RobotFile);
	QDir dir = info.absoluteDir();
	curDir = dir.path();
	if (RobotFile.contains("*") || RobotFile.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return;
	}

	if (RobotFile.right(4) != ".fil") {
		RobotFile += ".fil";
	}
	CurrentFileName = RobotFile;
	fOpenErr = false;
	if (field->saveToFile(CurrentFileName) != 0) {
		QMessageBox::information(0, "", QString::fromUtf8("Ошибка записи в файл!"), 0, 0, 0);
		return;
	} else if (lastFiles.indexOf(CurrentFileName) == -1) {
		lastFiles.prepend(CurrentFileName);
	}

	QFileInfo fi = QFileInfo(CurrentFileName);
	setWindowTitle(QString::fromUtf8("Обстановка - ") + fi.fileName());
	append2LastFiles(CurrentFileName);
	createRescentMenu();
	this->setWindowModified(false);
	fOpenErr = false;
};

void editField::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	app()->settings->setValue("Robot/LastFiles", lastFiles.join(";"));
	Close();
};
void editField::SaveAsStart()
{
	//Save();
//    QFileDialog dialog(this,QString::fromUtf8 ("Сохранить файл"),CurrentFileName, "(*.fil)");
//    dialog.setAcceptMode(QFileDialog::AcceptSave);
//    if(!dialog.exec())return;

	//QString   RobotFile=dialog.selectedFiles().first();

	QString RobotFile = QFileDialog::getSaveFileName(this, QString::fromUtf8("Сохранить файл"), CurrentFileName, "(*.fil)");


	//QString   RobotFile=dialog.selectedFiles().first();
	QFileInfo info(RobotFile);
	QDir dir = info.absoluteDir();


	//QDir dir=dialog.directory();
	curDir = dir.path();
	if (RobotFile.contains("*") || RobotFile.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return;
	}

	if (RobotFile.right(4) != ".fil") {
		RobotFile += ".fil";
	}
	CurrentFileName = RobotFile;

	if (field->saveToFile(CurrentFileName) != 0) {
		QMessageBox::information(0, "", QString::fromUtf8("Ошибка записи в файл!"), 0, 0, 0);
		return;
	} else if (lastFiles.indexOf(CurrentFileName) == -1) {
		lastFiles.prepend(CurrentFileName);
	}

	QFileInfo fi = QFileInfo(CurrentFileName);
	setWindowTitle(QString::fromUtf8("Обстановка - ") + fi.fileName());
	append2LastFiles(CurrentFileName);
	createRescentMenu();
	this->setWindowModified(false);
	app()->settings->setValue("Robot/StartField/File", CurrentFileName);
	KumRobot *robo = (KumRobot *)app()->compiler->Modules.module(2)->instrument();
	robo->reset();
	fOpenErr = false;
};

void editField::createRescentMenu()
{
	rescentMenu->clear();

	if (lastFiles.count() == 0) {
		rescentMenu->setEnabled(false);
	} else {
		rescentMenu->setEnabled(true);
	}

	for (int i = 0; i < lastFiles.count(); i++) {
		QAction *action = rescentMenu->addAction("&" + QString::number(i + 1) + " " + lastFiles[i], this, SLOT(openRecent()));
		Q_UNUSED(action);
	}
};

void editField::openRecent()
{

	if (field->WasEdit()) {
		if (QMessageBox::question(this, "", QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			Save();
		};

	};
	QAction *s = qobject_cast<QAction *>(sender());
	QString txt = s->text();
	txt.remove(0, 1);
	QStringList words = txt.split(" ");
	int no = words[0].toInt();
	QString fileName = lastFiles[no - 1];


	CurrentFileName = fileName;
	int err = field->loadFromFile(fileName);
	qDebug() << fileName;
	if (err == 0) {
		MoveUp(CurrentFileName);
		field->destroyField();
		setSizes();
		field->drawField(FIELD_SIZE_SMALL);

		QFileInfo fi(CurrentFileName);

		QString name = fi.fileName();
		setWindowTitle(QString::fromUtf8("Обстановка - ") + name);
		this->setWindowModified(false);
		createRescentMenu();
	} else {
		QMessageBox::information(this, "", QString::fromUtf8("Ошибка открытия файла!"), 0, 0, 0);
		return;
	};
	fOpenErr = false;

};
void editField::fieldChange()
{

	if (!this-> isWindowModified()) {
		if (!windowTitle().endsWith("*]")) {
			setWindowTitle(windowTitle() + " [*]");
		}
		this->setWindowModified(true);
	};
};

void editField::PrintPDF()
{
	pDialog->view = View;
	pDialog->field = field;

	pDialog->setRoboMode(true);

	pDialog->setFieldSize(field->columns(), field->rows());
	pDialog->setSizes(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2
		, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4);
	if (pDialog->exec() != 1) {
		return;
	}


};
