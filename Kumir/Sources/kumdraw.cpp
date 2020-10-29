//****************************************************************************
//**
//** Copyright (C) 2004-now IMPB RAS. All rights reserved.
//**
//** This file is part of the KuMir.
//**
//** This file may be used under the terms of the GNU General Public
//** License version 2.0 as published by the Free Software Foundation
//** and appearing in the file LICENSE.GPL included in the packaging of
//** this file.
//**
//** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
//** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//**
//****************************************************************************/
//draw_isp.cpp
//begin 14-02-2008
#include "kum_instrument.h"
#include "kumdraw.h"
#include <math.h>
#include "kumfiledialog.h"
#include "secondarywindow.h"
#include "kumscene.h"

#include <QMessageBox>
#include <QMenuBar>
#include <QPrinter>
#include <QPainter>
#include <QToolButton>
#include <QPushButton>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>


class DrawTopFrame : public QFrame
{
protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
private:
	bool moving;
};

void DrawView:: scrollContentsBy(int dx, int dy)
{
	Q_UNUSED(dx);
	Q_UNUSED(dy);
	qDebug() << QString("SCROLL EVENT ") << dx << " " << dy;
}


DrawView::DrawView(QGraphicsScene *scene, QWidget *window)
	: QGraphicsView(scene, window)
{
}
void KumDraw::showHideWindow(bool show)//Show - true
{

	Q_UNUSED(show);

	if (!w_window->isVisible()) {
		w_window->show();
	} else {
		w_window->raise();
	}

}

void KumDraw::close()
{
//    app()->settings->setValue("Draw/Geometry", MV->saveGeometry());
	MV->close();

	w_window->close();

}
//-----------------------------------------------------
KumDraw::KumDraw(QWidget *parent): KumInstrument(parent)
{
	curDir = app()->settings->value("Directories/IO", "").toString();
	curMode = 4;

	// MV=new QMainWindow(parent,Qt::CustomizeWindowHint|Qt::WindowSystemMenuHint);
	//MV = new QWidget(parent,Qt::CustomizeWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint);
	MV = new QWidget();
	w_window = new SecondaryWindow(MV, QString::fromUtf8("Чертежник"));
	w_window->setWindowIcon(QIcon(QString(":/icons/module_window.png")));


	MV->setLayout(new QVBoxLayout());
	MV->layout()->setContentsMargins(3, 3, 3, 3);
	topFrame = new QFrame(MV);
	topFrame->setFixedHeight(32);

	MV->layout()->addWidget(topFrame);
	topFrame->setLayout(new QHBoxLayout());
	topFrame->layout()->setContentsMargins(0, 0, 0, 0);
	MV->setAttribute(Qt::WA_KeyboardFocusChange, false);
	//MV->setGeometry(10,10,500,500);

	MV->setWindowTitle(QString::fromUtf8("Чертежник - Начало"));
	//MV->show();

	//drawHeader->show();
	//QPalette PallGrey;
	//PallGrey.setBrush(QPalette::Window,QBrush(QColor(140,140,100)));
	//MV->setPalette (PallGrey);
	//CreateMenu();
	CreateNetWindow();
	CreateInfoWindow();

	scene = new KumScene(MV);

	connect(scene, SIGNAL(MousePress(qreal, qreal, bool, qreal, qreal)), this, SLOT(MousePress(qreal, qreal, bool, qreal, qreal)));
	connect(scene, SIGNAL(MouseRelease(qreal, qreal, bool)), this, SLOT(MouseRelease(qreal, qreal, bool)));
	connect(scene, SIGNAL(MouseMove(qreal, qreal, bool)), this, SLOT(MouseMove(qreal, qreal, bool)));
	connect(scene, SIGNAL(MouseWheel(int)), this, SLOT(MouseWheel(int)));


	scene->setBackgroundBrush(QBrush(QColor("lightgreen")));
	//scene->setBackgroundBrush (QBrush(QColor("white")));
	StepX = 1.;
	StepY = 1.;
	SizeX = 462;
	SizeY = 260;
	ButtonSdwig = 100;
	SqwareNetFlag = true;
	NetShowFlag = true;
	TmpNetShowFlag = true;


	//CurColorName = QString::fromUtf8("черный");
	lines.clear();

	Createbtn11();
	Createbtn12();
	Createbtn13();
	Createbtn21();
	Createbtn22();
	Createbtn23();
	Createbtn31();
	Createbtn32();
	Createbtn33();

	connect(btn11, SIGNAL(clicked()), this, SLOT(ZoomUp()));
	connect(btn12, SIGNAL(clicked()), this, SLOT(LoadFromFileActivated()));
	connect(btn13, SIGNAL(clicked()), this, SLOT(ZoomDown()));
	connect(btn32, SIGNAL(clicked()), this, SLOT(SaveToFileActivated()));
	connect(btn21, SIGNAL(clicked()), this, SLOT(ShowNetWindow()));
	connect(btn22, SIGNAL(clicked()), this, SLOT(ToCenter()));
	connect(btn23, SIGNAL(clicked()), this, SLOT(ClearPicture()));
	connect(btn31, SIGNAL(clicked()), this, SLOT(ToWindow()));
	connect(btn33, SIGNAL(clicked()), this, SLOT(PrintToPDF()));

	WindowZoom = 1;
	WindowX0 = -SizeX / 2;
	WindowY0 = -SizeY / 2;

	NLastVec = 0;
	CurColor.Red = 1;
	CurColor.Green = 1;
	CurColor.Blue = 1;
	CurY = 0;
	CurX = 0;
	CurZ = 1;

	PenPosition = true;
	mPen = NULL;
	SdwigNetX = 0.;
	SdwigNetY = 0.;

	CreatePen();

	SetColorString(trUtf8("черный"));
	MinZoom = 0.00001;
	MaxZoom = 2000000;
	pDialog = new PrintDialog(MV);

	SetWindowSize(10, 10, SizeX, SizeY);
	// HorizWindow();
//	setWindowPos(app()->settings->value("Draw/Xpos").toInt(),
//				 app()->settings->value("Draw/Ypos").toInt());
	qDebug() << "DrawPos" << app()->settings->value("Draw/Xpos").toInt();
	ToDubl();

	RightMousePressFlag = false;
	CreateCoordFrame();
	CreateXYLabel();
}


//----------------------------------------------------
//сдвиг на вектор
int KumDraw::GoVector(double DeltaXX, double DeltaYY)
{



	qreal x1 = CurX;
	qreal y1 = -CurY;

	qreal x2 = CurX + DeltaXX;
	qreal y2 = -(CurY + DeltaYY);

	if (PenPosition) {
		mPen->setBrush(QBrush(QColor(CurColor.Red, CurColor.Green, CurColor.Blue)));

		lines.append(scene->addLine(x1, y1, x2, y2));
		lines.last()->setZValue(CurZ);
		lines.last()->setPen(QPen(QColor(CurColor.Red, CurColor.Green, CurColor.Blue)));
		CurZ += 0.01;
		CurX += DeltaXX;
		CurY += DeltaYY;
	} else {
		//QGraphicsLineItem *tmp = new QGraphicsLineItem(x1, y1 , x2, y2 );
		//lines.append(tmp);
		mPen->setBrush(QBrush(QColor("white")));
		CurX += DeltaXX;
		CurY += DeltaYY;
	}
	mPen->setPos(CurX, -CurY);
	return 0;

}

//опустить перо
//------------------------------------
void KumDraw::PenDown()
{
	PenPosition = true;
	GoVector(0, 0); //пересветка пера
}
//------------------------------------------------------
//поднять перо
void KumDraw::PenUp()
{
	PenPosition = false;
	GoVector(0, 0); //пересветка пера
}
//------------------------------------------
//переход в точку
int KumDraw::GoPoint(double xx, double yy)
{
	double DeltaX = xx - CurX;
	double DeltaY = yy - CurY;

	GoVector(DeltaX, DeltaY);

	return 0;
}

//----------------------------------------------------------------------------
//--------------------------------------------------------------
// сдвиг окна в мировых координатах
//void KumDraw::SetWindowPosition(double x0,double y0, double zoom)
//{
//}
//-----------------------------------------------------------------------

