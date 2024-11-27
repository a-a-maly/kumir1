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

#ifndef PUSHDOWNAUTOMATA_H
#define PUSHDOWNAUTOMATA_H

#include <QScriptable>
#include <QScriptEngine>
#include <QStack>
#include <QPointer>
#include <QMutex>
#include "kum_tables.h"
#include "ui_PDDebugWindow.h"

class PDDebugWindow;

Q_DECLARE_METATYPE(QStack<int>);


class Script : public QObject
{
public:
	inline Script(const QString &value, QObject *parent) : QObject(parent)
	{
		m_value = value;
	}
	inline bool process(QScriptEngine *engine, QString &result)
	{
		QScriptValue value = engine->evaluate(m_value);
		result = value.toString();
		return !value.isError();
	}
	inline QString toString() const
	{
		return m_value.simplified();
	}
	QString ruleDesc;
private:
	QString m_value;
};

struct RuleRightPart {
	QStringList nonTerminals;
	QPointer<Script> script;
	bool isEpsilon;
	int priority;
	inline bool operator< (const RuleRightPart &other) const
	{
		return priority < other.priority;
	}
	int ruleLine;
};

// Определение списка правил
typedef QList<RuleRightPart> Rules;

// Определение типа матрицы правил.
// Здесь ключ RulesKey -- это пара ТЕРМИНАЛ + "/" + НЕТЕРМИНАЛ
typedef QMap<QString, Rules> Matrix;


typedef QList<proga> Isp;

struct PDStackElem {
	QString nonTerminal;
	int iterateStart;
	int priority;
};

class AutomataData : public QObject, protected QScriptable
{
	Q_OBJECT
	Q_PROPERTY(int currentPosition READ currentPosition SCRIPTABLE true)
	Q_PROPERTY(int currentModuleSize READ currentModuleSize SCRIPTABLE true)
	Q_PROPERTY(int errorsCount READ errorsCount SCRIPTABLE true)

public:
	/**
	 * Конструктор
	 * @param parent ссылка на объект-хозяин
	 */
	AutomataData(QObject *parent) : QObject(parent), QScriptable() { }

	/**
	 * Выполняет инициализацию (сброс до начального состояния)
	 * @param source исходный список ProgaText
	 * @param fileName имя файла исполнителя (прописывается в proga.real_line)
	 * @param tab ссылка на вкладку редактора (прописывается в proga.real_line)
	 */
	virtual void init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab);

	/**
	 * Возвращает список ProgaText, включая отступы и разметку ошибок
	 * @return список ProgaText
	 */
	QList<ProgaText> text() const
	{
		QList<ProgaText> all = m_text;
		all.pop_front();
		return all;
	}

	/**
	 * Возвращает список исполнителей
	 * @return список списков proga
	 */
	QList<Isp> modules() const
	{
		return m_modules;
	}

	/**
	 * Возвращает главный (безымянный) исполнитель
	 * @return список proga
	 */
	Isp mainModule() const
	{
		return m_mainModule;
	}

	/**
	 * Возвращает список исполнителей, которые по какой-либо причине не разобрались
	 * (например нет кон_исп)
	 * @return список списков proga
	 */
	QList<Isp> brokenModules() const
	{
		return m_brokenModules;
	}

	/**
	 * Возвращает список строк "использовать"
	 * @return список proga
	 */
	QList<proga> loads() const
	{
		return m_loads;
	}

	int errorsCount() const
	{
		return i_errorsCount;
	}

protected:
	int currentPosition() const
	{
		return m_currentPosition;
	}

	void clearAll();
	int currentModuleSize()
	{
		return m_currentModule.count();
	}

	QList<ProgaText> m_text;
	int m_currentPosition;
	QMap<int, QPoint> m_translationsMap;

protected slots:
	void setError(int no, int errorCode);
	void setIndentRank(int no, int start, int end);
	void addToCurrentModule(int no);
	void setJump(int elem, int value);
	void pushMainIsp();
	void pushIsp();
	void beginBrokenIsp();
	void endBrokenIsp();
	void addDummyAlgHeader();
	void addLoad(int value);
	int getError(int no) const;
	void changeInstructionType(int no, const QString &newInstr);
	void changeError(int no, int offset, int newError);

protected:
	QList<Isp> m_modules;
	Isp m_mainModule;
	Isp m_currentModule;
	QList<proga> m_loads;
	QStack<Isp> m_incompleteModules;
	QList<Isp> m_brokenModules;
	QString m_fileName;
	ProgramTab *m_tab;
	int m_dummyNumber;
	int i_errorsCount;
};

