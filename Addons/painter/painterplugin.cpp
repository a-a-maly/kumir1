#include "painterplugin.h"
#include "painterwindow.h"
#include "paintertools.h"

#include <QPainter>
#include <QPen>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QStack>

struct PainterPluginPrivate
{
	PainterWindow *mainWidget;
	QWidget *view;
	QString templateName;
	QVariant lastResult;
	QList<QVariant> results;
	QString lastError;
	QPen pen;
	QBrush brush;
	Qt::BrushStyle style;
	bool transparent;
	QPoint point;
	QFont font;
	QMutex *canvasLock;
	QImage *canvas;
	QImage *originalCanvas;
	void drawPoint(int x, int y, const QColor &color);
	void drawLine(int x0, int y0, int x1, int y1);
	void drawPolygon(const QVector<QPoint> &points);
	void drawEllipse(int x0, int y0, int x1, int y1);
	void drawText(int x, int y, const QString &text);
	void fill(int x, int y);
	QString ioDir;
};

PainterPlugin::PainterPlugin(QObject *parent) : QObject(parent)
{
	d = new PainterPluginPrivate;
	d->originalCanvas = new QImage(QSize(640, 480), QImage::Format_RGB32);
	d->originalCanvas->fill(QColor("white").rgb());
	d->canvas = new QImage(QSize(640, 480), QImage::Format_RGB32);
	d->canvas->fill(QColor("white").rgb());
	d->mainWidget = 0;
	d->view = 0;
	d->canvasLock = new QMutex;
	if (QFile::exists(QApplication::applicationDirPath() + "/Addons/painter/resources/default.png")) {
		QSettings * s = NULL;
		if (QFile::exists(QApplication::applicationDirPath() + "/Addons/painter/resources/templates.ini")) {
			s = new QSettings(QApplication::applicationDirPath() + "/Addons/painter/resources/templates.ini", QSettings::IniFormat);
			s->setIniCodec("UTF-8");
			d->templateName = s->value("Names/default.png", "[новый лист]").toString();
			s->deleteLater();

		}
		handleLoadImageRequest(QApplication::applicationDirPath() + "/Addons/painter/resources/default.png");
	}
}