//-------------------------------------------------------------------
//сдвиг окна вправо
void KumDraw::MoveRight(void)
{

	WindowX0 = WindowX0 - SizeX / WindowZoom / 4;

	WindowRedraw();

}
//------------------------------------------------------
//сдвиг окна влево
void KumDraw::MoveLeft(void)
{

	WindowX0 = WindowX0 + SizeX / WindowZoom / 4;

	WindowRedraw();

}
//-----------------------------------------------------
//сдвиг окна вверх
void KumDraw::MoveUp(void)
{
	WindowY0 = WindowY0 - SizeY / WindowZoom / 4;

	WindowRedraw();
}
//-----------------------------------------------------
//сдвиг окна вниз
void KumDraw::MoveDown(void)
{
	WindowY0 = WindowY0 + SizeY / WindowZoom / 4;

	WindowRedraw();
}
//-----------------------------------------------------
void KumDraw::ZoomUp(void)
{
	if (WindowZoom > MaxZoom) {
		return;
	}
	//HideCoord();

	WindowX0 = WindowX0 + SizeX / WindowZoom / 4;
	WindowY0 = WindowY0 + SizeY / WindowZoom / 4;

	WindowZoom = WindowZoom * 2;

	//mPen->scale(0.5,.5);
	view->scale(2., 2.);

	WindowRedraw();

}
//--------------------------------------------------
void KumDraw::ZoomDown(void)
{
	if (WindowZoom < MinZoom) {
		return;
	}

	HideCoord();
	WindowX0 = WindowX0 - SizeX / WindowZoom / 2;
	WindowY0 = WindowY0 - SizeY / WindowZoom / 2;

	WindowZoom = WindowZoom / 2;
	view->scale(0.5, 0.5);

	WindowRedraw();
}
//-------------------------------------------------------------------
void KumDraw::lockControls()
{
	/*
	if  ( btnBox->isEnabled() )
	    btnBox->setEnabled(false);

	*/
}
//----------------------------------------
void KumDraw::unlockControls()
{
	/*
	if ( !btnBox->isEnabled() )
	    btnBox->setEnabled(true);
	*/
}
//----------------------------------------------------------
//сохранение в ps-файл
int KumDraw::SaveToFile(QString p_FileName)
{
	QFile l_File(p_FileName);
	QChar Bukva;
	char ctmp[200];
	if (!l_File.open(QIODevice::WriteOnly)) {
		return 1;
	}

	//QString ttt = QString::fromUtf8("Чертежник - Начало");

	//l_File.write( ttt.toUtf8());
	l_File.write("%!PS-Adobe-1.0 EPSF-1.0\n");
	QString l_String;
	l_File.write("%%Creator: Cherteznik\n");
	l_File.write("%%Pages: 1\n");
	l_File.write("%%Orientation: Portrait\n");


	// maximum, minimum

	qreal MinX, MaxX, MinY, MaxY, VecX1, VecX2, VecY1, VecY2;

	QLineF TmpLine;
	MinX = 1000000;
	MinY = 1000000;

	MaxX = -1000000;
	MaxY = -1000000;


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		VecX1 = TmpLine.x1();
		VecY1 = -TmpLine.y1();
		VecX2 = TmpLine.x2();
		VecY2 = -TmpLine.y2();
		if (VecX1 < MinX) {
			MinX = VecX1;
		}
		if (VecY1 < MinY) {
			MinY = VecY1;
		}
		if (VecX1 > MaxX) {
			MaxX = VecX1;
		}
		if (VecY1 > MaxY) {
			MaxY = VecY1;
		}

		if (VecX2 < MinX) {
			MinX = VecX2;
		}
		if (VecY2 < MinY) {
			MinY = VecY2;
		}
		if (VecX2 > MaxX) {
			MaxX = VecX2;
		}
		if (VecY2 > MaxY) {
			MaxY = VecY2;
		}

	}
	double Scale;

	if (MaxX - MinX > MaxY - MinY) {
		Scale = (596 - 10) / (MaxX - MinX);
	} else {
		Scale = (842 - 10) / (MaxY - MinY);
	}
	Scale = Scale * 0.9;

	//  QString tmp1 = QString(ctmp)+" scale\n";



	l_File.write("%%BoundingBox: 0 0 596 842\n");
	l_File.write("%%HiResBoundingBox: 0 0 596 842\n");
	l_File.write("%%EndComments\n");
	l_File.write("%%Page: 1 1\n");
	sprintf(ctmp, "%f %f translate\n", -MinX, -MinY);
	l_File.write(ctmp);
	sprintf(ctmp, "%f %f scale\n", Scale, Scale);
	l_File.write(ctmp);
	l_File.write("gsave [1 0 0 1 0 0] concat\n");
	l_File.write("0 0 0 setrgbcolor\n");
	l_File.write("[] 0 setdash\n");
	l_File.write("1 setlinewidth\n");
	l_File.write("0 setlinejoin\n");
	l_File.write("0 setlinecap\n");
	l_File.write("newpath\n");
	//QColor TmpColor;
	QPen TmpPen;
	QColor TmpColor;
	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		TmpPen = lines[i]->pen();
		TmpColor = TmpPen.color();
		sprintf(ctmp, "%i %i %i setrgbcolor\n", TmpColor.red(),  TmpColor.green(), TmpColor.blue());
		l_File.write(ctmp);

		VecX1 = TmpLine.x1();
		VecY1 = -TmpLine.y1();
		VecX2 = TmpLine.x2();
		VecY2 = -TmpLine.y2();

		sprintf(ctmp, "%f %f moveto\n", VecX1, VecY1);
		l_File.write(ctmp);

		sprintf(ctmp, "%f %f lineto\n", VecX2, VecY2);

		l_File.write(ctmp);


	}

	//77777777777777777777777777777777777
	QString TmpText;
	QByteArray ccc;
	qreal tmpX, tmpY, FontSize;
	for (int i = 0; i < kumtexts.count(); i++) {
		FontSize = kumtexts[i]->Size;
		sprintf(ctmp, "/Curier findfont %f scalefont setfont\n", FontSize);
		l_File.write(ctmp);

		tmpX = kumtexts[i]->x;
		tmpY = kumtexts[i]->y;
		sprintf(ctmp, "%f %f moveto\n", tmpX, tmpY);
		l_File.write(ctmp);


		//TmpPen = texts[i]->pen();
		//TColor = TmpPen.color();
		sprintf(ctmp, "%i %i %i setrgbcolor\n", kumtexts[i]->color.Red,  kumtexts[i]->color.Green, kumtexts[i]->color.Blue);
		l_File.write(ctmp);

		TmpText = "(" + texts[i]->text() + ") show\n";
		ccc = TmpText.toUtf8();
		l_File.write(ccc);
	}

	//777777777777777777777777777

	l_File.write("stroke\n");
	l_File.write("grestore\n");
	l_File.write("showpage\n");
	l_File.close();
	return 0;


	l_File.close();
	return 0;

}
//--------------------------------------------
//запись строки в файл
//void KumDraw::WriteLine(QFile *p_File, QString p_String)
//{
/*
    p_File->writeBlock(p_String, p_String.length());
*/
//}
//---------------------------------------------------------------------------
//Загрузка из файла
int KumDraw::LoadFromFile(QString p_FileName)
{
	ClearPicture();
	QFileInfo fi(p_FileName);
	QString name = fi.fileName();                // name = "archive.tar.gz"

	QString Title = QString::fromUtf8("Чертежник - ") + name;

	MV->setWindowTitle(Title);
	qreal CurrentScale = 1.0;

	QString tmp = "";
	char ctmp[200];
	QString l_String;
	QFile l_File(p_FileName);

	int NStrok;
	NStrok = 0;

	qreal x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	if (!l_File.open(QIODevice::ReadOnly)) {
		QMessageBox::information(MV, "", QString::fromUtf8("Ошибка открытия файла"), 0, 0, 0);
		return 1;
	}

	//l_String = l_File.readLine();
	//QMessageBox::information( 0, "", tmp, 0,0,0);
	QByteArray ttt;
	ttt = l_File.readLine();
	l_String = QString::fromUtf8(ttt);

	//QMessageBox::information( MV, "",l_String , 0,0,0);
	if (l_String.isNull()) {
		l_File.close();
		QMessageBox::information(MV, "", QString::fromUtf8("Ошибка чтения строки"), 0, 0, 0);
		return 1;
	}
	l_String = l_String.simplified();
	QStringList l_List = l_String.split(' ');


	if (!(l_String == "%!PS-Adobe-1.0 EPSF-1.0")) {
		l_File.close();
		QMessageBox::information(MV, "", QString::fromUtf8("Это не PS - файл"), 0, 0, 0);

		return 1;
	}

	// Вторая строка %%Creator: Cherteznik
	//  l_String = l_File.readLine();
	ttt = l_File.readLine();
	l_String = QString::fromUtf8(ttt);

	NStrok++;

	l_String = l_String.simplified();
	l_List = l_String.split(' ');
	if (!(l_List[1] == "Cherteznik"))
		//if (!(l_String == "%Creator: Cherteznik"))
	{

		QMessageBox::information(MV, "", QString::fromUtf8("Это не файл чережника"), 0, 0, 0);

		l_File.close();
		return 1;
	}


	for (int i = 0; i < 15; i++) {
		//  l_String = l_File.readLine();
		ttt = l_File.readLine();
		l_String = QString::fromUtf8(ttt);
		NStrok++;
	}

	// koordinaty vektorov
	CurZ = 1.;
	while (!l_File.atEnd()) {
		//считываем цвет

		//  l_String = l_File.readLine();
		ttt = l_File.readLine();
		l_String = QString::fromUtf8(ttt);
		NStrok++;


		if (l_String.isNull()) {
			QMessageBox::information(MV, "", QString::fromUtf8("Ошибка чтения строки"), 0, 0, 0);
			l_File.close();
			return 1;
		}

		l_String = l_String.simplified();
		l_List = l_String.split(' ');
		if (l_List.count() == 0) {
			continue;
		}


		if (l_List[l_List.count() - 1] == "stroke") {
			break;
		}

		if (l_List[l_List.count() - 1] == "setfont") {
			CurrentScale = x1 = l_List[2].toFloat();
			continue;
		}

		if (l_List[l_List.count() - 1] == "show") {
			tmp = "";
			int NWord = l_List.count() - 1;
			for (int j = 0; j < NWord; j++) {
				tmp += l_List[j] + " ";
			}
			//      QMessageBox::information( 0, "",  QString::fromUtf8(tmp), 0,0,0);
			int l = tmp.length();
			tmp.chop(2);
			tmp = tmp.right(l - 3);
			drawText(tmp, CurrentScale);
			continue;
		}

		if (l_List[l_List.count() - 1] == "setrgbcolor") {
			CurColor.Red = l_List[0].toInt();
			CurColor.Green = l_List[1].toInt();
			CurColor.Blue = l_List[2].toInt();
			continue;
		}

		if (l_List[l_List.count() - 1] == "moveto") {
			x1 = l_List[0].toFloat();
			y1 = -l_List[1].toFloat();
			CurX = x1;
			CurY = -y1;
			continue;
		}

		if (l_List[l_List.count() - 1] == "lineto") {
			x2 = l_List[0].toFloat();
			y2 = -l_List[1].toFloat();
			lines.append(scene->addLine(x1, y1, x2, y2));
			lines.last()->setZValue(CurZ);
			lines.last()->setPen(QPen(QColor(CurColor.Red, CurColor.Green, CurColor.Blue)));
			CurZ += 0.01;
			CurX = x2;
			CurY = -y2;
			continue;
		}

		sprintf(ctmp, "%4i", NStrok);
		tmp = QString::fromUtf8("Ошибка в строке ");
		tmp.append(QString(ctmp));

		break;
	}

	l_File.close();



	//CurY = -CurY;
	mPen->setPos(CurX, -CurY);
	GoVector(0, 0);
	return 0;

}
//------------------------------------------------

float KumDraw::GetMinX(void)
{

	qreal MinX = 1000000;
	qreal VecX1, VecX2;
	QLineF TmpLine;


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		VecX1 = TmpLine.x1();
		VecX2 = TmpLine.x2();

		if (VecX1 < MinX) {
			MinX = VecX1;
		}
		if (VecX2 < MinX) {
			MinX = VecX2;
		}

	}
	return MinX;

}
//--------------------------------------
float KumDraw::GetMinY(void)
{

	qreal MinY = 1000000;
	qreal VecY1, VecY2;
	QLineF TmpLine;


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		VecY1 = -TmpLine.y1();
		VecY2 = -TmpLine.y2();


		if (VecY1 < MinY) {
			MinY = VecY1;
		}
		if (VecY2 < MinY) {
			MinY = VecY2;
		}

	}

	return MinY;

}
//------------------------------------------
float KumDraw::GetMaxX(void)
{

	qreal MaxX = -1000000;
	qreal VecX1, VecX2;
	QLineF TmpLine;


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		VecX1 = TmpLine.x1();
		VecX2 = TmpLine.x2();


		if (VecX1 > MaxX) {
			MaxX = VecX1;
		}
		if (VecX2 > MaxX) {
			MaxX = VecX2;
		}

	}
	return MaxX;


}
//---------------------------------------
float KumDraw::GetMaxY(void)
{

	qreal MaxY = -1000000;
	qreal VecY1, VecY2;
	QLineF TmpLine;


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		VecY1 = -TmpLine.y1();
		VecY2 = -TmpLine.y2();

		if (VecY1 > MaxY) {
			MaxY = VecY1;
		}
		if (VecY2 > MaxY) {
			MaxY = VecY2;
		}

	}

	return MaxY;




}
//-------------------------------------------
//загоняет картинку в окно
void KumDraw::ToWindow(void)
{

	qreal MinX = GetMinX();
	qreal MaxX = GetMaxX();
	qreal MinY = GetMinY();
	qreal MaxY = GetMaxY();
	qreal TextX, TextY;
	for (int i = 0; i < kumtexts.count(); i++) {
		TextX = kumtexts[i]->x;
		TextY = kumtexts[i]->y;
		int len = kumtexts[i]->text.length();
		qreal TextScale = kumtexts[i]->Size;
		if (TextX < MinX) {
			MinX = TextX;
		}
		if (TextY < MinY) {
			MinY = TextY;
		}
		if (TextX + len * TextScale > MaxX) {
			MaxX = TextX + len * TextScale;
		}
		if (TextY + TextScale > MaxY) {
			MaxY = TextY + TextScale;
		}
	}

	MinX = MinX - (MaxX - MinX) * 0.1;
	MaxX = MaxX + (MaxX - MinX) * 0.1;

	MinY = MinY - (MaxY - MinY) * 0.1;
	MaxY = MaxY + (MaxY - MinY) * 0.1;

	qreal tmp, tmpy;
	Q_UNUSED(tmpy);
	qreal tmpZoom;
	qreal XSdwig = 0;
	qreal YSdwig = 0;
	if ((lines.count() == 0) && (kumtexts.count() == 0)) {
		ToCenter();
	};
	if ((MaxX - MinX < 0.00001) && (MaxY - MinY < 0.00001)) {
		return;
	}

	//if (MaxX - MinX > MaxY - MinY)
	if ((MaxX - MinX) / SizeX > (MaxY - MinY) / SizeY)
		//if ((MaxX - MinX)/SizeY > (MaxY - MinY)/SizeX)
	{
		tmp = MaxX - MinX;
		//tmpy = MaxY - MinY;
		tmpZoom = (SizeX) / tmp;
		//if(SizeX<SizeY) tmpZoom = (SizeY)/tmpy;
		//YSdwig = (MaxX-MinX)/2 - (MaxY-MinY)/2;
		YSdwig = SizeY / 2 / tmpZoom - (MaxY - MinY) / 2;


		//Delta = (MaxX-MinX)/50;
		qDebug() << "WAR1";
	} else {
		tmp = MaxY - MinY;
		tmpZoom = (SizeY) / tmp;
		//XSdwig = (MaxY-MinY)/2 - (MaxX-MinX)/2;
		XSdwig = SizeX / 2 / tmpZoom - (MaxX - MinX) / 2;

		//Delta = (MaxY-MinY)/50;
		qDebug() << "WAR2";
	}

	qreal Scale = tmpZoom / WindowZoom;

	WindowZoom = tmpZoom;
	WindowX0 = MinX  - XSdwig;

	//WindowY0 = MinY - YSdwig;
	WindowY0 = -MaxY - YSdwig;
	//qDebug(",WindowY0 %f %f",WindowX0,WindowY0);
	//qDebug("WindowX0,WindowY0 %f %f",WindowX0,WindowY0);
	//mPen->scale(1/Scale,1/Scale);
	view->scale(Scale, Scale);


	WindowRedraw();

}
//------------------------------------------------
void KumDraw::ClearPicture(void)
{
	HideCoord();
	MV->setWindowTitle(QString::fromUtf8("Чертежник - Начало"));

	for (int i = 0; i < lines.count(); i++) {
		delete lines[i];
	}
	lines.clear();

	for (int i = 0; i < texts.count(); i++) {
		delete texts[i];
	}
	texts.clear();

	for (int i = 0; i < kumtexts.count(); i++) {
		delete kumtexts[i];
	}
	kumtexts.clear();

	WindowRedraw();

	CurX = 0;
	CurY = 0;
	mPen->setPos(0, 0);
	mPen->setBrush(QBrush(QColor("blue")));
	ToDubl();
}