class PushdownAutomata : public AutomataData
{
	friend class PDDebugWindow;
	friend class Runner;
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent ссылка на объект-хозяин
	 */
	PushdownAutomata(QObject *parent = 0, PDDebugWindow *parentWindow = 0);
	/**
	 * Выполняет инициализацию (сброс до начального состояния)
	 * @param source исходный список ProgaText
	 * @param fileName имя файла исполнителя (прописывается в proga.real_line)
	 * @param tab ссылка на вкладку редактора (прописывается в proga.real_line)
	 */
	void init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab);
	/**
	 * Чтение файла с правилами вывода
	 * @param source ссылка на источник данных
	 * @return код ошибки
	 */
	QPoint loadRules(QIODevice *source);
	/**
	 * Выполняет поиск наиболее подходящего варианта разбора
	 * @return 0, если разбор успешный, 1, если правило отклонено
	 * 2, если превышен лимит подряд идущих неверных направлений
	 */
	uint process(bool debugMode = false);
	/**
	 * Выполняет скрипты для найденного варианта разбора.
	 * При этом порождаются исполнители
	 * @return true, если выполнение всех скриптов успешно
	 */
	bool postProcess(bool debugMode = false);

protected slots:
	void debug(const QString &text);

private:
	static QString terminalByCode(Kumir::InstructionType code);
	static bool isCorrectTerminal(const QString &terminal);
	void saveData();
	void clearDataHistory();
	void restoreData();
	void popHistory();
	void nextStep();
	void setTooManyErrors();
	void finalizeIterativeRule(const PDStackElem &elem);
	void beginIterativeRule(const RuleRightPart &);

	int m_maxPriorityValue;
	bool m_allowSkipParts;

	Matrix m_matrix;
	QStack< PDStackElem > m_stack;
	QScriptEngine m_scriptEngine;
	QVector< QPointer<Script> > m_scripts;
	QVector< QPointer<Script> > m_fixedScripts;
	QVector<int> m_nextPointers;
	QString m_initScript;

	QStack< int > h_currentPosition;
	QStack < QStack<PDStackElem> > h_working_stack;
	QStack< QVector< QPointer<Script> > > h_scripts;

	PDDebugWindow *debugWindow;
	bool DEBUG_AUTOMATA;
	QObject *m_parent;
//      int m_errorWayCounter;
//      int m_processDeep;
//      int m_recursionCallNo;
	//QStack<int> m_iterativeStartPos;
	QString m_lastUnfoundRule;
};

class Runner;
class RulesBrowser;

class PDDebugWindow : public QWidget, public Ui::PDDebugWindowBase
{
	Q_OBJECT

public:
	PDDebugWindow(PushdownAutomata *automata, QWidget *parent = 0, Qt::WindowFlags fl = 0);
	void setProgaText(const QList<ProgaText> &source);
	void setCurrentPosition(int position);
	void setStack(const QStack<QString> &stack);
	void setCurrentRulesList(const PDStackElem &nonTerminal, const QString &terminal, const Rules &rules);
	void init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab);
	void setCurrentRuleIndex(int index);
	inline void setErrorCounter(int c)
	{
		emit errorCounterChanged(c);
	}
	~PDDebugWindow();
	bool isProcessing();
	void startProcess();
	bool result();
	void waitForNextStep();
	void reset();
	void save();
	void restore();
	inline void clearScripts()
	{
		bool a = false;
		m_scUpdated = false;
		emit scriptClearRequest();
		while (!a) {
			m_scUpdateLocker->lock();
			a = m_scUpdated;
			m_scUpdateLocker->unlock();
		}
	}
	inline void appendScript(const QString &txt)
	{
		bool a = false;
		m_scUpdated = false;
		emit sctiptAppendRequest(txt);
		while (!a) {
			m_scUpdateLocker->lock();
			a = m_scUpdated;
			m_scUpdateLocker->unlock();
		}
	}

public slots:
	void step();
	void play();

protected:


protected slots:
	inline void updateErrorCounter(int c)
	{
		errorCounter->setText(QString::number(c));
	}
	void updateState();
	void updateRulesListSlot();
	void processFinished();
	void highlightCurrentRule();
	void fallbackToRestoredItem();
	void doClearScript();
	void doAppendScript(const QString &txt);

private:
	QMutex *m_stepWaiter;
	PushdownAutomata *m_automata;
	bool m_continuePressed;
	Runner *m_runner;
	QString m_currentTerminal;
	PDStackElem m_currentStackElem;
	QList<RuleRightPart> m_currentRules;
	int m_currentRuleIndex;
	QTreeWidgetItem *m_currentRuleItem;
	QTreeWidgetItem *m_nextRuleItem;
	QTreeWidgetItem *m_restoredRuleItem;
	QStack<QTreeWidgetItem *> m_savedRuleItem;
	QMutex *m_treeUpdateLocker;
	bool m_treeUpdated;
	bool m_scUpdated;
	QMutex *m_scUpdateLocker;
	RulesBrowser *rulesBrowser;


signals:
	void updateStep();
	void updateTree();
	void updateTreeItem();
	void treeRestored();
	void scriptClearRequest();
	void sctiptAppendRequest(const QString &txt);
	void errorCounterChanged(int c);
};


class RulesBrowser
	: public QTextBrowser
{
	Q_OBJECT
public slots:
	void setSelectedRule(QTreeWidgetItem *, int);
protected:
signals:
	void visiblityChanged(bool);
private:
	QByteArray m_geometry;
};

#endif
