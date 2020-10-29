#ifndef KUMROBOT_H
#define KUMROBOT_H

#include "kum_instrument.h"
#include <QGraphicsObject>
class QMainWindow;
class QToolButton;


class SimpleRobot: public QGraphicsObject
{
	Q_OBJECT;

public:
	SimpleRobot(QGraphicsItem *parent = 0);
	QRectF boundingRect() const;

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget);
	QGraphicsPolygonItem *RoboItem();
	void setCrash(uint dirct);
	void move(QPoint point);
signals:
	void moved(QPointF point);
protected:
	void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
	void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
private:

	QGraphicsPolygonItem *Robot;
	bool ready;
	uint crash;

};





//Элемент поля робота
class FieldItm: public QWidget
{
	Q_OBJECT
public:

	FieldItm(QWidget *parent = 0, QGraphicsScene *scene = 0);


	~FieldItm();
	FieldItm *Copy();
	void cleanSelf();


	void setLeftsepItem(FieldItm *ItmLeft);

	void setRightsepItem(FieldItm *ItmRight);

	void setUpsepItem(FieldItm *ItmUp);

	void setDownsepItem(FieldItm *ItmDown);

	void setWalls(int wallByte);

	int wallByte();

	void setUpLine(QGraphicsLineItem *Line, QPen pen);

	void showCharMark(qreal upLeftCornerX, qreal upLeftCornerY, int size);
	void showUpChar(qreal upLeftCornerX, qreal upLeftCornerY, int size);
	void showDownChar(qreal upLeftCornerX, qreal upLeftCornerY, int size);

	void showMark(qreal upLeftCornerX, qreal upLeftCornerY, int size);

	void setColorRect(QGraphicsRectItem *Rect);
	void setDownLine(QGraphicsLineItem *Line, QPen pen);
	void setLeftLine(QGraphicsLineItem *Line, QPen pen);
	void setRightLine(QGraphicsLineItem *Line, QPen pen);
	bool hasUpWall();
	bool hasDownWall();

	bool hasLeftWall();
	bool hasRightWall();

	bool canUp();
	bool canDown();
	bool canLeft();
	bool canRight();
	bool hasUpSep();
	bool hasDownSep();

	bool hasLeftSep();
	bool hasRightSep();
	QGraphicsLineItem *UpWallLine();
	QGraphicsLineItem *DownWallLine();
	QGraphicsLineItem *LeftWallLine();
	QGraphicsLineItem *RightWallLine();
	bool isColored();

	void removeUpWall();

	void setUpWall(QGraphicsLineItem *Line, QPen pen);
	void setDownWall(QGraphicsLineItem *Line, QPen pen);
	void removeDownWall();

	void removeLeftWall();
	void removeRightWall();



	void setLeftWall(QGraphicsLineItem *Line, QPen pen);
	void setRightWall(QGraphicsLineItem *Line, QPen pen);

	void removeColor();

	void removeMark();

	void removeUpChar();

	void removeDownChar();
	bool emptyCell();
	void setScene(QGraphicsScene *scene);
	void wbWalls();
	void colorWalls();
	void setTextColor()
	{
		TextColor = QColor(app()->settings->value("Robot/TextColor", "#FFFFFF").toString());
	};

public:
	bool IsColored;
	float radiation;
	float temperature;
	QChar upChar;
	QChar downChar;
	bool mark;
	bool upWall;
	bool downWall;
	bool rightWall;
	bool leftWall;
private:

	QFont font;


	QPen wallPen;
	QGraphicsScene *Scene;
	QGraphicsLineItem *upWallLine;
	QGraphicsLineItem *downWallLine;
	QGraphicsLineItem *leftWallLine;
	QGraphicsLineItem *rightWallLine;
	QGraphicsRectItem *ColorRect;

	QGraphicsTextItem *upCharItm;
	QGraphicsTextItem *downCharItm;
	QGraphicsTextItem *markItm;
	QColor TextColor;