QList<Alg> PainterPlugin::algList()
{
	QList<Alg> l;
	Alg a;

	a.kumirHeader = QString::fromUtf8("алг перо(цел толщина, лит цвет)");
	l << a;

//    a.kumirHeader = QString::fromUtf8("алг перо_RGB(цел толщина, цел R, G, B)");
//    l << a;

//    a.kumirHeader = QString::fromUtf8("алг перо_RGBA(цел толщина, цел R, G, B, A)");
//    l << a;

//    a.kumirHeader = QString::fromUtf8("алг перо_CMYK(цел толщина, цел C, M, Y, K)");
//    l << a;

	a.kumirHeader = QString::fromUtf8("алг кисть(лит цвет)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг плотность(цел пл)");
	l << a;

//    a.kumirHeader = QString::fromUtf8("алг кисть_RGB(цел R, G, B)");
//    l << a;

//    a.kumirHeader = QString::fromUtf8("алг кисть_RGBA(цел R, G, B, A)");
//    l << a;

//    a.kumirHeader = QString::fromUtf8("алг кисть_CMYK(цел C, M, Y, K)");
//    l << a;

	a.kumirHeader = QString::fromUtf8("алг убрать кисть");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг пиксель(цел x, y, лит цвет)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг в точку(цел x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг линия в точку(цел x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг линия(цел x0, y0, x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг прямоугольник(цел x0, y0, x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг эллипс(цел x0, y0, x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг окружность(цел x, y, r)");
	l << a;

//    a.kumirHeader = QString::fromUtf8("алг многоугольник(цел N, целтаб xx[1:N], yy[1:N])");
//    l << a;

	a.kumirHeader = QString::fromUtf8("алг шрифт(лит название, цел размер, лог жирный, курсив)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг цел ширина текста(лит текст)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг надпись(цел x,y, лит текст)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг залить(цел x, y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит RGB(цел r, g, b)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит RGBA(цел r, g, b, a)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит CMYK(цел c, m, y, k)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит CMYKA(цел c, m, y, k, a)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит HSV(цел h, s, v)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит HSVA(цел h, s, v, a)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит HSL(цел h, s, l)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит HSLA(цел h, s, l, a)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг цел ширина листа");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг цел высота листа");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг цел центр x");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг цел центр y");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг лит цвет в точке(цел x,y)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг новый лист(цел ширина, высота, лит цвет фона)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг загрузить лист(лит имя файла)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг сохранить лист(лит имя файла)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг разложить цвет в RGB(лит цвет, рез цел R, G, B)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг разложить цвет в CMYK(лит цвет, рез цел C, M, Y, K)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг разложить цвет в HSL(лит цвет, рез цел H, S, L)");
	l << a;

	a.kumirHeader = QString::fromUtf8("алг разложить цвет в HSV(лит цвет, рез цел H, S, V)");
	l << a;


	return l;
}

QString PainterPlugin::name()
{
	return QString::fromUtf8("Рисователь");
}

QList<QVariant> PainterPlugin::algOptResults()
{
	return d->results;
}

QVariant PainterPlugin::result()
{
	QVariant v = d->lastResult;
	return v;
}

void PainterPlugin::runAlg(QString alg, QList<QVariant> params)
{
	d->lastResult = QVariant();
	d->results.clear();
	d->lastError = "";
	if (alg == QString::fromUtf8("перо")) {
		int width = params[0].toInt();
		const QString cs = params[1].toString().trimmed();
		QColor c = PainterTools::parceColor(cs);
		if (cs.isEmpty() || width < 1) {
			d->pen = Qt::NoPen;
		} else {
			d->pen = QPen(c, width);
		}
	} else if (alg == QString::fromUtf8("плотность")) {
		int dens = params[0].toInt();
		dens = qMax(8, dens);
		dens = qMin(0, dens);
		if (dens == 0) {
			d->brush.setStyle(Qt::NoBrush);
		} else if (dens == 1) {
			d->brush.setStyle(Qt::Dense7Pattern);
		} else if (dens == 2) {
			d->brush.setStyle(Qt::Dense6Pattern);
		} else if (dens == 3) {
			d->brush.setStyle(Qt::Dense5Pattern);
		} else if (dens == 4) {
			d->brush.setStyle(Qt::Dense4Pattern);
		} else if (dens == 5) {
			d->brush.setStyle(Qt::Dense3Pattern);
		} else if (dens == 6) {
			d->brush.setStyle(Qt::Dense2Pattern);
		} else if (dens == 7) {
			d->brush.setStyle(Qt::Dense1Pattern);
		} else if (dens == 8) {
			d->brush.setStyle(Qt::SolidPattern);
		}
		d->style = d->brush.style();
		if (d->transparent) {
			d->brush.setStyle(Qt::NoBrush);
		} else {
			d->brush.setStyle(d->style);
		}
	} else if (alg == QString::fromUtf8("кисть")) {
		QColor c = PainterTools::parceColor(params[0].toString());
		if (params[0].toString().trimmed().isEmpty()) {
			d->transparent = true;
		} else {
			d->transparent = false;
		}
		if (!d->transparent && !c.isValid()) {
			d->lastError = QString::fromUtf8("Недопустимый цвет");
		}
		d->brush.setColor(c);
		if (d->transparent) {
			d->brush.setStyle(Qt::NoBrush);
		} else {
			d->brush.setStyle(d->style);
		}
	} else if (alg.startsWith(QString::fromUtf8("разложить цвет в "))) {
		const QString model = alg.mid(17).trimmed().toLower();
		const QString cs = params[0].toString().trimmed();
		QColor c = PainterTools::parceColor(cs);
		if (c.isValid()) {
			if (model == "rgb") {
				int R, G, B;
				R = c.red();
				G = c.green();
				B = c.blue();
				d->results << R << G << B;
			} else if (model == "cmyk") {
				int C, M, Y, K;
				C = c.cyan();
				M = c.magenta();
				Y = c.yellow();
				K = c.black();
				d->results << C << M << Y << K;
			} else if (model == "hsl") {
				int H, S, L;
				H = c.hue();
				S = c.saturation();
				L = c.lightness();
				d->results << H << S << L;
			} else if (model == "hsv") {
				int H, S, V;
				H = c.hue();
				S = c.saturation();
				V = c.value();
				d->results << H << S << V;
			}
		} else {
			d->lastError = QString::fromUtf8("Недопустимый цвет");
		}
	} else if (alg == QString::fromUtf8("убрать кисть")) {
		d->brush.setStyle(Qt::NoBrush);
		d->transparent = true;
	} else if (alg == QString::fromUtf8("пиксель")) {
		int x = params[0].toInt();
		int y = params[1].toInt();
		QString cs = params[2].toString();
		if (!cs.isEmpty()) {
			QColor c = PainterTools::parceColor(cs);
			if (c.isValid()) {
				d->drawPoint(x, y, c);
			} else {
				d->lastError = QString::fromUtf8("Недопустимый цвет");
			}
		}
	} else if (alg == QString::fromUtf8("в точку")) {
		int x = params[0].toInt();
		int y = params[1].toInt();
		d->point.setX(x);
		d->point.setY(y);
	} else if (alg == QString::fromUtf8("линия в точку")) {
		int x0 = d->point.x();
		int y0 = d->point.y();
		int x1 = params[0].toInt();
		int y1 = params[1].toInt();
		d->drawLine(x0, y0, x1, y1);
		d->point.setX(x1);
		d->point.setY(y1);
	} else if (alg == QString::fromUtf8("линия")) {
		int x0 = params[0].toInt();
		int y0 = params[1].toInt();
		int x1 = params[2].toInt();
		int y1 = params[3].toInt();
		d->drawLine(x0, y0, x1, y1);
		d->point.setX(x1);
		d->point.setY(y1);
	} else if (alg == QString::fromUtf8("прямоугольник")) {
		int x0 = params[0].toInt();
		int y0 = params[1].toInt();
		int x1 = params[2].toInt();
		int y1 = params[3].toInt();
		QVector<QPoint> points(4);
		points[0] = QPoint(x0, y0);
		points[1] = QPoint(x1, y0);
		points[2] = QPoint(x1, y1);
		points[3] = QPoint(x0, y1);
		d->drawPolygon(points);
		d->point.setX(x1);
		d->point.setY(y1);
	} else if (alg == QString::fromUtf8("эллипс")) {
		int x0 = params[0].toInt();
		int y0 = params[1].toInt();
		int x1 = params[2].toInt();
		int y1 = params[3].toInt();
		d->drawEllipse(x0, y0, x1, y1);
	} else if (alg == QString::fromUtf8("окружность")) {
		int xc = params[0].toInt();
		int yc = params[1].toInt();
		int r = params[2].toInt();
		int x0 = xc - r;
		int y0 = yc - r;
		int x1 = xc + r;
		int y1 = yc + r;
		d->drawEllipse(x0, y0, x1, y1);
	} else if (alg == QString::fromUtf8("многоугольник")) {
		int N = params[0].toInt();
		QVector<QPoint> points(N);
		QList<QVariant> xx = params[1].toList();
		QList<QVariant> yy = params[2].toList();
		if (N != xx.length() || N != yy.length()) {
			d->lastError = QString::fromUtf8("Неверное число точек");
		} else {
			for (int i = 0; i < N; i++) {
				points[i] = QPoint(xx[i].toInt(), yy[i].toInt());
			}
			d->drawPolygon(points);
		}
	} else if (alg == QString::fromUtf8("шрифт")) {
		const QString family = params[0].toString();
		int size = params[1].toInt();
		bool bold = params[2].toBool();
		bool italic = params[3].toBool();
		if (!family.isEmpty()) {
			d->font.setFamily(family);;
		}
		if (size > 0) {
			d->font.setPixelSize(size);
		}
		d->font.setBold(bold);
		d->font.setItalic(italic);
	} else if (alg == QString::fromUtf8("ширина текста")) {
		const QString text = params[0].toString();
		const QFontMetrics fm(d->font);
		int width = fm.width(text);
		d->lastResult = QVariant(width);
	} else if (alg == QString::fromUtf8("надпись")) {
		int x = params[0].toInt();
		int y = params[1].toInt();
		const QString text = params[2].toString();
		d->drawText(x, y, text);
	} else if (alg == QString::fromUtf8("залить")) {
		int x = params[0].toInt();
		int y = params[1].toInt();
		if (!d->transparent) {
			d->fill(x, y);
		}
	} else if (alg == "RGB") {
		int r = params[0].toInt();
		int g = params[1].toInt();
		int b = params[2].toInt();
		d->lastResult = tr("rgb(%1,%2,%3)").arg(r).arg(g).arg(b);
	} else if (alg == "RGBA") {
		int r = params[0].toInt();
		int g = params[1].toInt();
		int b = params[2].toInt();
		int a = params[3].toInt();
		d->lastResult = tr("rgba(%1,%2,%3,%4)").arg(r).arg(g).arg(b).arg(a);
	} else if (alg == "CMYK") {
		int c = params[0].toInt();
		int m = params[1].toInt();
		int y = params[2].toInt();
		int k = params[3].toInt();
		d->lastResult = tr("cmyk(%1,%2,%3,%4)").arg(c).arg(m).arg(y).arg(k);
	} else if (alg == "CMYKA") {
		int c = params[0].toInt();
		int m = params[1].toInt();
		int y = params[2].toInt();
		int k = params[3].toInt();
		int a = params[4].toInt();
		d->lastResult = tr("cmyka(%1,%2,%3,%4,%5)").arg(c).arg(m).arg(y).arg(k).arg(a);
	} else if (alg == "HSL") {
		int h = params[0].toInt();
		int s = params[1].toInt();
		int l = params[2].toInt();
		d->lastResult = tr("hsl(%1,%2,%3)").arg(h).arg(s).arg(l);
	} else if (alg == "HSLA") {
		int h = params[0].toInt();
		int s = params[1].toInt();
		int l = params[2].toInt();
		int a = params[3].toInt();
		d->lastResult = tr("hsla(%1,%2,%3,%4)").arg(h).arg(s).arg(l).arg(a);
	} else if (alg == "HSV") {
		int h = params[0].toInt();
		int s = params[1].toInt();
		int v = params[2].toInt();
		d->lastResult = tr("hsv(%1,%2,%3)").arg(h).arg(s).arg(v);
	} else if (alg == "HSVA") {
		int h = params[0].toInt();
		int s = params[1].toInt();
		int v = params[2].toInt();
		int a = params[3].toInt();
		d->lastResult = tr("hsva(%1,%2,%3,%4)").arg(h).arg(s).arg(v).arg(a);
	} else if (alg == QString::fromUtf8("ширина листа")) {
		d->lastResult = QVariant(d->canvas->width());
	} else if (alg == QString::fromUtf8("высота листа")) {
		d->lastResult = QVariant(d->canvas->height());
	} else if (alg == QString::fromUtf8("центр x")) {
		d->lastResult = QVariant(d->canvas->width() / 2);
	} else if (alg == QString::fromUtf8("центр y")) {
		d->lastResult = QVariant(d->canvas->height() / 2);
	} else if (alg == QString::fromUtf8("новый лист")) {
		int w = params[0].toInt();
		int h = params[1].toInt();
		QString bg = params[2].toString();
		if (w < 1 || h < 1) {
			d->lastError = QString::fromUtf8("Слишком маленький размер листа");
		} else if (w > 3000 || h > 3000) {
			d->lastError = QString::fromUtf8("Слишком большой размер листа (макс. 3000 пикс.)");
		} else {
			handleNewImageRequest(w, h, bg, false, "");
		}
	} else if (alg == QString::fromUtf8("загрузить лист")) {
		QString fileName = params[0].toString();
		if (QFileInfo(fileName).isRelative()) {
			fileName = QDir(d->ioDir).absoluteFilePath(fileName);
		}
		if (!QFile::exists(fileName)) {
			d->lastError = QString::fromUtf8("Файл не найден");
		} else {
			handleLoadImageRequest(fileName);
		}
	} else if (alg == QString::fromUtf8("сохранить лист")) {
		QString fileName = params[0].toString();
		if (QFileInfo(fileName).isRelative()) {
			fileName = QDir(d->ioDir).absoluteFilePath(fileName);
		}
		QFile f(fileName);
		if (f.open(QIODevice::WriteOnly)) {
			d->canvas->save(&f, "PNG");
			f.close();
		} else {
			d->lastError = QString::fromUtf8("Не могу открыть файл на запись");
		}
	} else if (alg == QString::fromUtf8("цвет в точке")) {
		int x = params[0].toInt();
		int y = params[1].toInt();
		if (x < 0 || y < 0) {
			d->lastError = QString::fromUtf8("Координата меньше 0");
		} else if (x >= d->canvas->width() || y >= d->canvas->height()) {
			d->lastError = QString::fromUtf8("Выход за границу рисунка");
		} else {
			QColor c = QColor::fromRgb(d->canvas->pixel(x, y));
			d->lastResult = c.name();
		}
	}
	emit sync();
}

void PainterPlugin::showField()
{
	d->mainWidget->show();
}

void PainterPlugin::hideField()
{
	d->mainWidget->hide();
}

void PainterPlugin::setMode(int mode)
{
	Q_UNUSED(mode);
}

void PainterPlugin::reset()
{
	QImage * del = d->canvas;
	d->lastError = "";
	d->lastResult = QVariant();
	d->point = QPoint(0, 0);
	d->font = QFont();
	d->brush = QBrush();
	d->style = Qt::SolidPattern;
	d->transparent = false;
	d->pen = QPen();
	d->canvasLock->lock();
	d->canvas = new QImage(d->originalCanvas->copy());
	d->mainWidget->setCanvas(d->canvas, d->canvasLock);
	d->canvasLock->unlock();
	if (d->view) {
		d->view->update();
	}
	delete del;
}

QString PainterPlugin::errorText() const
{
	return d->lastError;
}

quint8 PainterPlugin::check(const QString &script)
{
	Q_UNUSED(script);
	return 10;
}

void PainterPlugin::setParameter(const QString &paramName, const QVariant &paramValue)
{
	if (paramName.toLower() == "iodir") {
		d->ioDir = paramValue.toString();
	}
}

void PainterPlugin::connectSignalSendText(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sendText(QString)), obj, method);
}

void PainterPlugin::connectSignalSync(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sync()), obj, method);
}