//---------------------------------------------------------
void KumDraw::ToLeftDownCorner(void)
{
	WindowX0 = 0;
	WindowY0 = 0;
	//WindowZoom = 1;
	WindowRedraw();
}
//---------------------------------------------------
void KumDraw::CreatePen(void)
{
	static const int points[] = {  0, 0, 6, -3, 18, -15, 15, -18, 3, -6 };
	QPolygon polygon;
	polygon.setPoints(5, points);
	QPolygonF polygonf = QPolygonF(polygon);

	mPen = new QGraphicsPolygonItem();
	mPen->setPolygon(polygonf);
	mPen->setZValue(100);
	mPen->setBrush(QBrush(QColor("black")));
	scene->addItem(mPen);
#if 1
	mPen->setScale(0.125 * 0.125 * mPen->scale());
#else
	mPen->scale(0.5, 0.5);
	mPen->scale(0.5, 0.5);
	mPen->scale(0.5, 0.5);
	mPen->scale(0.5, 0.5);
	mPen->scale(0.5, 0.5);
	mPen->scale(0.5, 0.5);
#endif
}
//--------------------------------------------

//void KumDraw::DestroyPen(void)
//{
/*
    delete m_Pen;
    m_Pen = NULL;
*/
//}

//---------------------------------------------
//---------------------------------------------------
void KumDraw::SetLineColor(int LRed, int LGreen, int LBlue)
{

	CurColor.Red = LRed;
	CurColor.Green = LGreen;
	CurColor.Blue = LBlue;
	mPen->setBrush(QBrush(QColor(CurColor.Red, CurColor.Green, CurColor.Blue)));
}

//--------------------------------------
bool KumDraw::SetColorString(QString Color)
{
	CurColorName = Color;
	if (Color == trUtf8("черный")) {
		SetLineColor(1, 1, 1);
		return true;
	}
	if (Color == trUtf8("белый")) {
		SetLineColor(254, 254, 254);
		return true;
	}
	if (Color == trUtf8("красный")) {
		SetLineColor(254, 0, 0);
		return true;
	}
	if (Color == trUtf8("желтый")) {
		SetLineColor(254, 254, 0);
		return true;
	}
	if (Color == trUtf8("оранжевый")) {
		SetLineColor(254, 128, 0);
		return true;
	}
	if (Color == trUtf8("зеленый")) {
		SetLineColor(0, 254, 0);
		return true;
	}
	if (Color == trUtf8("голубой")) {
		SetLineColor(137, 175, 245);
		return true;
	}
	if (Color == trUtf8("синий")) {
		SetLineColor(0, 0, 254);
		return true;
	}
	if (Color == trUtf8("фиолетовый")) {
		SetLineColor(128, 0, 255);
		return true;
	}
	return false;
}

//---------------------------------------------------------------
void KumDraw::Createbtn11()
{
	btn11 = new QToolButton(topFrame);
	QPixmap pm(QString::fromUtf8(":/icons/zoom-in.png"));
	QSize sz = pm.rect().size();
	fprintf(stderr, "W=%d, H=%d\n", sz.width(), sz.height());
	QIcon icon(pm);
	btn11->setIcon(icon);
	btn11->setIconSize(QSize(32, 32));
	btn11->setToolTip(trUtf8("Крупнее"));
	//btn11->move(SizeX/2 - BUTTON_SIZE/2 -BUTTON_SIZE+3, SizeY+20-BUTTON_SIZE+2+ MenuHigth);
	//btn11->move(SizeX/2 - BUTTON_SIZE/2 -BUTTON_SIZE+3, SizeY+20+2+ MenuHigth);
	//btn11->move(SizeX/2 - BUTTON_SIZE/2 -BUTTON_SIZE+3, SizeY+15+ MenuHigth);
	//  btn11->move(ButtonSdwig, 5);
	//  btn11->resize(BUTTON_SIZE,BUTTON_SIZE);
	topFrame->layout()->addWidget(btn11);
	btn11->show();
}
//------------------------------------------
void KumDraw::Createbtn12()
{

	btn12 = new QToolButton(topFrame);
	//btn12->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	btn12->setIcon(QIcon(QString::fromUtf8(":/icons/document-open.png")));
	btn12->setIconSize(QSize(20, 20));
	btn12->setToolTip(trUtf8("Загрузить"));
	//btn12->move(SizeX/2 - BUTTON_SIZE/2+3, SizeY+15+ MenuHigth);
	//  btn12->move(100+BUTTON_SIZE, 5);
	btn12->resize(BUTTON_SIZE, BUTTON_SIZE);
	topFrame->layout()->addWidget(btn12);
	//  btn12->show();
	btn12->hide();
}
//--------------------------------------------------------
void KumDraw::Createbtn13()
{
	btn13 = new QToolButton(topFrame);
	btn13->setIcon(QIcon(QString::fromUtf8(":/icons/zoom-out.png")));
	//btn22->setIcon(QIcon(QString::fromUtf8("1uparrow.png")));
	btn13->setIconSize(QSize(22, 22));
	btn13->setToolTip(trUtf8("Мельче"));
	//btn13->move(SizeX/2 - BUTTON_SIZE/2+BUTTON_SIZE+3, SizeY+15+ MenuHigth);
	//  btn13->move(ButtonSdwig+BUTTON_SIZE, 5);
//	btn13->resize(BUTTON_SIZE,BUTTON_SIZE);
	topFrame->layout()->addWidget(btn13);
	btn13->show();
}
//---------------------------------------------------------------
void KumDraw::Createbtn21()
{
	btn21 = new QToolButton(topFrame);
	QIcon iconGrid;
	QPixmap pxGrid = QPixmap(":/icons/draw-grid.png");
	Q_ASSERT(!pxGrid.isNull());
	iconGrid.addPixmap(pxGrid);

	btn21->setIcon(iconGrid);
	btn21->setIconSize(QSize(22, 22));
	btn21->setToolTip(trUtf8("Сетка"));

	topFrame->layout()->addWidget(btn21);
	btn21->show();
}
//------------------------------------------
void KumDraw::Createbtn22()
{

	btn22 = new QToolButton(topFrame);
	btn22->setIcon(QIcon(QString::fromUtf8(":/icons/go-home.png")));
	//btn22->setIcon(QIcon(QString::fromUtf8("1uparrow.png")));
	btn22->setIconSize(QSize(20, 20));
	btn22->setToolTip(trUtf8("Восстановить"));
	//btn22->move(SizeX/2 - BUTTON_SIZE/2+3, SizeY+15+BUTTON_SIZE+ MenuHigth);
	//  btn22->move(ButtonSdwig+3*BUTTON_SIZE, 5);
	btn22->resize(BUTTON_SIZE, BUTTON_SIZE);
	topFrame->layout()->addWidget(btn22);
	//btn22->show();
	btn22->hide();
}
//--------------------------------------------------------
void KumDraw::Createbtn23()
{

	btn23 = new QToolButton(topFrame);
	//btn23->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	btn23->setIcon(QIcon(QString::fromUtf8(":/icons/document-new.png")));
	btn23->setIconSize(QSize(20, 20));
	btn23->setToolTip(trUtf8("Очистить"));
	//btn23->move(SizeX/2 - BUTTON_SIZE/2+BUTTON_SIZE+3, SizeY+15+BUTTON_SIZE+ MenuHigth);
	//  btn23->move(100, 5);
	btn23->resize(BUTTON_SIZE, BUTTON_SIZE);
	topFrame->layout()->addWidget(btn23);
	//btn23->show();
	btn23->hide();
}
//---------------------------------------------------------------
void KumDraw::Createbtn31()
{
	btn31 = new QToolButton(topFrame);
	btn31->setIcon(QIcon(QString::fromUtf8(":/icons/zoom-fitdraw.png")));
	//btn31->setIcon(QIcon(QString::fromUtf8("1leftarrow.png")));
	btn31->setIconSize(QSize(22, 22));
	btn31->setToolTip(trUtf8("Весь чертеж"));
	//  btn31->move(ButtonSdwig + 2*BUTTON_SIZE,5);
	//  btn31->resize(BUTTON_SIZE,BUTTON_SIZE);
	topFrame->layout()->addWidget(btn31);
	btn31->show();
}
//------------------------------------------
void KumDraw::Createbtn32()

{

	btn32 = new QToolButton(topFrame);
	//btn32->setIcon(QIcon(QString::fromUtf8(":/icons/tab-new.png")));
	btn32->setIcon(QIcon(QString::fromUtf8(":/icons/document-save.png")));
	btn32->setIconSize(QSize(20, 20));
	btn32->setToolTip(trUtf8("Сохранить"));
	//btn32->move(SizeX/2 - BUTTON_SIZE/2+3, SizeY+2*BUTTON_SIZE+15+ MenuHigth);
	//  btn32->move(100 + 2*BUTTON_SIZE,5);
	btn32->resize(BUTTON_SIZE, BUTTON_SIZE);
	topFrame->layout()->addWidget(btn32);
	//btn32->show();
	btn32->hide();
}
//--------------------------------------------------------
void KumDraw::Createbtn33()
{

	btn33 = new QToolButton(topFrame);
	btn33->setIcon(QIcon(QString::fromUtf8(":/icons/document-print.png")));
	btn33->setIconSize(QSize(20, 20));
	btn33->setToolTip(trUtf8("Информация"));
	//  btn33->move(100 + 3*BUTTON_SIZE,5);
	//btn33->move(SizeX/2 - BUTTON_SIZE/2+BUTTON_SIZE+3, SizeY+2*BUTTON_SIZE+15+ MenuHigth);
	btn33->resize(BUTTON_SIZE, BUTTON_SIZE);
	topFrame->layout()->addWidget(btn33);
	//btn33->show();
	btn33->hide();
}
//----------------------------------------------------------
void KumDraw::ToCenter(void)
{


	WindowX0 = -SizeX / 2;
	WindowY0 = -SizeY / 2;
	qDebug("SizeX,SizeY %i %i", SizeX, SizeY);

	qreal WindowZoomOld = WindowZoom;
	WindowZoom = 1;

	qreal Scale = WindowZoom / WindowZoomOld;
	//mPen->scale(1/Scale,1/Scale);
	view->scale(Scale, Scale);

	bool OldNetFlag = NetShowFlag;
	NetShowFlag = false;
	ZoomUp();
	ZoomUp();
	ZoomUp();
	ZoomUp();
	ZoomUp();
	ZoomUp();
	NetShowFlag = OldNetFlag;
	WindowRedraw();
}
//-----------------------------------------------------------------------