	FieldItm *sepItmUp;
	FieldItm *sepItmDown;
	FieldItm *sepItmLeft;
	FieldItm *sepItmRight;

};

class RoboField: public QGraphicsScene
{
	Q_OBJECT
public:
	RoboField(QWidget *parent, class KumRobot *robot_module);
	RoboField *Clone();
	~RoboField();

public:
	void drawField(uint cellSize);//TODO Document
	void destroyField();

	void destroyRobot();

	/**
	* Количество строк поля робота
	* @return количество строк
	*/
	inline int rows() const
	{
		return Items.count();
	}
	/**
	* Количество столбцов поля робота
	* @return количество столбцов
	*/
	inline int columns() const
	{
		return Items.isEmpty() ? 0 : Items.last().size();
	}
	void drawNet();
	void setItem(FieldItm *item, uint str, uint stlb);
	void destroyNet();
	void destroyScene();
	void createField(int shirina, int visota);
	FieldItm *getFieldItem(int str, int stlb);
	void debug();

	int loadFromFile(QString fileName);
	int saveToFile(QString fileName);
	void createRobot();
	void UpdateColors();

	/**
	* Удаляет / ставит верхнюю стену
	* @param row Ряд
	* @param col столбец
	*
	*/
	void reverseUpWall(int row, int col);

	/**
	* Удаляет / ставит нижнюю стену
	* @param row Ряд
	* @param col столбец
	*
	*/
	void reverseDownWall(int row, int col);
	void reverseLeftWall(int row, int col);
	void reverseRightWall(int row, int col);

	void reverseColor(int row, int col);
	void reverseColorCurrent();

	void reverseMark(int row, int col);



	inline QList<QList<FieldItm * > > FieldItems()
	{
		return Items;
	}
	void setFieldItems(QList<QList<FieldItm * > > FieldItems);


	bool stepUp();
	bool stepDown();
	bool stepLeft();
	bool stepRight();
	void editField();
	void setEditMode(bool EditMode)
	{
		editMode = EditMode;
		LineColor = QColor(app()->settings->value("Robot/LineColor", "#C8C800").toString());
		WallColor = QColor(app()->settings->value("Robot/WallColor", "#C8C800").toString());
		EditColor = QColor(app()->settings->value("Robot/EditColor", "#00008C").toString());
		NormalColor = QColor(app()->settings->value("Robot/NormalColor", "#289628").toString());
	}
	void showCellDialog(FieldItm *cellClicked);
	inline FieldItm *currentCell()
	{
		return getFieldItem(robo_y, robo_x);
	}
	inline FieldItm *cellAt(int x, int y)
	{
		return getFieldItem(x, y);
	}

	inline void setMarkMode(bool isColor)
	{
		markMode = isColor;
	}

	inline bool WasEdit() const
	{
		return wasEdit;
	}

	void setRoboPos(int roboX, int roboY);
	inline int robotX() const
	{
		return robo_x;
	}
	inline int robotY() const
	{
		return robo_y;
	}


	inline void dropWasEdit()
	{
		wasEdit = false;
	}
	SimpleRobot *robot;
	void wbMode();
	void colorMode();
	public

slots:
	void roboMoved(QPointF pos);
	void cellDialogOk();
signals:
	void MousePress(qreal x, qreal y, bool Flag);
	void was_edit();



private:

	void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	QList<QList<FieldItm * > > Items;
	QList<QGraphicsLineItem *> setka;
	//QGraphicsView * scena;
	bool editMode, markMode, wasEdit;
	QColor LineColor, WallColor, EditColor, NormalColor, Color;
	uint fieldSize;
	uint robo_x, robo_y;
	QWidget *Parent;
	QPen BortLine, StLine;
	CellDialog *cellDialog;
	bool WasEditFlag;
	//TOOLS
	QPoint upLeftCorner(int str, int stlb);
	KumRobot *m_robot;
};