QUrl PainterPlugin::pdfUrl() const
{
	return QUrl::fromLocalFile("../Addons/painter/resources/painter.pdf");
}

QUrl PainterPlugin::infoXmlUrl() const
{
	return QUrl::fromLocalFile("../../Addons/painter/resources/painter.info.xml");
}

QWidget *PainterPlugin::mainWidget()
{
	if (!d->mainWidget) {
		d->mainWidget = new PainterWindow;
		d->mainWidget->setCanvas(d->canvas, d->canvasLock);
		d->mainWidget->setWindowTitle(name() + " [" + d->templateName + "]");
		d->view = d->mainWidget->view();
		connect(d->mainWidget, SIGNAL(newImageRequest(int, int, QString, bool, QString)),
			this, SLOT(handleNewImageRequest(int, int, QString, bool, QString)));
		connect(d->mainWidget, SIGNAL(loadImageRequest(QString)),
			this, SLOT(handleLoadImageRequest(QString)));
		connect(d->mainWidget, SIGNAL(resetRequest()),
			this, SLOT(handleResetRequest()));
	}
	return d->mainWidget;
}

// private functions implementation


void PainterPlugin::handleNewImageRequest(int w, int h, const QString &bg, bool useTemplate, const QString &fileName)
{
	if (useTemplate) {
		handleLoadImageRequest(QApplication::applicationDirPath() + "/Addons/painter/resources/" + fileName);
	} else {
		QColor c = PainterTools::parceColor(bg);
		QImage * oldCanvas = d->canvas;
		QImage * oldCanvas2 = d->originalCanvas;
		d->canvas = new QImage(w, h, QImage::Format_RGB32);
		d->canvas->fill(c.rgb());
		d->originalCanvas = new QImage(w, h, QImage::Format_RGB32);
		d->originalCanvas->fill(c.rgb());
		reset();
		d->mainWidget->setCanvas(d->canvas, d->canvasLock);
		delete oldCanvas;
		delete oldCanvas2;
	}
}