// пересветка окна
void KumDraw::WindowRedraw(void)
{


	QRectF curentRect;
	//=view->sceneRect();
	qreal fSizeX = SizeX;
	qreal fSizeY = SizeY;
	curentRect.setRect(WindowX0, WindowY0, fSizeX / WindowZoom, fSizeY / WindowZoom);
	//qDebug("WindowZoom %f SizeX,SizeY %f %f",WindowZoom,fSizeX,fSizeY);
	//qDebug("Redraw WindowX0,WindowY0 %f %f SizeX/WindowZoom,SizeY/WindowZoom %f %f",WindowX0,WindowY0,fSizeX/WindowZoom,fSizeY/WindowZoom);
	DrawNet();
	view->show();

	scene->setSceneRect(curentRect);

}
//--------------------------------------------------
void KumDraw::DrawNet()
{
	//Рисуем сетку и оси

	int i;
	for (i = 0; i < Netlines.count(); i++) {
		delete Netlines[i];
	}

	Netlines.clear();

	// вертикальная сетка

	qreal BegX, BegY;
	qreal EndX, EndY;
	qreal fx1, fy1, fx2, fy2;

	qreal SdvigY = fmod(WindowY0, StepY) + fmod(SdwigNetY, StepY);
	qreal SdvigX = fmod(WindowX0, StepX) - fmod(SdwigNetX, StepX);


	//  if (NetShowFlag )
	//  {

	//Stepen = log(WindowZoom)/2;
	//Step = 10./pow(10,Stepen);

	qreal StepScreenX = WindowZoom * StepX;
	qreal StepScreenY = WindowZoom * StepY;

	qreal StepXNew = StepX;
	qreal StepYNew = StepY;

	int NLinesX = SizeX / WindowZoom / StepX + 2;
	//qDebug("NLineX %i",NLinesX);
	BegX = WindowX0 -  SdvigX - NLinesX * StepX;
	EndX = WindowX0 + SizeX / WindowZoom + SdvigX + NLinesX * StepX;
	int NLinesY = SizeY / WindowZoom / StepY + 2;

	BegY = WindowY0 -  SdvigY - NLinesY * StepY;
	EndY = WindowY0 + SizeY / WindowZoom + SdvigY + NLinesY * StepY;

	if (NetShowFlag) {

		btn21->setIcon(QIcon(QString::fromUtf8(":/icons/draw-grid.png")));
		GoodNetFlag = true;
		if (StepScreenX <= 3) {
			btn21->setIcon(QIcon(QString::fromUtf8(":/icons/stop.png")));
			TmpNetShowFlag = false;
			StepXNew = 3 / WindowZoom;
			GoodNetFlag = false;
		}


		if (StepScreenY <= 3) {
			btn21->setIcon(QIcon(QString::fromUtf8(":/icons/stop.png")));
			TmpNetShowFlag = false;
			StepYNew = 3 / WindowZoom;
			GoodNetFlag = false;
		}

		fx1 = BegX;
		TmpNetShowFlag = true;
		while (fx1 < EndX) {
			fx1 = fx1 + StepXNew;
			fx2 = fx1;
			fy1 = BegY;
			fy2 = EndY;

			Netlines.append(scene->addLine(fx1, fy1, fx2, fy2));
			Netlines.last()->setZValue(0.5);
			Netlines.last()->setPen(QPen(QColor("gray")));
		}
		//горизональная сетка




		fy1 = BegY;

		while (fy1 < EndY) {
			fy1 = fy1 + StepYNew;
			fy2 = fy1;
			fx1 = BegX;
			fx2 = EndX;

			Netlines.append(scene->addLine(fx1, fy1, fx2, fy2));
			Netlines.last()->setZValue(0.5);
			Netlines.last()->setPen(QPen(QColor("gray")));

		}

	}// drawnetflag


	//fini:
	//Оси координат
	//qDebug("osi");
	//fx1 = WindowX0-(Zapas*StepX/WindowZoom);
	fx1 = BegX;
	fy1 = 0;
	//fx2 = WindowX0+SizeX/WindowZoom+Zapas*StepX;
	fx2 = EndX;
	fy2 = 0;


	Netlines.append(scene->addLine(fx1, fy1, fx2, fy2));
	Netlines.last()->setZValue(0.6);
	Netlines.last()->setPen(QPen(QColor("blue")));

	fx1 = 0;
	fy1 = BegY;
	fx2 = 0;
	fy2 = EndY;

	Netlines.append(scene->addLine(fx1, fy1, fx2, fy2));
	Netlines.last()->setZValue(0.6);
	Netlines.last()->setPen(QPen(QColor("blue")));


}
//----------------------------------------------------------
int KumDraw::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	symbols = Symbols;
	functions = Functions;
	int count = 0;

	Functions->addInternalFunction(trUtf8("опустить перо"), none, 0);

	Functions->addInternalFunction(trUtf8("поднять перо"), none, 0);

	Functions->addInternalFunction(trUtf8("сместиться в точку"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;
	Functions->setArgType(Functions->size() - 1, 1, real);
	count++;

	Functions->addInternalFunction(trUtf8("сместиться на вектор"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;
	Functions->setArgType(Functions->size() - 1, 1, real);
	count++;
	/*
	Functions->addInternalFunction(QString::fromUtf8("установить цвет"), none,3);
	Functions->func_table[Functions->count].perems[0]=integer;
	count++;
	Functions->func_table[Functions->count].perems[1]=integer;
	count++;
	Functions->func_table[Functions->count].perems[2]=integer;
	count++;
	*/

	Functions->addInternalFunction(trUtf8("установить цвет"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	count++;

	Functions->addInternalFunction(trUtf8("надпись"), none, 2);
	Functions->setArgType(Functions->size() - 1, 1, kumString);
	count++;
	Functions->setArgType(Functions->size() - 1, 0, real);
	count++;

	return 0;
}
//------------------------------------------------------------------
void KumDraw::runFunc(QString name, QList<KumValueStackElem> *arguments, int *err)
{
	showHideWindow(true);
	*err = 0;
	//qDebug("Draw:runFunc");
	//qDebug() <<"Name1 - "<< name<<"!";
	if (name == trUtf8("опустить перо")) {
		//qDebug("Draw:runFunc:op_pero");
		PenDown();
		emit sync();
		return;
	}
	//qDebug() <<"Name2 - "<< name<<"!";
	if (name == trUtf8("поднять перо")) {
		//view->scale(0.5,0.5);
		PenUp();
		emit sync();
		return;
	}


	//qDebug("Draw:runFunc:mid");
	//qDebug() << name <<"  " << arguments;
	if (name == trUtf8("сместиться в точку")) {
		if (arguments->count() < 2) {
			qDebug("Draw:runFunc:sm_v_tochku:args.count()<2");
			emit sync();
			return;
		};
		KumValueStackElem argument1 = arguments->first();
		double xx = argument1.asFloat();

		KumValueStackElem argument2 = arguments->at(1);
		double yy = argument2.asFloat();

		GoPoint(xx, yy);
		//view->ensureVisible (500, 500 ,500,500);

		//QRectF curentRect=view->sceneRect();
		//qDebug("Scene rect x %f",curentRect.x());
		//qDebug("Scene rect y %f",curentRect.y());
		//curentRect.setX(curentRect.x()+50);
		//view->setSceneRect(curentRect);
		emit sync();

		return;

	};
	//qDebug("Draw:runFunc:vector");
	if (name == trUtf8("сместиться на вектор")) {
		KumValueStackElem argument1 = arguments->first();
		double xx = argument1.asFloat();
		KumValueStackElem argument2 = arguments->at(1);
		double yy = argument2.asFloat();

		GoVector(xx, yy);
		emit sync();
		return;
	}

	if (name == trUtf8("установить цвет")) {
		KumValueStackElem argument1 = arguments->first();
		QString SColor = argument1.asString();
		if (!SetColorString(SColor)) {
			*err = 8049;
		}

		emit sync();

	};

	if (name == trUtf8("надпись")) {
		KumValueStackElem argument1 = arguments->first();
		KumValueStackElem argument2 = arguments->at(1);
		qreal Scale = argument1.asFloat();
		QString Text = argument2.asString();
		drawText(Text, Scale);
		emit sync();
		return;
	}



}


//-----------------------------------------------

void KumDraw::LoadFromFileActivated()
{




	QFileDialog dialog(MV, tr("Load drawing..."), curDir, "(*.psk)");

	if (!dialog.exec()) {
		return;
	}
	QString DrawFile = dialog.selectedFiles().first();
	QDir dir = dialog.directory();
	curDir = dir.path();
	if (DrawFile.isEmpty()) {
		return;
	}
	//CurrentFileName = DrawFile;
	LoadFromFile(DrawFile);
	ToWindow();



	/*

	QString curDir = "";
	#ifdef WIN32
	curDir += "\\";
	#endif
	QString  DrawFile =  QFileDialog::getOpenFileName(MV,QString::fromUtf8 ("Открыть файл"),"/home", "(*.ps)");
	if ( DrawFile.isEmpty())return;

	LoadFromFile(DrawFile);
	*/

}

//-----------------------------------------------------

void KumDraw::SaveToFileActivated()
{



	QFileDialog dialog(MV, tr("Save drawing..."), curDir, "(*.psk)");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (!dialog.exec()) {
		return;
	}

	QString DrawFile = dialog.selectedFiles().first();
	QDir dir = dialog.directory();
	curDir = dir.path();
	if (DrawFile.contains("*") || DrawFile.contains("?")) {
		QMessageBox::information(0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0, 0, 0);
		return;
	}

	if (DrawFile.right(4) != ".psk") {
		DrawFile += ".psk";
	}

	SaveToFile(DrawFile);
	app()->settings->setValue("Draw/StartField/File", DrawFile);
	QFileInfo fi = QFileInfo(DrawFile);
	MV->setWindowTitle(QString::fromUtf8("Чертежник - ") + fi.fileName());








	/*



	QString curDir = "";
	#ifdef WIN32
	curDir += "\\";
	#endif



	//QString    DrawFile =  QFileDialog::getSaveFileName(MV,QString::fromUtf8 ("Сохранить в файл"),"/home", "(*.ps)");
	//if ( DrawFile.isEmpty())return;



	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if(!dialog.exec())return ;

	QString  DrawFile = dialog.selectedFiles().first();
	QDir dir=dialog.directory();
	curDir=dir.path();
	if (DrawFile.contains("*") || DrawFile.contains("?"))
	{
	QMessageBox::information( 0, "", QString::fromUtf8("Недопустимый символ в имени файла!"), 0,0,0);
	return ;
	}

	if(DrawFile.right(3) != ".ps")DrawFile += ".ps";

	SaveToFile(DrawFile);
	*/

}
//--------------------------------------------
void KumDraw::ToDubl()
{
	QLineF TmpLine;
	QPen TmpPen;
	QGraphicsLineItem *TmpItem;
	for (int i = 0; i < linesDubl.count(); i++) {
		delete linesDubl[i];
	}

	linesDubl.clear();


	for (int i = 0; i < lines.count(); i++) {

		TmpLine = lines[i]->line();
		TmpPen = lines[i]->pen();
		//TmpItem = lines[i];
		TmpItem = new QGraphicsLineItem(TmpLine);
		TmpItem->setPen(TmpPen);
		linesDubl.append(TmpItem);
	}

	PenXDubl = CurX;
	PenYDubl = CurY;
	PenPositionDubl = PenPosition;
}
//--------------------------------------------
void KumDraw::FromDubl()
{
	if (linesDubl.count() == 0) {
		ClearPicture();
		return;
	};

	QLineF TmpLine;

	for (int i = 0; i < lines.count(); i++) {
		delete lines[i];
	}

	lines.clear();

	QGraphicsLineItem *TmpItem;
	QPen TmpPen;
	QColor TmpColor;

	for (int i = 0; i < linesDubl.count(); i++) {

		TmpItem = linesDubl[i];
		TmpLine = TmpItem->line();
		TmpPen = TmpItem->pen();
		TmpColor = TmpPen.color();
		lines.append(scene->addLine(TmpLine));
		lines.last()->setPen(QPen(TmpColor));
	}

	CurX = PenXDubl;
	CurY = PenYDubl;
	PenPosition = PenPositionDubl;
	mPen->setPos(CurX, -CurY);
}
//-------------------------------------------------------------
void KumDraw::Info()
{
	char ctmp[200];
	QString tmp;

	sprintf(ctmp, "%7.2f", WindowX0);
	tmp = QString(ctmp);
	lX0->setText(tmp);

	sprintf(ctmp, "%7.2f", -WindowY0 - SizeY / WindowZoom);
	tmp = QString(ctmp);
	lY0->setText(tmp);

	sprintf(ctmp, "%7.2f", WindowX0 + SizeX / WindowZoom);
	tmp = QString(ctmp);
	lX1->setText(tmp);

	sprintf(ctmp, "%7.2f", -WindowY0);
	tmp = QString(ctmp);
	lY1->setText(tmp);


	sprintf(ctmp, "%7.2f", WindowX0 + SizeX / WindowZoom / 2);
	tmp = QString(ctmp);
	lXCen->setText(tmp);

	sprintf(ctmp, "%7.2f", -WindowY0 - SizeY / WindowZoom / 2);
	tmp = QString(ctmp);
	lYCen->setText(tmp);

	sprintf(ctmp, "%7.2f", SdwigNetX);
	tmp = QString(ctmp);
	lXNetX0->setText(tmp);


	sprintf(ctmp, "%7.2f", SdwigNetY);
	tmp = QString(ctmp);
	lYNetY0->setText(tmp);

	sprintf(ctmp, "%7.2f", StepX);
	tmp = QString(ctmp);
	lXNetDX->setText("(" + tmp);

	sprintf(ctmp, "%7.2f", StepY);
	tmp = QString(ctmp);
	lYNetDY->setText(tmp + ")");

	QPalette Pall;
	Pall.setColor(QPalette::BrightText, QColor(CurColor.Red, CurColor.Green, CurColor.Blue));
	Pall.setBrush(QPalette::WindowText, QBrush(QColor(CurColor.Red, CurColor.Green, CurColor.Blue)));
	lColor->setPalette(Pall);

	tmp = QString::fromUtf8("Цвет ") + CurColorName;
	lColor->setText(tmp);

	if (PenPosition) {
		tmp = QString::fromUtf8("Перо опущено");
	} else {
		tmp = QString::fromUtf8("Перо поднято");
	}

	lPen->setText(tmp);
	QPoint position = MV->pos();
	QSize diff = MV->size() - InfoWindow->size();
	position += QPoint(diff.width() / 2, diff.height() / 2);
	InfoWindow->move(position);
	InfoWindow->show();
}
//-------------------------------------------------------
bool KumDraw::PrintToPDF(void)
{
	pDialog->view = view;
	pDialog->setSizes(SizeX, SizeY);
	if (pDialog->exec() != 1) {
		return false;
	}


	QString PrintFileName = pDialog->fileName();

	QPrinter *RPrinter = new QPrinter(QPrinter::PrinterResolution);
	RPrinter->setOutputFileName(PrintFileName);
	RPrinter->setFullPage(false);
	RPrinter->setPageSize(QPrinter::A4);


	QPainter *RPainter = new QPainter();
	RPainter->begin(RPrinter);
	RPainter->setBrush(Qt::SolidPattern);
	QColor RLineColor = QColor(1, 1, 1);


	//RPainter->setViewTransformEnabled(true);

	QLineF TmpLine;

	qreal MinX, MaxX, MinY, MaxY, VecX1, VecX2, VecY1, VecY2;
	MinX = WindowX0;
	MinY = -WindowY0 - SizeY / WindowZoom;
	MaxX = WindowX0 + SizeX / WindowZoom;
	MaxY = -WindowY0 ;

	qreal Scale;
	int PageWidth = 540;

	if (MaxX - MinX > MaxY - MinY) {
		Scale = MaxX - MinX;
	} else {
		Scale = MaxY - MinY;
	}

	if (Scale < 0.000001) {
		return false;
	}

	int PenWidth = 1;
	QPen Pen;


	//   int Stepen = log10(Scale);
	//   Step = 0.1*pow(10,Stepen);

	Scale = PageWidth / Scale;
	PenWidth = 2;
	Pen = QPen(RLineColor, PenWidth);
	RPainter->setPen(Pen);
	RPainter->setClipRect(0, 0, (MaxX - MinX)*Scale, (MaxY - MinY)*Scale);
	//RPainter->setWindow ( 0, -5, 10,10 );
	//RPainter->setViewport ( MinX, MinY, Scale, Scale*1.41 );
	//RPainter->setWindow ( MinX, MinY, Scale+1, (Scale+1)*1.41 );
	qreal x1, y1, x2, y2;


	x1 = (MinX - MinX) * Scale + 1;
	y1 = (MinY - MinY) * Scale + 1;
	x2 = (MaxX - MinX) * Scale - 2;
	y2 = (MaxY - MinY) * Scale - 1;
	//рисуем границы

	RPainter->drawLine(x1, y1, x1, y2);
	RPainter->drawLine(x1, y2, x2, y2);
	RPainter->drawLine(x2, y2, x2, y1);
	RPainter->drawLine(x2, y1, x1, y1);



	//qreal Stepen = log(Scale)/2;
	//qreal Step = 10./pow(10,Stepen);
	//qDebug ("Stepen Step %f %f",Stepen,Step);

	//Рисуем сетку
	Pen = QPen(QColor("gray"), 1);
	RPainter->setPen(Pen);
	Pen.setWidthF(0.5);
	qreal XCur = 0;

	if (NetShowFlag && TmpNetShowFlag) {
		while (XCur < MaxX - StepX) {
			XCur += StepX;
			if (XCur < MinX) {
				continue;
			}

			x1 = (XCur - MinX) * Scale + 1;
			y1 = (MinY - MinY) * Scale + 1;
			x2 = (XCur - MinX) * Scale - 1;
			y2 = (MaxY - MinY) * Scale - 1;

			RPainter->drawLine(x1, y1, x2, y2);
		}

		XCur = 0;
		while (XCur > MinX + StepX) {
			XCur -= StepX;
			if (XCur > MaxX) {
				continue;
			}

			x1 = (XCur - MinX) * Scale + 1;
			y1 = (MinY - MinY) * Scale + 1;
			x2 = (XCur - MinX) * Scale - 1;
			y2 = (MaxY - MinY) * Scale - 1;
			RPainter->drawLine(x1, y1, x2, y2);
		}


		qreal YCur = 0;
		while (YCur < MaxY - StepY) {
			YCur += StepY;
			if (YCur < MinY) {
				continue;
			}
			x1 = (MinX - MinX) * Scale + 1;
			y1 = (MaxY - YCur) * Scale + 1;
			x2 = (MaxX - MinX) * Scale - 1;
			y2 = (MaxY - YCur) * Scale - 1;

			RPainter->drawLine(x1, y1, x2, y2);
		}

		YCur = 0;
		while (YCur > MinY + StepY) {
			YCur -= StepY;
			if (YCur > MaxY) {
				continue;
			}
			x1 = (MinX - MinX) * Scale + 1;
			y1 = (MaxY - YCur) * Scale;
			x2 = (MaxX - MinX) * Scale - 1;
			y2 = (MaxY - YCur) * Scale;
			RPainter->drawLine(x1, y1, x2, y2);
		}
	}

	//рисуем оси
	Pen.setWidthF(2);
	RPainter->setPen(Pen);
	//вертикальная ось
	if (MinX < 0 && MaxX > 0) {
		x1 = (0 - MinX) * Scale;
		y1 = (MaxY - MinY) * Scale;
		x2 = (0 - MinX) * Scale;
		y2 = (MaxY - MaxY) * Scale;
		RPainter->drawLine(x1, y1, x2, y2);
	}
	//горизонтальная ось
	if (MinY < 0 && MaxY > 0) {
		x1 = (MinX - MinX) * Scale;
		y1 = MaxY * Scale;
		x2 = (MaxX - MinX) * Scale;
		y2 = MaxY * Scale;
		RPainter->drawLine(x1, y1, x2, y2);
	}



	//Pen.setStyle(Qt::DashDotDotLine);
	Pen.setColor("black");
	Pen.setWidthF(1);
	RPainter->setPen(Pen);
	//рисуем картинку
	for (int i = 0; i < lines.count(); i++) {
		TmpLine = lines[i]->line();
		VecX1 = TmpLine.x1();
		VecY1 = -TmpLine.y1();
		VecX2 = TmpLine.x2();
		VecY2 = -TmpLine.y2();

		x1 = (VecX1 - MinX) * Scale;
		y1 = (MaxY - VecY1) * Scale;
		x2 = (VecX2 - MinX) * Scale;
		y2 = (MaxY - VecY2) * Scale;

		RPainter->drawLine(x1, y1, x2, y2);
	}
	//рисуем текст
	qreal FontSize, tmpX, tmpY;
	QString TmpText;
	//QFont font ( "Courier", -1, -1,  false );
	QFont font("Courier", 40);
	RPainter->setFont(font);
	for (int i = 0; i < kumtexts.count(); i++) {
		FontSize = kumtexts[i]->Size * Scale / 24; //Подобрано экспериментально
		tmpX = kumtexts[i]->x;
		tmpY = kumtexts[i]->y;
		x1 = (tmpX - MinX) * Scale / FontSize;
		y1 = (MaxY - tmpY) * Scale / FontSize;

		TmpText = texts[i]->text();
		RPainter->scale(FontSize, FontSize);
		RPainter->drawText(x1, y1, TmpText);
		RPainter->scale(1 / FontSize, 1 / FontSize);
	}

	RPainter->end();
	return true;
}


//--------------------------------------------------------------
void KumDraw::MousePress(qreal x, qreal y, bool LeftButtonFlag, qreal xScene, qreal yScene)
{

	//qDebug("Draw::MousePress %i",LeftButtonFlag);


	if (LeftButtonFlag) {
		LeftMousePressFlag = true;
		OldX = x;
		OldY = y;
		MVxmin = MV->x();
		MVxmax = MVxmin + MV->width();
		MVymin = MV->y();
		MVymax = MVymin + MV->height();
		QApplication::setOverrideCursor(Qt::PointingHandCursor);
	} else {
		coordFrame->move(x - MV->pos().x() - 10, y - MV->pos().y() - 40);
		ShowCoord(xScene, -yScene);
		RightMousePressFlag = true;
	};

}
//---------------------------------
void KumDraw::MouseRelease(qreal x, qreal y, bool LeftButtonFlag)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
	if (LeftButtonFlag) {
		LeftMousePressFlag = false;
		QApplication::restoreOverrideCursor();

		DrawNet();
		qDebug("DrawNet");
	} else {
		RightMousePressFlag = false;
	}
	HideCoord();
	qDebug("HideCoord");
}
//-------------------------------------------
void KumDraw::MouseMove(qreal x, qreal y, bool LeftButtonFlag)
{
	Q_UNUSED(LeftButtonFlag);
	//if(!LeftMousePressFlag){if(!RightMousePressFlag)return;ShowCoord(xScene,-yScene);return;};

	if (!LeftMousePressFlag) {
		return;
	}
	qDebug() << "MMove " << x;
	if ((abs(x - OldX) < 3) && (abs(y - OldY) < 3)) {
		qDebug() << "Return2 x" << x << " oldX" << OldX;
		return;
	};
	//if (x <= MVxmin || x >=MVxmax || y <= MVymin + 40|| y >= MVymax + 40)
	//{
	//qDebug()<<"Return2 x"<<x<<" y"<<y <<" MVxmin"<<MVxmin;
	//qDebug()<<"MVxmax"<<MVxmax<<" MVymin"<<MVymin <<" MVymax"<<MVymax;
	//return;
	//}
	//QApplication::setOverrideCursor(Qt::PointingHandCursor);
	WindowX0 = WindowX0 - (x - OldX) / WindowZoom;
	WindowY0 = WindowY0 - (y - OldY) / WindowZoom;
	QRectF curentRect;

	curentRect.setRect(WindowX0, WindowY0, SizeX / WindowZoom, SizeY / WindowZoom);

	scene->setSceneRect(curentRect);
	qDebug() << "SetRec";
	OldX = x;
	OldY = y;
	//qDebug("Draw::MouseMove %f %f",x,y);
}
//-----------------------------------------------------------------
void KumDraw::MouseWheel(int Delta)
{
	//qDebug("wheel %i",Delta);
	if (Delta > 0) {
		ZoomUp();
	} else {
		ZoomDown();
	}
}
//-------------------------------------------------------
void KumDraw::SetWindowSize(int x1, int y1, int w, int h)
{
	Q_UNUSED(x1);
	Q_UNUSED(y1);
	Q_UNUSED(w);
	Q_UNUSED(h);
	//SizeX = w;
	//SizeY = h;
	// MV->setGeometry(x1,y1,w,h);
	// //MV->setFixedSize(w,h);
	MV->resize(SizeX, SizeY + DPANEL_HIGTH);
	MV->setMinimumSize(SizeX, SizeY + DPANEL_HIGTH);
	MV->setMaximumSize(SizeX, SizeY + DPANEL_HIGTH);
	//  MV->setMinimumSize(462,260);
	//  MV->setMaximumSize(462,260);
	//  MV->resize(462,260);
	view = new DrawView(scene, MV);

	MV->layout()->addWidget(view);
	//view->setMinimumSize (SizeX, SizeY+5);
	//view->setMinimumSize (SizeX+20, SizeY+25);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	view->setMinimumSize(SizeX, SizeY + 5);
	view->move(0, MenuHigth);
	view->show();
	WindowZoom = 1;
	WindowX0 = -SizeX / 2;
	WindowY0 = -SizeY / 2;
	//view->setMinimumSize (w, h);

	qreal OldStepX = StepX;
	qreal OldStepY = StepY;
	StepX = 256;
	StepY = 256;
	ToCenter();
	StepX = OldStepX;
	StepY = OldStepY;
	WindowRedraw();
	qDebug() << "MV geomerty" << MV->geometry();
#ifndef Q_OS_MAC
//	drawHeader->setCorrection(3,3);
//	drawHeader->setChildWidget(MV);
#endif
}
//----------------------------------------------
void KumDraw::CreateMenu(void)
{

	// menu =MV->menuBar()->addMenu(QString::fromUtf8("&Чертеж"));
	//  menu = new QMenu(QString::fromUtf8("&Чертеж"),topFrame);
	QMenuBar *menuBar = new QMenuBar(topFrame);

	topFrame->layout()->addWidget(menuBar);
	menu = menuBar->addMenu(QString::fromUtf8("&Чертеж"));


	QAction *ClearPicture = new QAction(QString::fromUtf8("&Очистить"), topFrame);
	menu->addAction(ClearPicture);

	QAction *LoadPicture = new QAction(QString::fromUtf8("&Загрузить"), topFrame);
	menu->addAction(LoadPicture);

	QAction *SavePicture = new QAction(QString::fromUtf8("&Сохранить"), topFrame);
	menu->addAction(SavePicture);

	QAction *aPrint = new QAction(QString::fromUtf8("&Печать в файл"), topFrame);
	menu->addAction(aPrint);

	//QAction* aToDubl =new QAction(QString::fromUtf8("&Запомнить стартовый"),MV);
	//menu->addAction(aToDubl);

	//QAction* aFromDubl =new QAction(QString::fromUtf8("&Восстановить стартовый"),MV);
	//menu->addAction(aFromDubl);

	connect(ClearPicture, SIGNAL(triggered()), this, SLOT(ClearPicture()));
	connect(LoadPicture, SIGNAL(triggered()), this, SLOT(LoadFromFileActivated()));
	connect(SavePicture, SIGNAL(triggered()), this, SLOT(SaveToFileActivated()));
	//connect ( aToDubl, SIGNAL(triggered()), this, SLOT(ToDubl()) );
	//connect ( aFromDubl, SIGNAL(triggered()), this, SLOT(FromDubl()) );
	connect(aPrint, SIGNAL(triggered()), this, SLOT(PrintToPDF()));
	//MenuHigth = 20;
	// MenuHigth =  MV->menuBar()->height()-1;
	MenuHigth = topFrame->height();

	// menu1 =MV->menuBar()->addMenu(QString::fromUtf8("&Вид"));
	// menu1 = new QMenu(QString::fromUtf8("&Вид"),topFrame);
	menu1 = menuBar->addMenu(QString::fromUtf8("&Вид"));
	// topFrame->layout()->addWidget(menu1);
	QAction *UpAction = new QAction(QString::fromUtf8(" &Крупнее"), topFrame);
	menu1->addAction(UpAction);
	connect(UpAction, SIGNAL(triggered()), this, SLOT(ZoomUp()));

	QAction *DownAction = new QAction(QString::fromUtf8(" &Мельче"), topFrame);
	menu1->addAction(DownAction);
	connect(DownAction, SIGNAL(triggered()), this, SLOT(ZoomDown()));

	QAction *SdwigAction = new QAction(QString::fromUtf8(" &Сдвиги"), topFrame);
	menu1->addAction(SdwigAction);
	connect(SdwigAction, SIGNAL(triggered()), this, SLOT(SdwigHelp()));


	QAction *FullPicture = new QAction(QString::fromUtf8(" &Весь чертеж"), topFrame);
	menu1->addAction(FullPicture);
	connect(FullPicture, SIGNAL(triggered()), this, SLOT(ToWindow()));


	QAction *Home = new QAction(QString::fromUtf8(" &Восстановить"), topFrame);
	menu1->addAction(Home);
	connect(Home, SIGNAL(triggered()), this, SLOT(ToCenter()));

	QAction *Net = new QAction(QString::fromUtf8(" &Сетка"), topFrame);
	menu1->addAction(Net);
	connect(Net, SIGNAL(triggered()), this, SLOT(ShowNetWindow()));

	QAction *AInfo = new QAction(QString::fromUtf8(" &Информация"), topFrame);
	menu1->addAction(AInfo);
	connect(AInfo, SIGNAL(triggered()), this, SLOT(Info()));
	HorizPicture = new QAction(QString::fromUtf8(" &Горизонтальное окно"), topFrame);
	menu1->addAction(HorizPicture);
	//HorizPicture->setCheckable(true);
	HorizPicture->setText(QString::fromUtf8("*Горизонтальное окно"));
	connect(HorizPicture, SIGNAL(triggered()), this, SLOT(HorizWindow()));

	VertPicture = new QAction(QString::fromUtf8(" &Вертикальное окно"), topFrame);
	menu1->addAction(VertPicture);
	connect(VertPicture, SIGNAL(triggered()), this, SLOT(VertWindow()));


}

//------------------------------
void KumDraw:: HorizWindow()
{

	//  MV->setMinimumSize(462,260);
	//  MV->setMaximumSize(462,260);
	SizeX = 462;
	SizeY = 260;
	delete view;
	SetWindowSize(10, 35, SizeX, SizeY);
	// MV->resize(SizeX, SizeY+DPANEL_HIGTH +MenuHigth);
	//MV->setMinimumSize(SizeX,SizeY + DPANEL_HIGTH + MenuHigth);
	//MV->setMaximumSize(SizeX,SizeY + DPANEL_HIGTH + MenuHigth);
	CreateCoordFrame();
	CreateXYLabel();
	ToCenter();
	//HorizPicture->setText(QString::fromUtf8("*Горизонтальное окно"));
	//VertPicture->setText(QString::fromUtf8(" &Вертикальное окно"));
	//view->setMinimumSize (SizeX, SizeY+5);
	//view->move (0, MenuHigth);


	//  MV->resize(462,260);
	//  MV->move(10,35);
}//------------------------------
void KumDraw:: VertWindow()
{

	//  MV->setMinimumSize(260,462);
	//  MV->setMaximumSize(260,462);
	SizeX = 260;
	SizeY = 462;
	delete view;
	SetWindowSize(10, 25, SizeX, SizeY);
	CreateCoordFrame();
	CreateXYLabel();
	//MV->resize(SizeX, SizeY+DPANEL_HIGTH +MenuHigth);
	//MV->setMinimumSize(SizeX,SizeY + DPANEL_HIGTH + MenuHigth);
	//MV->setMaximumSize(SizeX,SizeY + DPANEL_HIGTH + MenuHigth);



	ToCenter();
	//HorizPicture->setText(QString::fromUtf8(" &Горизонтальное окно"));
	//VertPicture->setText(QString::fromUtf8("*Вертикальное окно"));

	//view->setMinimumSize (SizeX, SizeY+5);
	//view->move (0, MenuHigth);

	//  MV->resize(260,462);
	//  MV->move(10,25);
}
//-------------------------------------
void KumDraw::CreateNetWindow(void)
{
	int NetWinX0  = 40;
	int NetWinY0  = 80;
	int NetWinSizeX  = 200;
	int NetWinSizeY  = 180;
	//NetWindow = new QWidget(MV,Qt::Tool);;
	NetWindow = new QWidget(MV, Qt::Dialog);;
	NetWindow->setWindowTitle(QString::fromUtf8("Сетка"));
	//NetWindow->setAutoFillBackground ( false);
	//NetWindow->setWindowModality(Qt::ApplicationModal);
	NetWindow->setGeometry(NetWinX0, NetWinY0, NetWinSizeX, NetWinSizeY);
	NetWindow->setMinimumSize(NetWinSizeX, NetWinSizeY);
	NetWindow->setMaximumSize(NetWinSizeX, NetWinSizeY);

	QGridLayout *l = new QGridLayout(NetWindow);

	NetWindow->setLayout(l);
	//NetWindow->setAutoFillBackground ( false );

	BoxNet = new QCheckBox(QString::fromUtf8("Показывать сетку?"), NetWindow);
	// BoxNet->move(20,10 );
	// BoxNet->resize(150,20);
	l->addWidget(BoxNet, 0, 0, 1, 2);
	BoxNet->show();

	BoxSqware = new QCheckBox(QString::fromUtf8("Сетка квадратная?"), NetWindow);
	// BoxSqware->move(20,30 );
	// BoxSqware->resize(150,20);
	l->addWidget(BoxSqware, 1, 0, 1, 2);
	BoxSqware->show();



	/*
	QLabel *lX = new QLabel(QString::fromUtf8("Начальная точка.X   Шаг.X "),NetWindow,0);
	lX->setGeometry( QRect(5, 60, 150, 20 ) );

	QLabel *lY = new QLabel(QString::fromUtf8("Начальная точка.Y   Шаг.Y"),NetWindow,0);
	lY->setGeometry( QRect(5, 110,150, 20 ) );

	eX0 = new QLineEdit("",NetWindow);
	eX0->setGeometry( 25, 85 , 60,20 );

	eStepX = new QLineEdit("",NetWindow);
	eStepX->setGeometry( 110, 85 , 60,20 );

	eY0 = new QLineEdit("",NetWindow);
	eY0->setGeometry( 25, 130 , 60,20 );

	eStepY = new QLineEdit("",NetWindow);
	eStepY->setGeometry( 110, 130 , 60,20 );
	*/


	//QLabel *lX = new QLabel(QString::fromUtf8("Шаг.X"),NetWindow,0);
	lX = new QLabel(QString::fromUtf8("Шаг X"), NetWindow, 0);
	lY = new QLabel(QString::fromUtf8("Шаг Y"), NetWindow, 0);
	//  lX->setGeometry( QRect(5, 60, 150, 20 ) );
	QFont font("Helvetica", 10, QFont::Bold);
	lX->setFont(font);
	lY->setFont(font);

	//QPalette Pall;
	//Pall.setColor(QPalette::BrightText,QColor(200,0,0));
	//Pall.setBrush(QPalette::WindowText,QBrush(QColor(200,0,0)));



	//lX->setAutoFillBackground ( false);
	//lX->setPalette(Pall);

	//l->addWidget(lX,2,0,1,1);
	//l->addWidget(lY,2,1,1,1);
	l->addWidget(lX, 2, 0, 1, 1);
	l->addWidget(lY, 2, 1, 1, 1);

	lX->show();

	eStepX = new QLineEdit("", NetWindow);
	//eStepX->setGeometry( 10, 85 , 80,26 );
	//PallRed.setColor(QPalette::BrightText,QColor(0,0,255));
	//PallRed.setBrush(QPalette::WindowText,QBrush(QColor(0,0,255)));

	//QFont font("Helvetica", 12, QFont::Bold);
	//font.setColor("red");
	//eStepX->setFont(font);
	//eStepX->setAutoFillBackground ( false);
	//eStepX->setFrame(true);
	//eStepX->setCenterOnScroll (false );
	//eStepX->setPalette(Pall);
	//eStepX->setLineWrapMode(QTextEdit::NoWrap);
	//eStepX->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	//eStepX->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

	//eStepX->setMaximumHeight(20);
	//eStepX->setInputMask("NNNN");

	//eStepX->setForegroundRole (PallRed);
	//eStepX->setForegroundRole (QPalette::LinkVisited);

	//l->addWidget(eStepX,3,0,1,1);
	//eStepX->show();
	//QPlainTextEdit *tmp = new QPlainTextEdit("000.00",NetWindow);
	//tmp->setGeometry( 10, 85 , 90,20 );

	//tmp->setAutoFillBackground ( true);
	//tmp->setBackgroundVisible(true);
	//tmp->setPalette(PallRed);
	l->addWidget(eStepX, 3, 0, 1, 1);
	eStepY = new QLineEdit("", NetWindow);
	eStepY->setAutoFillBackground(false);
	//eStepY->setCenterOnScroll (false );

	l->addWidget(eStepY, 3, 1, 1, 1);
	// eStepY->setGeometry( 100, 85 , 80,26 );
	//  выключены
	//eX0 = new QLineEdit("",NetWindow);
	//  eX0->setGeometry( 25, 85 , 60,20 );
	//l->addWidget(eX0,4,0,1,1);
	//eX0->hide();
	// eY0 = new QLineEdit("",NetWindow);
	//  eY0->setGeometry( 25, 130 , 60,20 );
	//l->addWidget(eY0,4,1,1,1);

	//eY0->hide();
	connect(eStepX, SIGNAL(editingFinished()), this, SLOT(StepXChanged()));
	connect(eStepY, SIGNAL(editingFinished()), this, SLOT(StepYChanged()));
	connect(BoxSqware, SIGNAL(stateChanged(int)), this, SLOT(SQNetChange(int)));

	//connect (eStepX, SIGNAL(textChanged (QString)), this, SLOT(StepXChanged()));
	//connect (eStepY, SIGNAL(textChanged (QString)), this, SLOT(StepYChanged()));
	//connect (SQNetChange, SIGNAL(stateChanged  (int)), this, SLOT(SQNetChange(int)));
	QDialogButtonBox *buttons = new QDialogButtonBox(NetWindow);

	//l->addWidget(buttons,5,0,1,2);
	l->addWidget(buttons, 5, 0, 1, 2);
	QPushButton *btnCancel = buttons->addButton(trUtf8("Отменить"), QDialogButtonBox::RejectRole);
	// btnCancel->setText(trUtf8("Отменить"));
	//btnCancel->setToolTip(trUtf8("Отменить"));
	// btnCancel->move(10,130 );
	// btnCancel->resize(60,20);
	// btnCancel->show();

	QPushButton *btnOK = buttons->addButton("   OK   ", QDialogButtonBox::AcceptRole);
	// QToolButton *btnOK = new QToolButton(buttons);
	// btnOK->setText(trUtf8("OK"));
	//btnOK->setToolTip(trUtf8("OK"));
	// btnOK->move(90,130 );
	// btnOK->resize(60,20);
	// btnOK->show();
	connect(btnOK, SIGNAL(clicked()), this, SLOT(NetOKClick()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(NetCancelClick()));
}
//--------------------------------------
void KumDraw::ShowNetWindow(void)
{
	if (SqwareNetFlag) {
		BoxSqware->setCheckState(Qt::Checked);
		eStepY->setVisible(false);
		lX->setText(QString::fromUtf8("Шаг сетки"));
		lY->setVisible(false);
	} else {
		BoxSqware->setCheckState(Qt::Unchecked);
		eStepY->setVisible(true);
		lX->setText(QString::fromUtf8("Шаг X"));
		lY->setVisible(true);
	}

	if (NetShowFlag) {
		BoxNet->setCheckState(Qt::Checked);
	} else {
		BoxNet->setCheckState(Qt::Unchecked);
	}


	char ctmp[200];
	QString tmp;

	//sprintf(ctmp,"%7.2f",SdwigNetX);
	//tmp = QString(ctmp);
	//eX0->setText(tmp);

	//sprintf(ctmp,"%7.2f",SdwigNetY);
	//tmp = QString(ctmp);
	//eY0->setText(tmp);

	sprintf(ctmp, "%7.2f", StepX);
	tmp = QString(ctmp);
	eStepX->setText(tmp);

	sprintf(ctmp, "%7.2f", StepY);
	tmp = QString(ctmp);
	eStepY->setText(tmp);

	QPalette PallRed, PallBlack;
	//QPalette Pall;
	//Pall.setColor(QPalette::BrightText,QColor(200,0,0));
	//Pall.setBrush(QPalette::WindowText,QBrush(QColor(200,0,0)));

	PallBlack.setBrush(QPalette::WindowText, QBrush(QColor(1, 1, 1)));
	PallBlack.setColor(QPalette::BrightText, QColor(1, 1, 1));

	PallRed.setBrush(QPalette::WindowText, QBrush(QColor(220, 0, 0)));
	PallRed.setColor(QPalette::BrightText, QColor(220, 0, 0));

	if (GoodNetFlag) {
		lX->setPalette(PallBlack);
		lY->setPalette(PallBlack);
	} else {
		lX->setPalette(PallRed);
		lY->setPalette(PallRed);
	}

	QPoint position = MV->pos();
	QSize diff = MV->size() - NetWindow->size();
	position += QPoint(diff.width() / 2, diff.height() / 2);
	NetWindow->move(position);
	NetWindow->show();
}

//--------------------------
/*
void KumDraw::NetSqware(int mode)
{
if (mode == 0)
SqwareNetFlag = false;
else
SqwareNetFlag = true;
qDebug("mode %i",SqwareNetFlag);
}
*/
//--------------------------------------------------------
void KumDraw::StepXChanged(void)
{
	//if (BoxSqware->isChecked())
	//{
	//eStepY->setText(eStepX->displayText());
	//}

	QPalette PallRed, PallBlack;

	PallBlack.setBrush(QPalette::WindowText, QBrush(QColor(1, 1, 1)));
	PallBlack.setColor(QPalette::BrightText, QColor(1, 1, 1));

	PallRed.setBrush(QPalette::WindowText, QBrush(QColor(220, 0, 0)));
	PallRed.setColor(QPalette::BrightText, QColor(220, 0, 0));

	bool ok;


	qreal NewStepX = eStepX->displayText().toFloat(&ok);

	qreal StepScreenX = WindowZoom * NewStepX;
	qDebug("StepChanged %f", StepScreenX);
	if (StepScreenX <= 3) {
		lX->setPalette(PallRed);
	} else {
		lX->setPalette(PallBlack);
	}

}
//---------------------
void KumDraw::StepYChanged(void)
{
	//  if (BoxSqware->isChecked())
	//  {
	//  eStepX->setText(eStepY->displayText());
	//  }
	QPalette PallRed, PallBlack;

	PallBlack.setBrush(QPalette::WindowText, QBrush(QColor(1, 1, 1)));
	PallBlack.setColor(QPalette::BrightText, QColor(1, 1, 1));

	PallRed.setBrush(QPalette::WindowText, QBrush(QColor(220, 0, 0)));
	PallRed.setColor(QPalette::BrightText, QColor(220, 0, 0));

	bool ok;


	qreal NewStepY = eStepY->displayText().toFloat(&ok);

	qreal StepScreenY = WindowZoom * NewStepY;
	qDebug("StepChanged %f", StepScreenY);
	if (StepScreenY <= 3) {
		lY->setPalette(PallRed);
	} else {
		lY->setPalette(PallBlack);
	}

}
//---------------------------------------------------------
void KumDraw::NetCancelClick(void)
{
	NetWindow->close();
}
//---------------------------------------------------------
void KumDraw::NetOKClick(void)
{
	if (BoxNet->isChecked()) {
		NetShowFlag = true;
	} else {
		NetShowFlag = false;
	}

	if (BoxSqware->isChecked()) {
		SqwareNetFlag = true;
	} else {
		SqwareNetFlag = false;
	}

	bool ok, ok1;


	StepX = eStepX->displayText().toFloat(&ok);
	StepY = eStepY->displayText().toFloat(&ok1);
	//SdwigNetX = eX0->displayText().toFloat(&ok2);
	//SdwigNetY = eY0->displayText().toFloat(&ok3);
	SdwigNetX = 0.;
	SdwigNetY = 0.;

	if (!(ok && ok1)) {
		QString tmp = QString::fromUtf8("Проверьте данные!");
		QMessageBox::warning(0, QString::fromUtf8("Ошибки ввода чисел"), tmp, 0, 0, 0);
		return;
	}
	if (SqwareNetFlag) {
		StepY = StepX;
	}

	NetWindow->close();
	DrawNet();
}

//---------------------------------------
void KumDraw::drawText(const QString &Text, qreal Scale)
{
	QString t = Text;
	KumDrawText  *KText = new KumDrawText();
	KText->x = CurX;
	KText->y = CurY;
	KText->text = t;
	KText->color.Red = CurColor.Red;
	KText->color.Green = CurColor.Green;
	KText->color.Blue = CurColor.Blue;
	KText->Size = Scale;

	QFont font("Courier", 12, Scale,  false);
	font.setPointSizeF(12.0);
	//font.setOverline(true);
	//QGraphicsTextItem *m_text = scene->addText(t,font);

	QFontMetrics fontMetric(font);
	qDebug() << "Rect Before Scale:" << fontMetric.boundingRect(t) << " x:" << fontMetric.width(t) << " Old Point Size:" << font.pointSizeF() ;
	double subScale = (fontMetric.width("X") * 0.1) / Scale;
	font.setPointSizeF(font.pointSizeF() / subScale);

	fontMetric = QFontMetrics(font);
	qDebug() << "Rect:" << fontMetric.boundingRect(t) << " x:" << fontMetric.width(t) << " SubScale:" << subScale;

	QGraphicsSimpleTextItem *m_text = scene->addSimpleText(t, font);
	//m_text->setDefaultTextColor(QColor(CurColor.Red,CurCoizelor.Green,CurColor.Blue));
	m_text->setBrush(QColor(CurColor.Red, CurColor.Green, CurColor.Blue));
	m_text->setPen(QColor(CurColor.Red, CurColor.Green, CurColor.Blue));
	qreal x0 = CurX;
	//- Scale*0.2; //Цифры подобраны экспериментально
	qreal y0 = -CurY - fontMetric.ascent() * 0.1; //-abs(fontMetric.boundingRect("X").bottom())*0.1;
	//-fontMetric.boundingRect("X").height()*0.1;//-CurY-Scale*1.4;
	qDebug() << "Rect X:" << fontMetric.boundingRect("X").bottom() << " ascent" << fontMetric.ascent(); //("X");
	m_text->setFont(font);
	texts.append(m_text);
	kumtexts.append(KText);
	QList<QGraphicsItem *> scene_items = scene->items();
#if 1
	scene_items.last()->setScale(0.1 * scene_items.last()->scale());
#else
	scene_items.last()->scale(0.1, 0.1);
#endif
	//scene_items.last()->scale(0.1*Scale,0.1*Scale);
	scene_items.last()->setPos(x0, y0);

	CurZ += 0.01;
	scene_items.last()->setZValue(CurZ);

	qreal TextWidth = t.length() * Scale;

	bool OldPenPosition = PenPosition;
	PenPosition = false;
	GoVector(TextWidth, 0);
	PenPosition = OldPenPosition;
	GoVector(0, 0);   // Чтобы цвет пера был правильным
	WindowRedraw();
}

//-------------------------------------
void KumDraw::CreateInfoWindow(void)
{
	QPalette PallBlue;
	PallBlue.setColor(QPalette::BrightText, QColor(0, 0, 100));
	PallBlue.setBrush(QPalette::WindowText, QBrush(QColor(0, 0, 200)));
	InfoWindow = new QDialog(MV, Qt::WindowSystemMenuHint);;
	QGridLayout *l = new QGridLayout(InfoWindow);
	InfoWindow->setLayout(l);
	InfoWindow->setWindowTitle(QString::fromUtf8("Информация"));
	//NetWindow->setWindowModality(Qt::ApplicationModal);
	InfoWindow->setWindowModality(Qt::ApplicationModal);
	((QDialog *)InfoWindow)->setModal(true);
	InfoWindow->setGeometry(40, 80, 210, 240);
	// InfoWindow->setMinimumSize(220,220);
	// InfoWindow->setMaximumSize(220,220);

	QLabel *lWin = new QLabel(QString::fromUtf8("<font color='blue'><b>ЧЕРТЕЖНИК</b></font>"), InfoWindow, 0);
	//  lWin->setGeometry( QRect(90, 1, 110, 20 ) );
	//  lWin->setPalette(PallBlue);
	l->addWidget(lWin, 0, 0, 1, 5, Qt::AlignHCenter);

	QLabel *lX = new QLabel(QString::fromUtf8("<font color='blue'>X<sub>МИН</sub>=</font>"), InfoWindow, 0);
	//  lX->setGeometry( QRect(10, 20, 110, 20 ) );
	//  lX->setPalette(PallBlue);
	l->addWidget(lX, 1, 0, 1, 1, Qt::AlignRight);
	QLabel *lY = new QLabel(QString::fromUtf8("<font color='blue'>Y<sub>МИН</sub>=</font>"), InfoWindow, 0);
	l->addWidget(lY, 1, 3, 1, 1, Qt::AlignRight);
	//  lY->setGeometry( QRect(110, 20,110, 20 ) );
	//  lY->setPalette(PallBlue);
	lX0 = new QLabel("1.12", InfoWindow, 0);
	//  lX0->setGeometry( QRect(60, 20,50, 20 ) );
	l->addWidget(lX0, 1, 1, 1, 1, Qt::AlignLeft);

	lX1 = new QLabel("1.12", InfoWindow, 0);
	//  lX1->setGeometry( QRect(60, 40,50, 20 ) );
	l->addWidget(lX1, 2, 1, 1, 1, Qt::AlignLeft);

	QLabel *lXX1 = new QLabel(QString::fromUtf8("<font color='blue'>X<sub>МАКС</sub>=</font>"), InfoWindow, 0);
	//  lXX1->setGeometry( QRect(10, 40, 110, 20 ) );
	//  lXX1->setPalette(PallBlue);
	l->addWidget(lXX1, 2, 0, 1, 1, Qt::AlignRight);
	QLabel *lYY1 = new QLabel(QString::fromUtf8("<font color='blue'>Y<sub>МАКС</sub>=</font>"), InfoWindow, 0);
	//  lYY1->setGeometry( QRect(110, 40,110, 20 ) );
	//  lYY1->setPalette(PallBlue);
	l->addWidget(lYY1, 2, 3, 1, 1, Qt::AlignRight);
	lY0 = new QLabel("1.12", InfoWindow, 0);
	//  lY0->setGeometry( QRect(160, 20,50, 20 ) );
	l->addWidget(lY0, 1, 4, 1, 1, Qt::AlignLeft);
	lY1 = new QLabel("1.12", InfoWindow, 0);
	//  lY1->setGeometry( QRect(160, 40,50, 20 ) );
	l->addWidget(lY1, 2, 4, 1, 1, Qt::AlignLeft);

	QLabel *lXXCen = new QLabel(QString::fromUtf8("<font color='blue'>X<sub>ЦЕН</sub>=</font>"), InfoWindow, 0);
	//  lXXCen->setGeometry( QRect(10, 60, 110, 20 ) );
	//  lXXCen->setPalette(PallBlue);
	l->addWidget(lXXCen, 3, 0, 1, 1, Qt::AlignRight);
	QLabel *lYYCen = new QLabel(QString::fromUtf8("<font color='blue'>Y<sub>ЦЕН</sub>=</font>"), InfoWindow, 0);
	//  lYYCen->setGeometry( QRect(110, 60,110, 20 ) );
	//  lYYCen->setPalette(PallBlue);
	l->addWidget(lYYCen, 3, 3, 1, 1, Qt::AlignRight);
	lXCen = new QLabel("1.12", InfoWindow, 0);
	//  lXCen->setGeometry( QRect(60, 60,50, 20 ) );
	l->addWidget(lXCen, 3, 1, 1, 1, Qt::AlignLeft);
	lYCen = new QLabel("1.12", InfoWindow, 0);
	//  lYCen->setGeometry( QRect(160, 60,50, 20 ) );
	l->addWidget(lYCen, 3, 4, 1, 1, Qt::AlignLeft);
	QLabel *lXXNet = new QLabel(QString::fromUtf8("<font color='blue'>ШАГ СЕТКИ</font>"), InfoWindow, 0);
	//  lXXNet->setGeometry( QRect(10, 90, 110, 20 ) );
	//  lXXNet->setPalette(PallBlue);
	l->addWidget(lXXNet, 5, 0, 1, 2, Qt::AlignRight);
	QLabel *lXXNetUzX0 = new QLabel(QString::fromUtf8("Узел X0="), InfoWindow, 0);
	//  lXXNetUzX0->setGeometry( QRect(10, 110, 110, 20 ) );
	lXXNetUzX0->setPalette(PallBlue);
	lXXNetUzX0->hide();
	QLabel *lXXNetUzY0 = new QLabel(QString::fromUtf8("Y0="), InfoWindow, 0);
	lXXNetUzY0->setGeometry(QRect(130, 110, 110, 20));
	lXXNetUzY0->setPalette(PallBlue);
	lXXNetUzY0->hide();
	//QLabel *lXXNetUzDX = new QLabel(QString::fromUtf8(" DX="),InfoWindow,0);
	//  lXXNetUzDX->setGeometry( QRect(10, 110, 110, 20 ) );
	//  lXXNetUzDX->setPalette(PallBlue);
	//QLabel *lXXNetUzDY = new QLabel(QString::fromUtf8("DY="),InfoWindow,0);
	//  lXXNetUzDY->setGeometry( QRect(130, 110, 110, 20 ) );
	//  lXXNetUzDY->setPalette(PallBlue);

	lXNetX0 = new QLabel("1.12", InfoWindow, 0);
	lXNetX0->setGeometry(QRect(70, 110, 50, 30));
	lXNetX0->hide();
	lYNetY0 = new QLabel("1.12", InfoWindow, 0);
	lYNetY0->setGeometry(QRect(160, 110, 50, 30));
	lYNetY0->hide();

	lXNetDX = new QLabel("1.12", InfoWindow, 0);
	//  lXNetDX->setGeometry( QRect(90, 90,50, 20 ) );
	l->addWidget(lXNetDX, 5, 3, 1, 1, Qt::AlignLeft);

	lYNetDY = new QLabel("1.12", InfoWindow, 0);
	//  lYNetDY->setGeometry( QRect(140, 90,50, 20 ) );
	l->addWidget(lYNetDY, 5, 4, 1, 1, Qt::AlignLeft);

	//lZ0 = new QLabel("1.12",InfoWindow,0);
	//lZ0->setGeometry( QRect(120, 80,50, 20 ) );


	lColor = new QLabel("цвет", InfoWindow, 0);
	//  lColor->setGeometry( QRect(10, 130,120, 20 ) );
	l->addWidget(lColor, 7, 0, 1, 5, Qt::AlignHCenter);

	lPen = new QLabel("", InfoWindow, 0);
	//  lPen->setGeometry( QRect(10, 150,120, 20 ) );
	l->addWidget(lPen, 8, 0, 1, 5, Qt::AlignHCenter);

	QDialogButtonBox *buttons = new QDialogButtonBox(InfoWindow);
	l->addWidget(buttons, 10, 0, 1, 5);

	QPushButton *btnOK = buttons->addButton("OK", QDialogButtonBox::AcceptRole);


	// btnOK->setText(trUtf8("OK"));
	//btnOK->setToolTip(trUtf8("OK"));
	//btnOK->move(80,175);
	// btnOK->resize(60,20);
	// btnOK->show();
	connect(btnOK, SIGNAL(clicked()), this, SLOT(InfoOKClick()));

	//  InfoWindow->move(40,80);
	InfoWindow->setMinimumSize(InfoWindow->size() + QSize(50, 50));
	InfoWindow->setMaximumSize(InfoWindow->size() + QSize(50, 50));
	//InfoWindow->show();
}

//---------------------------------------------------------
void KumDraw::InfoOKClick(void)
{
	InfoWindow->close();
}
//-----------------------------------------
void KumDraw::CreateXYLabel(void)
{
	lXCur = new QLabel(QString::fromUtf8("X= "), coordFrame, 0);
	lXCur->setGeometry(QRect(5, 2, 80, 20));
	lXCur->hide();

	lYCur = new QLabel(QString::fromUtf8("Y= "), coordFrame, 0);
	lYCur->setGeometry(QRect(5, 14, 80, 20));
	lYCur->hide();
	//qDebug("XYYYYYYYY");
}
//---------------------------------------------
void KumDraw::ShowCoord(qreal xScene, qreal yScene)
{
	QString tmp;
	char ctmp[50];
	sprintf(ctmp, "X = %7.4f", xScene);
	tmp = QString(ctmp);
	//QMessageBox::information( 0, "", tmp, 0,0,0);
	lXCur->setText(tmp);
	sprintf(ctmp, "Y = %7.4f", yScene);
	tmp = QString(ctmp);
	lYCur->setText(tmp);

	QPalette PallBlue;
	PallBlue.setColor(QPalette::BrightText, QColor(1, 1, 1));
	PallBlue.setBrush(QPalette::WindowText, QBrush(QColor(1, 1, 1)));
	lXCur->setPalette(PallBlue);
	lYCur->setPalette(PallBlue);
	coordFrame->show();
	lXCur->show();
	lYCur->show();
}
//-------------------------------------
void KumDraw::HideCoord(void)
{

	coordFrame->hide();
	lXCur->hide();
	lYCur->hide();

}
//-------------------------------------
void KumDraw::CreateCoordFrame()
{
	coordFrame = new QFrame(view);
	coordFrame->setLineWidth(2);
	coordFrame->setFrameShadow(QFrame::Sunken);
	coordFrame->setFrameShape(QFrame::Panel);
	coordFrame->move(100, 100);
	coordFrame->hide();
}
//coordFrame->setGeometry(QRect(50,40,80,30));

void KumDraw::SQNetChange(int state)
{
	if (state != 0) {
		//BoxSqware->setCheckState(Qt::Checked);
		eStepY->setVisible(false);
		lX->setText(QString::fromUtf8("Шаг сетки"));
		lY->setVisible(false);
	} else {
		//BoxSqware->setCheckState(Qt::Unchecked);
		eStepY->setVisible(true);
		lX->setText(QString::fromUtf8("Шаг X"));
		lY->setVisible(true);
	}



};
//------------------------------------------------
void KumDraw::SdwigHelp()
{
	//qDebug("help");
	HelpWindow = new QDialog(MV, Qt::WindowSystemMenuHint);;
	QGridLayout *l = new QGridLayout(HelpWindow);
	HelpWindow->setLayout(l);
	HelpWindow->setWindowTitle(QString::fromUtf8("Сдвиги"));
	//NetWindow->setWindowModality(Qt::ApplicationModal);
	HelpWindow->setWindowModality(Qt::ApplicationModal);
	((QDialog *)InfoWindow)->setModal(true);
	HelpWindow->setGeometry(40, 80, 410, 140);



	QLabel *lWin = new QLabel(QString::fromUtf8("<font color='blue'><b>Чтобы переместить картинку, нажмите левую кнопку и передвигайте мышь.</b></font>"), HelpWindow, 0);
	l->addWidget(lWin, 0, 0, Qt::AlignTop);
	QLabel *lWin1 = new QLabel(QString::fromUtf8("<font color='blue'><b>Чтобы приблизить или удалить картинку, покрутите колесо мыши.</b></font>"), HelpWindow, 0);
	l->addWidget(lWin1, 2, 0, Qt::AlignTop);

	QLabel *lWin2 = new QLabel(QString::fromUtf8("<font color='blue'><b>Чтобы узнать координаты точки, нажмите правую кнопку мыши</b></font>"), HelpWindow, 0);
	l->addWidget(lWin2, 6, 0, Qt::AlignTop);

	QPoint position = MV->pos();
	QSize diff = MV->size() - HelpWindow->size();
	position += QPoint(diff.width() / 2, diff.height() / 2);
	HelpWindow->move(position);

	HelpWindow->show();

}


void KumDraw::SwitchMode(int mode)
{
	if ((mode == 1) && (curMode != 1)) {
		rreset();
	}
	curMode = mode;
	emit sync();
}

void KumDraw::setWindowPos(int x, int y)
{
	w_window->move(x, y);
}

void KumDraw::getWindowPos(int *x, int *y)
{
	*x = w_window->x();
	*y = w_window->y();
}

QRect KumDraw::getWindowGeometry() const
{
	return w_window->geometry();
}

void KumDraw::rreset()
{
	PenPosition = false;
	FromDubl();
	SetColorString(QString::fromUtf8("черный"));
	GoPoint(0, 0);
}