class KumRobot
	: public KumInstrument

{
	friend class Application;
	Q_OBJECT

	struct RobotFieldItem {

		~RobotFieldItem()
		{
			m_ErrorWall = 0;
			if (m_t) {
				delete m_t;
			}
		};
		char m_Wall;
		char m_ErrorWall;  // Walls
		double m_Radiation;     // Radiation value
		bool m_Is_Colored;   // If the Field Item Colored
		QChar m_Text ; // буква в левом верхнем углу
		QChar m_Text1; // буква в левом нижнем углу
		bool Point;  // точка в правом нижнем углу
		double m_Temperature;
		QGraphicsRectItem *m_t;
	};


public:
#ifdef Q_OS_MAC
	//  inline void setWindowMenuBar(QWidget *menuBar) { robotWindow->setMenuWidget(menuBar); }
#endif
	/**
	             * Конструктор
	             * @param parent сслыка на объект-владелец
	             */
	KumRobot(QWidget *parent = 0);
public:
	int getFunctionList(function_table *Functions, symbol_table *Symbols);
	bool init();
	inline bool isReady() const
	{
		return ready;
	}

	inline void setMode(bool EditMode)
	{
		isEditor = EditMode;
	}
	void setRobotStartField(RoboField *StartField);


	void restoreRobotStartField();
	int SaveToFile(QString p_FileName);
	int LoadFromFile(QString p_FileName);

	void ColorCell(int, int);
	void UnColorCell(int, int);
	void MapCell(int ix, int iy);

	bool IsColored(int ix, int iy);
	bool LeftWall(int ix, int iy);
	bool RightWall(int ix, int iy);
	bool UpWall(int ix, int iy);
	bool DownWall(int ix, int iy);

	bool LeftErrorWall(int ix, int iy);
	bool RightErrorWall(int ix, int iy);
	bool UpErrorWall(int ix, int iy);
	bool DownErrorWall(int ix, int iy);

	//bool LeftFree();
	//bool RightFree();
	//bool UpFree();
	//bool DownFree();
	void SetLeftWall();
	void SetRightWall();
	void SetUpWall();
	void SetDownWall();
	void SetLeftErrorWall();

	void SetRightErrorWall();
	void SetUpErrorWall();
	void SetDownErrorWall();
	void UpdateColors();


	void SetSceneSize(uint dx, uint dy);


	void DeleteLeftWall();
	void DeleteRightWall();
	void DeleteUpWall();
	void DeleteDownWall();
	double GetRadiation(int ix, int iy);
	double GetTemperature(int ix, int iy); // NEW V.K. 05-10
	QChar GetBukva(int ix, int iy); // NEW V.K. 05-10
	QChar GetBukva1(int ix, int iy); // NEW V.K. 05-10
	void SetTemperature(int ix, int iy, double); // NEW V.K. 05-10
	void SetRadiation(int ix, int iy, double); // NEW V.K. 05-10
	void SetBukva(int ix, int iy, QChar Bukv);
	void SetBukva1(int ix, int iy, QChar Bukv);
	bool SetEditTemperature(); // NEW V.K. 05-10
	bool SetEditRadiation();// NEW V.K. 05-10
	bool SetEditBukva();
	bool SetEditBukva1();
	bool WasEdit();
	void SetRobotMode(int mode);
	void PrepareRunMode();



public slots:
	void setStartEnvironment(const QString &);
	void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err);
	void fieldEdit();
	void NewRobotField();
	void LoadFromFileActivated();
	void ToInitialPos();
	bool StepLeft();
	bool StepRight();
	bool StepUp();
	bool StepDown();
	void ReversLeftWall();
	void ReversRightWall();
	void ReversUpWall();
	void ReversDownWall();
	void ReversColor();
	void close();

	void lockControls(void);
	void unLockControls(void);
	void MousePress(qreal x, qreal y, bool Flag);
	void ToDubl();
	void FromDubl();
	void ReverseEditFlag();
	void ReversePult();
	void SaveToFile1();
	void SaveToFileActivated();
	void SaveToFileCopy();
	void SaveCurrent();
	void btnOKClick();
	void btnCancelClick();
	void btnOK1Click();
	void btnCancel1Click();

	void FindRobot();
	void btnRobotClick();

	void btnLeftClick();
	void btnRightClick();
	void btnUpClick();
	void btnDDownClick();
	void btnCenterClick();

	void btnLeft1Click();
	void btnRight1Click();
	void btnUp1Click();
	void btnCenter1Click();
	void btnDown1Click();
	void Print2PDF();
	void btnLeft2Click();
	void btnRight2Click();
	void btnUp2Click();
	void btnCenter2Click();
	void btnDown2Click();
	void ShowTemperature1();
	void ShowRadiation1();
	void CloseInst();
	void reset(void);

	void showHideWindow(bool show = false);

	void setWindowPos(int x, int y);
	void getWindowPos(int *x, int *y);
	inline QRect getWindowGeometry() const
	{
		return robotWindow->geometry();
	}
	void SwitchMode(int mode);
	void  ajustWindowSize();