void PainterPlugin::handleLoadImageRequest(const QString &fileName)
{
	QImage * oldCanvas = d->canvas;
	QImage * oldCanvas2 = d->originalCanvas;
	d->canvas = new QImage(fileName);
	d->originalCanvas = new QImage(fileName);
	if (d->mainWidget) {
		d->mainWidget->setCanvas(d->canvas, d->canvasLock);
	}
	delete oldCanvas;
	delete oldCanvas2;
}

void PainterPlugin::handleResetRequest()
{
	reset();
}

void PainterPluginPrivate::drawEllipse(int x0, int y0, int x1, int y1)
{
	canvasLock->lock();
	QPainter p(canvas);
	p.setPen(pen);
	p.setBrush(brush);
	p.drawEllipse(x0, y0, x1 - x0, y1 - y0);
	canvasLock->unlock();
	if (view) {
		view->update();
	}
}

void PainterPluginPrivate::drawLine(int x0, int y0, int x1, int y1)
{
	canvasLock->lock();
	QPainter p(canvas);
	p.setPen(pen);
	p.setBrush(brush);
	p.drawLine(x0, y0, x1, y1);
	canvasLock->unlock();
	p.end();
	if (view) {
		view->update();
	}

}

void PainterPluginPrivate::drawPoint(int x, int y, const QColor &color)
{
	canvasLock->lock();
	QPainter p(canvas);
	p.setPen(QPen(color));
//    p.setBrush(brush);
	p.drawPoint(x, y);
	canvasLock->unlock();
	if (view) {
		view->update();
	}
}

