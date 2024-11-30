#include "kumirplugin.h"
#include "robot25dwindow.h"
#include "robotview.h"

Robot25DPlugin::Robot25DPlugin()
{
	m_window = 0;
}

void Robot25DPlugin::start()
{
}

QList<Alg> Robot25DPlugin::algList()
{
	QList<Alg> result;

	Alg a;
	a.kumirHeader = QString::fromUtf8("алг вперед");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг повернуть налево");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг повернуть направо");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг закрасить");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг лог впереди стена");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг лог клетка закрашена");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг лог @@клетка закрашена(цел x, цел y)");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг лог @@есть точка(цел x, цел y)");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг цел @@размер поля x");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг цел @@размер поля y");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг цел @@положение робота x");
	result << a;

	a.kumirHeader = QString::fromUtf8("алг цел @@положение робота y");
	result << a;

	return result;
}

QString Robot25DPlugin::name()
{
	return QString::fromUtf8("Вертун");
}

QList<QVariant> Robot25DPlugin::algOptResults()
{
	return QList<QVariant>();
}

QVariant Robot25DPlugin::result()
{
	return v_result;
}


void Robot25DPlugin::showField()
{
	if (!m_window) {
		m_window = new Robot25DWindow;

	}
	if (m_window->isVisible()) {
		m_window->raise();
	} else {
		m_window->show();
	}
}


void Robot25DPlugin::showPult()
{
	if (pultWidget()) {
		if (pultWidget()->isVisible()) {
			pultWidget()->raise();
		} else {
			pultWidget()->show();
		}
	}
}

void Robot25DPlugin::showOpt()
{

}

void Robot25DPlugin::hidePult()
{
	if (pultWidget()) {
		pultWidget()->hide();
	}
}

void Robot25DPlugin::hideField()
{
	if (m_window) {
		m_window->hide();
	}
}

bool Robot25DPlugin::hasPult()
{
	return pultWidget() != 0;
}

bool Robot25DPlugin::hasOpt()
{
	return false;
}

QString Robot25DPlugin::optText()
{
	return "";
}

void Robot25DPlugin::setMode(int mode)
{
	Robot25DWindow * w = qobject_cast<Robot25DWindow *>(mainWidget());
	if (mode == 1) {
		w->lock();
	} else if (mode == 0) {
		w->unlock();
	}

}

void Robot25DPlugin::reset()
{
	Robot25DWindow * w = qobject_cast<Robot25DWindow *>(mainWidget());
	w->robotView()->reset();
}

QString Robot25DPlugin::errorText() const
{
	return s_error;
}

quint8 Robot25DPlugin::check(const QString &)
{
	return 10;
}

void Robot25DPlugin::setParameter(const QString &name, const QVariant &value)
{
	if (name == "environment") {
		QString fileName = value.toString();
		m_window->loadEnvironment(fileName);
	}
}

void Robot25DPlugin::connectSignalSendText(const QObject *object, const char *method)
{
	Q_UNUSED(object);
	Q_UNUSED(method);
//    connect(this, SIGNAL(error(QString)), object, method);
}

void Robot25DPlugin::connectSignalSync(const QObject *obj, const char *method)
{
	connect(this, SIGNAL(sync()), obj, method);
}

QUrl Robot25DPlugin::pdfUrl() const
{
	return QUrl::fromLocalFile("../Addons/robot25d/resources/robot25d.pdf");
}

QUrl Robot25DPlugin::infoXmlUrl() const
{
	return QUrl::fromLocalFile("../../Addons/robot25d/resources/robot25d.info.xml");
}

QWidget *Robot25DPlugin::mainWidget()
{
	if (!m_window) {
		Robot25DWindow * w = new Robot25DWindow;
		m_window = w;
		connect(w->robotView(), SIGNAL(sync()), this, SLOT(handleEvaluationFinished()));


	}
	Q_CHECK_PTR(m_window);
	return m_window;
}

void Robot25DPlugin::handleEvaluationFinished()
{
	Robot25DWindow * w = qobject_cast<Robot25DWindow *>(mainWidget());
	s_error = w->robotView()->lastError(QLocale::Russian);
	emit sync();
	if (!w->robotView()->lastError(QLocale::Russian).isEmpty()) {
//        emit error(w->robotView()->lastError(QLocale::Russian));
	}
}

QWidget *Robot25DPlugin::pultWidget()
{
	return 0;
}

void Robot25DPlugin::runAlg(QString alg, QList<QVariant> args)
{
	v_result = QVariant::Invalid;
	s_error = "";
	Robot25DWindow * w = qobject_cast<Robot25DWindow *>(mainWidget());
	w->robotView()->setAnimated(true);
	if (alg == QString::fromUtf8("вперед")) {
		w->robotView()->goForward();
	} else if (alg == QString::fromUtf8("повернуть налево")) {
		w->robotView()->turnLeft();
	} else if (alg == QString::fromUtf8("повернуть направо")) {
		w->robotView()->turnRight();
	} else if (alg == QString::fromUtf8("закрасить")) {
		w->robotView()->doPaint();
	} else if (alg == QString::fromUtf8("впереди стена")) {
		v_result = w->robotView()->isWall();
		emit sync();
	} else if (alg == QString::fromUtf8("клетка закрашена")) {
		v_result = w->robotView()->isPainted();
		emit sync();
	} else if (alg == QString::fromUtf8("@@клетка закрашена")) {
		int x = args[0].toInt() - 1;
		int y = args[1].toInt() - 1;
		v_result = w->robotView()->isPainted(x, y);
		emit sync();
	} else if (alg == QString::fromUtf8("@@есть точка")) {
		int x = args[0].toInt() - 1;
		int y = args[1].toInt() - 1;
		v_result = w->robotView()->isPointed(x, y);
		emit sync();
	} else if (alg == QString::fromUtf8("@@размер поля x")) {
		v_result = w->robotView()->sizeX();
		emit sync();
	} else if (alg == QString::fromUtf8("@@размер поля y")) {
		v_result = w->robotView()->sizeY();
		emit sync();
	} else if (alg == QString::fromUtf8("@@положение робота x")) {
		v_result = w->robotView()->positionX() + 1;
		emit sync();
	} else if (alg == QString::fromUtf8("@@положение робота y")) {
		v_result = w->robotView()->positionY() + 1;
		emit sync();
	}
}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(robot25d, Robot25DPlugin)
#endif