signals:
	void sync();
	void FieldEdit();
	void pultCommand(QString text);
protected:
	class SecondaryWindow *w_window;
	class SecondaryWindow *w_pultWindow;
	RobotWindow *robotWindow;  //Okno robota
	RoboField *field;
	class RobotPultWK *roboPult;
	//RoboField* fieldClone;
	RoboField *startField;

	// QMainWindow* MV;
	QMainWindow *PW;
	int PWWidth, PWHieght;
	QMainWindow *InputWindow;
	QWidget *NewWindow;
	QMenu *menu;
	QMenu *PopMenu;
	function_table *functions;
	symbol_table *symbols;
	QTextCodec *codec;
	bool ready;
	//KumScene *scene;
	class KumScene *old_scene;
	//QGraphicsScene *Panel;
	//QGraphicsView *view;
	//QGraphicsView *Pview;
	QList<QGraphicsLineItem *> lines;


	QToolButton *btnLeft;
	QToolButton *btnRight;
	QToolButton *btnUp;
	QToolButton *btnDDown;
	QToolButton *btnCenter;
	QToolButton *btnLeft1;
	QToolButton *btnRight1;
	QToolButton *btnUp1;
	QToolButton *btnDown1;
	QToolButton *btnCenter1;

	QToolButton *btnLeft2;
	QToolButton *btnRight2;
	QToolButton *btnUp2;
	QToolButton *btnDown2;
	QToolButton *btnCenter2;

	QToolButton *btnOK;
	QToolButton *btnCancel;
	QToolButton *btnSaveCurrent;

	QToolButton *BtnNewField;
	QToolButton *BtnLoadFromFile;
	QToolButton *BtnSaveAs;
	//QToolButton *BtnSave;
	//QToolButton *BtnSaveCopy;
	QToolButton *BtnEdit;
	QToolButton *BtnPult;
	//QToolButton *BtnSaveStart;
	//QToolButton *BtnLoadStart;


	//QToolButton *btnRobot;
	QCheckBox *btnRobot;
	QCheckBox *btnMetka;
	//QToolButton *btnFind;
	QLabel *lX;
	QLabel *lY;
	/* MR-41
	    QLabel *lColored;
	    QLabel *lUnColored;
	    QLabel *lColored1;
	    QLabel *lUnColored1;
	    QLabel *lpTemp;
	    QLabel *lpRad;
	    QLabel *lpShag;
	*/
	QLabel *lpShag;
	QLabel *lOtkaz;
	QLabel *lAction;
	//QAction* MakeDubl;
	//QAction* RestoreDubl;
	//QAction* SaveRobot1;
	QAction *NewRobot;
	QString PathToBaseFil;
	QAction *LoadRobot;
	QAction *SaveRobot;
	QAction *Edit;
	//QAction* MFindRobot;
	QAction *MPult;


	QPalette PallRed, PalErr;

	void loadEnveroment_activated();

	void CreateFieldDubl(int p_X, int p_Y);
	void CreateRobot();
	void DrawField();
	void CreatebtnLeft(int H0);
	void CreatebtnRight(int H0);
	void CreatebtnUp(int H0);
	void CreatebtnDDown(int H0);
	void CreatebtnCenter(int H0);
	void CreatebtnLeft1(int H0);
	void CreatebtnRight1(int H0);
	void CreatebtnUp1(int H0);
	void CreatebtnDown1(int H0);
	void CreatebtnCenter1(int H0);
	void CreatebtnLeft2(int H0);
	void CreatebtnRight2(int H0);
	void CreatebtnUp2(int H0);
	void CreatebtnDown2(int H0);
	void CreatebtnCenter2(int H0);
	//void CreatebtnFind();

	void CreatebtnOK();
	void CreatebtnRobot();
	void CreatebtnMetka();
	void CreatebtnCancel();
	void CreateButtons(int H0);
	void DeleteButtons();
	void DeleteEdits();
	void CreateEdits();
	void ShowTemperature();
	void ShowRadiation();

	void ShowBukva();
	void ShowBukva1();
	bool EmptyCell(int ix, int iy);
	void CreateNewField();
	void CreatePult(QWidget *);
	void ClearLabels();
	bool isRobotInView();
	void LoadInitialFil(QString FileName);
	void SetPultTexts();

	void CreateMainButtons();
	void CreateBtnNewField();
	void CreateBtnLoadFromFile();
	void CreateBtnSaveAs();
	//void CreateBtnSave();
	//void CreateBtnSaveCopy();
	void CreateBtnEdit();
	void CreateBtnPult();
	//void CreateBtnSaveStart();
	//void CreateBtnLoadStart();
	void DisableMainButtons();


	int CurrentRobotMode;
	RobotFieldItem *m_Field; // Field Data Pointer
	RobotFieldItem *m_FieldDubl; // Field Data Pointer
	RobotFieldItem *GetFieldItem(int p_X, int p_Y);
	RobotFieldItem *GetFieldItemDubl(int p_X, int p_Y);
