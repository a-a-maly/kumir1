#include "kumrobot.h"
#include "secondarywindow.h"
#include "pult_webkit.h"
#include "error.h"
#include "compiler.h"
#include "application.h"
#include "printDialog.h"
#include "cell_dialog.h"

#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QPrinter>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>


QString MR1, MR2, MR3;
QString MB7, MB2;
QString MRazd;

QString MSleva, MSPrava, MSverhu, MSnizu;

QString MStena, MSvobodno;

QString MKletka, MZakr, MChist;

QString MTemp, MRad;

QString MDa, MNet;

QString MOK, MOtkaz;

#define EXT_WALLS_COLOR "yellow"
#define INT_WALLS_COLOR "white"

void KumRobot::showHideWindow(bool show)//Show - true
{
	if (app()->isExamMode()) {
		return;
	}
	Q_UNUSED(show);
	if (!w_window->isVisible()) {
		w_window->show();
	} else {
		w_window->raise();
	}

	return;

};

void KumRobot::close()
{
//    robotWindow->hide();
//    roboPult->hide();
	w_pultWindow->hide();
	w_window->hide();
}
//--------------------------------------------------------------------
KumRobot::KumRobot(QWidget *parent)
	: KumInstrument(parent)
{
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	curDir = app()->settings->value("Robot/StartField/File", "").toString();

	robotWindow = new RobotWindow();
	w_window = new SecondaryWindow(robotWindow, "Robot");
	w_window->setWindowTitle(tr("Robot"));
	w_window->setWindowIcon(QIcon(QString(":/icons/module_window.png")));

	//robotWindow->show();
	//robotWindow->setWindowFlags( Qt::FramelessWindowHint);

	//header->setResize(true);

	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	roboPult = new RobotPultWK(0);
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	roboPult->setWindowIcon(QIcon(QString(":/icons/pult_window.png")));
	w_pultWindow = new SecondaryWindow(roboPult, "Robot - Pult");
	w_pultWindow->setWindowTitle(tr("Robot - Pult"));
	w_pultWindow->setWindowIcon(QIcon(QString(":/icons/pult_window.png")));
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;

	pDialog = new PrintDialog(0);
	//    Logger=roboPult->pltLogger();
	//roboPult->show();
	connect(roboPult, SIGNAL(goUp()), this, SLOT(btnUpClick()));
	connect(roboPult, SIGNAL(goDown()), this, SLOT(btnDDownClick()));
	connect(roboPult, SIGNAL(goLeft()), this, SLOT(btnLeftClick()));
	connect(roboPult, SIGNAL(goRight()), this, SLOT(btnRightClick()));

	connect(roboPult, SIGNAL(hasUpWall()), this, SLOT(btnUp1Click()));
	connect(roboPult, SIGNAL(hasDownWall()), this, SLOT(btnDown1Click()));
	connect(roboPult, SIGNAL(hasLeftWall()), this, SLOT(btnLeft1Click()));
	connect(roboPult, SIGNAL(hasRightWall()), this, SLOT(btnRight1Click()));

	connect(roboPult, SIGNAL(noUpWall()), this, SLOT(btnUp2Click()));
	connect(roboPult, SIGNAL(noDownWall()), this, SLOT(btnDown2Click()));
	connect(roboPult, SIGNAL(noLeftWall()), this, SLOT(btnLeft2Click()));
	connect(roboPult, SIGNAL(noRightWall()), this, SLOT(btnRight2Click()));

	connect(roboPult, SIGNAL(Color()), this, SLOT(btnCenterClick()));
	//connect (roboPult, SIGNAL(Color()), this, SLOT(Print2PDF()) );
	connect(roboPult, SIGNAL(Colored()), this, SLOT(btnCenter1Click()));
	connect(roboPult, SIGNAL(Clean()), this, SLOT(btnCenter2Click()));
	//robotWindow->show();
	field = new RoboField(parent, this);
	//field->editField();
	field->createField(10, 15);
	field->setRoboPos(0, 0);
	field->createRobot();

//    setWindowPos(app()->settings->value("Robot/Xpos").toInt(),
//                 app()->settings->value("Robot/Ypos").toInt()); //Не удобно на ноутах и с проектором


	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	PathToBaseFil = app()->settings->value("Robot/StartField/File", "").toString();
	//MV->setGeometry(10,10,500,550);
	SceneSizeX = 460;
	SceneSizeY = 400;
	PanelHigth = 60;
	PWWidth = 200;
	PWHieght = 420;
	QColor color;


	//4 LineColor
	color = QColor(app()->settings->value("Robot/LineColor", "#C8C800").toString());
	LineColor = color;






	//NormalColor = QColor(40,150,40);
	//EditColor = QColor(0,0,140);
	//WallColor = QColor(250,250,0);


	ButtonSdvig = 40;
	MainButtonY = 35;
	//----------------------------------------
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	CreateMainButtons();

	CurrentRobotMode = ANALYZE_MODE;

	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	createWindowsLabelsActions();

	connect(NewRobot, SIGNAL(triggered()), this, SLOT(NewRobotField()));
	connect(LoadRobot, SIGNAL(triggered()), this, SLOT(LoadFromFileActivated()));
	connect(SaveRobot, SIGNAL(triggered()), this, SLOT(SaveToFileActivated()));
	//connect ( SaveRobot1, SIGNAL(triggered()), this, SLOT(SaveToFile1()) );
	//connect ( SaveRobotCopy, SIGNAL(triggered()), this, SLOT(SaveToFileCopy()) );
	connect(Edit, SIGNAL(triggered()), this, SLOT(ReverseEditFlag()));
	connect(MPult, SIGNAL(triggered()), this, SLOT(ReversePult()));
	//connect ( MakeDubl, SIGNAL(triggered()), this, SLOT(ToDubl()) );
	//connect ( RestoreDubl, SIGNAL(triggered()), this, SLOT(FromDubl()) );
	//connect ( MFindRobot, SIGNAL(triggered()), this, SLOT(FindRobot()) );
	MenuHigth =  10;
	//MenuHigth = 25;
	//qDebug("temp %f",Temp);
	//qDebug("MENU %i ",MenuHigth);


	//int p_X = DEFAULT_X;
	//int p_Y = DEFAULT_Y;

	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	CreatePult(parent);


	lockFlag = false;
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	CreateEdits();
	//CreatebtnFind();



	//m_Robot = NULL;
	m_x = 0;
	m_y = 0;
	InitialX = 0;
	InitialY = 0;
	WallErrorFlag = false;
	CreateRobot();
	DublFlag = false;
	isEditor = true;
	CurrentFileName = "";
	WasEditFlag = false;
	ReverseEditFlag();

	m_Size_xDubl = 0;
	m_Size_yDubl = 0;
	LoadInitialFil(PathToBaseFil);

	ToDubl();
	tmpm_x = -1;
	tmpm_y = -1;
	SetRobotMode(SEE_MODE);
	robotWindow->graphicsView->setScene(field);
	field->drawField(FIELD_SIZE_SMALL);
	ajustWindowSize();
	qDebug() << "KumRobot::KumRobot; Line = " << __LINE__;
	//dd
	//menu->setVisible(false);
};

void KumRobot::createWindowsLabelsActions()
{
	InputWindow = new QMainWindow(robotWindow, Qt::Tool);;
	InputWindow->setWindowTitle(tr("Cell editing"));//(QString::fromUtf8("� едактирование клетки"));
	//InputWindow = new QMainWindow(MV,Qt::Desktop);
	// || Qt::Popup);
	//InputWindow->setWindowModality(Qt::ApplicationModal);

	lX = new QLabel(tr("Column"), InputWindow, 0);
	lX->setGeometry(QRect(80, 3, 150, 20));

	lY = new QLabel(tr("Row"), InputWindow, 0);
	lY->setGeometry(QRect(5, 3, 80, 20));


	QLabel *lTemp = new QLabel(tr("Temperature"), InputWindow, 0);
	lTemp->setGeometry(QRect(8, 30, 75, 20));

	QLabel *lRad = new QLabel(tr("Radiation"), InputWindow, 0);
	lRad->setGeometry(QRect(95, 30, 75, 20));

	QLabel *lBukv = new QLabel(tr("Letters (↓,↑)"), InputWindow, 0);
	lBukv->setGeometry(QRect(155, 30, 60, 20));

	QPalette PallGrey, BackPall;
	BackPall.setBrush(QPalette::Window, QBrush(QColor("lightgrey")));
	PallGrey.setBrush(QPalette::Window, QBrush(QColor(140, 160, 140)));
	//MV->setPalette (PallGrey);

	//Окно для нового робота
	NewWindow = new QDialog(robotWindow);
	NewWindow->setWindowModality(Qt::ApplicationModal);
	NewWindow->setPalette(BackPall);
	QGridLayout *nwl = new QGridLayout;
	NewWindow->setFixedSize(200, 100);
	NewWindow->setLayout(nwl);
	NewWindow->setWindowTitle(tr("New field"));
	QLabel *lKol = new QLabel(tr("Number:"), NewWindow, 0);
	//    lKol->setGeometry( QRect( 50, 1, 75, 20 ) );
	nwl->addWidget(lKol, 0, 0, 0, 2, Qt::AlignCenter);


	QLabel *lXSize = new QLabel(tr("columns"), NewWindow, 0);
	//    lXSize->setGeometry( QRect( 90, 13, 75, 20 ) );
	nwl->addWidget(lXSize, 1, 1, 1, 1, Qt::AlignCenter);


	QLabel *lYSize = new QLabel(tr("rows"), NewWindow, 0);
	//    lYSize->setGeometry( QRect( 25, 13, 75, 20 ) );
	nwl->addWidget(lYSize, 1, 0, 1, 1, Qt::AlignCenter);
	eXSizeEdit = new QSpinBox(NewWindow);
	eXSizeEdit->setRange(MIN_SIZE, MAX_SIZE);
	nwl->addWidget(eXSizeEdit, 2, 1, 1, 1, Qt::AlignCenter);

	eYSizeEdit = new QSpinBox(NewWindow);
	eYSizeEdit->setRange(MIN_SIZE, MAX_SIZE);
	nwl->addWidget(eYSizeEdit, 2, 0, 1, 1, Qt::AlignCenter);
	QDialogButtonBox *buttonBox = new QDialogButtonBox(NewWindow);
	QPushButton *btnNewFieldOk = buttonBox->addButton(QDialogButtonBox::Ok);
	QPushButton *btnNewFieldCancel = buttonBox->addButton(QDialogButtonBox::Ok);
	connect(btnNewFieldOk, SIGNAL(clicked()), this, SLOT(btnOK1Click()));
	connect(btnNewFieldCancel, SIGNAL(clicked()), this, SLOT(btnCancel1Click()));

	isEditor = false;
	//menu =MV->menuBar()->addMenu(QString::fromUtf8("&Обстановка"));

	NewRobot = new QAction(tr("New field"), robotWindow);
	//menu->addAction(NewRobot);

	LoadRobot = new QAction(tr("Load from file..."), robotWindow);
	//menu->addAction(LoadRobot);

	SaveRobot = new QAction(tr("Save as..."), robotWindow);
	//menu->addAction(SaveRobot);

	//SaveRobot1 =new QAction(QString::fromUtf8("&Сохранить"),MV);
	//menu->addAction(SaveRobot1);


	//QAction* SaveRobotCopy =new QAction(QString::fromUtf8("&Сохранить копию"),MV);
	//menu->addAction(SaveRobotCopy);


	Edit = new QAction(tr("Edit"), robotWindow);
	Edit->setCheckable(true);
	//menu->addAction(Edit);

	MPult = new QAction(tr("Edit"), robotWindow);

	robotWindow->setWindowTitle(tr("Robot - no file"));
}


void KumRobot::setWindowPos(int x, int y)
{
	w_window->move(x, y);
}

void KumRobot::getWindowPos(int *x, int *y)
{
	*x = w_window->x();
	*y = w_window->y();
}

void KumRobot::SwitchMode(int mode)
{
	SetRobotMode(mode);
	emit sync();
}

void KumRobot::ajustWindowSize()
{
	int sizeAdd = 0;
	if (field->columns() < 3) {
		sizeAdd = FIELD_SIZE_SMALL;
	}
	robotWindow->setMaximumSize(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd);

	robotWindow->setMinimumSize(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd);
	robotWindow->graphicsView->setSceneRect(-FIELD_SIZE_SMALL / 2 - sizeAdd, -FIELD_SIZE_SMALL / 2 - sizeAdd,
		FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + 2 * sizeAdd, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + 2 * sizeAdd);
	robotWindow->resize(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd,
		FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2 * sizeAdd);
	robotWindow->graphicsView->update();
}



//------------------------------------

bool KumRobot::IsColored(int ix, int iy)
{
	return GetFieldItem(ix, iy)->m_Is_Colored;
}
//------------------------------------------------------
double KumRobot::GetRadiation(int ix, int iy)
{
	return GetFieldItem(ix, iy)->m_Radiation;
}
//------------------------------------------------
double KumRobot::GetTemperature(int ix, int iy)
{
	return GetFieldItem(ix, iy)->m_Temperature;
}
//------------------------------------------------
QChar KumRobot::GetBukva(int ix, int iy)
{
	return GetFieldItem(ix, iy)->m_Text;
}

QChar KumRobot::GetBukva1(int ix, int iy)
{
	return GetFieldItem(ix, iy)->m_Text1;
}
//---------------------------------------------------
void KumRobot::SetTemperature(int ix, int iy, double Temp)
{
	GetFieldItem(ix, iy)->m_Temperature = Temp;
}
//------------------------------------------
void KumRobot::SetRadiation(int ix, int iy, double Rad)
{
	GetFieldItem(ix, iy)->m_Radiation = Rad;
}
//----------------------------------------------------
void KumRobot::SetBukva(int ix, int iy, QChar Bukv)
{
	if (Bukv == '_' || Bukv == ' ') {
		Bukv = '$';
	}
	GetFieldItem(ix, iy)->m_Text = Bukv;
}
//
void KumRobot::SetBukva1(int ix, int iy, QChar Bukv1)
{
	if (Bukv1 == '_' || Bukv1 == ' ') {
		Bukv1 = '$';
	}
	GetFieldItem(ix, iy)->m_Text1 = Bukv1;
}
//------------------------------------------------------
int KumRobot::LoadFromFile(QString p_FileName)
{
	if (field->loadFromFile(p_FileName) != 0) {
		return 1;
	}
	startField = field->Clone();

	ajustWindowSize();//NEW ROBOT

	QFileInfo fi(CurrentFileName);
	QString name = fi.fileName();

	QString Title = tr("Robot") + " - " + name;
	robotWindow->setWindowTitle(Title);
#ifdef Q_OS_WIN32
	//    header->setWMTitle(Title);
	w_window->setWindowTitle(Title);
#endif


	ToDubl();


	SizeX = m_Size_x * m_FieldSize;
	SizeY = m_Size_y * m_FieldSize + MenuHigth;



	WasEditFlag = false;
	//delete btnFind;
	//CreatebtnFind();

	app()->settings->setValue("Robot/StartField/File", p_FileName);
	field->drawField(FIELD_SIZE_SMALL);
	if (CurrentRobotMode == ANALYZE_MODE) {
		SetRobotMode(SEE_MODE);
	}
	qDebug() << "File " << p_FileName ;

	return (0);
}

//------------------------------------------------
void KumRobot::DrawField()
{


}
//--------------------------------------------------------
void KumRobot::ColorCell(int i, int j)
{

	GetFieldItem(i, j)->m_Is_Colored = true;
	qreal x0 = i * m_FieldSize + BORT - 2;
	qreal y0 = j * m_FieldSize + BORT;
	qreal Size = m_FieldSize;
	//if(!GetFieldItem(i,j)->m_t){
	//if(true){
	delete GetFieldItem(i, j)->m_t;
	GetFieldItem(i, j)->m_t = new QGraphicsRectItem(x0 + 1, y0 + 1 - BORT, Size - 2, Size - 2);

	GetFieldItem(i, j)->m_t->setBrush(QBrush(QColor("gray")));
	GetFieldItem(i, j)->m_t->setPen(QPen(QColor("gray")));
	GetFieldItem(i, j)->m_t->setZValue(0);

	//}else
	//{
	//GetFieldItem(i,j)->m_t->setRect( x0+1,y0+1-BORT, Size-2, Size-2);
	//GetFieldItem(i,j)->m_t->setBrush(QBrush(QColor("gray")));
	//GetFieldItem(i,j)->m_t->setPen(QPen(QColor("gray")));
	//GetFieldItem(i,j)->m_t->setZValue(0);
	//};




}

//-----------------------------------------------

void KumRobot::setStartEnvironment(const QString &file)
{
	CurrentFileName = file;
	if (LoadFromFile(file) != 0) {
		QMessageBox::information(robotWindow, "", QString::fromUtf8("Ошибка открытия файла ") + file, 0, 0, 0);
	}
}


void KumRobot::LoadFromFileActivated()
{

	if (WasEditFlag) {
		if (QMessageBox::question(robotWindow, "", QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			SaveToFileActivated();
			WasEditFlag = false;
		}

	}



//

	QString RobotFile = QFileDialog::getOpenFileName(robotWindow, QString::fromUtf8("Открыть файл"), curDir, "(*.fil)");



	QFileInfo info(RobotFile);
	QDir dir = info.absoluteDir();


	if (RobotFile.isEmpty()) {
		return;
	}
	CurrentFileName = RobotFile;
	if (LoadFromFile(RobotFile) != 0) {
		QMessageBox::information(robotWindow, "", QString::fromUtf8("Ошибка открытия файла! ") + CurrentFileName, 0, 0, 0);
	}

}

//-----------------------------------------------------
void KumRobot::SaveToFileActivated()
{



	QString RobotFile = QFileDialog::getSaveFileName(robotWindow, QString::fromUtf8("Сохранить файл"), curDir, "(*.fil)");


	//QString   RobotFile=dialog.selectedFiles().first();
	QFileInfo info(RobotFile);
	QDir dir = info.absoluteDir();
	curDir = dir.path();
	if (RobotFile.contains("*") || RobotFile.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return;
	}
	//QString   RobotFile =  QFileDialog::getSaveFileName(MV,QString::fromUtf8 ("Сохранить в файл"),"/home", "(*.fil)");
	//if ( RobotFile.isEmpty())return;

	if (RobotFile.right(4) != ".fil") {
		RobotFile += ".fil";
	}
	CurrentFileName = RobotFile;

	//SaveToFile(RobotFile);
	app()->settings->setValue("Robot/StartField/File", RobotFile);
	QFileInfo fi = QFileInfo(CurrentFileName);
	robotWindow->setWindowTitle(QString::fromUtf8("Робот - ") + fi.fileName());
#ifdef Q_OS_WIN32
	//    header->setWMTitle(QString::fromUtf8("Робот - ") + fi.fileName());
	w_window->setWindowTitle(QString::fromUtf8("Робот - ") + fi.fileName());
#endif
}

//------------------------------------------------
int KumRobot::SaveToFile(QString p_FileName)
{

	//SetSceneSize(700,100);
	//return 1;

	QFileInfo fi(CurrentFileName);
	QString name = fi.fileName();
	if (field->saveToFile(p_FileName) != 0) {
		QMessageBox::warning(0, tr("Robot"), QString::fromUtf8("Не удалось сохранить файл"));
	};

	WasEditFlag = false;
	return 0;

}
//--------------------------------------------
bool KumRobot::StepLeft()
{

	return field->stepLeft();
}
//-----------------------------------------------------
bool KumRobot::StepRight()
{
	return field->stepRight();
}
//-------------------------------------------------------------------
bool KumRobot::StepUp()
{
	return field->stepUp();
}
//-----------------------------------------------------------------

bool KumRobot::StepDown()
{

	return field->stepLeft();
}

//----------------------------------------------------------------------------------
bool KumRobot::RepaintRobot()
{

	double XNew = m_FieldSize * m_x + 3;
	double YNew = m_FieldSize * m_y + 0;

	QPointF old_p = m_Robot->pos();

	if (isEditor) {
		m_Robot->setBrush(QBrush(QColor("lightgray")));
	} else {
		if (WallErrorFlag) {
			m_Robot->setBrush(QBrush(QColor("red")));
		} else {
			m_Robot->setBrush(QBrush(QColor("lightgray")));
		}
	}
	m_Robot->setPos(XNew, YNew);
	return true;
}
//-------------------------------------------------
bool KumRobot::LeftWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canLeft();
}

