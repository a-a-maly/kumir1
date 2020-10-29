#include "kumscene.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>


KumScene::KumScene(QWidget *parent) : QGraphicsScene(parent)
{
};


void KumScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	bool LeftButtonFlag;
	//if (mouseEvent->type() == QEvent::NonClientAreaMouseButtonDblClick)

	//qDebug("doubleclick %i",mouseEvent->type());
	//if (mouseEvent->type() == QEvent::GraphicsSceneMouseDoubleClick)
	//{
	//qDebug("doubleclick");
	//}
	if (mouseEvent->button() == 0x00000001) {
		LeftButtonFlag = true;
	} else {
		LeftButtonFlag = false;
	}

	QPointF posScene = mouseEvent->scenePos();
	QPointF pos = mouseEvent->screenPos();
	qreal x = pos.x();
	qreal y = pos.y();
	qreal xScene = posScene.x();
	qreal yScene = posScene.y();


	//qDebug("RobotPressEvent");
	emit MousePress(x, y, LeftButtonFlag, xScene, yScene);
};

void KumScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	bool LeftButtonFlag;
	//qDebug("robot release");
	if (mouseEvent->button() == 0x00000001) {
		LeftButtonFlag = true;
	} else {
		LeftButtonFlag = false;
	}

	//QPointF pos=mouseEvent->scenePos();
	QPointF pos = mouseEvent->screenPos();
	qreal x = pos.x();
	qreal y = pos.y();
	emit MouseRelease(x, y, LeftButtonFlag);

}

void KumScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	bool LeftButtonFlag;
	//qDebug("robot move");
	if (mouseEvent->button() == 0x00000001) {
		LeftButtonFlag = true;
	} else {
		LeftButtonFlag = false;
	}

	//QPointF pos=mouseEvent->scenePos();
	QPointF pos = mouseEvent->screenPos();
	qreal x = pos.x();
	qreal y = pos.y();
	//QPointF posScene=mouseEvent->scenePos();
	emit MouseMove(x, y, LeftButtonFlag);

}
//------------------------
void KumScene::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{

	int Delta = wheelEvent->delta();
	//qDebug("wheelEvent %i",Delta);
	emit MouseWheel(Delta);

}