//    pultLogger * Logger;
	int m_Size_xDubl;
	int m_Size_yDubl;
	bool RepaintRobot();
	void DisableButtons();
	void EnableButtons();
	int m_Size_x; // Sizes of the
	int m_Size_y; // field
	int m_x;
	int m_y;
	int SizeX, SizeY; //размеры окна в пикселах
	int InitialX, InitialY;
	uint m_FieldSize;
	uint MenuHigth;
	uint PanelHigth;
	uint SceneSizeX, SceneSizeY;
	//  QGraphicsRectItem *m_Robot;
	QGraphicsPolygonItem *m_Robot;
	bool DublFlag;
	int RobotXDubl;
	int RobotYDubl;
	bool lockFlag;
	bool isEditor;
	bool WallErrorFlag;
	int Mouseix;
	int Mouseiy;
	int ButtonSdvig;
	QLineEdit *eTemp;
	QLineEdit *eRad;
	QLineEdit *eBukv;
	QLineEdit *eBukv1;
	QSpinBox *eXSizeEdit;
	QSpinBox *eYSizeEdit;
	PrintDialog *pDialog;
	//QLineEdit *eXSizeEdit;
	//QLineEdit *eYSizeEdit;

	int OldMapedCellX;
	int OldMapedCellY;
	int tmpm_x;
	int tmpm_y;
	int MainButtonY;
	bool WasEditFlag;
	QColor LineColor;
	QColor EditColor;
	QColor NormalColor;
	QColor WallColor;

	QString CurrentFileName;
	QString curDir;

	void createWindowsLabelsActions();
};



#endif // KUMROBOT_H