//-----------------------------------------------
bool KumRobot::LeftErrorWall(int ix, int iy)
{
	if ((!ix) || ((GetFieldItem(ix, iy)->m_ErrorWall & LEFT_WALL) == LEFT_WALL)) {
		GetFieldItem(ix, iy)->m_ErrorWall = 0;
		return true;
	}
	if ((GetFieldItem(ix - 1, iy)->m_ErrorWall & RIGHT_WALL) == RIGHT_WALL) {
		GetFieldItem(ix - 1, iy)->m_ErrorWall = 0;
		return true;
	}
	return false;
}

//------------------------------------------------
bool KumRobot::RightWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canRight();
}

//------------------------------------------------
bool KumRobot::RightErrorWall(int ix, int iy)
{
	if ((ix == m_Size_x - 1) || ((GetFieldItem(ix, iy)->m_ErrorWall & RIGHT_WALL) == RIGHT_WALL)) {
		//GetFieldItem(ix, iy)->m_ErrorWall = 0;
		return true;
	}
	if ((GetFieldItem(ix + 1, iy)->m_ErrorWall & LEFT_WALL) == LEFT_WALL) {
		//GetFieldItem(ix + 1, iy)->m_ErrorWall = 0;
		return true;
	}
	return false;
}

//---------------------------------------------
bool KumRobot::UpWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canUp();
}


//---------------------------------------------
bool KumRobot::UpErrorWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canUp();
}




//-----------------------------------------------------
bool KumRobot::DownWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canDown();
}

//-----------------------------------------------------
bool KumRobot::DownErrorWall(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);
	return !field->currentCell()->canDown();
}


//-----------------------------------------------------------------------
void KumRobot::CreateRobot()
{

	//  qreal l_BaseX = m_x * m_FieldSize;
	//  qreal l_BaseY = m_y * m_FieldSize;
	//static const int points[] = {  6,3, 6,10, 2,10, 2,11, 6,11, 6,14, 8,14, 8,17, 9,18, 10,18, 11,17, 11,14, 13,14, 13,11, 17,11, 17,10, 13,10, 13,3 };
	//static const int points[] = {  4,17, 4,6, 7,6, 7,2, 12,2, 12,6, 15,6, 15,17 };
	//static const int points[] = {  6,6, FIELD_SIZE_SMALL-6,6, FIELD_SIZE_SMALL-6,FIELD_SIZE_SMALL-6, 6,FIELD_SIZE_SMALL-6 };

	static const int points[] = {  14, 6, 22, 14, 14, 22, 6, 14 };
	QPolygon polygon;
	polygon.setPoints(4, points);
	QPolygonF polygonf = QPolygonF(polygon);

	m_Robot = new QGraphicsPolygonItem();
	m_Robot->setPolygon(polygonf);
	m_Robot->setZValue(100);
	//m_Robot->setBrush(QBrush(QColor("lightgray")));

	/*
	    m_Robot = new QGraphicsRectItem ( l_BaseX + m_FieldSize/4+BORT,l_BaseY + m_FieldSize/4, m_FieldSize/2, m_FieldSize/2);
	    m_Robot->setZValue(100);
	    m_Robot->setBrush(QBrush(QColor("black")));
	    scene->addItem(m_Robot);
	*/
	//  QPointF new_p=m_Robot->pos();

}
//------------------------------------------------

//-----------------------------------------
KumRobot::RobotFieldItem *KumRobot::GetFieldItem(int p_X, int p_Y)
{
	return &(m_Field[p_Y * m_Size_x + p_X]);
}
//----------------------------
KumRobot::RobotFieldItem *KumRobot::GetFieldItemDubl(int p_X, int p_Y)
{
	return &(m_FieldDubl[p_Y * m_Size_x + p_X]);
}

/**
 * Добавляет в таблицы функции и переменные,устанавливает таблицы класса
 *  function_table и symbol_table
 * @param Functions Таблица функций
 * @param Symbols Таблица переменных
 * @return количество функций
 */
