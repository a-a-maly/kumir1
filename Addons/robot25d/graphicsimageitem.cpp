#include "graphicsimageitem.h"
#include <QPainter>


namespace Robot25D
{
GraphicsImageItem::GraphicsImageItem()
{
	mutex_image = new QMutex;
}

QRectF GraphicsImageItem::boundingRect() const
{
	mutex_image->lock();
	QRectF result;
	if (!m_image.isNull()) {
		result = QRectF(QPointF(0.0, 0.0), m_image.size()).adjusted(-0.5, -0.5, 0.5, 0.5);
	}
	mutex_image->unlock();
	return result;
}

void GraphicsImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	mutex_image->lock();
	painter->drawImage(0, 0, m_image);
	mutex_image->unlock();
}

QImage GraphicsImageItem::image() const
{
	mutex_image->lock();
	QImage result = m_image;
	mutex_image->unlock();
	return result;
}

void GraphicsImageItem::setImage(const QImage &image)
{
	mutex_image->lock();
	m_image = image;
	mutex_image->unlock();
}

GraphicsImageItem::~GraphicsImageItem()
{
	delete mutex_image;
}

}