void PainterPluginPrivate::drawPolygon(const QVector<QPoint> &points)
{
	canvasLock->lock();
	QPainter p(canvas);
	p.setPen(pen);
	p.setBrush(brush);
	p.drawPolygon(QPolygon(points));
	canvasLock->unlock();
	if (view) {
		view->update();
	}
}

void PainterPluginPrivate::drawText(int x, int y, const QString &text)
{
	canvasLock->lock();
	QPainter p(canvas);
	p.setPen(pen);
	p.setBrush(brush);
	p.setFont(font);
	p.drawText(x, y, text);
	canvasLock->unlock();
	if (view) {
		view->update();
	}
}

void PainterPluginPrivate::fill(int x, int y)
{

	QStack<QPoint> stack;
	QRgb replaceColor = canvas->pixel(x, y);
	if (replaceColor == brush.color().rgb()) {
		return;
	}
	stack.push(QPoint(x, y));
	while (!stack.isEmpty()) {
		QPoint pnt = stack.pop();
		if (pnt.x() < 0 || pnt.y() < 0 || pnt.x() >= canvas->width() || pnt.y() >= canvas->height()) {
			continue;
		}
		QRgb value = canvas->pixel(pnt);
		if (value == replaceColor) {
			canvasLock->lock();
			canvas->setPixel(pnt, brush.color().rgb());
			canvasLock->unlock();
			view->update();
			stack.push(QPoint(pnt.x() - 1, pnt.y()));
			stack.push(QPoint(pnt.x() + 1, pnt.y()));
			stack.push(QPoint(pnt.x(), pnt.y() - 1));
			stack.push(QPoint(pnt.x(), pnt.y() + 1));
		}
	}
	if (view) {
		view->update();
	}
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(painter, PainterPlugin)
#endif