int KumRobot::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	symbols = Symbols;
	functions = Functions;
	int count = 0;

	Functions->addInternalFunction(trUtf8("вверх"), none, 0);

	Functions->addInternalFunction(trUtf8("вниз"), none, 0);

	Functions->addInternalFunction(trUtf8("вправо"), none, 0);

	Functions->addInternalFunction(trUtf8("влево"), none, 0);

	Functions->addInternalFunction(trUtf8("закрасить"), none, 0);

	if (isEditor) {
		//         Functions->addInternalFunction(QString::fromUtf8("установить стену справа"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("установить стену сверху"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("установить стену снизу"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("установить стену слева"), none,0);
		//
		//         Functions->addInternalFunction(QString::fromUtf8("удалить стену слева"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("удалить стену справа"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("удалить стену снизу"), none,0);
		//         Functions->addInternalFunction(QString::fromUtf8("удалить стену сверху"), none,0);
	};

	Functions->addInternalFunction(trUtf8("радиация"), real, 0);
	Symbols->add_symbol(trUtf8("радиация"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);

	Symbols->add_symbol(trUtf8("справа стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("справа стена"), kumBoolean, 0);




	Symbols->add_symbol(trUtf8("справа свободно"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);

	Functions->addInternalFunction(trUtf8("справа свободно"), kumBoolean, 0);


	Symbols->add_symbol(trUtf8("слева стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);

	Functions->addInternalFunction(trUtf8("слева стена"), kumBoolean, 0);




	Symbols->add_symbol(trUtf8("снизу свободно"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("снизу свободно"), kumBoolean, 0);



	Symbols->add_symbol(trUtf8("сверху свободно"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("сверху свободно"), kumBoolean, 0);

	Symbols->add_symbol(trUtf8("снизу стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("снизу стена"), kumBoolean, 0);



	Symbols->add_symbol(trUtf8("сверху стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("сверху стена"), kumBoolean, 0);



	Symbols->add_symbol(trUtf8("слева свободно"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("слева свободно"), kumBoolean, 0);




	Symbols->add_symbol(trUtf8("клетка закрашена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("клетка закрашена"), kumBoolean, 0);

	Symbols->add_symbol(trUtf8("клетка чистая"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("клетка чистая"), kumBoolean, 0);

	Symbols->add_symbol(trUtf8("температура"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("температура"), real, 0);
	count++;

	Symbols->add_symbol(trUtf8("@@температура"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);

	Functions->addInternalFunction(trUtf8("@@температура"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);

	Functions->setTeacher(count, true);
	count++;
	Symbols->add_symbol(trUtf8("@@радиация"), -1, real, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@радиация"), real, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@нижняя буква"), -1, charect, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@нижняя буква"), charect, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@верхняя буква"), -1, charect, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@верхняя буква"), charect, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;
	Functions->addInternalFunction(trUtf8("@@робот"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 0, true);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;

	Functions->addInternalFunction(trUtf8("@@размер поля"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 0, true);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;
	;
	Symbols->add_symbol(trUtf8("@@метка"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@метка"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@закрашена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@закрашена"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@справа стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@справа стена"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;


	Symbols->add_symbol(trUtf8("@@слева стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@слева стена"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@сверху стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@сверху стена"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;

	Symbols->add_symbol(trUtf8("@@снизу стена"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("@@снизу стена"), kumBoolean, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;


	Functions->addInternalFunction(trUtf8("@@послать робота"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setTeacher(count, true);
	count++;


	if (isEditor) {
		//Functions->addInternalFunction(QString::fromUtf8("установить температуру"), none,1);
		//Functions->func_table[Functions->count].perems[0]=real;
		//count++;


		//Functions->addInternalFunction(QString::fromUtf8("установить радиацию"), none,1);
		//Functions->func_table[Functions->count].perems[0]=real;
		//count++;
	};
	//     Functions->addInternalFunction(QString::fromUtf8("загрузить обстановку"), none,1);
	//     Functions->func_table[Functions->count].perems[0]=kumString;
	//     count++;
	if (isEditor) {
		//Functions->addInternalFunction(QString::fromUtf8("сохранить обстановку"), none,1);
		//Functions->func_table[Functions->count].perems[0]=kumString;
		// count++;
	};
	ToDubl();
	robotWindow->restoreGeometry(app()->settings->value("Robot/Geometry", "").toByteArray());
	//MV->show();
	return count;
};




/**
 * Выполнение функций. эта функция вызывается из run.
 * @param name Имя функции
 * @param aruments список аргументов
 */
void KumRobot::runFunc(QString name, QList<KumValueStackElem> *arguments, int *err)
{
	//*err=0;
	//TODO Функции Robot
	//   field.destroyField();
	//  field.destroyNet();
	if (name == trUtf8("закрасить")) {
		if (!field->currentCell()->IsColored) {
			field->reverseColorCurrent();
		}

		emit sync();
	};

	if (name == trUtf8("клетка закрашена")) {
		int perem_id = symbols->inTable(trUtf8("клетка закрашена"), "global");

		if (!field->currentCell()->IsColored) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};

	if (name == trUtf8("клетка чистая")) {
		int perem_id = symbols->inTable(trUtf8("клетка чистая"), "global");

		if (field->currentCell()->IsColored) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};


	if (name == "LoadStart") {

		LoadFromFileActivated();
		emit sync();
	};

	if (name == "EditStart") {

		ReverseEditFlag();
		emit sync();
	};
	if (name == "SaveCurrent") {

		SaveCurrent();
		emit sync();
	};


	if (name == "ShowPult") {

		ReversePult();
		emit sync();
	};

	if (name == trUtf8("снять окраску")) {

		GetFieldItem(m_x, m_y)->m_Is_Colored = false;
		ColorCell(m_x, m_y);
		emit sync();
	};



	if (name == trUtf8("вниз")) {
		//        field.destroyField();
		//field.drawNet();
		// field.debug();

		if (!field->stepDown()) {
			*err = ROBOT_STENA;
			emit sync();
			field->robot->setCrash(DOWN_CRASH);
			return;
		}
		emit sync();

	};


	if (name == trUtf8("вверх")) {

		if (!field->stepUp()) {
			*err = ROBOT_STENA;
			emit sync();
			field->robot->setCrash(UP_CRASH);
			return;
		}
		emit sync();

	};

	if (name == trUtf8("вправо")) {

		if (!field->stepRight()) {
			*err = ROBOT_STENA;
			emit sync();
			field->robot->setCrash(RIGHT_CRASH);
			return;
		}
		emit sync();
	};
	if (name == trUtf8("влево")) {

		if (!field->stepLeft()) {
			*err = ROBOT_STENA;
			emit sync();
			field->robot->setCrash(LEFT_CRASH);
			return;
		}
		emit sync();
	};

	if (name == trUtf8("установить стену слева")) {
		SetLeftWall();
		emit sync();
	}

	if (name == trUtf8("установить стену справа")) {
		SetRightWall();
		emit sync();
	}

	if (name == trUtf8("установить стену сверху")) {
		SetUpWall();
		emit sync();
	}

	if (name == trUtf8("установить стену снизу")) {
		SetDownWall();
		emit sync();
	}


	if (name == trUtf8("удалить стену слева")) {
		DeleteLeftWall();
		emit sync();
	}

	if (name == trUtf8("удалить стену справа")) {
		DeleteRightWall();
		emit sync();
	}

	if (name == trUtf8("удалить стену сверху")) {
		DeleteUpWall();
		emit sync();
	}

	if (name == trUtf8("удалить стену снизу")) {
		DeleteDownWall();
		emit sync();
	}



	if (name == trUtf8("радиация")) {
		int perem_id = symbols->inTable(trUtf8("радиация"), "global");
		//float rrr = GetRadiation(m_x,m_y);
		float rrr = field->currentCell()->radiation;
		symbols->symb_table[perem_id].value.setFloatValue(rrr);
		symbols->setUsed(perem_id, true);
		emit sync();
	};



	if (name == trUtf8("установить радиацию")) {
		KumValueStackElem argument = arguments->first();
		double rrr = argument.asFloat();
		SetRadiation(m_x, m_y, rrr);
		emit sync();

	};

	if (name == trUtf8("установить температуру")) {
		KumValueStackElem argument = arguments->first();
		double rrr = argument.asFloat();
		SetTemperature(m_x, m_y, rrr);
		emit sync();

	};


	if (name == trUtf8("температура")) {
		int perem_id = symbols->inTable(trUtf8("температура"), "global");
		float rrr = field->currentCell()->temperature;

		symbols->symb_table[perem_id].value.setFloatValue(rrr);
		symbols->setUsed(perem_id, true);
		emit sync();
	};


	if (name == trUtf8("слева стена")) {
		int perem_id = symbols->inTable(trUtf8("слева стена"), "global");

		if (field->currentCell()->canLeft()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};

	if (name == trUtf8("слева свободно")) {
		int perem_id = symbols->inTable(trUtf8("слева свободно"), "global");

		if (!field->currentCell()->canLeft()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};


	if (name == trUtf8("справа стена")) {

		int perem_id = symbols->inTable(trUtf8("справа стена"), "global");

		if (field->currentCell()->canRight()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}
	}

	if (name == trUtf8("справа свободно")) {

		int perem_id = symbols->inTable(trUtf8("справа свободно"), "global");

		if (!field->currentCell()->canRight()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}
	}



	if (name == trUtf8("сверху стена")) {

		int perem_id = symbols->inTable(trUtf8("сверху стена"), "global");

		if (field->currentCell()->canUp()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};

	if (name == trUtf8("сверху свободно")) {

		int perem_id = symbols->inTable(trUtf8("сверху свободно"), "global");

		if (!field->currentCell()->canUp()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}

	};



	if (name == trUtf8("снизу стена")) {

		int perem_id = symbols->inTable(trUtf8("снизу стена"), "global");

		if (field->currentCell()->canDown()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}


	};

	if (name == trUtf8("снизу свободно")) {

		int perem_id = symbols->inTable(trUtf8("снизу свободно"), "global");

		if (!field->currentCell()->canDown()) {
			symbols->symb_table[perem_id].value.setBoolValue(false);
			symbols->setUsed(perem_id, true);
			emit sync();
			return;
		} else {
			symbols->symb_table[perem_id].value.setBoolValue(true);
			symbols->setUsed(perem_id, true);
			emit sync();
		}



		emit sync();
	} else if (name == trUtf8("загрузить обстановку")) {
		KumValueStackElem argument = arguments->first();
		QString obst_file_name = argument.asString();
		LoadFromFile(obst_file_name);
		emit sync();
	} else if (name == trUtf8("сохранить обстановку")) {
		KumValueStackElem argument = arguments->first();
		QString obst_file_name = argument.asString();
		SaveToFile(obst_file_name);
		emit sync();
	};

	if (name == trUtf8("@@размер поля")) {
		PeremPrt link_w, link_h;
		link_w.perem_id = arguments->first().asInt();
		KumValueStackElem argument = arguments->at(1);
		link_w.mod_id = argument.asInt();

		argument = arguments->at(2);
		link_h.perem_id = argument.asInt();
		argument = arguments->at(3);
		link_h.mod_id = argument.asInt();
		app()->compiler->modules()->setIntegerValue(link_h, field->columns());
		app()->compiler->modules()->setIntegerValue(link_w, field->rows());

		app()->compiler->modules()->setUsed(link_w, true);
		app()->compiler->modules()->setUsed(link_h, true);



		emit sync();
		return;
	};

	if (name == trUtf8("@@робот")) {
		PeremPrt link_w, link_h;
		link_w.perem_id = arguments->first().asInt();
		KumValueStackElem argument = arguments->at(1);
		link_w.mod_id = argument.asInt();

		argument = arguments->at(2);
		link_h.perem_id = argument.asInt();
		argument = arguments->at(3);
		link_h.mod_id = argument.asInt();
		app()->compiler->modules()->setIntegerValue(link_h, field->robotX() + 1);
		app()->compiler->modules()->setIntegerValue(link_w, field->robotY() + 1);

		app()->compiler->modules()->setUsed(link_w, true);
		app()->compiler->modules()->setUsed(link_h, true);



		emit sync();
		return;
	};
	if (name == trUtf8("@@температура")) {

		int perem_id = symbols->inTable(trUtf8("@@температура"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();


		symbols->symb_table[perem_id].value.setIntegerValue(field->cellAt(w - 1, h - 1)->temperature);
		symbols->setUsed(perem_id, true);
		emit sync();
		return;
	};
	if (name == trUtf8("@@радиация")) {

		int perem_id = symbols->inTable(trUtf8("@@радиация"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();


		symbols->symb_table[perem_id].value.setFloatValue(field->cellAt(w - 1, h - 1)->radiation);
		symbols->setUsed(perem_id, true);
		emit sync();
		return;
	};
	if (name == trUtf8("@@нижняя буква")) {

		int perem_id = symbols->inTable(trUtf8("@@нижняя буква"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();


		symbols->symb_table[perem_id].value.setCharectValue(field->cellAt(w - 1, h - 1)->downChar);
		symbols->setUsed(perem_id, true);
		emit sync();
		return;
	};
	if (name == trUtf8("@@верхняя буква")) {

		int perem_id = symbols->inTable(trUtf8("@@верхняя буква"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();


		symbols->symb_table[perem_id].value.setCharectValue(field->cellAt(w - 1, h - 1)->upChar);
		symbols->setUsed(perem_id, true);
		emit sync();
		return;
	};
	if (name == trUtf8("@@метка")) {

		int perem_id = symbols->inTable(trUtf8("@@метка"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();


		symbols->symb_table[perem_id].value.setBoolValue(field->cellAt(w - 1, h - 1)->mark);
		symbols->setUsed(perem_id, true);
		emit sync();
		return;
	};
	if (name == trUtf8("@@закрашена")) {

		int perem_id = symbols->inTable(trUtf8("@@закрашена"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			bool value = cell->isColored();
			symbols->symb_table[perem_id].value.setBoolValue(value);
			symbols->setUsed(perem_id, true);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	};

	if (name == trUtf8("@@справа стена")) {

		int perem_id = symbols->inTable(trUtf8("@@справа стена"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			bool value = cell->hasRightWall();
			symbols->symb_table[perem_id].value.setBoolValue(value);
			symbols->setUsed(perem_id, true);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	}

	if (name == trUtf8("@@слева стена")) {

		int perem_id = symbols->inTable(trUtf8("@@слева стена"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			bool value = cell->hasLeftWall();
			symbols->symb_table[perem_id].value.setBoolValue(value);
			symbols->setUsed(perem_id, true);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	}

	if (name == trUtf8("@@сверху стена")) {

		int perem_id = symbols->inTable(trUtf8("@@сверху стена"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			bool value = cell->hasUpWall();
			symbols->symb_table[perem_id].value.setBoolValue(value);
			symbols->setUsed(perem_id, true);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	}


	if (name == trUtf8("@@снизу стена")) {

		int perem_id = symbols->inTable(trUtf8("@@снизу стена"), "global");
		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			bool value = cell->hasDownWall();
			symbols->symb_table[perem_id].value.setBoolValue(value);
			symbols->setUsed(perem_id, true);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	}



	if (name == trUtf8("@@послать робота")) {


		KumValueStackElem argument = arguments->at(0);
		int w = argument.asInt();
		argument = arguments->at(1);
		int h = argument.asInt();

		Q_CHECK_PTR(field);
		Q_CHECK_PTR(symbols);
		FieldItm *cell = field->cellAt(w - 1, h - 1);
		if (cell) {
			field->setRoboPos(w - 1, h - 1);
		} else {
			*err = ROBOT_BAD_INDEX;
		}
		emit sync();
		return;
	}
};


void KumRobot::ToInitialPos()
{
	m_x = InitialX;
	m_y = InitialY;
	RepaintRobot();
}


void KumRobot::CreateEdits()
{
	eTemp = new QLineEdit("", InputWindow);
	eTemp->setGeometry(20, 50, 60, 20);
	eTemp->show();
	eRad = new QLineEdit("", InputWindow);
	eRad->setGeometry(95, 50, 60, 20);
	eRad->show();
	eBukv = new QLineEdit("", InputWindow);
	//eBukv->setInputMask("N");
	eBukv->setInputMask("X");
	eBukv->setMaxLength(1);
	eBukv->setGeometry(160, 50, 20, 20);
	eBukv->show();
	eBukv1 = new QLineEdit("", InputWindow);
	eBukv1->setInputMask("X");
	eBukv->setMaxLength(1);
	eBukv1->setGeometry(185, 50, 20, 20);
	eBukv1->show();
}

//---------------------------------------------------------------
void KumRobot::CreatebtnOK()
{
	btnOK = new QToolButton(InputWindow);
	//btnOK->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	//btnOK->setIconSize(QSize(40, 20));
	btnOK->setText("OK");
	btnOK->setToolTip("OK");
	btnOK->move(100, 85);
	btnOK->resize(40, 20);
	btnOK->show();
}

//---------------------------------------------------------------
void KumRobot::CreatebtnCancel()
{
	btnCancel = new QToolButton(InputWindow);
	//btnOK->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	//btnOK->setIconSize(QSize(40, 20));
	btnCancel->setText(trUtf8("Отменить"));
	btnCancel->setToolTip(trUtf8("Отменить"));
	btnCancel->move(150, 85);
	btnCancel->resize(60, 20);
	btnCancel->show();
}

void KumRobot::CreatebtnRobot()
{
	btnRobot = new QCheckBox(QString::fromUtf8("Роботa сюда"), InputWindow);
	//btnOK->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	//btnOK->setIconSize(QSize(40, 20));
	//btnRobot->setText(trUtf8("Уст.робота"));
	//  btnRobot->setToolTip(trUtf8("Уст.робота"));
	btnRobot->move(10, 72);
	btnRobot->resize(95, 20);
	btnRobot->show();
}

void KumRobot::CreatebtnMetka()
{
	btnMetka = new QCheckBox(QString::fromUtf8("Точка"), InputWindow);
	//btnOK->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	//btnOK->setIconSize(QSize(40, 20));
	//btnRobot->setText(trUtf8("Уст.робота"));
	//  btnRobot->setToolTip(trUtf8("Уст.робота"));
	btnMetka->move(10, 92);
	btnMetka->resize(95, 20);
	btnMetka->show();
}
//-----------------------------------------
void KumRobot::CreatebtnLeft(int H0)
{
	btnLeft = new QToolButton(PW);
	btnLeft->setObjectName(QString::fromUtf8("btnLeft"));
	btnLeft->setIcon(QIcon(QString::fromUtf8(":/1leftarrow.png")));
	btnLeft->setToolTip(trUtf8("Влево"));
	btnLeft->setIconSize(QSize(20, 20));
	btnLeft->move(PWWidth / 2 - BUTTON_SIZE - BUTTON_SIZE / 2, 2 * BUTTON_SIZE + H0);
	btnLeft->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnLeft->show();
	connect(btnLeft, SIGNAL(clicked()), this, SLOT(btnLeftClick()));
}

//-----------------------------------------
void KumRobot::CreatebtnLeft1(int H0)
{
	btnLeft1 = new QToolButton(PW);
	btnLeft1->setObjectName(QString::fromUtf8("btnLeft"));
	btnLeft1->setIcon(QIcon(QString::fromUtf8(":/RobotLeftWall.png")));
	btnLeft1->setToolTip(trUtf8("Слева стена?"));
	btnLeft1->setIconSize(QSize(20, 20));
	btnLeft1->move(PWWidth / 2 - BUTTON_SIZE - BUTTON_SIZE / 2 - ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	btnLeft1->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnLeft1->show();
	connect(btnLeft1, SIGNAL(clicked()), this, SLOT(btnLeft1Click()));

}

//-----------------------------------------
void KumRobot::CreatebtnLeft2(int H0)
{
	btnLeft2 = new QToolButton(PW);
	btnLeft2->setObjectName(QString::fromUtf8("btnLeft"));
	//btnLeft2->setIcon(QIcon(QString::fromUtf8("1leftarrow.png")));
	btnLeft2->setIcon(QIcon(QString::fromUtf8(":/RobotLeft.png")));
	btnLeft2->setToolTip(trUtf8("Слева свободно?"));
	btnLeft2->setIconSize(QSize(20, 20));
	btnLeft2->move(PWWidth / 2 - BUTTON_SIZE - BUTTON_SIZE / 2 + ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	btnLeft2->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnLeft2->show();
	connect(btnLeft2, SIGNAL(clicked()), this, SLOT(btnLeft2Click()));
}

//----------------------------------------------
void KumRobot::CreatebtnRight(int H0)
{
	btnRight = new QToolButton(PW);
	//btnright->setIcon(QIcon(QString::fromUtf8(":/icons/1rightarrow.png")));
	btnRight->setIcon(QIcon(QString::fromUtf8(":/1rightarrow.png")));
	btnRight->setIconSize(QSize(20, 20));
	btnRight->setToolTip(trUtf8("Вправо"));
	btnRight->move(PWWidth / 2 + BUTTON_SIZE / 2, 2 * BUTTON_SIZE + H0);
	//btnright->move( BUTTON_SIZE/2+3 - BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnRight->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnRight->show();
	connect(btnRight, SIGNAL(clicked()), this, SLOT(btnRightClick()));
}

//----------------------------------------------
void KumRobot::CreatebtnRight1(int H0)
{
	btnRight1 = new QToolButton(PW);
	//btnright->setIcon(QIcon(QString::fromUtf8(":/icons/1rightarrow.png")));
	btnRight1->setIcon(QIcon(QString::fromUtf8(":/RobotRightWall.png")));
	btnRight1->setIconSize(QSize(20, 20));
	btnRight1->setToolTip(trUtf8("Справа стена?"));
	btnRight1->move(PWWidth / 2 + BUTTON_SIZE / 2 - ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	//btnright->move( BUTTON_SIZE/2+3 - BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnRight1->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnRight1->show();
	connect(btnRight1, SIGNAL(clicked()), this, SLOT(btnRight1Click()));
}
//-------------------------------------------------------
void KumRobot::CreatebtnRight2(int H0)
{
	btnRight2 = new QToolButton(PW);
	//btnright->setIcon(QIcon(QString::fromUtf8(":/icons/1rightarrow.png")));
	btnRight2->setIcon(QIcon(QString::fromUtf8(":/RobotRight.png")));
	btnRight2->setIconSize(QSize(20, 20));
	btnRight2->setToolTip(trUtf8("Справа свободно?"));
	btnRight2->move(PWWidth / 2 + BUTTON_SIZE / 2 + ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	//btnright->move( BUTTON_SIZE/2+3 - BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnRight2->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnRight2->show();
	connect(btnRight2, SIGNAL(clicked()), this, SLOT(btnRight2Click()));
}


//---------------------------------------------------
void KumRobot::CreatebtnUp(int H0)
{
	btnUp = new QToolButton(PW);
	btnUp->setIcon(QIcon(QString::fromUtf8(":/1uparrow.png")));
	btnUp->setToolTip(trUtf8("Вверх"));
	btnUp->setIconSize(QSize(20, 20));
	btnUp->move(PWWidth / 2  - BUTTON_SIZE / 2, BUTTON_SIZE + H0);
	btnUp->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnUp->show();
	connect(btnUp, SIGNAL(clicked()), this, SLOT(btnUpClick()));
}

//---------------------------------------------------
void KumRobot::CreatebtnUp1(int H0)
{
	btnUp1 = new QToolButton(PW);
	btnUp1->setIcon(QIcon(QString::fromUtf8(":/RobotUpWall.png")));
	btnUp1->setToolTip(trUtf8("Сверху стена?"));
	btnUp1->setIconSize(QSize(20, 20));
	btnUp1->move(PWWidth / 2  - BUTTON_SIZE / 2 - ButtonSdvig, BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	btnUp1->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnUp1->show();
	connect(btnUp1, SIGNAL(clicked()), this, SLOT(btnUp1Click()));
}
//---------------------------------------------------
void KumRobot::CreatebtnUp2(int H0)
{
	btnUp2 = new QToolButton(PW);
	btnUp2->setIcon(QIcon(QString::fromUtf8(":/RobotUp.png")));
	btnUp2->setToolTip(trUtf8("Сверху свободно?"));
	btnUp2->setIconSize(QSize(20, 20));
	btnUp2->move(PWWidth / 2  - BUTTON_SIZE / 2 + ButtonSdvig, BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	btnUp2->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnUp2->show();
	connect(btnUp2, SIGNAL(clicked()), this, SLOT(btnUp2Click()));
}

//----------------------------------------------------
void KumRobot::CreatebtnDDown(int H0)
{
	btnDDown = new QToolButton(PW);
	btnDDown->setIcon(QIcon(QString::fromUtf8(":/1downarrow.png")));
	btnDDown->setToolTip(trUtf8("Вниз"));
	btnDDown->setIconSize(QSize(20, 20));
	btnDDown->move(PWWidth / 2  - BUTTON_SIZE / 2, 3 * BUTTON_SIZE + H0);
	btnDDown->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnDDown->show();
	connect(btnDDown, SIGNAL(clicked()), this, SLOT(btnDDownClick()));

	/*
	    btnDDown = new QToolButton(PW);
	    btnDDown->setIcon(QIcon(QString::fromUtf8("1downarrow.png")));
	    btnDDown->setToolTip(trUtf8("Вниз"));
	    btnDDown->setIconSize(QSize(20, 20));
	    btnDDown->move(PWWidth/2  - BUTTON_SIZE/2 , 3*BUTTON_SIZE);
	    btnDDown->resize(BUTTON_SIZE,BUTTON_SIZE);
	    btnDDown->show();
	    btnDown->setEnabled(true);
	    connect ( btnDDown, SIGNAL(clicked()), this, SLOT(btnDDownClick()) );
	*/
}

//----------------------------------------------------
void KumRobot::CreatebtnDown1(int H0)
{
	btnDown1 = new QToolButton(PW);
	//btnDown->setIcon(QIcon(QString::fromUtf8(":/icons/1downarrow.png")));
	btnDown1->setIcon(QIcon(QString::fromUtf8(":/RobotDownWall.png")));
	btnDown1->setToolTip(trUtf8("Снизу стена?"));
	btnDown1->setIconSize(QSize(20, 20));
	btnDown1->move(PWWidth / 2  - BUTTON_SIZE / 2 - ButtonSdvig, 3 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	btnDown1->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnDown1->show();
	connect(btnDown1, SIGNAL(clicked()), this, SLOT(btnDown1Click()));
}

//----------------------------------------------------
void KumRobot::CreatebtnDown2(int H0)
{
	btnDown2 = new QToolButton(PW);
	//btnDown->setIcon(QIcon(QString::fromUtf8(":/icons/1downarrow.png")));
	btnDown2->setIcon(QIcon(QString::fromUtf8(":/RobotDown.png")));
	btnDown2->setToolTip(trUtf8("Снизу свободно?"));
	btnDown2->setIconSize(QSize(20, 20));
	btnDown2->move(PWWidth / 2  - BUTTON_SIZE / 2 + ButtonSdvig, 3 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	//btnDown->move(SizeX/2 - BUTTON_SIZE/2+3-BTNXSDVIG+BORT,3*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnDown2->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnDown2->show();
	connect(btnDown2, SIGNAL(clicked()), this, SLOT(btnDown2Click()));
}


//------------------------------------------
void KumRobot::CreatebtnCenter(int H0)
{

	btnCenter = new QToolButton(PW);

	btnCenter->setIcon(QIcon(QString::fromUtf8(":/ToColor.png")));
	btnCenter->setToolTip(trUtf8("Закрасить"));
	btnCenter->setIconSize(QSize(20, 20));
	btnCenter->move(PWWidth / 2  - BUTTON_SIZE / 2, 2 * BUTTON_SIZE + H0);
	//btnCenter->move(SizeX/2 - BUTTON_SIZE/2+3-BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnCenter->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnCenter->show();
	connect(btnCenter, SIGNAL(clicked()), this, SLOT(btnCenterClick()));
}

//-------------------------------------------------


//------------------------------------------
void KumRobot::CreatebtnCenter1(int H0)
{
	btnCenter1 = new QToolButton(PW);

	btnCenter1->setIcon(QIcon(QString::fromUtf8(":/RobotColored.png")));
	btnCenter1->setToolTip(trUtf8("Клетка закрашена?"));
	btnCenter1->setIconSize(QSize(20, 20));
	btnCenter1->move(PWWidth / 2  - BUTTON_SIZE / 2 - ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	//btnCenter->move(SizeX/2 - BUTTON_SIZE/2+3-BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnCenter1->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnCenter1->show();
	connect(btnCenter1, SIGNAL(clicked()), this, SLOT(btnCenter1Click()));
}

//------------------------------------------
void KumRobot::CreatebtnCenter2(int H0)
{
	btnCenter2 = new QToolButton(PW);
	btnCenter2->setIcon(QIcon(QString::fromUtf8(":/RobotUnColored.png")));
	btnCenter2->setToolTip(trUtf8("Клетка чистая?"));
	btnCenter2->setIconSize(QSize(20, 20));
	btnCenter2->move(PWWidth / 2  - BUTTON_SIZE / 2 + ButtonSdvig, 2 * BUTTON_SIZE + 4 * BUTTON_SIZE + H0);
	//btnCenter->move(SizeX/2 - BUTTON_SIZE/2+3-BTNXSDVIG+BORT, 2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
	btnCenter2->resize(BUTTON_SIZE, BUTTON_SIZE);
	btnCenter2->show();
	connect(btnCenter2, SIGNAL(clicked()), this, SLOT(btnCenter2Click()));
}



//------------------------------------
void KumRobot::SetLeftWall()
{
	if (m_x == 0) {
		return;
	}
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall | LEFT_WALL;
	DrawField();
}

//------------------------------------
void KumRobot::SetLeftErrorWall()
{
	if (m_x == 0) {
		return;
	}
	//qDebug("SetLeftErrorWall");
	GetFieldItem(m_x, m_y)->m_ErrorWall = GetFieldItem(m_x, m_y)->m_ErrorWall | LEFT_WALL;
	GetFieldItem(m_x - 1, m_y)->m_ErrorWall = GetFieldItem(m_x - 1, m_y)->m_ErrorWall | RIGHT_WALL;
	//qDebug("SetLeftErrorWall %i",GetFieldItem(m_x, m_y)->m_ErrorWall);
	DrawField();
}


//------------------------------------
void KumRobot::SetRightWall()
{
	if (m_x == m_Size_x - 1) {
		return;
	}
	//      QGraphicsRectItem *old_m_t;
	//    old_m_t= GetFieldItem(m_x, m_y)->m_t;
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall | RIGHT_WALL;
	//        GetFieldItem(m_x, m_y)->m_t=old_m_t;
	DrawField();
}

//------------------------------------
void KumRobot::SetRightErrorWall()
{
	if (m_x == m_Size_x - 1) {
		return;
	}
	//      QGraphicsRectItem *old_m_t;
	//    old_m_t= GetFieldItem(m_x, m_y)->m_t;
	GetFieldItem(m_x, m_y)->m_ErrorWall = GetFieldItem(m_x, m_y)->m_ErrorWall | RIGHT_WALL;
	GetFieldItem(m_x + 1, m_y)->m_ErrorWall = GetFieldItem(m_x, m_y)->m_ErrorWall | LEFT_WALL;
	//GetFieldItem(m_x, m_y)->m_t=old_m_t;
	DrawField();
}


void KumRobot::SetUpWall()
{
	if (m_y == 0) {
		return;
	}
	//      QGraphicsRectItem *old_m_t;
	//        old_m_t= GetFieldItem(m_x, m_y)->m_t;
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall | UP_WALL;
	DrawField();
	//GetFieldItem(m_x, m_y)->m_t=old_m_t;
}
//----------------------
void KumRobot::SetUpErrorWall()
{
	if (m_y == 0) {
		return;
	}
	//      QGraphicsRectItem *old_m_t;
	//        old_m_t= GetFieldItem(m_x, m_y)->m_t;
	GetFieldItem(m_x, m_y)->m_ErrorWall = GetFieldItem(m_x, m_y)->m_ErrorWall | UP_WALL;
	GetFieldItem(m_x, m_y - 1)->m_ErrorWall = GetFieldItem(m_x, m_y - 1)->m_ErrorWall | DOWN_WALL;
	DrawField();
	//GetFieldItem(m_x, m_y)->m_t=old_m_t;
}
//------------------------------------
void KumRobot::SetDownWall()
{
	if (m_y == m_Size_y - 1) {
		return;
	}
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall | DOWN_WALL;
	DrawField();
}
void KumRobot::SetDownErrorWall()
{
	if (m_y == m_Size_y - 1) {
		return;
	}
	GetFieldItem(m_x, m_y)->m_ErrorWall = GetFieldItem(m_x, m_y)->m_ErrorWall | DOWN_WALL;
	GetFieldItem(m_x, m_y + 1)->m_ErrorWall = GetFieldItem(m_x, m_y + 1)->m_ErrorWall | UP_WALL;
	DrawField();
}
//------------------------------------
void KumRobot::DeleteLeftWall()
{

	if (m_x == 0) {
		return;
	}

	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall & NOT_LEFT_WALL;
	GetFieldItem(m_x - 1, m_y)->m_Wall = GetFieldItem(m_x - 1, m_y)->m_Wall & NOT_RIGHT_WALL;
	DrawField();
}

//------------------------------------
void KumRobot::DeleteRightWall()
{
	if (m_x == m_Size_x - 1) {
		return;
	}
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall & NOT_RIGHT_WALL;
	GetFieldItem(m_x + 1, m_y)->m_Wall = GetFieldItem(m_x + 1, m_y)->m_Wall & NOT_LEFT_WALL;
	DrawField();
}
//------------------------------------------
void KumRobot::DeleteUpWall()
{
	if (m_y == 0) {
		return;
	}

	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall & NOT_UP_WALL;
	GetFieldItem(m_x, m_y - 1)->m_Wall = GetFieldItem(m_x, m_y - 1)->m_Wall & NOT_DOWN_WALL;
	DrawField();
}

//------------------------------------------------------------------------------------
void KumRobot::DeleteDownWall()
{
	if (m_y == m_Size_y - 1) {
		return;
	}
	GetFieldItem(m_x, m_y)->m_Wall = GetFieldItem(m_x, m_y)->m_Wall & NOT_DOWN_WALL;
	GetFieldItem(m_x, m_y + 1)->m_Wall = GetFieldItem(m_x, m_y + 1)->m_Wall & NOT_UP_WALL;
	DrawField();
}
//-------------------------------------------
void KumRobot::lockControls(void)
{
	DisableButtons();
	//menu->setEnabled(false);
	lockFlag = true;
};
//------------------------------------
void KumRobot::unLockControls(void)
{
	EnableButtons();
	//menu->setEnabled(true);
	lockFlag = false;
};


void KumRobot::MousePress(qreal x, qreal y, bool LeftButtonFlag)
{
	Q_UNUSED(x);
	Q_UNUSED(y);




	if (!LeftButtonFlag) {
		//qDebug("ix,iy %f %f",ix,iy);
		Print2PDF();

	}



};
//---------------------------
void KumRobot::ReversLeftWall()
{
	if (LeftWall(m_x, m_y)) {
		DeleteLeftWall();
	} else {
		SetLeftWall();
	}

}
//----------------------------------------
void KumRobot::ReversRightWall()
{


}
//----------------------------------------
void KumRobot::ReversUpWall()
{


}
//----------------------------------------
void KumRobot::ReversDownWall()
{






}
void KumRobot::ReversColor()
{


}
//----------------------------------
void KumRobot::UnColorCell(int ix, int iy)
{
	Q_UNUSED(ix);
	Q_UNUSED(iy);

}

//----------------------------------
void KumRobot::MapCell(int i, int j)
{

	qreal x0 = i * m_FieldSize + BORT;
	qreal y0 = j * m_FieldSize + BORT;
	//      qreal Size = m_FieldSize;
	//DrawField();
	if (!GetFieldItem(i, j)->m_t) {
		GetFieldItem(i, j)->m_t = new QGraphicsRectItem(x0 + 2, y0 + 2 - BORT, 18, 18);

		//GetFieldItem(i,j)->m_t->setBrush(QBrush(QColor("blue")));
		GetFieldItem(i, j)->m_t->setBrush(QBrush(QColor(0, 250, 0)));
		//GetFieldItem(i,j)->m_t->setPen(QPen(QColor("blue")));
		GetFieldItem(i, j)->m_t->setZValue(0);

	} else {
		GetFieldItem(i, j)->m_t->setZValue(0);
		GetFieldItem(i, j)->m_t->setRect(x0 + 10, y0 - 5, 10, 10);
		GetFieldItem(i, j)->m_t->setBrush(QBrush(QColor(0, 250, 0)));
		//GetFieldItem(i,j)->m_t->setPen(QPen(QColor("blue")));

	};







}

//----------------------------------------------------
void KumRobot::CreateFieldDubl(int p_X, int p_Y)
{
	Q_UNUSED(p_X);
	Q_UNUSED(p_Y);
	//uint i,j;

	/*
	if(m_Size_xDubl > 0)
	{
	    for ( i = 0; i < m_Size_xDubl; i++)
	    {
	            for (j = 0; j < m_Size_yDubl; j++)
	            {
	            delete &m_FieldDubl[i,j];
	            }
	    }
	}
	*/
	//qDebug("debug20 %i %i %i",p_X,p_Y,m_Size_xDubl);
	//if(m_Size_xDubl > 0)delete []m_FieldDubl;
	//qDebug("debug21");

	//qDebug("%i %idebug22",p_X,p_Y);


	//qDebug("debug23");
}

//-------------------------------------
void KumRobot::ToDubl()
{

	//qDebug("ToDouble");
	//  uint i,j;

	startField = field->Clone();
	DublFlag = true;
	WasEditFlag = false;

}
//-------------------------------------
void KumRobot::FromDubl()
{
	qDebug("FromDouble");

	if (!DublFlag) {
		qDebug("! Double Flag");
	}
	//  uint i,j;
	//if(field)delete field;
	//    QList<QGraphicsItem*> oldItems = field->items();
	//    for (int i=0; i<oldItems.size(); i++) {
	//        field->removeItem(oldItems[i]);
	//        delete oldItems[i];
	//    }
	delete field;
	field = startField->Clone();
	robotWindow->graphicsView->setScene(field);
	field->drawNet();
	field->drawField(FIELD_SIZE_SMALL);
	ajustWindowSize();



}
//------------------------
void KumRobot::DisableButtons()
{
	btnLeft->setDisabled(true);
	//btnLeft1->setDisabled(true);
	btnRight->setDisabled(true);
	//btnright1->setDisabled(true);
	btnUp->setDisabled(true);
	//btnUp1->setDisabled(true);
	btnDDown->setDisabled(true);
	//btnDown1->setDisabled(true);
	btnCenter->setDisabled(true);
	//btnCenter1->setDisabled(true);



}
//-------------------------------------------------
void KumRobot::EnableButtons()
{
	btnLeft->setDisabled(false);
	//btnLeft1->setDisabled(false);
	btnRight->setDisabled(false);
	//btnright1->setDisabled(false);
	btnUp->setDisabled(false);
	//btnUp1->setDisabled(false);
	btnDDown->setDisabled(false);
	//btnDown1->setDisabled(false);
	btnCenter->setDisabled(false);
	//btnCenter1->setDisabled(false);



}
//--------------------------------------------------
void KumRobot::ReverseEditFlag()
{



}
//-------------------------------------------------
//меню "Сохранить"
void KumRobot::SaveToFile1()
{

	if (CurrentFileName == "") {
		SaveToFileActivated();
	} else {
		SaveToFile(CurrentFileName);
	}

}
void KumRobot::SaveCurrent()
{
	QFileDialog dialog(robotWindow, QString::fromUtf8("Сохранить файл"), curDir, "(*.fil)");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (!dialog.exec()) {
		return;
	}

	QString RobotFile = dialog.selectedFiles().first();
	QDir dir = dialog.directory();
	curDir = dir.path();
	if (RobotFile.contains("*") || RobotFile.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return;
	}
	//QString   RobotFile =  QFileDialog::getSaveFileName(MV,QString::fromUtf8 ("Сохранить в файл"),"/home", "(*.fil)");
	//if ( RobotFile.isEmpty())return;

	if (RobotFile.right(4) != ".fil") {
		RobotFile += ".fil";
	}
	CurrentFileName = RobotFile;

	SaveToFile(RobotFile);
};
//-----------------------------------------
void KumRobot::CreateButtons(int H0) // MR Height position of the Panel
{

	CreatebtnLeft(H0);
	CreatebtnRight(H0);
	CreatebtnUp(H0);
	CreatebtnDDown(H0);
	CreatebtnCenter(H0);

	CreatebtnLeft1(H0);
	CreatebtnRight1(H0);
	CreatebtnUp1(H0);
	CreatebtnDown1(H0);
	CreatebtnCenter1(H0);

	CreatebtnLeft2(H0);
	CreatebtnRight2(H0);
	CreatebtnUp2(H0);
	CreatebtnDown2(H0);
	CreatebtnCenter2(H0);



}

//-----------------------------------------
void KumRobot::DeleteButtons()
{

}

void KumRobot::DeleteEdits()
{
	delete eTemp;
	delete eRad;
	delete eBukv;
}


void KumRobot::ShowTemperature()
{
	qreal Temp = GetTemperature(Mouseix, Mouseiy);
	char ctmp[200];
	QString tmp;

	MR3 = QString::fromUtf8("........");  // 8
	MR1 = MSnizu + MSvobodno + MR3 + MRazd + MDa;
	MR2 = MSnizu + MSvobodno + MR3 + MRazd + MNet;
	sprintf(ctmp, "%7.2f", Temp);
	tmp = QString(ctmp);
	eTemp->setText(tmp);
	//qDebug("temp %f",Temp);
}

void KumRobot::ShowTemperature1()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	};
	lOtkaz->setText(MOK);
	qreal Temp = field->currentCell()->temperature;
	field->robot->setCrash(NO_CRASH);
	char ctmp[200];
	QString tmp;
	// MR3 = QString::fromUtf8("............");  // 12  Linux
	MR3 = QString::fromUtf8(".........");  // 9 Windows
	//if (LeftWall(m_x,m_y))lAction->setText(QString::fromUtf8("<b>СЛЕВА СТЕНА    </b>  .  <b><i>ДА       </i></b>"));
	sprintf(ctmp, "%7.2f", Temp);
	tmp = QString(ctmp);
	MR1 = MTemp + MR3 + MRazd + tmp;
	lAction->setText(MR1);
	//    Logger->appendText(QString::fromUtf8("вывод \'Температура:\',температура,нс"),QString::fromUtf8("температура  "),tmp);
	roboPult->setCommandResult(tmp);
	emit pultCommand(QString::fromUtf8("вывод \'Температура:\',температура,нс"));
}
//------------------------------
void KumRobot::ShowBukva()
{
	QString tmp = QString(GetBukva(Mouseix, Mouseiy));

	if (tmp == "$") {
		tmp = "_";
	}
	eBukv->setText(tmp);

}
//------------------------------
void KumRobot::ShowBukva1()
{
	QString tmp = QString(GetBukva1(Mouseix, Mouseiy));

	if (tmp == "$") {
		tmp = "_";
	}
	eBukv1->setText(tmp);

}

//-----------------------------
void KumRobot::ShowRadiation()
{

	qreal Rad = field->currentCell()->radiation;
	char ctmp[200];
	QString tmp;


	sprintf(ctmp, "%7.2f", Rad);
	tmp = QString(ctmp);
	eRad->setText(tmp);



}
//--------------------------------
void KumRobot::ShowRadiation1()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	qreal Rad = field->currentCell()->radiation;;
	char ctmp[200];
	QString tmp;
	field->robot->setCrash(NO_CRASH);
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("..................");  // 18   Linux
	MR3 = QString::fromUtf8("............");  // 12   Windows

	// sprintf(ctmp,"%7.2f",Rad);
	sprintf(ctmp, "%7.2f", Rad);
	tmp = QString(ctmp);

	MR1 = MRad + MR3 + MRazd + tmp;
	lAction->setText(MR1);
	//    Logger->appendText(QString::fromUtf8("вывод \'� адиация:\',радиация,нс"),QString::fromUtf8("� адиация  "),tmp);
	roboPult->setCommandResult(tmp);
	emit pultCommand(QString::fromUtf8("вывод \'� адиация:\',радиация,нс"));
}
//
//---------------------------------------------------
bool KumRobot::SetEditTemperature()
{

	QString tmp = eTemp->displayText();


	bool ok;
	qreal Temp = tmp.toFloat(&ok);
	if (!ok) {
		tmp = QString::fromUtf8("Температура - не число!");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return false;
	}

	if (Temp <  -273 || Temp >  233) {
		tmp = QString::fromUtf8("Температура должна находиться в пределах от -273 до 233 °С !");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return false;
	}
	SetTemperature(Mouseix, Mouseiy, Temp);
	return true;
}
//------------------------------------------
bool KumRobot::SetEditRadiation()
{

	QString tmp = eRad->displayText();
	bool ok;
	qreal Rad = tmp.toFloat(&ok);

	if (!ok) {
		tmp = QString::fromUtf8("� адиация - не число!");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return false;
	}

	if (Rad >= 1000000) {
		tmp = QString::fromUtf8("� адиация должна быть меньше 1000000!");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return false;
	}

	if (Rad < 0) {
		tmp = QString::fromUtf8("� адиация не может быть отрицательной!");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return false;
	}
	SetRadiation(Mouseix, Mouseiy, Rad);
	return true;

}
//-------------------------------|
bool KumRobot::SetEditBukva()
{

	QString tmp = eBukv->displayText();
	//uint tt = tmp.toInt();
	QChar Bukv = tmp[0];

	SetBukva(Mouseix, Mouseiy, Bukv);
	return true;

}

//-------------------------------
bool KumRobot::SetEditBukva1()
{

	QString tmp = eBukv1->displayText();
	//uint tt = tmp.toInt();
	QChar Bukv1 = tmp[0];

	SetBukva1(Mouseix, Mouseiy, Bukv1);
	return true;

}
//-----------------------------------------------
void KumRobot::btnOKClick()
{
	if (!SetEditTemperature()) {
		return;
	}
	if (!SetEditRadiation()) {
		return;
	}
	if (!SetEditBukva()) {
		return;
	}
	if (!SetEditBukva1()) {
		return;
	}
	if (btnRobot->checkState() == Qt::Checked) {
		btnRobotClick();
	}
	if (tmpm_x >= 0) {
		//qDebug("tmpx,tmpy %i %i",tmpm_x,tmpm_y);
		m_x = tmpm_x;
		m_y = tmpm_y;
		RepaintRobot();
	}
	bool tochka;
	if (btnMetka->checkState() == Qt::Checked) {
		tochka = true;
	} else {
		tochka = false;
	}
	{
		tmpm_x = Mouseix;
		tmpm_y = Mouseiy;

		//if(GetFieldItem(tmpm_x,tmpm_y)->Point)
		GetFieldItem(tmpm_x, tmpm_y)->Point = tochka;
		//else
		//GetFieldItem(tmpm_x,tmpm_y)->Point = true;
		//qDebug("Point %i ",GetFieldItem(tmpm_x,tmpm_y)->Point);
		//DrawField();
	}
	//очищаем сцену

	btnCancelClick();
	MapCell(Mouseix, Mouseiy);
	WasEditFlag = true;
	//MV->setWindowTitle ( QString::fromUtf8("Робот - Временная обстановка"));
}
//----------------------------------------
void KumRobot::btnCancelClick()
{
	tmpm_x = -1;
	tmpm_y = -1;
	DrawField();
	InputWindow->close();
}
//-------------------------------------------
void KumRobot::btnRobotClick()
{
	tmpm_x = Mouseix;
	tmpm_y = Mouseiy;
	if (tmpm_x >= 0) {
		m_x = tmpm_x;
		m_y = tmpm_y;
		RepaintRobot();
	}

	//btnCancelClick();
	//MapCell(Mouseix,Mouseiy);
	WasEditFlag = true;
	//MV->setWindowTitle ( QString::fromUtf8("Робот - Временная обстановка"));
}

void KumRobot::NewRobotField()
{
	if (WasEditFlag) {
		if (QMessageBox::question(robotWindow, "", QString::fromUtf8("Сохранить изменения?"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {
			//ToDubl();
			if (CurrentFileName == "") {
				SaveToFileActivated();
			} else {
				SaveToFile(CurrentFileName);
			}
			WasEditFlag = false;
		}

	}
	//    NewWindow->resize(200,120);
	QPoint pos = robotWindow->pos();
	pos.setX(pos.x() + robotWindow->width() / 2 - 100);
	pos.setY(pos.y() + robotWindow->height() / 2 - 40);
	NewWindow->move(pos);
	//    NewWindow->setMaximumSize(160,80);
	//    eXSizeEdit = new QSpinBox(NewWindow);
	//    eXSizeEdit->setValue(m_Size_x);
	//    eXSizeEdit->setRange ( MIN_SIZE, MAX_SIZE );
	//    eTemp->show();
	//    eYSizeEdit = new QSpinBox(NewWindow);
	//    eXSizeEdit->setGeometry( 90, 32, 60,20 );
	//
	//    eYSizeEdit->setGeometry( 20, 32, 60,20 );
	//    eYSizeEdit->setValue(m_Size_y);
	//    eYSizeEdit->setRange ( MIN_SIZE, MAX_SIZE );
	//
	//    QToolButton *btnOK1 = new QToolButton(NewWindow);
	//    btnOK1->setText("OK");
	//    btnOK1->setToolTip("OK");
	//    btnOK1->move(20,55 );
	//    btnOK1->resize(50,20);
	//    //btnOK1->show();
	//    connect ( btnOK1, SIGNAL(clicked()), this, SLOT(btnOK1Click()) );
	//    QToolButton *btnCancel1 = new QToolButton(NewWindow);
	//    btnCancel1->setText(trUtf8("Отмена"));
	//    btnCancel1->move(90,55 );
	//    btnCancel1->resize(50,20);
	//    connect ( btnCancel1, SIGNAL(clicked()), this, SLOT(btnCancel1Click()) );

	NewWindow->show();
	field->destroyField();
	field->createField(5, 7);
	field->drawField(30);
}
//------------------------------------
void KumRobot::btnOK1Click()
{

	int tmpx = eXSizeEdit->value();

	int tmpy =  eYSizeEdit->value();


	if (tmpx > MAX_SIZE || tmpy > MAX_SIZE) {
		QMessageBox::information(0, "", QString::fromUtf8("Слишком большое поле"), 0, 0, 0);
		return;
	}
	if (tmpx < MIN_SIZE || tmpy < MIN_SIZE) {
		QMessageBox::information(0, "", QString::fromUtf8("Слишком маленькое поле"), 0, 0, 0);
		return;
	}
	m_Size_x = tmpx;
	m_Size_y = tmpy;
	//qDebug("debug1");



	m_x = 0;
	m_y = 0;


	//delete btnFind;
	//CreatebtnFind();

	NewWindow->close();

	//MV->setWindowTitle ( QString::fromUtf8("Робот - Начало"));
	WasEditFlag = false;
	CurrentFileName = "";
	if (isEditor) {
		ToDubl();
	}
}
//----------------------
void KumRobot::CreateNewField()
{


	WasEditFlag = false;
}
//----------------------
void KumRobot::btnCancel1Click()
{
	//qDebug("btnCancel");
	NewWindow->close();
}
//------------------------------------
void KumRobot::FindRobot()
{
	//SetRobotMode(RUN_MODE);

	//if (m_Size_x <= DEFAULT_X && m_Size_y <= DEFAULT_Y)return;

	//qreal SceneX0 = m_x*FIELD_SIZE_SMALL;
	//qreal SceneY0 = m_y*FIELD_SIZE_SMALL;
	//if (m_x >=  m_Size_x-DEFAULT_X)SceneX0 =(m_Size_x - DEFAULT_X/2+1)*FIELD_SIZE_SMALL;
	//if (m_y >=  m_Size_y-DEFAULT_Y)SceneY0 =(m_Size_y - DEFAULT_Y/2+1)*FIELD_SIZE_SMALL;

	//if (m_x <= DEFAULT_X-2) SceneX0 = DEFAULT_X*FIELD_SIZE_SMALL/2;
	//if (m_y <= DEFAULT_Y-2) SceneY0 = DEFAULT_Y*FIELD_SIZE_SMALL/2;
	//qDebug("FindRobot");


}
//-------------------------------------
bool KumRobot::EmptyCell(int ix, int iy)
{

	if (!(GetFieldItem(ix, iy)->m_Wall == 0)) {
		return false;
	}
	if (GetFieldItem(ix, iy)->m_Is_Colored == true) {
		return false;
	}
	if (!(GetFieldItem(ix, iy)->m_Radiation == 0)) {
		return false;
	}
	if (!(GetFieldItem(ix, iy)->m_Temperature == 0)) {
		return false;
	}

	if (!(GetFieldItem(ix, iy)->m_Text == '$')) {
		return false;
	}

	return true;
}


//-----------------------------------------------
void KumRobot::CreatePult(QWidget *parent)
{
	qDebug() << "CREATE PULT";
	QPalette PallGrey;
	PallGrey.setBrush(QPalette::Window, QBrush(QColor(140, 160, 140)));

	PW = new QMainWindow(parent, Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint);
	//,Qt::Tool);
	PW->setGeometry(m_Size_x * FIELD_SIZE_SMALL + 70, 10, PWWidth, PWHieght + 20);
	// PW->setPalette (PallGrey);
	PW->setWindowTitle(QString::fromUtf8("Пульт"));
	PW->setWindowFlags(Qt::Tool);
	//RoboPult Pult=new RoboPult(PW);


	//qDebug()<<Logger->geometry();
	//Logger->show();
	//loggerButton* downButton;


	PW->hide();
	//MR-02 07.17
	int Height0 = 60;
	int Height1 = 200;
	//int Height2 = 60;
	int TopMargin = 180;
	int PanelInterval = 3;
	//int BottomMargin = 3;
	int HStart0, HStart1, HStart2;
	//MR-02 End

	QRect rrr = QRect(0, 0, 0, 0);

	PallRed.setColor(QPalette::BrightText, QColor(100, 0, 0)); //???????????????????
	PallRed.setBrush(QPalette::WindowText, QBrush(QColor(0, 0, 200)));
	PalErr.setColor(QPalette::BrightText, QColor(200, 10, 20)); //???????????????????
	PalErr.setBrush(QPalette::WindowText, QBrush(QColor(200, 10, 20)));

	//MR-07-17-01
	HStart0 = TopMargin;    //MR-03
	QFrame *Separator0 =  new QFrame(PW);
	Separator0->setLineWidth(2);
	QRect rrr0 = QRect(0, 0, 0, 0);
	Separator0->setFrameRect(rrr0);
	// Separator0->setGeometry(5,2,PWWidth-10,200);
	Separator0->setGeometry(5, HStart0, PWWidth - 10, Height0); //MR-05
	HStart1 = HStart0 + Height0 + PanelInterval; //MR-06
	// Separator0->setFrameShape(QFrame::Panel) ;
	Separator0->setFrameShape(QFrame::Box) ;
	// Separator0->setFrameShadow(QFrame::Raised);
	Separator0->setFrameShadow(QFrame::Sunken);
	Separator0->show();
	/*MR End*/

	// MR Define 1st Panel
	QFrame *Separator1 =  new QFrame(PW);
	Separator1->setLineWidth(2);
	//QRect rrr = QRect(0,0,0,0);
	Separator1->setFrameRect(rrr);
	//  Separator1->setGeometry(5,2,PWWidth-10,200);
	Separator1->setGeometry(5, HStart1, PWWidth - 10, Height1); //MR-07
	HStart2 = HStart1 + Height1 + PanelInterval; //MR-08
	Separator1->setFrameShape(QFrame::Panel) ;
	Separator1->setFrameShadow(QFrame::Raised);
	Separator1->show();

	SetPultTexts();
	CreateButtons(HStart1);

	// PallRed.setColor(QPalette::BrightText,QColor(100,0,0));//???????????????????
	// PallRed.setBrush(QPalette::WindowText,QBrush(QColor(0,0,200)));
	// PalErr.setColor(QPalette::BrightText,QColor(200,10,20));//???????????????????
	// PalErr.setBrush(QPalette::WindowText,QBrush(QColor(200,10,20)));
	/*MR-21
	    QLabel *lRob = new QLabel(QString::fromUtf8("_____________________________"),PW,0);
	    lRob->setGeometry( QRect(8,3 * BUTTON_SIZE+22, 184, 16 ) );
	    //lRob->setPalette(PallRed);
	    //lRob->setPalette(PallRed);
	    lRob->show();
	*/ //MR-21 End

	// //   QLabel *lOtkaz = new QLabel(QString::fromUtf8("<p align=center><b>ОТКАЗ</b></p>"),PW,0);
	//  QLabel *lOtkaz = new QLabel(QString::fromUtf8(""),PW,0);
	// //   lOtkaz->setGeometry( QRect(8,3 * BUTTON_SIZE+22, 184, 16 ) );
	//  lOtkaz->setGeometry( QRect(13,HStart0+3, 184, 16 ) );
	//  lOtkaz->setPalette(PalErr);
	//  //lRob->setPalette(PallRed);
	//  lOtkaz->show();
	/*MR-41
	//  QLabel *lAction = new QLabel(QString::fromUtf8("<b>КЛЕТКА ЗАК� АШЕНА</b>  .  <b><i>999999.99</i></b>"),PW,0);
	    QLabel *lAction = new QLabel(QString::fromUtf8(""),PW,0);
	//  lOtkaz->setGeometry( QRect(8,3 * BUTTON_SIZE+22, 184, 16 ) );
	//  lAction->setGeometry( QRect(13,HStart0+22, 184, 16 ) );
	    lAction->setGeometry( QRect(13,HStart0+22, 184, 20 ) );
	    lAction->setPalette(PallRed);
	    //lRob->setPalette(PallRed);
	    lAction->show();
	*/
	//lColored = new QLabel(QString::fromUtf8("Закрашено?"),PW,0);
	//lColored->setGeometry( QRect(30,8 * BUTTON_SIZE, 70, 20 ) );
	//lColored->show();

	//lUnColored = new QLabel(QString::fromUtf8("Стена?"),PW,0);
	//lUnColored->setGeometry( QRect(120,8 * BUTTON_SIZE, 70, 20 ) );
	//lUnColored->show();
	/* MR-31
	    lColored1 = new QLabel(QString::fromUtf8(""),PW,0);
	    lColored1->setGeometry( QRect(20,9 * BUTTON_SIZE, 170, 20 ) );
	    lColored1->setPalette(PallRed);
	    lColored1->show();
	*/
	//lUnColored1 = new QLabel(QString::fromUtf8(""),PW,0);
	//lUnColored1->setGeometry( QRect(125,9 * BUTTON_SIZE, 70, 20 ) );
	//lUnColored1->setPalette(PallRed);
	//lUnColored1->show();

	// MR Kill Bottom Panel
	// QFrame *Separator2 =  new QFrame(PW);
	// Separator2->setLineWidth(2);
	// Separator2->setFrameRect(rrr);
	// // Separator2->setGeometry(5,205,PWWidth-10,60);
	// Separator2->setGeometry(5,HStart2,PWWidth-10,Height2);  //MR-09
	//
	// Separator2->setFrameShape(QFrame::Panel) ;
	// Separator2->setFrameShadow(QFrame::Raised);
	// Separator2->show();

	int D0 = 20; //MR-11
	// int DH = Height0; //MR-11a
	int DH = D0 + TopMargin; //MR-11a
	QToolButton *btnTemp = new QToolButton(PW);
	btnTemp->setText(QString::fromUtf8("Температура?"));
	//btnOK->setToolTip(trUtf8("Температура"));
	//  btnTemp->move(9,215);

	btnTemp->move(9, 215 + DH); // MR-11b
	btnTemp->resize(90, 20);
	btnTemp->show();
	connect(roboPult, SIGNAL(Temp()), this, SLOT(ShowTemperature1()));

	QToolButton *btnRad = new QToolButton(PW);
	btnRad->setText(QString::fromUtf8("� адиация?"));
	//btnOK->setToolTip(trUtf8("Температура"));
	//  btnRad->move(101,215);
	btnRad->move(101, 215 + DH); //MR-11c
	btnRad->resize(90, 20);
	btnRad->show();

	connect(roboPult, SIGNAL(Rad()), this, SLOT(ShowRadiation1()));
	/*MR-32
	    lpTemp = new QLabel(QString::fromUtf8(""),PW,0);
	    lpTemp->setGeometry( QRect(30,12 * BUTTON_SIZE, 70, 20 ) );
	    lpTemp->setPalette(PallRed);
	    lpTemp->show();

	    lpRad = new QLabel(QString::fromUtf8(""),PW,0);
	    lpRad->setGeometry( QRect(120,12 * BUTTON_SIZE, 70, 20 ) );
	    lpRad->setPalette(PallRed);
	    lpRad->show();

	    lpShag = new QLabel(QString::fromUtf8(""),PW,0);
	    lpShag->setGeometry( QRect(3*BUTTON_SIZE,4 * BUTTON_SIZE, 90, 20 ) );
	    lpShag->setPalette(PallRed);
	    lpShag->show();
	*/ //MR-32-End
	//         lpShag = new QLabel(QString::fromUtf8(""),PW,0);
	// //   lpShag->setGeometry( QRect(3*BUTTON_SIZE,4 * BUTTON_SIZE, 90, 20 ) );
	//  lpShag->setGeometry( QRect(13,HStart0+22, 184, 20 ) );
	//  lpShag->setPalette(PallRed);
	//  lpShag->show();

	lAction = new QLabel(QString::fromUtf8(""), PW, 0);
	//  lpShag->setGeometry( QRect(3*BUTTON_SIZE,4 * BUTTON_SIZE, 90, 20 ) );
	lAction->setGeometry(QRect(13, HStart0 + 22, 184, 20));
	lAction->setPalette(PallRed);
	lAction->show();

	//  QLabel *lOtkaz = new QLabel(QString::fromUtf8("<p align=center><b>ОТКАЗ</b></p>"),PW,0);
	lOtkaz = new QLabel(QString::fromUtf8(""), PW, 0);
	//  lOtkaz->setGeometry( QRect(8,3 * BUTTON_SIZE+22, 184, 16 ) );
	lOtkaz->setGeometry(QRect(13, HStart0 + 3, 184, 16));
	lOtkaz->setPalette(PalErr);
	//lRob->setPalette(PallRed);
	lOtkaz->show();

	/*MR-43
	//  QLabel *lAction = new QLabel(QString::fromUtf8("<b>КЛЕТКА ЗАК� АШЕНА</b>  .  <b><i>999999.99</i></b>"),PW,0);
	    QLabel *lAction = new QLabel(QString::fromUtf8(""),PW,0);
	//  lOtkaz->setGeometry( QRect(8,3 * BUTTON_SIZE+22, 184, 16 ) );
	//  lAction->setGeometry( QRect(13,HStart0+22, 184, 16 ) );
	    lAction->setGeometry( QRect(13,HStart0+22, 184, 20 ) );
	    lAction->setPalette(PallRed);
	    //lRob->setPalette(PallRed);
	    lAction->show();
	*/
}
//-------------------------------
void KumRobot::ClearLabels()
{
	/* MR-33
	    lColored1->setText(QString::fromUtf8(""));
	    //lUnColored1->setText(QString::fromUtf8(""));
	    lpTemp->setText(QString::fromUtf8(""));
	    lpRad->setText(QString::fromUtf8(""));
	*/  //MR-33-End
}
//MR-07-20 ========================================
void KumRobot::SetPultTexts()
{

	MB7 = QString::fromUtf8("<pre> </pre>");
	//MB11 = QString::fromUtf8("...........");
	MRazd = QString::fromUtf8("<small> | </small>");

	MSleva = QString::fromUtf8("<b>СЛЕВА </b>");
	MSPrava = QString::fromUtf8("<b>СПРАВА</b>");
	MSverhu = QString::fromUtf8("<b>СВЕРХУ </b>");
	MSnizu = QString::fromUtf8("<b>СНИЗУ </b>");

	MStena = QString::fromUtf8("<b>СТЕНА </b>");
	MSvobodno = QString::fromUtf8("<b>СВОБОДНО </b>");

	MKletka = QString::fromUtf8("<b>КЛЕТКА </b>");

	MZakr = QString::fromUtf8("<b>ЗАКААШЕНА</b>");
	MChist = QString::fromUtf8("<b>ЧИСТАЯ </b>");

	MTemp = QString::fromUtf8("<b>ТЕМПЕРАТУРА </b>");
	MRad = QString::fromUtf8("<b>РАДИАЦИЯ</b>");
	MDa = QString::fromUtf8("<b><i>ДА </i></b>");
	MNet = QString::fromUtf8("<b><i>НЕТ </i></b>");

	MOK = QString::fromUtf8("");
}
//-------------------------
void KumRobot::btnLeft1Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->canLeft() ?
		QString::fromUtf8("Нет") :
		QString::fromUtf8("Да"));
	//    if (!field->currentCell()->canLeft())
	//        Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                           trUtf8("слева стена"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                               trUtf8("слева стена"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"));
}
//---------------------------------
void KumRobot::btnRight1Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	field->robot->setCrash(NO_CRASH);
	// ClearLabels() ##MR-07-14
	MR3 = QString::fromUtf8("..........");  // 10
	MR1 = MSPrava + MStena + MR3 + MRazd + MDa;
	MR2 = MSPrava + MStena + MR3 + MRazd + MNet;

	roboPult->setCommandResult(field->currentCell()->canRight() ?
		QString::fromUtf8("Нет") :
		QString::fromUtf8("Да"));
	//    if (!field->currentCell()->canLeft())
	//        Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                           trUtf8("слева стена"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                               trUtf8("слева стена"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Справа стена: \',справа стена,нс"));
}
//-------------------------------------
void KumRobot::btnUp1Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("..........");  // 10
	MR1 = MSverhu + MStena + MR3 + MRazd + MDa;
	MR2 = MSverhu + MStena + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	// ClearLabels() ##MR-07-14;
	roboPult->setCommandResult(field->currentCell()->canUp() ?
		QString::fromUtf8("Нет") :
		QString::fromUtf8("Да"));
	//    if (!field->currentCell()->canLeft())
	//        Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                           trUtf8("слева стена"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                               trUtf8("слева стена"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Сверху стена: \',сверху стена,нс"));
}
//------------------------------------------
void KumRobot::btnDown1Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("............");  // 12
	MR1 = MSnizu + MStena + MR3 + MRazd + MDa;
	MR2 = MSnizu + MStena + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	// ClearLabels() ##MR-07-14;
	roboPult->setCommandResult(field->currentCell()->canDown() ?
		QString::fromUtf8("Нет") :
		QString::fromUtf8("Да"));
	//    if (!field->currentCell()->canLeft())
	//        Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                           trUtf8("слева стена"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Слева стена: \',слева стена,нс"),
	//                               trUtf8("слева стена"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Снизу стена: \',снизу стена,нс"));

}
//------------------------------------------
void KumRobot::btnCenter1Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("......");  // 06
	MR1 = MKletka + MZakr + MR3 + MRazd + MDa;
	MR2 = MKletka + MZakr + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->isColored() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	//    if (field->currentCell()->IsColored)
	//        Logger->appendText(QString::fromUtf8("вывод \'Клетка закрашена: \',клетка закрашена,нс"),
	//                           QString::fromUtf8("клетка закрашена"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Клетка закрашена: \',клетка закрашена,нс"),
	//                               QString::fromUtf8("клетка закрашена"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Клетка закрашена: \',клетка закрашена,нс"));
}

//-------------------------
void KumRobot::btnLeft2Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("......");  // 6
	MR1 = MSleva + MSvobodno + MR3 + MRazd + MDa;
	MR2 = MSleva + MSvobodno + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->canLeft() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	//qDebug("left2")
	// ClearLabels() ##MR-07-14;
	//    if (field->currentCell()->canLeft())
	//        Logger->appendText(QString::fromUtf8("вывод \'Слева свободно: \',слева свободно,нс"),
	//                           trUtf8("слева свободно"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Слева свободно: \',слева свободно,нс"),
	//                               trUtf8("слева свободно"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Слева свободно: \',слева свободно,нс"));
}
//---------------------------------
void KumRobot::btnRight2Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("....");  // 4
	MR1 = MSPrava + MSvobodno + MR3 + MRazd + MDa;
	MR2 = MSPrava + MSvobodno + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->canRight() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	// ClearLabels() ##MR-07-14;
	//    if (field->currentCell()->canRight())
	//        Logger->appendText(QString::fromUtf8("вывод \'Справа свободно: \',справа свободно,нс"),
	//                           trUtf8("справа свободно"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Справа свободно: \',справа свободно,нс"),
	//                               trUtf8("справа свободно"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Справа свободно: \',справа свободно,нс"));
}
//--------------------------------
void KumRobot::btnUp2Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("....");  // 4
	MR1 = MSverhu + MSvobodno + MR3 + MRazd + MDa;
	MR2 = MSverhu + MSvobodno + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->canUp() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	// ClearLabels() ##MR-07-14;
	//    if (field->currentCell()->canUp())
	//        Logger->appendText(QString::fromUtf8("вывод \'Сверху свободно: \',сверху свободно,нс"),
	//                           trUtf8("сверху свободно"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Сверху свободно: \',сверху свободно,нс"),
	//                               trUtf8("сверху свободно"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Сверху свободно: \',сверху свободно,нс"));
}
//------------------------------------------
void KumRobot::btnDown2Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8("......");  // 6
	MR1 = MSnizu + MSvobodno + MR3 + MRazd + MDa;
	MR2 = MSnizu + MSvobodno + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(field->currentCell()->canDown() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	//    if (field->currentCell()->canDown())
	//        Logger->appendText(QString::fromUtf8("вывод \'Снизу свободно: \',снизу свободно,нс"),
	//                           trUtf8("снизу свободно"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Снизу свободно: \',снизу свободно,нс"),
	//                               trUtf8("снизу свободно"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Снизу свободно: \',снизу свободно,нс"));
}
//------------------------------------------
void KumRobot::btnCenter2Click()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	MR3 = QString::fromUtf8(".......");  // 7.
	MR1 = MKletka + MChist + MR3 + MRazd + MDa;
	MR2 = MKletka + MChist + MR3 + MRazd + MNet;
	field->robot->setCrash(NO_CRASH);
	roboPult->setCommandResult(!field->currentCell()->isColored() ?
		QString::fromUtf8("Да") :
		QString::fromUtf8("Нет"));
	// ClearLabels() ##MR-07-14;
	//    if (!field->currentCell()->IsColored)
	//        Logger->appendText(QString::fromUtf8("вывод \'Клетка чистая: \',клетка чистая,нс"),
	//                           QString::fromUtf8("клетка чистая"),trUtf8("Да"));
	//    else  Logger->appendText(QString::fromUtf8("вывод \'Клетка чистая: \',клетка чистая,нс"),
	//                               QString::fromUtf8("клетка чистая"),trUtf8("Нет"));
	emit pultCommand(QString::fromUtf8("вывод \'Клетка чистая: \',клетка чистая,нс"));
}
//----------------------------------------------
void KumRobot::ReversePult()
{
	if (w_pultWindow->isVisible()) {
		w_pultWindow->raise();
	} else {
		w_pultWindow->show();
	}

}

void KumRobot::setRobotStartField(RoboField *StartField)
{
	startField = StartField;
	QString Title = QString::fromUtf8("Робот - временная");
	robotWindow->setWindowTitle(Title);
}

//---------------------------------------------
bool KumRobot::isRobotInView()
{
	return true;

}
//-----------------------------------
void KumRobot::btnLeftClick()
{
	field->setProperty("fromPult", true);
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	lAction->setText(QString::fromUtf8("<b>ВЛЕВО           </b>  .                         "));
	if (field->stepLeft()) {
		roboPult->setCommandResult(QString::fromUtf8("ОК"));
		//        Logger->appendText(trUtf8("влево"),QString::fromUtf8("влево       "),"OK");
		field->robot->setCrash(NO_CRASH);
	} else {

		field->robot->setCrash(LEFT_CRASH);
		roboPult->setCommandResult(QString::fromUtf8("Отказ"));
		//        Logger->appendText(trUtf8("влево"),QString::fromUtf8("влево     "),trUtf8("Отказ"));
	};

	emit pultCommand(trUtf8("влево"));
}

//-----------------------------------
void KumRobot::btnRightClick()
{
	field->setProperty("fromPult", true);
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	lAction->setText(QString::fromUtf8("<b>ВПРАВО          </b>  .                         "));
	if (field->stepRight()) {
		roboPult->setCommandResult(QString::fromUtf8("ОК"));
		field->robot->setCrash(NO_CRASH);
		//        Logger->appendText(trUtf8("вправо"),QString::fromUtf8("вправо     "), "OK");
	} else {
		field->robot->setCrash(RIGHT_CRASH);
		//        Logger->appendText(trUtf8("вправо"),QString::fromUtf8("вправо   "),trUtf8("Отказ"));
		roboPult->setCommandResult(QString::fromUtf8("Отказ"));
	};

	emit pultCommand(trUtf8("вправо"));
}
//-----------------------------------
void KumRobot::btnUpClick() //MRS
{
	field->setProperty("fromPult", true);
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	lAction->setText(trUtf8("<b>ВВЕ� Х           </b>  .                         "));
	if (field->stepUp()) {
		roboPult->setCommandResult(QString::fromUtf8("ОК"));
		//        Logger->appendText(trUtf8("вверх"),QString::fromUtf8("вверх    "), "OK");
		field->robot->setCrash(NO_CRASH);
	} else {
		field->robot->setCrash(UP_CRASH);
		//        Logger->appendText(trUtf8("вверх"),QString::fromUtf8("вверх     "),trUtf8("Отказ"));
		roboPult->setCommandResult(QString::fromUtf8("Отказ"));
	};

	emit pultCommand(trUtf8("вверх"));
}
//-----------------------------------
void KumRobot::btnDDownClick()
{
	field->setProperty("fromPult", true);
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	lOtkaz->setText(MOK);
	lAction->setText(trUtf8("<b>ВНИЗ           </b>  .                         "));


	if (field->stepDown()) {
		field->robot->setCrash(NO_CRASH);
		roboPult->setCommandResult(QString::fromUtf8("ОК"));
		//        Logger->appendText(trUtf8("вниз"),QString::fromUtf8("вниз     "), "OK");
	} else {
		field->robot->setCrash(DOWN_CRASH);
		roboPult->setCommandResult(QString::fromUtf8("Отказ"));
		//        Logger->appendText(trUtf8("вниз"),QString::fromUtf8("вниз     "),trUtf8("Отказ"));
	};

	emit pultCommand(trUtf8("вниз"));
	//exit(5);
}

//--------------------------------------------
void KumRobot::SaveToFileCopy()
{

	if (CurrentFileName == "") {
		SaveToFileActivated();
		WasEditFlag = false;
		return;
	}

	QString CopyFile = CurrentFileName;
	CopyFile.replace(".fil", ".copy.fil");
	SaveToFile(CopyFile);
	WasEditFlag = false;
}
//-----------------------------------------
void KumRobot::SetSceneSize(uint dx, uint dy)
{
	Q_UNUSED(dx);
	Q_UNUSED(dy);
}
//-----------------------------------
void KumRobot::btnCenterClick()
{
	if (!roboPult->isHasLink()) {
		//        Logger->appendText("",QString::fromUtf8("НЕТ СВЯЗИ"),"");
		return;
	}
	field->robot->setCrash(NO_CRASH);
	//    Logger->appendText(trUtf8("закрасить"),trUtf8("закрасить"), "OK");
	roboPult->setCommandResult(QString::fromUtf8("ОК"));
	if (!field->currentCell()->IsColored) {
		field->reverseColorCurrent();
	}
	emit pultCommand(trUtf8("закрасить"));
}
//-------------------------------------
void KumRobot::LoadInitialFil(QString FileName)
{
	QFileInfo fi(FileName);
	if (!fi.exists()) {
		return;
	}
	int err = LoadFromFile(FileName);

	QString name = fi.fileName();
	CurrentFileName = name;
	if (err == 0) {
		robotWindow->setWindowTitle(QString::fromUtf8("Робот - ") + name);
#ifdef Q_OS_WIN32
		//        header->setWMTitle( QString::fromUtf8("Робот - ")+name);
		w_window->setWindowTitle(QString::fromUtf8("Робот - ") + name);
#endif
	};
}

//-----------------------------------------
void KumRobot::CreateMainButtons()
{

}
//-------------------------------------
void KumRobot::CreateBtnNewField()
{


}
//----------------------------------
void KumRobot::CreateBtnLoadFromFile()
{

	//BtnLoadFromFile->setEnabled(false);
}
//----------------------------------
void KumRobot::CreateBtnSaveAs()
{


}
//------------------------------
/*
void KumRobot::CreateBtnSave()
{
        BtnSave = new QToolButton(MV);
        BtnSave->setIcon(QIcon(QString::fromUtf8("document-save.png")));
        //btnOK->setIconSize(QSize(40, 20));
        BtnSave->setText(QString::fromUtf8(""));
        BtnSave->setToolTip(trUtf8("Сохранить "));
        BtnSave->move(3+3*BUTTON_SIZE,MainButtonY );
        BtnSave->resize(BUTTON_SIZE,BUTTON_SIZE);
        BtnSave->show();
        connect ( BtnSave, SIGNAL(clicked()), this, SLOT(SaveToFile1()) );
}
*/
//---------------------------------

void KumRobot::CreateBtnEdit()
{


}
//--------------------------------
void KumRobot::CreateBtnPult()
{

}
//--------------------------------
/*
void KumRobot::CreateBtnSaveStart()
{
        BtnSaveStart = new QToolButton(MV);
        BtnSaveStart->setIcon(QIcon(QString::fromUtf8("media-seek-forward.png")));
        //btnOK->setIconSize(QSize(40, 20));
        BtnSaveStart->setText(QString::fromUtf8(""));
        BtnSaveStart->setToolTip(trUtf8("Запомнить как стартовую"));
        BtnSaveStart->move(3+6*BUTTON_SIZE,MainButtonY );
        BtnSaveStart->resize(BUTTON_SIZE,BUTTON_SIZE);
        BtnSaveStart->show();
        connect ( BtnSaveStart, SIGNAL(clicked()), this, SLOT(ToDubl()) );
}

//---------------------------------
void KumRobot::CreateBtnLoadStart()
{
        BtnLoadStart = new QToolButton(MV);
        BtnLoadStart->setIcon(QIcon(QString::fromUtf8("media-seek-backward")));
        //btnOK->setIconSize(QSize(40, 20));
        BtnLoadStart->setText(QString::fromUtf8(""));
        BtnLoadStart->setToolTip(trUtf8("Восстановить стартовую"));
        BtnLoadStart->move(3+7*BUTTON_SIZE,MainButtonY );
        BtnLoadStart->resize(BUTTON_SIZE,BUTTON_SIZE);
        BtnLoadStart->show();
        connect ( BtnLoadStart, SIGNAL(clicked()), this, SLOT(FromDubl()) );

}
*/
/*
//------------------------------------------------------------------------------
void KumRobot::CreatebtnFind()
{
        btnFind = new QToolButton(MV);
        btnFind->setIcon(QIcon(QString::fromUtf8("edit-find.png")));
        btnFind->setToolTip(trUtf8("Найти робота"));
        btnFind->setIconSize(QSize(20, 20));
        btnFind->move(3 +5*BUTTON_SIZE ,MainButtonY);
//qDebug("Sizex,Sizey %f %f",SizeX,SizeY);
        //btnFind->move(SizeX/2 - BUTTON_SIZE/2+3-BTNXSDVIG+BORT,SizeY+30);
         //SizeY+2*BUTTON_SIZE+BORT+BORT+BORT+BORT+MenuHigth);
        btnFind->resize(BUTTON_SIZE,BUTTON_SIZE);
        btnFind->show();
        connect ( btnFind, SIGNAL(clicked()), this, SLOT(FindRobot()) );
}
*/
//------------------------------------------
void KumRobot::SetRobotMode(int mode)
{

	QFileInfo fi(CurrentFileName);

	switch (mode) {
	case SEE_MODE:
		if (WasEditFlag) {
			qDebug("SEE_MODE-WAS_EDIT");
			if (QMessageBox::question(0, "", QString::fromUtf8("Сохранить изменения"), QMessageBox::Yes, QMessageBox::No, 0) == QMessageBox::Yes) {

				ToDubl();
				if (CurrentFileName == "") {
					SaveToFileActivated();
				} else {
					SaveToFile(CurrentFileName);
				}
				WasEditFlag = false;
				app()->settings->setValue("Robot/StartField/File", CurrentFileName);
			} else {
				PathToBaseFil = app()->settings->value("Robot/StartField/File", "").toString();
				LoadFromFile(PathToBaseFil);
				ToDubl();
			};

		}
		qDebug("SEE_MODE");
		//        roboPult->LinkOK();
		roboPult->setHasLink(true);
		CurrentRobotMode = SEE_MODE;
		//fi=QFileInfo(CurrentFileName);

		break;

	case RUN_MODE:

		if (isEditor) {
			ReverseEditFlag();
		}
		qDebug("ROBO_RUN_MODE");
		//DrawField();
		//        roboPult->noLink();
		roboPult->setHasLink(false);
		CurrentRobotMode = RUN_MODE;
		break;

	case PAUSE_MODE:

		CurrentRobotMode = PAUSE_MODE;
		//        roboPult->LinkOK();
		roboPult->setHasLink(false);
		break;

	case EDIT_MODE:

		BtnSaveAs->setToolTip(trUtf8("Сохранить как стартовую..."));

		CurrentRobotMode = EDIT_MODE;
		break;


	case ANALYZE_MODE:

		if (isEditor) {
			ReverseEditFlag();
		}
		//        roboPult->LinkOK();
		roboPult->setHasLink(true);
		CurrentRobotMode = ANALYZE_MODE;
		qDebug("ROBO_ANALIZE_MODE");
		break;

	};

}
//---------------------------------------------

void KumRobot::DisableMainButtons()
{

}
//--------------------
void KumRobot::PrepareRunMode()
{


	FromDubl();

}

void KumRobot::restoreRobotStartField()
{
	qDebug() << "Restore start file";
	if (startField->loadFromFile(app()->settings->value("Robot/StartField/File", "").toString()) != 0) {
		qDebug() << "Cant load start field:" + app()->settings->value("Robot/StartField/File", "").toString();
		return;
	};
	CurrentFileName = app()->settings->value("Robot/StartField/File", "").toString();
	QFileInfo fi(CurrentFileName);
	QString name = fi.fileName();

	QString Title = QString::fromUtf8("Робот - ") + name;

	robotWindow->setWindowTitle(Title);
#ifdef Q_OS_WIN32
	//    header->setWMTitle(Title);
	w_window->setWindowTitle(Title);
#endif
};
void KumRobot::Print2PDF()
{
	pDialog->view = robotWindow->graphicsView;
	pDialog->field = field;

	pDialog->setRoboMode(true);

	pDialog->setFieldSize(field->columns(), field->rows());
	pDialog->setSizes(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4 + 2
		, FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL + FIELD_SIZE_SMALL / 4);
	if (pDialog->exec() != 1) {
		return;
	}
	return;
	QFileDialog dialog(robotWindow, QString::fromUtf8("Печатать в файл"), curDir, "(*.pdf)");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (!dialog.exec()) {
		return ;
	}

	QString PrintFileName = dialog.selectedFiles().first();
	QDir dir = dialog.directory();
	curDir = dir.path();

	if (PrintFileName.contains("*") || PrintFileName.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return ;
	}

	if (PrintFileName.right(4) != ".pdf") {
		PrintFileName += ".pdf";
	}

	QPrinter printer(QPrinter::HighResolution);
	printer.setPageSize(QPrinter::A4);
	printer.setOutputFileName(PrintFileName);
	printer.setFullPage(false);
	QPainter painter(&printer);



	// print, fitting the viewport contents into a full page
	field->render(&painter, QRect(-FIELD_SIZE_SMALL / 2, -FIELD_SIZE_SMALL / 2,
			(FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL) * 8,
			(FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL) * 8),
		QRect(-FIELD_SIZE_SMALL / 2, -FIELD_SIZE_SMALL / 2,
			FIELD_SIZE_SMALL * field->columns() + FIELD_SIZE_SMALL,
			FIELD_SIZE_SMALL * field->rows() + FIELD_SIZE_SMALL));
};

FieldItm::FieldItm(QWidget *parent, QGraphicsScene *scene)
{
	Q_UNUSED(parent);
	upWallLine = NULL;
	downWallLine = NULL;
	leftWallLine = NULL;
	rightWallLine = NULL;
	sepItmUp = NULL;
	sepItmDown = NULL;
	sepItmLeft = NULL;
	sepItmRight = NULL;
	Scene = scene;
	upWall = false;
	downWall = false;
	leftWall = false;
	rightWall = false;
	IsColored = false;
	mark = false;
	ColorRect = NULL;
	upChar = ' ';
	downChar = ' ';
	upCharItm = NULL;
	downCharItm = NULL;
	markItm = NULL;

	font.setPixelSize(8);
	font.setPointSize(8);
	font.setStyle(QFont::StyleNormal);
	font.setBold(true);
	font.setStyleHint(QFont::SansSerif);
	radiation = 0;
	temperature = 0;
	font.setWeight(2);
	TextColor = QColor(app()->settings->value("Robot/TextColor", "#FFFFFF").toString());
}

FieldItm::~FieldItm()
{
	cleanSelf();
}

void FieldItm::cleanSelf()
{
	if (upWallLine) {
		if (Scene) {
			Scene->removeItem(upWallLine);
		}
		if (upWallLine && upWallLine->scene() == Scene) {
			delete upWallLine;
		}
		upWallLine = NULL;
	}
	if (downWallLine) {
		if (Scene)
			if (downWallLine && downWallLine->scene() == Scene) {
				Scene->removeItem(downWallLine);
			}
		delete downWallLine;
		downWallLine = NULL;
	}
	if (leftWallLine) {
		if (Scene) {
			Scene->removeItem(leftWallLine);
		}
		if (leftWallLine && leftWallLine->scene() == Scene) {
			delete leftWallLine;
		}
		leftWallLine = NULL;
	}
	if (rightWallLine) {
		if (Scene) {
			Scene->removeItem(rightWallLine);
		}
		if (rightWallLine && rightWallLine->scene() == Scene) {
			delete rightWallLine;
		}
		rightWallLine = NULL;
	}
	if (ColorRect) {
		if (Scene) {
			Scene->removeItem(ColorRect);
		}
		delete ColorRect;
		ColorRect = NULL;
	}
	if (upCharItm) {
		if (Scene) {
			Scene->removeItem(upCharItm);
		}
		delete upCharItm;
		upCharItm = NULL;
	}
	if (downCharItm) {
		if (Scene) {
			Scene->removeItem(downCharItm);
		}
		delete downCharItm;
		downCharItm = NULL;
	}
	if (markItm) {
		if (Scene) {
			Scene->removeItem(markItm);
		}
		delete markItm;
		markItm = NULL;
	}
}

void FieldItm::setLeftsepItem(FieldItm *ItmLeft)
{
	sepItmLeft = ItmLeft;
	ItmLeft->setRightsepItem(this); //Obratnaya ssilka u soseda
}

void FieldItm::setRightsepItem(FieldItm *ItmRight)
{
	sepItmRight = ItmRight;
}

void FieldItm::setUpsepItem(FieldItm *ItmUp)
{
	sepItmUp = ItmUp;
	ItmUp->setDownsepItem(this);
}

void FieldItm::setDownsepItem(FieldItm *ItmDown)
{
	sepItmDown = ItmDown;
}

void FieldItm::setWalls(int wallByte)
{
	if ((wallByte & UP_WALL) == UP_WALL) {
		upWall = true;
	} else {
		upWall = false;
	}
	if ((wallByte & DOWN_WALL) == DOWN_WALL) {
		downWall = true;
	} else {
		downWall = false;
	}
	if ((wallByte & LEFT_WALL) == LEFT_WALL) {
		leftWall = true;
	} else {
		leftWall = false;
	}
	if ((wallByte & RIGHT_WALL) == RIGHT_WALL) {
		rightWall = true;
	} else {
		rightWall = false;
	}
}

int FieldItm::wallByte()
{
	int toret = 0;
	if (upWall) {
		toret += UP_WALL;
	}
	if (downWall) {
		toret += DOWN_WALL;
	}
	if (leftWall) {
		toret += LEFT_WALL;
	}
	if (rightWall) {
		toret += RIGHT_WALL;
	}
	return toret;
}

void FieldItm::setUpLine(QGraphicsLineItem *Line, QPen pen)
{
	upWallLine = Line;
	upWallLine->setPen(pen);
	Scene->addItem(upWallLine);
	upWallLine->setZValue(1);
}

void FieldItm::showCharMark(qreal upLeftCornerX, qreal upLeftCornerY, int size)
{
	showUpChar(upLeftCornerX, upLeftCornerY, size);
	showDownChar(upLeftCornerX, upLeftCornerY, size);
	showMark(upLeftCornerX, upLeftCornerY, size);
}

void FieldItm::showUpChar(qreal upLeftCornerX, qreal upLeftCornerY, int size)
{
	Q_UNUSED(size);
	if (upCharItm) {
		if (Scene) {
			Scene->removeItem(upCharItm);
		}
		delete upCharItm;
		upCharItm = NULL;
	}
	if (upChar.isPrint() && upChar != ' ') {
		upCharItm = Scene->addText(upChar, font);
		upCharItm->setDefaultTextColor(TextColor);
		upCharItm->setPos(upLeftCornerX + 2, upLeftCornerY + 2);
		upCharItm->setZValue(1);
	}
}

void FieldItm::showDownChar(qreal upLeftCornerX, qreal upLeftCornerY, int size)
{
	Q_UNUSED(size);
	if (downCharItm) {
		if (Scene) {
			Scene->removeItem(downCharItm);
		}
		delete downCharItm;
		downCharItm = NULL;
	}
	if (downChar.isPrint() && downChar != ' ') {
		downCharItm = Scene->addText(downChar, font);
		downCharItm->setDefaultTextColor(TextColor);
		downCharItm->setPos(upLeftCornerX + 2, upLeftCornerY + size - 16);
		downCharItm->setZValue(1);
	}
}

void FieldItm::showMark(qreal upLeftCornerX, qreal upLeftCornerY, int size)
{
	if (mark) {
		if (markItm) {
			if (Scene) {
				Scene->removeItem(markItm);
			}
			delete markItm;
			markItm = NULL;
		}
		markItm = Scene->addText(QChar(9787), font);
		markItm->setDefaultTextColor(TextColor);
		markItm->setPos(upLeftCornerX + size - (size / 3) - 3, upLeftCornerY - 15 + size);
		markItm->setZValue(1);
	}
}

void FieldItm::setColorRect(QGraphicsRectItem *Rect)
{
	ColorRect = Rect;
	ColorRect->setPen(QPen("gray"));
	ColorRect->setBrush(QBrush(QColor("gray")));

	Scene->addItem(Rect);

	Rect->setZValue(0.2);
	IsColored = true;
}

void FieldItm::setDownLine(QGraphicsLineItem *Line, QPen pen)
{
	downWallLine = Line;
	downWallLine->setPen(pen);
	Scene->addItem(downWallLine);
	downWallLine->setZValue(1);
}

void FieldItm::setLeftLine(QGraphicsLineItem *Line, QPen pen)
{
	leftWallLine = Line;
	leftWallLine->setPen(pen);
	Scene->addItem(leftWallLine);
	leftWallLine->setZValue(1);
}

void FieldItm::setRightLine(QGraphicsLineItem *Line, QPen pen)
{
	rightWallLine = Line;
	rightWallLine->setPen(pen);
	Scene->addItem(rightWallLine);

	rightWallLine->setZValue(1);
}

bool FieldItm::hasUpWall()
{
	return upWall;
}

bool FieldItm::hasDownWall()
{
	return downWall;
}
bool FieldItm::hasLeftWall()
{
	return leftWall;
}

bool FieldItm::hasRightWall()
{
	return rightWall;
}

bool FieldItm::canUp()
{
	if (!hasUpSep()) {
		return false;
	}
	return !(upWall || sepItmUp->hasDownWall());
}

bool FieldItm::canDown()
{
	if (!hasDownSep()) {
		return false;
	}
	return !(downWall || sepItmDown->hasUpWall());
}

bool FieldItm::canLeft()
{
	if (!hasLeftSep()) {
		return false;
	}
	return !(leftWall || sepItmLeft->hasRightWall());
}

bool FieldItm::canRight()
{
	if (!hasRightSep()) {
		return false;
	}
	return !(rightWall || sepItmRight->hasLeftWall());
}

bool FieldItm::hasUpSep()
{
	if (sepItmUp) {
		return true;
	}
	return false;
}

bool FieldItm::hasDownSep()
{
	if (sepItmDown) {
		return true;
	}
	return false;
}

bool FieldItm::hasLeftSep()
{
	if (sepItmLeft) {
		return true;
	}
	return false;
}

bool FieldItm::hasRightSep()
{
	if (sepItmRight) {
		return true;
	}
	return false;
}

QGraphicsLineItem *FieldItm::UpWallLine()
{
	return upWallLine;
}

QGraphicsLineItem *FieldItm::DownWallLine()
{
	return downWallLine;
}

QGraphicsLineItem *FieldItm::LeftWallLine()
{
	return leftWallLine;
}

QGraphicsLineItem *FieldItm::RightWallLine()
{
	return rightWallLine;
}

bool FieldItm::isColored()
{
	return IsColored;
}

void FieldItm::removeUpWall()
{
	if (upWallLine) {
		Scene->removeItem(upWallLine);
		delete upWallLine;
		upWallLine = NULL;
		qDebug("UwallRemoved");
	};
	upWall = false;
	if (hasUpSep())
		if (sepItmUp->hasDownWall()) {
			sepItmUp->removeDownWall();
		}
}

void FieldItm::setUpWall(QGraphicsLineItem *Line, QPen pen)
{
	upWall = true;
	setUpLine(Line, pen);
}

void FieldItm::setDownWall(QGraphicsLineItem *Line, QPen pen)
{
	downWall = true;
	setDownLine(Line, pen);
}

void FieldItm::removeDownWall()
{
	if (downWallLine) {
		Scene->removeItem(downWallLine);
		delete downWallLine;
		downWallLine = NULL;
		qDebug("DwallRemoved");
	};
	downWall = false;

	if (hasDownSep())
		if (sepItmDown->hasUpWall()) {
			sepItmDown->removeUpWall();
		}
}

void FieldItm::removeLeftWall()
{
	if (leftWallLine) {
		Scene->removeItem(leftWallLine);
		delete leftWallLine;
		leftWallLine = NULL;
		qDebug("LwallRemoved");
	};
	leftWall = false;

	if (hasLeftSep())
		if (sepItmLeft->hasRightWall()) {
			sepItmLeft->removeRightWall();
		}
}

void FieldItm::removeRightWall()
{
	if (rightWallLine) {
		QGraphicsScene *debug = rightWallLine->scene();
		Q_UNUSED(debug);
		rightWallLine->setVisible(false);
		Scene->removeItem(rightWallLine);
		delete rightWallLine;
		rightWallLine = NULL;
		qDebug("RwallRemoved");
	};
	rightWall = false;

	if (hasRightSep())
		if (sepItmRight->hasLeftWall()) {
			sepItmRight->removeLeftWall();
		}
}

void FieldItm::setLeftWall(QGraphicsLineItem *Line, QPen pen)
{
	leftWall = true;
	setLeftLine(Line, pen);
}

void FieldItm::setRightWall(QGraphicsLineItem *Line, QPen pen)
{
	rightWall = true;
	setRightLine(Line, pen);
}

void FieldItm::removeColor()
{

	if (ColorRect) {
		Scene->removeItem(ColorRect);
		delete ColorRect;
		ColorRect = NULL;
	};
	IsColored = false;
}

void FieldItm::removeMark()
{
	if (markItm) {
		Scene->removeItem(markItm);
		delete markItm;
		markItm = NULL;
	};
	mark = false;
}

void FieldItm::removeUpChar()
{
	if (upCharItm) {
		Scene->removeItem(upCharItm);
		delete upCharItm;
		upCharItm = NULL;
	};

	upChar = ' ';
}

void FieldItm::removeDownChar()
{
	if (downCharItm) {
		Scene->removeItem(downCharItm);
		delete downCharItm;
		downCharItm = NULL;
	};
	downChar = ' ';
}

bool FieldItm::emptyCell()
{
	return (!upWall && !downWall && !leftWall && !rightWall && !IsColored && !mark)
		&& (radiation == 0) && (temperature == 0) && (upChar == ' ') && (downChar == ' ');
}

void FieldItm::setScene(QGraphicsScene *scene)
{
	Scene = scene;
}

void FieldItm::wbWalls()
{
	if (UpWallLine()) {
		wallPen = UpWallLine()->pen();
		int w = 3;
		if (!hasUpSep()) {
			w = 4;
		}
		UpWallLine()->setPen(QPen(QBrush(QColor("blue")), w));
	};
	if (DownWallLine()) {
		wallPen = DownWallLine()->pen();
		int w = 3;
		if (!hasDownSep()) {
			w = 4;
		}
		DownWallLine()->setPen(QPen(QBrush(QColor("blue")), w));
	};
	if (LeftWallLine()) {
		wallPen = LeftWallLine()->pen();
		int w = 3;
		if (!hasLeftSep()) {
			w = 4;
		}
		LeftWallLine()->setPen(QPen(QBrush(QColor("blue")), w));
	};
	if (RightWallLine()) {
		wallPen = RightWallLine()->pen();
		int w = 3;
		if (!hasRightSep()) {
			w = 4;
		}
		RightWallLine()->setPen(QPen(QBrush(QColor("blue")), w));
	};

	if (downCharItm) {
		downCharItm->setDefaultTextColor("black");
	}
	if (upCharItm) {
		upCharItm->setDefaultTextColor("black");
	}
	if (markItm) {
		markItm->setDefaultTextColor("black");
	}
}

void FieldItm::colorWalls()
{
	if (UpWallLine()) {
		if (!hasUpSep()) {
			wallPen.setWidth(4);
		} else {
			wallPen.setWidth(3);
		}
		UpWallLine()->setPen(wallPen);
	};
	if (DownWallLine()) {
		if (!hasDownSep()) {
			wallPen.setWidth(4);
		} else {
			wallPen.setWidth(3);
		}
		DownWallLine()->setPen(wallPen);
	};
	if (LeftWallLine()) {
		if (!hasLeftSep()) {
			wallPen.setWidth(4);
		} else {
			wallPen.setWidth(3);
		}
		LeftWallLine()->setPen(wallPen);
	};
	if (RightWallLine()) {
		if (!hasRightSep()) {
			wallPen.setWidth(4);
		} else {
			wallPen.setWidth(3);
		}
		RightWallLine()->setPen(wallPen);
	};
	if (downCharItm) {
		downCharItm->setDefaultTextColor("white");
	}
	if (upCharItm) {
		upCharItm->setDefaultTextColor("white");
	}
	if (markItm) {
		markItm->setDefaultTextColor("white");
	}
}




FieldItm *FieldItm::Copy()
{
	FieldItm *copy = new FieldItm();
	copy->leftWall = leftWall;
	copy->rightWall = rightWall;
	copy->upWall = upWall;
	copy->downWall = downWall;

	copy->upChar = upChar;
	copy->downChar = downChar;
	copy->mark = mark;
	copy->radiation = radiation;
	copy->temperature = temperature;
	copy->IsColored = IsColored;

	return copy;
};

RoboField::~RoboField()
{
	destroyField();
	destroyRobot();
	destroyNet();
	destroyScene();
	for (int i = 0; i < Items.size(); i++) {
		for (int j = 0; j < Items[i].size(); j++) {
			if (Items[i][j]) {
				delete Items[i][j];
			}
		}
	}
	Items.clear();
}

RoboField::RoboField(QWidget *parent, KumRobot *robot_module)
	: QGraphicsScene(parent)
	, m_robot(robot_module)

{
	Parent = parent;
	editMode = false;
	LineColor = QColor(app()->settings->value("Robot/LineColor", "#C8C800").toString());
	WallColor = QColor(app()->settings->value("Robot/WallColor", "#C8C800").toString());
	EditColor = QColor(app()->settings->value("Robot/EditColor", "#00008C").toString());
	NormalColor = QColor(app()->settings->value("Robot/NormalColor", "#289628").toString());




	fieldSize = 30;
	this->setItemIndexMethod(NoIndex);
	robot = NULL;
	markMode = true;
	cellDialog = new CellDialog(Parent);
	// cellDialog=CellDialog::instance();
	cellDialog->setParent(Parent);
	wasEdit = false;
};
void RoboField::editField()
{
	editMode = true;
	connect(cellDialog->buttonBox, SIGNAL(accepted()), this, SLOT(cellDialogOk()));
};
/**
 * Создание пустого поля робота
 * @param x количество строк
 * @param y количество столбцов
 */
void RoboField::createField(int x, int y)
{
	destroyNet();
	destroyField();
	destroyRobot();
	//qDebug("Before items destroy");
	while (Items.count() > 0) {
		Items.removeFirst();
	}
	//qDebug("After items destroy");
	for (int i = 0; i < x; i++) {
		QList<FieldItm *> row;
		for (int j = 0; j < y; j++) {

			row.append(new FieldItm(0, this));
			FieldItm *thisItm = row.last();
			if (j > 0) {
				thisItm->setLeftsepItem(row[j - 1]);    //Устанавливаем соседей
			}
			if (i > 0) {
				thisItm->setUpsepItem(Items[i - 1].at(j));    //Устанавливаем соседей
			}
		};
		Items.append(row);
	};
	wasEdit = true;
};

QPoint RoboField::upLeftCorner(int str, int stlb)
{
	int ddx = BORT - 2;
	return QPoint(stlb * fieldSize + ddx, str * fieldSize);
};

FieldItm *RoboField::getFieldItem(int str, int stlb)
{
	if (rows() < str) {
		qDebug("RoboField:rows()<str");
		return NULL;
	};
	if (columns() < stlb) {
		qDebug("RoboField:columns()<str");
		return NULL;
	};

	if (str >= 0 && stlb >= 0 && str < Items.size() && stlb < Items[str].size()) {
		return Items[str].at(stlb);
	} else {
		return NULL;
	}

};
void RoboField::drawField(uint FieldSize)
{
	if (rows() < 1 || columns() < 1) {
		return;
	}
	destroyNet();
	destroyField();
	// clear();
	//debug();

	if (!editMode) {
		Color = NormalColor;    //Normal Color
	} else {
		Color = EditColor;    //Edit Color
	}
	this->setBackgroundBrush(QBrush(Color));
	fieldSize = FieldSize;
	drawNet();

	BortLine = QPen(WallColor, 4);
	StLine = QPen(LineColor, 3);
	qDebug() << "Rows" << rows() << "Cols:" << columns();
	//if(rows()==2)return;
	for (int i = 0; i < rows(); i++) { //Cikl po kletkam
		QList<FieldItm *> *row = &Items[i];
		for (int j = 0; j < columns(); j++) {
			//Отрисовываем бордюры
			row->at(j)->setScene(this);
			if (!row->at(j)->hasUpSep())
				row->at(j)->setUpLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y()), BortLine);

			if (!row->at(j)->hasDownSep())
				row->at(j)->setDownLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y() + fieldSize,
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y() + fieldSize), BortLine);


			if (!row->at(j)->hasLeftSep()) {
				row->at(j)->setLeftLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y() + fieldSize), BortLine);
				//qDebug()<<"Line "<<row->at(j)->leftWallLine <<"Scene"<<row->at(j)->leftWallLine->scene();
			};
			if (!row->at(j)->hasRightSep())
				row->at(j)->setRightLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y() + fieldSize), BortLine);
			if (row->at(j)->hasDownWall()) {
				row->at(j)->setDownLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y() + fieldSize,
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y() + fieldSize), StLine);

			};
			if (row->at(j)->hasUpWall()) {
				row->at(j)->setUpLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y()), StLine);

			};
			if (row->at(j)->hasLeftWall()) {
				row->at(j)->setLeftLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y() + fieldSize), StLine);

			};
			if (row->at(j)->hasRightWall()) {
				row->at(j)->setRightLine(
					new QGraphicsLineItem(upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y(),
						upLeftCorner(i, j).x() + fieldSize,
						upLeftCorner(i, j).y() + fieldSize), StLine);

			};
			if (row->at(j)->isColored()) {
				row->at(j)->setColorRect(
					new QGraphicsRectItem(upLeftCorner(i, j).x(),
						upLeftCorner(i, j).y(),
						fieldSize,
						fieldSize));

			};

			row->at(j)->showCharMark(upLeftCorner(i, j).x(), upLeftCorner(i, j).y(), fieldSize);


		};
	};
	destroyRobot();
	createRobot();
}

void RoboField::destroyField()
{
	qDebug() << "cols" << columns();
	for (int i = 0; i < columns(); i++) {
		for (int j = 0; j < rows(); j++)  {
			FieldItm *itm = getFieldItem(j, i);
			if (itm) {
				itm->cleanSelf();
			}
		}
	};

	clear();
	setka.clear();
	robot = NULL;
	this->update();
}

void RoboField::destroyRobot()
{
	if (robot) {
		removeItem(robot);
		delete robot;
		robot = NULL;
	}
}

void RoboField::destroyNet()
{

	for (int i = 0; i < setka.count(); i++) {
		this->removeItem(setka[i]);
	}
	setka.clear();
}

void RoboField::destroyScene()
{
	QList<QGraphicsItem *> items = this->items();
	while (items.count() > 0) {
		this->removeItem(items.first());
		items = this->items();
	}
}

void RoboField::debug()
{
	QList<QGraphicsItem *> items = this->items();
	for (int i = 0; i < items.count(); i++) {
		qDebug() << "Scene debug" << items[i];
	}
}

void RoboField::createRobot()
{
	robot = new SimpleRobot();
	this->addItem(robot);
	robot->setPos(upLeftCorner(robo_y, robo_x).x(), upLeftCorner(robo_y, robo_x).y());
	connect(robot, SIGNAL(moved(QPointF)), this, SLOT(roboMoved(QPointF)));
}

void RoboField::reverseUpWall(int row, int col)
{
	if (!getFieldItem(row, col)->hasUpSep()) {
		return;
		qDebug("!UpSep");
	}
	if (!getFieldItem(row, col)->canUp()) {
		getFieldItem(row, col)->removeUpWall();
		qDebug("removeUp");
	} else {
		getFieldItem(row, col)->setUpWall(new QGraphicsLineItem(upLeftCorner(row, col).x(),
				upLeftCorner(row, col).y(),
				upLeftCorner(row, col).x() + fieldSize,
				upLeftCorner(row, col).y()), StLine);
	}
	wasEdit = true;
}

void RoboField::reverseDownWall(int row, int col)
{
	if (!getFieldItem(row, col)->hasDownSep()) {
		return;
	}
	if (!getFieldItem(row, col)->canDown()) {
		getFieldItem(row, col)->removeDownWall();
	} else {
		getFieldItem(row, col)->setDownWall(new QGraphicsLineItem(upLeftCorner(row, col).x(),
				upLeftCorner(row, col).y() + fieldSize,
				upLeftCorner(row, col).x() + fieldSize,
				upLeftCorner(row, col).y() + fieldSize), StLine);
	}
	wasEdit = true;
}

void RoboField::reverseLeftWall(int row, int col)
{
	if (!getFieldItem(row, col)->hasLeftSep()) {
		return;
	}
	if (!getFieldItem(row, col)->canLeft()) {
		getFieldItem(row, col)->removeLeftWall();
	} else {
		getFieldItem(row, col)->setLeftWall(new QGraphicsLineItem(upLeftCorner(row, col).x(),
				upLeftCorner(row, col).y(),
				upLeftCorner(row, col).x(),
				upLeftCorner(row, col).y() + fieldSize), StLine);
	}
	wasEdit = true;
}

void RoboField::reverseRightWall(int row, int col)
{
	if (!getFieldItem(row, col)->hasRightSep()) {
		return;
	}
	if (!getFieldItem(row, col)->canRight()) {
		getFieldItem(row, col)->removeRightWall();
	} else {
		getFieldItem(row, col)->setRightWall(new QGraphicsLineItem(upLeftCorner(row, col).x() + fieldSize,
				upLeftCorner(row, col).y(),
				upLeftCorner(row, col).x() + fieldSize,
				upLeftCorner(row, col).y() + fieldSize), StLine);
	}
	wasEdit = true;
}

void RoboField::reverseColor(int row, int col)
{
	if (getFieldItem(row, col)->isColored()) {
		getFieldItem(row, col)->removeColor();
	} else {
		getFieldItem(row, col)->setColorRect(
			new QGraphicsRectItem(upLeftCorner(row, col).x(),
				upLeftCorner(row, col).y(),
				fieldSize,
				fieldSize));
	};
	wasEdit = true;
}

void RoboField::reverseColorCurrent()
{
	reverseColor(robo_y, robo_x);
	wasEdit = true;
}

void RoboField::reverseMark(int row, int col)
{
	if (getFieldItem(row, col)->mark) {
		getFieldItem(row, col)->removeMark();
	} else {
		getFieldItem(row, col)->mark = true;
		getFieldItem(row, col)->showCharMark(upLeftCorner(row, col).x(), upLeftCorner(row, col).y(), fieldSize);
	}
	wasEdit = true;
}

void RoboField::setRoboPos(int roboX, int roboY)
{
	robo_x = roboX;
	robo_y = roboY;
}

void KumRobot::fieldEdit()
{
	emit FieldEdit();
}

void KumRobot::CloseInst()
{
	robotWindow->close();
}

void KumRobot::reset()
{
	PrepareRunMode();
}


void KumRobot::UpdateColors()
{
	field->UpdateColors();
};

void RoboField::roboMoved(QPointF pos)
{
	int roboRow = pos.y() / FIELD_SIZE_SMALL;
	int roboCol = pos.x() / FIELD_SIZE_SMALL;
	if (roboRow >= rows()) {
		roboRow = rows() - 1;
	}
	if (roboRow < 0) {
		roboRow = 0;
	}
	if (roboCol >= columns()) {
		roboCol = columns() - 1;
	}
	if (roboCol < 0) {
		roboCol = 0;
	}
	robot->setPos(upLeftCorner(roboRow, roboCol).x(), upLeftCorner(roboRow, roboCol).y());
	robo_x = roboCol;
	robo_y = roboRow;
	wasEdit = true;
};

void RoboField::UpdateColors()
{
	LineColor = QColor(app()->settings->value("Robot/LineColor", "#C8C800").toString());
	WallColor = QColor(app()->settings->value("Robot/WallColor", "#C8C800").toString());
	EditColor = QColor(app()->settings->value("Robot/EditColor", "#00008C").toString());
	NormalColor = QColor(app()->settings->value("Robot/NormalColor", "#289628").toString());

	destroyNet();
	destroyField();
	drawNet();
	drawField(FIELD_SIZE_SMALL);
};

void RoboField::drawNet()
{
	QPen Pen, PenError;
	int ddx = 3;
	qDebug() << "Bort " << BORT;
	int infin;



	Pen = QPen(LineColor, 1);
	PenError = QPen(LineColor, 1);
	infin = 5 * BORT + 1024;

	for (int i = -2; i < columns() + 2; i++) { //Vertikalnie linii
		setka.append(this->addLine(i * (int)fieldSize + ddx, 0 - infin, i * (int) fieldSize + ddx, infin, Pen));
		setka.last()->setZValue(0.5);
	}

	for (int i = -2; i < rows() + 2; i++) { //Horizontalnie linii
		setka.append(this->addLine(ddx - infin, i * (int)fieldSize, infin, i * (int) fieldSize, Pen));
		setka.last()->setZValue(0.5);
	}


};


/**
 * Загрузка обстановки из файла, отображение не производится.
 * @param fileName Имя файла
 * @return Код ошибки
 * 1 - Ошибка чтения файла
 * 2 - Пустой файл
 * 3 - Ошибка чтения размеров обстановки
 * 4 - Неверные размеры поля
 * 5 - Пустя строка
 * 6 - Неверное положение робота
 * 0< - Ошибка в строке
 * 10 - Ошибки основного прогона
 */
int RoboField::loadFromFile(QString fileName)
{

	//destroyField();

	QFileInfo fi(fileName);
	QString name = fi.fileName();

	QString Title = QString::fromUtf8("Робот - ") + name;


	QString tmp = "";
	QString ctmp;


	QFile l_File(fileName);




	int NStrok;
	NStrok = 0;
	QString l_String;
	//  long l_Err;
	int CurX, CurY;
	int SizeX, SizeY;
	destroyField();
	// Тестовый прогон

	if (!l_File.open(QIODevice::ReadOnly)) {

		return 1;
	}



	// 1 stroka - razmery polya
	tmp = l_File.readLine();
	//QMessageBox::information( 0, "", tmp, 0,0,0);


	if (tmp.isNull() || tmp.isEmpty()) {
		l_File.close();

		return 2;
	}

	//QMessageBox::information( 0, "", tmp, 0,0,0);

	while (tmp.left(1) == ";" || tmp == "") {
		tmp = l_File.readLine();
		NStrok++;
		if (tmp.isNull()) {
			return 1;
		}
	}
	tmp = tmp.simplified();
	QStringList l_List = tmp.split(' ');

	if (l_List.count() != 2) {
		l_File.close();


		return 3;
	}

	SizeX = (l_List[0]).toInt();
	SizeY = (l_List[1]).toInt();

	if ((SizeX <= 0) || (SizeY <= 0)) {
		return 4;
	}
	//            field.destroyField();
	// field.createField(l_List[0].toInt(),l_List[1].toInt());

	// Вторая строка - положение робота

	tmp = l_File.readLine();



	if (tmp.isNull()) {
		l_File.close();

		return 5;
	}



	while (tmp.left(1) == ";" || tmp == "") {
		tmp = l_File.readLine();
		NStrok++;
		if (tmp.isNull()) {
			l_File.close();

			return 5;
		}
	}
	tmp = tmp.simplified();
	l_List = tmp.split(' ');

	// koordinaty robota
	// proverka
	if ((l_List[0]).toInt() < 0 || (l_List[1]).toInt() < 0) {


		l_File.close();
		return 6;
	}

	if ((l_List[0]).toInt() > SizeX || (l_List[1]).toInt() > SizeY) {

		l_File.close();
		return 6;
	}


	//  m_DefaultSett = l_Sett;

	while (!l_File.atEnd()) {
		//l_Err = l_File.readLine(l_String, 255);
		tmp = QString::fromUtf8(l_File.readLine());
		NStrok++;
		if (tmp.isNull()) {

			return 5;
		}
		if (tmp.left(1) == ";" || tmp == "") {
			continue;
		}
		tmp = tmp.simplified();
		l_List = tmp.split(' ');
		if (l_List.count() == 0) {
			continue;
		}

		if (l_List.count() > 9) {

			l_File.close();
			return -NStrok;
		}
		if (l_List.count() < 6) {
			l_File.close();
			qDebug("N Lexem<6");
			return -NStrok;
		};
		bool ok;
		CurX = l_List[0].toInt(&ok);
		if (!ok) {
			l_File.close();
			qDebug("Bad cur X<6");
			return -NStrok;
		};

		CurY = l_List[1].toInt(&ok);

		if (!ok) {
			l_File.close();
			qDebug("Bad curY <6");
			return -NStrok;
		};

		if (CurX < 0 || CurX > SizeX || CurY < 0 || CurY > SizeY) {

			l_File.close();
			return -NStrok;
		}

		if (l_List[4].toFloat() < 0) {

			l_File.close();
			return -NStrok;
		}


		if (l_List[5].toFloat() < MIN_TEMP) {

			l_File.close();
			return -NStrok;
		}


		if (l_List.count() >= 7) {

			QString tmp1 = l_List[6];
			//dlina lexemy dolzna ravnyatsa 1
			if (!(tmp1.length() == 1)) {

				l_File.close();
				return -NStrok;
			}


		}


		if (l_List.count() >= 8) {

			QString tmp1 = l_List[7];
			//dlina lexemy dolzna ravnyatsa 1
			if (!(tmp1.length() == 1)) {
				l_File.close();
				return -NStrok;
			}


		}

	}
	l_File.close();

	//реальный прогон
	//destroyField();

	if (!l_File.open(QIODevice::ReadOnly)) {

		return 10;
	}



	// 1 stroka - razmery polya
	tmp = l_File.readLine();

	if (tmp.isNull()) {
		l_File.close();

		return 10;
	}


	while (tmp.left(1) == ";" || tmp == "") {
		tmp = QString::fromUtf8(l_File.readLine());
		NStrok++;
		if (tmp.isNull()) {
			l_File.close();

			return 10;
		}
	}
	tmp = tmp.simplified();
	l_List = tmp.split(' ');

	if (l_List.count() != 2) {
		l_File.close();

		return -NStrok;
	}

	SizeX = (l_List[0]).toInt();
	SizeY = (l_List[1]).toInt();
	//   //NEW ROBO CODE
	createField(SizeY, SizeX);

	//END
	if ((l_List[0]).toInt() <= 0 || (l_List[1]).toInt() <= 0) {

		l_File.close();
		return - NStrok;
	}


	// Вторая строка - положение робота

	tmp = l_File.readLine();



	if (tmp.isNull()) {
		l_File.close();

		return 10;
	}



	while (tmp.left(1) == ";" || tmp == "") {
		tmp = l_File.readLine();
		NStrok++;
		if (tmp.isNull()) {
			l_File.close();

			return 10;
		}
	}
	tmp = tmp.simplified();
	l_List = tmp.split(' ');

	// koordinaty robota

	if ((l_List[0]).toInt() < 0 || (l_List[1]).toInt() < 0) {

		l_File.close();
		return - NStrok;
	}

	if ((l_List[0]).toInt() > SizeX || (l_List[1]).toInt() > SizeY) {

		l_File.close();
		return - NStrok;
	}

	robo_x = (l_List[0]).toInt();
	robo_y = (l_List[1]).toInt();

	//InitialX = m_x;
	//InitialY = m_y;




	//  delete []m_FieldDubl;



	while (!l_File.atEnd()) {
		tmp = QString::fromUtf8(l_File.readLine());
		NStrok++;
		if (tmp.isNull()) {

			l_File.close();
			return 10;
		}
		if (tmp.left(1) == ";" || tmp == "") {
			continue;
		}
		tmp = tmp.simplified();
		l_List = tmp.split(' ');
		if (l_List.count() == 0) {
			continue;
		}

		if (l_List.count() > 9) {

			l_File.close();
			return -NStrok;
		}
		CurX = l_List[0].toInt();
		CurY = l_List[1].toInt();
		if (CurX < 0 || CurX > SizeX || CurY < 0 || CurY > SizeY) {

			l_File.close();
			return -NStrok;
		}
		// TODO STENI
		if (getFieldItem(CurY, CurX)) {
			getFieldItem(CurY, CurX)->setWalls((l_List[2]).toInt());
		}

		//      int ix = (l_List[0]).toInt();
		//      int iy = (l_List[1]).toInt();

		if (!((l_List[3]).toInt() == 0)) {
			getFieldItem(CurY, CurX)->IsColored = true;
			// //QMessageBox::information( 0, "","test1" , 0,0,0);
		} else {
			if (getFieldItem(CurY, CurX)) {
				getFieldItem(CurY, CurX)->IsColored = false;
			}
		}
		qreal radiation = (l_List[4].replace(",", ".")).toDouble();
		if (getFieldItem(CurY, CurX)) {
			getFieldItem(CurY, CurX)->radiation = radiation;
		}

		if (l_List[4].toFloat() < 0) {

			l_File.close();
			return -NStrok;
		}
		qreal temperature = (l_List[5].replace(",", ".")).toDouble();
		if (getFieldItem(CurY, CurX)) {
			getFieldItem(CurY, CurX)->temperature = temperature;
		}

		if (l_List[5].toFloat() < MIN_TEMP) {

			l_File.close();
			return -NStrok;
		}



		if (l_List.count() >= 7) {

			QString tmp1 = l_List[6];
			//dlina lexemy dolzna ravnyatsa 1
			if (!(tmp1.length() == 1)) {

				l_File.close();
				return -NStrok;
			}
			//qDebug()<<QString::fromUtf8("Тест Up:")<<tmp1[0];
			if (tmp1[0] != '$') {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->upChar = tmp1[0];
				}
			} else {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->upChar = ' ' ;
				}
			}
		} else {
			if (getFieldItem(CurY, CurX)) {
				getFieldItem(CurY, CurX)->upChar = ' ' ;
			}
		}



		if (l_List.count() >= 8) {

			QString tmp1 = l_List[7];

			//dlina lexemy dolzna ravnyatsa 1
			if (!(tmp1.length() == 1)) {

				l_File.close();
				return -NStrok;
			}
			//qDebug()<<QString::fromUtf8("Тест Down:")<<tmp1[0];
			if (tmp1[0] != '$') {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->downChar = tmp1[0];
				}
			} else {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->downChar = ' ' ;
				}
			}
		} else {
			if (getFieldItem(CurY, CurX)) {
				getFieldItem(CurY, CurX)->downChar = ' ' ;
			}
		}



		if (l_List.count() >= 9) {

			QString tmp1 = l_List[8];
			//dlina lexemy dolzna ravnyatsa 1
			if (!(tmp1.length() == 1)) {

				l_File.close();
				return -NStrok;
			}
			if (tmp1[0] == '1') {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->mark = true;
				}
			} else {
				if (getFieldItem(CurY, CurX)) {
					getFieldItem(CurY, CurX)->mark = false ;
				}
			}
		} else {
			if (getFieldItem(CurY, CurX)) {
				getFieldItem(CurY, CurX)->mark = false ;
			}
		}






	}

	l_File.close();



	qDebug() << "File " << fileName ;
	wasEdit = false;


	//robot->setZValue(100);
	return (0);



};

int RoboField::saveToFile(QString fileName)
{
	QFileInfo fi(fileName);
	QString name = fi.fileName();

	qDebug() << "NewRobot Save file: " << fileName;
	//QString Title = QString::fromUtf8("Робот - ") + name;


	QChar Bukva;
	char ctmp[200];
	QFile l_File(fileName);
	if (!l_File.open(QIODevice::WriteOnly)) {
		return 1;
	}

	l_File.write("; Field Size: x, y\n");

	//QString l_String;

	sprintf(ctmp, "%i %i\n", columns(), rows());
	l_File.write(ctmp);
	l_File.write("; Robot position: x, y\n");
	sprintf(ctmp, "%i %i\n", robo_x, robo_y);
	l_File.write(ctmp);
	l_File.write("; A set of special Fields: x, y, Wall, Color, Radiation, Temperature, Symbol, Symbol1, Point\n");

	for (int i = 0; i < rows(); i++) {
		for (int j = 0; j < columns(); j++) {

			Bukva = getFieldItem(i, j)->upChar;
			if (Bukva == ' ') {
				Bukva = '$';
			}
			qDebug() << QString::fromUtf8("Буква") << Bukva;
			QChar cc;
			cc = Bukva;


			Bukva = getFieldItem(i, j)->downChar;
			if (Bukva == ' ') {
				Bukva = '$';
				qDebug() << QString::fromUtf8("Буквы нет");
			} else {
				qDebug() << QString::fromUtf8("Буква ") << Bukva;
			}

			QChar cc1 = Bukva;


			char cc2[2];
			cc2[0] = 0;
			cc2[1] = 0;

			if (getFieldItem(i, j)->mark) {
				cc2[0] = '1';
			}
			//char cc = 'g';
			if (!getFieldItem(i, j)->emptyCell()) {
				qDebug() << " i:" << i << " j:" << j << " walls:" << getFieldItem(i, j)->wallByte();
				sprintf(ctmp, "%i %i %i %i %f %f ", j, i, getFieldItem(i, j)->wallByte(), getFieldItem(i, j)->IsColored, getFieldItem(i, j)->radiation, getFieldItem(i, j)->temperature);
				QString toWrite = QString(ctmp) + cc + " " + cc1 + " " + cc2 + "\n";
				l_File.write(toWrite.toUtf8());
				qDebug() << "ROBOT:WRITE 2 FILE";
			}
		}
	}
	l_File.write("; End Of File\n");

	l_File.close();
	if (fi.exists()) {
		//QMessageBox::information( 0, "", QString::fromUtf8("файл записан"), 0,0,0);

	} else {
		QMessageBox::information(0, "", QString::fromUtf8("Файл не записан - попробуйте снова!"), 0, 0, 0);
		return -1;
	}

	//ToDubl();
	wasEdit = false;
	return 0;

};

bool RoboField::stepUp()
{
	if (m_robot && !property("fromPult").toBool()) {
		m_robot->showHideWindow(true);
	}

	setProperty("fromPult", false);

	if (getFieldItem(robo_y, robo_x)->canUp()) {
		robot->setPos(QPointF(robot->pos().x(),
				robot->pos().y() - fieldSize));

		robo_y--;
		return true;
	} else {
		return false;
	}
};

bool RoboField::stepDown()
{
	if (m_robot && !property("fromPult").toBool()) {
		m_robot->showHideWindow(true);
	}

	setProperty("fromPult", false);

	if (getFieldItem(robo_y, robo_x)->canDown()) {
		robot->moveBy(0, fieldSize);
		robo_y++;
		return true;
	} else {
		return false;
	}
};

bool RoboField::stepLeft()
{
	if (m_robot && !property("fromPult").toBool()) {
		m_robot->showHideWindow(true);
	}
	setProperty("fromPult", false);

	if (getFieldItem(robo_y, robo_x)->canLeft()) {
		robot->setPos(QPointF(robot->pos().x() - fieldSize,
				robot->pos().y()));
		robo_x--;
		return true;
	} else {
		return false;
	}
};

bool RoboField::stepRight()
{
	if (m_robot && !property("fromPult").toBool()) {
		m_robot->showHideWindow(true);
	}
	setProperty("fromPult", false);

	if (getFieldItem(robo_y, robo_x)->canRight()) {
		robot->moveBy(fieldSize, 0);
		robo_x++;
		return true;
	} else {
		return false;
	}
};


/**
 * Обработка событий нажатий кнопок мыши, показ диалога редактирования *клетки
 * @param mouseEvent
 */
void RoboField::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (!editMode) {
		return;
	}
	QPointF scenePos = mouseEvent->scenePos();
	int rowClicked = scenePos.y() / fieldSize;
	int colClicked = (scenePos.x() - 3) / fieldSize;

	if (mouseEvent->button() == Qt::RightButton) { //Диалог редактирования
		cellDialog->setPos(rowClicked + 1, colClicked + 1);
		//         if (robo_y==rowClicked && robo_x==colClicked)cellDialog->setRobot->setChecked(true);
		//else cellDialog->setRobot->setChecked(false);
		showCellDialog(getFieldItem(rowClicked, colClicked));
		return;
	};

	bool left, right, up, down;
	left = false;
	right = false;
	up = false;
	down = false;
	int upD = fieldSize, downD = fieldSize, leftD = fieldSize, rightD = fieldSize;
	qreal delta_row = scenePos.y() - rowClicked * fieldSize;
	qreal delta_col = scenePos.x() - colClicked * fieldSize - 3;
	QGraphicsScene::mousePressEvent(mouseEvent);
	if (mouseEvent->isAccepted()) {
		return;
	}



	if ((rowClicked > rows() - 1) || (rowClicked < 0)) {
		mouseEvent->ignore();
		QGraphicsScene::mousePressEvent(mouseEvent);
		return;
	};

	if ((colClicked > columns() - 1) || (colClicked < 0)) {
		mouseEvent->ignore();
		QGraphicsScene::mousePressEvent(mouseEvent);
		return;
	};

	qDebug() << "ScenePos y " << scenePos.y() << " sceneposx" << scenePos.x() << "rowClick:" << rowClicked << " " << " colClick:" << colClicked <<
		" delta_row" << delta_row << " delta_col" << delta_col;
	if (delta_row <= MAX_CLICK_DELTA) {
		up = true;
		upD = delta_row;
		qDebug("UP");
	};

	if (fieldSize - delta_row <= MAX_CLICK_DELTA) {
		down = true;
		downD = fieldSize - delta_row;
	};


	if (delta_col <= MAX_CLICK_DELTA) {
		left = true;
		leftD = delta_col;
	};

	if (fieldSize - delta_col <= MAX_CLICK_DELTA) {
		right = true;
		rightD = fieldSize - delta_col;
	};
	if (mouseEvent->modifiers() == Qt::ControlModifier) {
		markMode = false;
	} else {
		markMode = true;
	}

	//Углы клетки
	if ((left) && (up)) {
		if (upD < leftD) {
			left = false;
		} else  {
			up = false;
		}
	}
	if ((left) && (down)) {
		if (downD < leftD) {
			left = false;
		} else  {
			down = false;
		}
	}
	if ((right) && (up)) {
		if (upD < rightD) {
			right = false;
		} else  {
			up = false;
		}
	}
	if ((right) && (down)) {
		if (downD < rightD) {
			right = false;
		} else  {
			down = false;
		}
	}

	//Ставим стены
	if (up) {
		reverseUpWall(rowClicked, colClicked);
		qDebug("ReversUP");
	};
	if (down) {
		reverseDownWall(rowClicked, colClicked);
	}
	if (left) {
		reverseLeftWall(rowClicked, colClicked);
	}
	if (right) {
		reverseRightWall(rowClicked, colClicked);
	}
	if (!up && !down && !right && !left) {
		if (markMode) {
			reverseColor(rowClicked, colClicked);    //Zakraska
		} else { //Otmetit tochkoy
			reverseMark(rowClicked, colClicked);
		};
	};
	if (wasEdit) {
		emit was_edit();
	}
};


void RoboField::showCellDialog(FieldItm *cellClicked)
{

	qDebug() << "showCellDialog";
	QPoint pos = views().first()->pos();
	pos = views().first()->mapToGlobal(pos);
	qDebug() << "Pos: " << pos;
	cellDialog->move(pos);

	cellDialog->setRowCols(rows(), columns());


	//       cellDialog->mark->setChecked(cellClicked->mark);


	//       cellDialog->colored->setChecked(cellClicked->IsColored);

	QChar upChar, downChar;
	if (cellClicked->upChar == ' ') {
		upChar = '_';
	} else {
		upChar = cellClicked->upChar;
	}
	if (cellClicked->downChar == ' ') {
		downChar = '_';
	} else {
		downChar = cellClicked->downChar;
	}

	cellDialog->upChar->setText(upChar);
	cellDialog->downChar->setText(downChar);

	cellDialog->RadSpinBox->setValue(cellClicked->radiation);
	cellDialog->TempSpinBox->setValue(cellClicked->temperature);

	cellDialog->show();
};

void RoboField::cellDialogOk()
{
	int rowP = cellDialog->row_pos;
	int colP = cellDialog->col_pos;
	// qDebug()<<"CellDalog:color:"<<cellDialog->colored->isChecked();
	//  if(cellDialog->colored->isChecked()!=getFieldItem(rowP,colP)->IsColored)
	//{reverseColor(rowP,colP);wasEdit=true;};

	//  if(cellDialog->mark->isChecked()!=getFieldItem(rowP,colP)->mark){reverseMark(rowP,colP);wasEdit=true;};
	if (cellDialog->upChar->text() != getFieldItem(rowP, colP)->upChar) {

		QString tmp = cellDialog->upChar->text();
		if ((tmp.length() == 0) || (tmp[0] != '_')) {
			getFieldItem(rowP, colP)->removeUpChar();
		}
		if ((tmp.length() == 1) && (tmp[0] != '_')) {
			getFieldItem(rowP, colP)->removeUpChar();



			getFieldItem(rowP, colP)->upChar = tmp[0];
			getFieldItem(rowP, colP)->showUpChar
			(upLeftCorner(rowP, colP).x(), upLeftCorner(rowP, colP).y(), fieldSize);
			wasEdit = true;
		};
	};
	if (cellDialog->downChar->text() != getFieldItem(rowP, colP)->downChar) {
		QString tmp = cellDialog->downChar->text();
		if ((tmp.length() == 0) || (tmp[0] != '_')) {
			getFieldItem(rowP, colP)->removeDownChar();
		}
		if ((tmp.length() == 1) && (tmp[0] != '_')) {

			getFieldItem(rowP, colP)->removeDownChar();

			getFieldItem(rowP, colP)->downChar = tmp[0];
			getFieldItem(rowP, colP)->showDownChar
			(upLeftCorner(rowP, colP).x(), upLeftCorner(rowP, colP).y(), fieldSize);
			wasEdit = true;
		};
	};
	if ((getFieldItem(rowP, colP)->radiation != cellDialog->RadSpinBox->value()) ||
		(getFieldItem(rowP, colP)->temperature != cellDialog->TempSpinBox->value())) {
		wasEdit = true;
	}
	getFieldItem(rowP, colP)->radiation = cellDialog->RadSpinBox->value();
	getFieldItem(rowP, colP)->temperature = cellDialog->TempSpinBox->value();
	/*
	if(cellDialog->setRobot->isChecked())
	 {
	 robo_y=rowP;
	 robo_x=colP;
	robot->setPos(upLeftCorner(robo_y,robo_x).x(),upLeftCorner(robo_y,robo_x).y());
	wasEdit=true;
	 };*/

	if (wasEdit) {
		emit was_edit();
	}
};

void RoboField::setFieldItems(QList<QList<FieldItm *> > FieldItems)
{

	Items = QList< QList<FieldItm *> >();
	for (int i = 0; i < FieldItems.size(); i++) {
		QList<FieldItm *> row;
		for (int j = 0; j < FieldItems[i].size(); j++) {
			row << NULL;
		}
		Items << row;
	}
}

void RoboField::setItem(FieldItm *item, uint str, uint stlb)
{
	if (Items[str][stlb]) {
		delete Items[str][stlb];
	}
	Items[str][stlb] = item;
}

RoboField *RoboField::Clone()
{
	RoboField *clone = new RoboField(Parent, m_robot);
	clone->setFieldItems(Items);
	clone->robo_x = robo_x;
	clone->robo_y = robo_y;

	for (int i = 0; i < rows(); i++) {
		for (int j = 0; j < columns(); j++) {
			clone->setItem(getFieldItem(i, j)->Copy(), i, j);
			if (j > 0) {
				clone->getFieldItem(i, j)->setLeftsepItem(clone->getFieldItem(i, j - 1));
			}
			if (i > 0) {
				clone->getFieldItem(i, j)->setUpsepItem(clone->getFieldItem(i - 1, j));
			}
		};
	};

	return clone;
};



void RoboField::wbMode()
{

	this->setBackgroundBrush(QBrush("white"));
	for (int i = 0; i < rows(); i++) {
		for (int j = 0; j < columns(); j++) {
			getFieldItem(i, j)->wbWalls();
		};
	};
};
void RoboField::colorMode()
{
	for (int i = 0; i < rows(); i++) {
		for (int j = 0; j < columns(); j++) {
			getFieldItem(i, j)->colorWalls();
		};
	};
	if (!editMode) {
		Color = QColor(40, 150, 40);    //Normal Color
	} else {
		Color = QColor(0, 0, 140);    //Edit Color
	}
	this->setBackgroundBrush(QBrush(Color));
};





//+++++++Simple Robot
SimpleRobot::SimpleRobot(QGraphicsItem *parent)
{
	Q_UNUSED(parent);
	setAcceptDrops(true);



	static const int points[] = {  14, 6, 22, 14, 14, 22, 6, 14 };
	QPolygon polygon;
	polygon.setPoints(4, points);
	QPolygonF polygonf = QPolygonF(polygon);
	Robot = new QGraphicsPolygonItem();
	Robot->setPolygon(polygonf);
	Robot->setZValue(100);
	setZValue(100);
	crash = NO_CRASH;
	// ainter->drawPolygon(polygonf);
	//Robot->setPolygon(polygonf);
};

void SimpleRobot::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
	event->setAccepted(event->mimeData()->hasFormat("text/plain"));
	qDebug("Robo Drag enter");
}
void SimpleRobot::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
	event->accept();
	qDebug("Robo Drag");
};

void SimpleRobot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();
	qDebug() << "Mouse press" << event->pos();
};

void SimpleRobot::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	//qDebug("Mouse move");
	setPos(event->scenePos());

};
void SimpleRobot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	//setZValue(100);
	emit moved(event->scenePos());

};
QRectF SimpleRobot::boundingRect() const
{
	//     qreal adjust = 0.5;
	return Robot->boundingRect();
};

void SimpleRobot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	// Body
	painter->setBrush(QColor("lightgray"));
	painter->setPen(QPen("black"));

	static const int points[] = {  14, 6, 22, 14, 14, 22, 6, 14 };
	QVector<QPointF> up_crash, down_crash, left_crash, right_crash ;
	up_crash << QPointF(14, 6) << QPointF(10, 11) <<  QPointF(19, 11);
	down_crash << QPointF(14, 22) << QPointF(11, 18) <<  QPointF(18, 18);
	left_crash << QPointF(7, 14) << QPointF(11, 10) <<  QPointF(11, 18);
	right_crash << QPointF(22, 14) << QPointF(18, 10) <<  QPointF(18, 18);
	QPolygon polygon;
	polygon.setPoints(4, points);
	QPolygonF polygonf = QPolygonF(polygon);

	painter->drawPolygon(polygonf);
	if (crash != NO_CRASH) {
		qDebug("crash!");
		QPen noPen(Qt::NoPen);
		painter->setPen(noPen);
		painter->setBrush(QColor("red"));
		if (crash == UP_CRASH) {
			painter->drawPolygon(QPolygonF(up_crash));
		}
		if (crash == DOWN_CRASH) {
			painter->drawPolygon(QPolygonF(down_crash));
		}
		if (crash == LEFT_CRASH) {
			painter->drawPolygon(QPolygonF(left_crash));
		}
		if (crash == RIGHT_CRASH) {
			painter->drawPolygon(QPolygonF(right_crash));
		}

	};
};

QGraphicsPolygonItem *SimpleRobot::RoboItem()
{
	return Robot;
}

void SimpleRobot::setCrash(uint dirct)
{
	crash = dirct;
	update();
}
