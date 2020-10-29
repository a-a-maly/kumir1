#ifndef KUMDRAW_H
#define KUMDRAW_H

#include "kum_instrument.h"
#include <QGraphicsView>
class QToolButton;



class DrawView : public QGraphicsView
{
public:
	DrawView(QGraphicsScene *, QWidget *);
protected:
	void scrollContentsBy(int dx, int dy);
};

//--------------------------------------------------------
class KumDraw : public KumInstrument
{
	Q_OBJECT
public:
	int getFunctionList(function_table *Functions, symbol_table *Symbols);
public:

	//       * Конструктор
	KumDraw(QWidget *parent = 0);
	void PenDown();
	void PenUp();
	int GoVector(double x, double y);
	int GoPoint(double x, double y);
	void Reset();
	void closeEvent(QCloseEvent *e);
	void SetWindowSize(int x1, int y1, int x2, int y2);
	QPoint lastPos;
	void drawText(const QString &Text, qreal Scale);
	//void show();
	//void hide();
	//void SetWindowPosition(double x0,double y0, double zoom);

	bool init();
	inline bool isReady() const
	{
		return ready;
	}

	inline void setMode(bool EditMode)
	{
		isEditor = EditMode;
	}

public slots:
	void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err);
	void lockControls();
	void unlockControls();
	void LoadFromFileActivated();
	void SaveToFileActivated();
	void ClearPicture();
	void ToDubl();
	void FromDubl();
	void ToWindow(void);
	void MoveLeft(void);
	void MoveRight(void);
	void MoveUp(void);
	void MoveDown(void);
	void ZoomUp(void);
	void ZoomDown(void);
	bool PrintToPDF(void);
	void close();
	void ToCenter(void);
	void ToLeftDownCorner();
	void Info();
	void MousePress(qreal x, qreal y, bool Flag, qreal xScene, qreal yScene);
	void MouseRelease(qreal x, qreal y, bool Flag);
	void MouseMove(qreal x, qreal y, bool Flag);
	void MouseWheel(int Delta);
	void HorizWindow();
	void VertWindow();
	void ShowNetWindow();
	//void NetSqware(int mode);
	void StepXChanged(void);
	void StepYChanged(void);
	void NetCancelClick(void);
	void NetOKClick(void);
	void InfoOKClick(void);
	void SdwigHelp(void);
	void SQNetChange(int state);


	inline void CloseInst(void) {}

	void SwitchMode(int mode);
	void showHideWindow(bool show = true); //Show - true

	void setWindowPos(int x, int y);

	void getWindowPos(int *x, int *y);
	QRect getWindowGeometry() const;
signals:
	void sync();



private:
	void rreset();
	struct LColor {
		int Red;
		int Green;
		int Blue;
	};

	struct KumDrawText {
		qreal x;
		qreal y;
		QString text;
		LColor color;
		qreal Size;
	};

	int SizeX, SizeY; //размеры окна в пикселах

	QWidget *MV;
	QMenu *menu;
	QMenu *menu1;
	QFrame *topFrame;
	class KumScene *scene;
	uint MenuHigth;
	QGraphicsScene *Panel;
	QGraphicsView *view;
	QGraphicsView *Pview;
	QList<QGraphicsLineItem *> lines;
	QList<QGraphicsLineItem *> Netlines;
	QList<QGraphicsLineItem *> linesDubl;
	//QList<QGraphicsTextItem*> texts;
	QList<QGraphicsSimpleTextItem *> texts;
	QList<KumDrawText *> kumtexts;
	qreal PenXDubl;
	qreal PenYDubl;
	bool PenPositionDubl;
	QGraphicsPolygonItem *mPen;
	double WindowX0;
	double WindowY0;
	double WindowZoom;
	PrintDialog *pDialog;
	//QLineEdit *eX0;
	//QLineEdit *eY0;
	QLineEdit *eStepX;
	QLineEdit *eStepY;
	//QPlainTextEdit *eStepY;
	QCheckBox *BoxNet;
	QLabel *lX0, *lY0, *lX1, *lY1, *lXCen, *lYCen, *lXNetX0, *lYNetY0, *lXNetDX, *lYNetDY, *lXCur, *lYCur, *lColor, *lPen, *lX, *lY;
	// *lZ0;
	QAction *HorizPicture;
	QAction *VertPicture;
	qreal StepX; //шаг сетки,
	qreal StepY;
	bool SqwareNetFlag;
	qreal SdwigNetX; //сдвиг сетки
	qreal SdwigNetY;
	bool NetShowFlag;
	bool TmpNetShowFlag;

	int NLastVec;
	LColor CurColor;
	QString CurColorName;
	double CurX; //
	double CurY;
	double CurZ;
	bool PenPosition;
	bool isEditor;
	bool ready;
	symbol_table *symbols;
	qreal MinInfinityX;
	qreal MaxInfinityX;
	qreal MinInfinityY;
	qreal MaxInfinityY;
	qreal MaxZoom;
	qreal MinZoom;
	qreal ButtonSdwig;
	int WindowSizeX;
	int WindowSizeY;
	int MVxmin, MVxmax, MVymin, MVymax;
	QWidget *NetWindow;
	QWidget *InfoWindow;
	QWidget *HelpWindow;
	QCheckBox *BoxSqware;
	function_table *functions;

	QToolButton *btn11;
	QToolButton *btn12;
	QToolButton *btn13;
	QToolButton *btn21;
	QToolButton *btn22;
	QToolButton *btn23;
	QToolButton *btn31;
	QToolButton *btn32;
	QToolButton *btn33;
	QFrame *coordFrame;
	QString curDir;
	qreal OldX, OldY;
	bool GoodNetFlag;
	//qreal VertWindowSdvig; // сдвиг картинки в вертикальном окне

	bool LeftMousePressFlag;
	bool RightMousePressFlag;
	void Createbtn11();
	void Createbtn12();
	void Createbtn13();
	void Createbtn21();
	void Createbtn22();
	void Createbtn23();
	void Createbtn31();
	void Createbtn32();
	void Createbtn33();
	void CreateXYLabel();
	void ShowCoord(qreal xScene, qreal yScene);
	void HideCoord();
	void WindowRedraw();
	void DrawNet();
	float GetMinX(void);
	float GetMinY(void);
	float GetMaxX(void);
	float GetMaxY(void);


	//void WriteLine(QFile *p_File, QString p_String);
	void SetLineColor(int LRed, int LGreen, int LBlue);
	bool SetColorString(QString Color);
	//void ShowInfo();

	void CreatePen(void);
	//void DestroyPen(void);
	int LoadFromFile(QString p_FileName);
	int SaveToFile(QString p_FileName);
	void CreateMenu(void);
	void CreateNetWindow(void);
	void CreateInfoWindow(void);
	void CreateCoordFrame();
	int curMode;
	class SecondaryWindow *w_window;
};

#endif // KUMDRAW_H
