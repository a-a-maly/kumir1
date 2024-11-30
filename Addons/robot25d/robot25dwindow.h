#ifndef ROBOT25DWINDOW_H
#define ROBOT25DWINDOW_H


#include "sch_game.h"

#include <QMainWindow>
#include <QActionGroup>

namespace Robot25D
{
class Plugin;
class RobotView;
}

namespace Ui
{
class Robot25DWindow;
}

class Robot25DWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit Robot25DWindow(QWidget *parent = 0);
	~Robot25DWindow();

	Robot25D::RobotView *robotView()
	{
		return m_robotView;
	}

	void lock()
	{
		group_lockedActionsDuringEvaluate->setEnabled(false);
	}

	void unlock()
	{
		group_lockedActionsDuringEvaluate->setEnabled(true);
	}

	void loadGame(const QString &fileName);
	void loadEnvironment(const QString &fileName);

private slots:
	void handleLoadAction();
	void handleNextAction();
	void handlePrevAction();
	void setTaskIndex(int index);

private:
	Schema::Game m_game;
	Ui::Robot25DWindow *ui;
	QActionGroup *group_lockedActionsDuringEvaluate;
	Robot25D::RobotView *m_robotView;
};

#endif // ROBOT25DWINDOW_H
