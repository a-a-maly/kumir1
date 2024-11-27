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

#ifndef Q_OS_WIN32
#ifndef Q_OS_SOLARIS
#include <unistd.h>
#endif
#endif

#include "pushdownautomata.h"
//#include "strtypes.h"
#include "kum_tables.h"
#include "error.h"
#include "const.h"

#include <QDir>
#include <QThread>
#include <QBuffer>

#define CRITICAL_PERFORMANCE_PROGRAM_SIZE 20
#define ALLOW_QDEBUG false
#define ALLOW_PDWINDOW false

#define MAXIMUM_ERRORS_COUNT 20

#include "pwrulesnormalizer.h"
#include "config.h"

PushdownAutomata::PushdownAutomata(QObject *parent, PDDebugWindow *parentWindow)
	: AutomataData(parent)
{
	DEBUG_AUTOMATA = ALLOW_PDWINDOW;
	//  foreach ( QString arg, QCoreApplication::arguments() ) {
	//      if ( arg==QString("--debug-pd") ) {
	//          DEBUG_AUTOMATA = true;
	//          break;
	//      }
	//  }

	qScriptRegisterSequenceMetaType< QStack<int> > (&m_scriptEngine);
	QString initFilePath;
#ifdef Q_OS_MAC
	initFilePath = QDir::cleanPath(QApplication::applicationDirPath() + "/../Resources/pd_init.js");
#else
	initFilePath = QDir::cleanPath(QApplication::applicationDirPath() + "/Kumir/PDScripts/pd_init.js");
#endif
	qDebug() << "Inti file path" << initFilePath;
	QFile initScriptFile(initFilePath);
	if (initScriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream ts(&initScriptFile);
		ts.setCodec("UTF-8");
		m_initScript = ts.readAll();
		initScriptFile.close();
	}
	if (parentWindow == NULL) {
		if (DEBUG_AUTOMATA) {
			debugWindow = new PDDebugWindow(this);
			debugWindow->show();
		} else {
			debugWindow = NULL;
		}
	} else {
		debugWindow = parentWindow;
	}
	m_parent = parent;
}

QPoint PushdownAutomata::loadRules(QIODevice *source)
{
	QTextStream ts(source);
	ts.setCodec("UTF-8");
	QStringList lines = ts.readAll().split("\n");

	QString leftPart;
	QString rightPart;
	QString script;
	QString terminal;

	int lineNo = 0;
	m_maxPriorityValue = 0;
	foreach (QString line, lines) {
		lineNo ++;
		// Обработка комментария
		if (line.trimmed().startsWith("#") || line.trimmed().isEmpty()) {
			continue;
		}

		// Выделение приоритета
		QString prior_s;

		int bracket_p = line.lastIndexOf("[");
		if (bracket_p == -1) {
			return QPoint(5, lineNo); // ошибка: нет приоритета
		} else {
			prior_s = line.mid(bracket_p);
			line = line.left(bracket_p).trimmed();
		}

		prior_s = prior_s.remove("[").remove("]").trimmed();
		bool ok;
		int prior = prior_s.toInt(&ok);
		if (! ok) {
			return QPoint(6, lineNo);    // ошибка: приоритет -- не целое число
		}
		// Выделение левой части
		int arrow_p = line.indexOf("->");
		if (arrow_p == -1) {
			return QPoint(1, lineNo);    // ошибка: неправильный формат правила
		}
		leftPart = line.left(arrow_p).trimmed();
		if (leftPart.isEmpty()) {
			return QPoint(2, lineNo); // ошибка: пустая левая часть
		}
		// Выделение правой части и скрипта
		rightPart = line.mid(arrow_p + 2).trimmed();
		int script_p = rightPart.indexOf(":");
		if (script_p != -1) {
			script = rightPart.mid(script_p + 1).trimmed();
			rightPart = rightPart.left(script_p).trimmed();
		} else {
			script = ""; // очистка, т.к. могло что-то остаться от пред. строки
		}
		// Выделение терминала в правой части
		int space_p = rightPart.indexOf(' ');
		if (space_p == -1) {
			if (rightPart.startsWith("0")) {
				// Правило \epsilon
				terminal = "";
				rightPart = "";
			} else {
				bool isCorrect = true;
				for (int j = 0; j < terminal.length(); j++) {
					if (!(terminal[j] == '_' || (terminal[j].isLetter() && terminal[j].isLower()))) {
						isCorrect = false;
						break;
					}
				}
				if (isCorrect) {
					terminal = rightPart;
					rightPart = "";
				} else {
					return QPoint(3, lineNo);    // ошибка: всего один ксимвол в правой части (не \epsilon и не терминал)
				}
			}
		} else {
			terminal = rightPart.left(space_p);
			bool isCorrect = true;
			for (int j = 0; j < terminal.length(); j++) {
				if (!(terminal[j] == '_' || (terminal[j].isLetter() && terminal[j].isLower()))) {
					isCorrect = false;
					break;
				}
			}
			if (!isCorrect) {
				return QPoint(4, lineNo);    // ошибка: первый ксимвол в правой части -- нетерминал
			}
			rightPart = rightPart.mid(space_p + 1).trimmed();
		}
		m_maxPriorityValue = qMax(m_maxPriorityValue, prior);

		// Добавляем в таблицу правил
		if (terminal.isEmpty()) {
			// Случай \epsilon-правила. В этом случае заполняем всю строку таблицы
			//          int allLines[] = { 0, P_VAR_IMPL,
			//                             A_IMPL, A_OTKR, A_ZAKR, O_OTKR, O_ZAKR, ELSE, EXIT, IF_BEGINS, TO_OTKR,
			//                             IF_ZAKR, VIBOR_BEGINS, PRI_BEGINS, PRISV, VIVOD, UTV, DANO, NADO,
			//                             VVOD, KC_PRI, ISP, ISP_END, USE_ISP, FVIVOD, FVVOD, DOC, ALG_USE, -1 };
			Kumir::InstructionType allLines[] = {
				Kumir::Empty,
				Kumir::VarDecl,
				Kumir::AlgDecl,
				Kumir::AlgBegin,
				Kumir::AlgEnd,
				Kumir::LoopBegin,
				Kumir::LoopEnd,
				Kumir::Else,
				Kumir::Exit,
				Kumir::If,
				Kumir::Then,
				Kumir::Switch,
				Kumir::Case,
				Kumir::Fin,
				Kumir::Assign,
				Kumir::Output,
				Kumir::Assert,
				Kumir::Pre,
				Kumir::Post,
				Kumir::Input,
				Kumir::LoopEndCond,
				Kumir::ModuleBegin,
				Kumir::ModuleEnd,
				Kumir::UseModule,
				Kumir::FileInput,
				Kumir::FileOutput,
				Kumir::Include,
				Kumir::Doc,
				Kumir::Comment,
				Kumir::CallAlg,
				Kumir::Undefined
			};
			int j = 0;
			do {
				QString key;
				RuleRightPart rule;
				if (allLines[j] != 0) {
					key = terminalByCode(allLines[j]) + "/" + leftPart;
				} else {
					key = "end/" + leftPart;
				}
				K_ASSERT(! key.startsWith("/"));
				rule.nonTerminals = QStringList();
				rule.script = new Script(script, this);
				rule.script->ruleDesc = leftPart + " -> 0";
				rule.isEpsilon = true;
				rule.priority = prior;
				if (m_matrix.contains(key)) {
					// Добавляем \epsilon-правило только если нет других правил
					// c тем же приоритетом
					bool allowToAdd = true;
					Rules rulesList = m_matrix[key];
					foreach (RuleRightPart rule, rulesList) {
						if (rule.priority == prior) {
							allowToAdd = false;
							break;
						}
					}
					if (allowToAdd) {
						m_matrix[key].append(rule);
					}
				} else {
					Rules newRulesList;
					newRulesList.append(rule);
					m_matrix[key] = newRulesList;
				}
				j++;
			} while (allLines[j] != Kumir::Undefined);
		} else {
			// Случай обычного правила
			QString key;
			if (!isCorrectTerminal(terminal)) {
				return QPoint(5, lineNo); // ошибка: неизвестный терминал
			}
			key = terminal + "/" + leftPart;
			RuleRightPart rule;
			rule.nonTerminals = rightPart.isEmpty() ? QStringList() : rightPart.split(" ");
			rule.script = new Script(script, this);
			rule.script->ruleDesc = leftPart + " -> " + terminal + " " + rightPart;
			rule.isEpsilon = false;
			rule.priority = prior;
			if (m_matrix.contains(key)) {
				m_matrix[key].append(rule);
				// Удаляем \epsilon-правило, если оно там есть
				// и имеет тот же приоритет
				for (int j = m_matrix[key].count() - 1; j >= 0; j--) {
					if (m_matrix[key][j].isEpsilon && m_matrix[key][j].priority == prior) {
						m_matrix[key].removeAt(j);
					}
				}
			} else {
				Rules newRulesList;
				newRulesList.append(rule);
				m_matrix[key] = newRulesList;
			}
		}
	}
	qDebug() << "fff";
	foreach (QString key, m_matrix.keys()) {
		Rules rulesList = m_matrix[key];
		qSort(rulesList);
		m_matrix[key] = rulesList;
	}

	if (ALLOW_QDEBUG) {
		qDebug() << "MAXIMUM PRIORITY VALUE: " << m_maxPriorityValue;
	}
	qDebug() << "End load rules";
	return QPoint(0, 0);
}

void PushdownAutomata::nextStep()
{
	if (m_currentPosition < m_nextPointers.count()) {
		m_currentPosition = m_nextPointers[m_currentPosition];
	} else {
		m_currentPosition = m_text.count();
	}
}

void PushdownAutomata::finalizeIterativeRule(const PDStackElem &stackElem)
{
	//  K_ASSERT ( !m_iterativeStartPos.isEmpty() );
	//  if ( m_iterativeStartPos.isEmpty() ) {
	//      return;
	//  }
	//  int iterationStartPos = m_iterativeStartPos.pop();
	int iterationStartPos = stackElem.iterateStart;
	for (int i = iterationStartPos + 1; i < m_currentPosition; i++) {
		if (m_fixedScripts[i].isNull()) {
			m_fixedScripts[i] = m_scripts[i];
		}
	}
	if (ALLOW_QDEBUG) {
		qDebug() << "END ITERATIVE RULE AT [" << m_currentPosition << "], POINTS TO: " << iterationStartPos;
	}
	//  for ( int i=iterationStartPos; i<m_currentPosition; i++ )
	//      m_nextPointers[i] = m_currentPosition;
	//  if ( stackElem.priority == 0 )
	if (m_allowSkipParts || stackElem.priority == 0) {
		m_nextPointers[iterationStartPos] = m_currentPosition;
	}
	if (ALLOW_QDEBUG) {
		qDebug() << "NEXT POINTERS: " << m_nextPointers;
	}
}

void PushdownAutomata::beginIterativeRule(const RuleRightPart &rule)
{
	for (int i = rule.nonTerminals.count() - 1; i >= 0 ; i--) {
		if (rule.nonTerminals[i].endsWith("*")) {
			//          m_iterativeStartPos.push(m_currentPosition>=0? m_currentPosition : 0);
			if (ALLOW_QDEBUG) {
				qDebug() << "BEGIN ITERATIVE RULE AT [" << m_currentPosition << "] : " << rule.nonTerminals[i];
			}
		}
	}
}

uint PushdownAutomata::process(bool debugMode)
{

	bool startWindowDebugProcess = !debugMode && debugWindow != NULL && !debugWindow->isProcessing();
	if (startWindowDebugProcess) {
		debugWindow->startProcess();
		//      return 1;
	}
	if (m_stack.isEmpty()) {
		if (m_currentPosition == m_text.count()) {
			return 0;
		} else {
			return 1;
		}
	}
	do {
		if (errorsCount() > MAXIMUM_ERRORS_COUNT) {
			setTooManyErrors();
			return 0;
		}
		PDStackElem currentStackElem = m_stack.pop();
		QString currentTerminal;
		QString key;
		if (m_currentPosition > m_text.count()) {
			break;
		}
		if (m_currentPosition < m_text.count() && m_currentPosition >= 0) {
			ProgaText pt = m_text[m_currentPosition];
			currentTerminal = terminalByCode(pt.Base_type);
		} else {
			currentTerminal = "end";
		}
		key = currentTerminal + "/" + currentStackElem.nonTerminal;
		if (ALLOW_QDEBUG) {
			qDebug() << "KEY " << key;
		}
		if (m_matrix.contains(key)) {
			Rules rulesList = m_matrix[key];
			if (ALLOW_QDEBUG) {
				for (int i = 0; i < rulesList.count(); i++) {
					qDebug() << "RULE: " << rulesList[i].nonTerminals << " [" << rulesList[i].priority << "]";
				}
			}
			//          if ( debugMode && debugWindow!=NULL ) {
			if (ALLOW_PDWINDOW) {
				debugWindow->setCurrentRulesList(currentStackElem, currentTerminal, rulesList);

			}
			if (rulesList.count() == 1) {
				// Линейный случай, когда паре {ТЕРМИНАЛ,НЕТЕРМИНАЛ}
				// соответствует только одно правило
				if (ALLOW_PDWINDOW) {
					debugWindow->setCurrentRuleIndex(0);
					debugWindow->waitForNextStep();
				}
				RuleRightPart rule = rulesList[0];

				if (!rule.isEpsilon && m_currentPosition >= 0) {
					m_scripts[currentPosition()] = rule.script;
				}
				// Если левая часть правила итеративная (*), то помещаем её обратно в стек
				if (!rule.isEpsilon && currentStackElem.nonTerminal.endsWith("*")) {
					PDStackElem backElem = currentStackElem;
					backElem.priority = rule.priority;
					m_stack.push(backElem);
				}
				PDStackElem cse;
				for (int j = rule.nonTerminals.count() - 1; j >= 0; j--) {
					cse.nonTerminal = rule.nonTerminals[j];
					cse.iterateStart = m_currentPosition;
					cse.priority = rule.priority;
					m_stack.push(cse);
				}
				// Если на вершину стека поместили итеративное правило,
				// то запоминаем текущую позицию
				beginIterativeRule(rule);
				// Если отрабатываем правило вида: ИТЕРАТИВНЫЙ_НЕТЕРМИНАЛ* -> 0,
				// то устанавливаем соответствующий next-указатель
				if (rule.isEpsilon && currentStackElem.nonTerminal.endsWith("*") && currentTerminal != "end") {
					finalizeIterativeRule(currentStackElem);
				}
				if (!rule.isEpsilon && !key.startsWith("end/")) {
					nextStep();
				}
			} else {
				// Разветвление. Сохраняем состояние и пытаемся применять
				// правила до тех пор, пока одно из них не будет успешным
				uint success = 0;

				//              int errorsBefore = m_errorWayCounter;
				for (int i = 0; i < rulesList.count(); i++)  {
					saveData();
					if (ALLOW_PDWINDOW) {
						debugWindow->save();
						debugWindow->setCurrentRuleIndex(i);
						debugWindow->waitForNextStep();
					}
					RuleRightPart rule = rulesList[i];

					if (!rule.isEpsilon && m_currentPosition >= 0) {
						m_scripts[currentPosition()] = rule.script;
					}
					// Если левая часть правила итеративная (*), то помещаем её обратно в стек
					if (!rule.isEpsilon && currentStackElem.nonTerminal.endsWith("*")) {
						PDStackElem backElem = currentStackElem;
						backElem.priority = rule.priority;
						m_stack.push(backElem);
					}
					PDStackElem cse;
					for (int j = rule.nonTerminals.count() - 1; j >= 0; j--) {
						cse.nonTerminal = rule.nonTerminals[j];
						cse.iterateStart = m_currentPosition;
						cse.priority = rule.priority;
						m_stack.push(cse);
					}
					// Если на вершину стека поместили итеративное правило,
					// то запоминаем текущую позицию
					beginIterativeRule(rule);
					// Если отрабатываем правило вида: ИТЕРАТИВНЫЙ_НЕТЕРМИНАЛ* -> 0,
					// то устанавливаем соответствующий next-указатель
					if (rule.isEpsilon && currentStackElem.nonTerminal.endsWith("*") && currentTerminal != "end") {
						finalizeIterativeRule(currentStackElem);
					}
					if (!rule.isEpsilon && !key.startsWith("end/")) {
						nextStep();
					}
					success = process(debugMode);
					if (success == 0) {
						popHistory();
						break;
					}
					if (success == 1) {
						restoreData();
						if (ALLOW_PDWINDOW) {
							debugWindow->restore();
						}
					}
				}
				if (success != 0) {
					break;
				}
			}
		} else {
			m_lastUnfoundRule = currentStackElem.nonTerminal + " -> " + currentTerminal;
			return 1;
		}
	} while (!m_stack.isEmpty());
	bool hasEnd = m_currentPosition == m_text.count();

	return (m_stack.isEmpty() && hasEnd) ? 0 : 1;
}


void PushdownAutomata::restoreData()
{
	K_ASSERT(!h_currentPosition.isEmpty());
	K_ASSERT(!h_working_stack.isEmpty());
	K_ASSERT(!h_scripts.isEmpty());

	m_currentPosition = h_currentPosition.pop();
	m_stack = h_working_stack.pop();
	m_scripts = h_scripts.pop();
}

void PushdownAutomata::saveData()
{
	h_working_stack.push(m_stack);
	h_currentPosition.push(m_currentPosition);
	h_scripts.push(m_scripts);
}

void PushdownAutomata::clearDataHistory()
{
	h_working_stack.clear();
	h_currentPosition.clear();
	h_scripts.clear();
}

void PushdownAutomata::popHistory()
{
	h_working_stack.pop();
	h_currentPosition.pop();
	h_scripts.pop();
}

void AutomataData::init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab)
{
	i_errorsCount = 0;
	m_translationsMap.clear();
	m_text.clear();
	ProgaText pt;
	pt.Text = "begin";
	pt.Base_type = Kumir::Undefined;
	m_text << pt;
	m_text << source;
	for (int i = 0; i < m_text.count(); i++) {
		m_text[i].indentRank = QPoint(0, 0);
	}
	m_currentPosition = 0;
	m_fileName = fileName;
	m_tab = tab;
}

void PushdownAutomata::init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab)
{
	AutomataData::init(source, fileName, tab);

	if (debugWindow != NULL && debugWindow != m_parent) {
		debugWindow->init(source, fileName, tab);
	}

	m_stack.clear();
	clearDataHistory();
	PDStackElem se;
	se.nonTerminal = "START";
	se.iterateStart = 0;
	se.priority = 0;
	m_stack.push(se);
	/*  m_errorWayCounter = 0;
	m_processDeep = 0;
	m_recursionCallNo = 0;*/
	m_scripts = QVector< QPointer<Script> >(source.count() + 2, NULL);
	m_fixedScripts = QVector< QPointer<Script> >(source.count() + 2, NULL);
	m_nextPointers = QVector<int>(source.count());
	for (int i = 0; i < m_nextPointers.count(); i++) {
		m_nextPointers[i] = i + 1;
	}

	m_allowSkipParts = m_text.count() >= CRITICAL_PERFORMANCE_PROGRAM_SIZE;

}

void PushdownAutomata::debug(const QString &text)
{
	if (debugWindow) {
		debugWindow->appendScript("DEBUG: " + text);
	}
	qDebug() << "PD DEBUG: " << text;
}

bool PushdownAutomata::postProcess(bool debugMode)
{
	if (ALLOW_QDEBUG) {
		qDebug() << "SCRIPTS :";
		QString a;
		for (int i = 0; i < m_scripts.count(); i++) {
			if (m_scripts[i].isNull()) {
				a = "NULL";
			} else {
				a = m_scripts[i]->toString();
			}
			qDebug() << i << " : " << a;
		}
		//  qDebug() << "FIXED SCRIPTS :";

		for (int i = 0; i < m_fixedScripts.count(); i++) {
			if (m_fixedScripts[i].isNull()) {
				a = "NULL";
			} else {
				a = m_fixedScripts[i]->toString();
			}
			//          qDebug() << i << " : " << a;
		}
	}
	clearAll();
	if (debugWindow != NULL && debugMode) {
		debugWindow->clearScripts();
	}
	QScriptValue pool = m_scriptEngine.newQObject(this);
	m_scriptEngine.currentContext()->setThisObject(pool);
	m_scriptEngine.collectGarbage();
	QScriptValue result;
	//  long pagesize = sysconf(_SC_PAGE_SIZE);
	//  long before = sysconf(_SC_PHYS_PAGES);
	result = m_scriptEngine.evaluate(m_initScript);
	//  long after = sysconf(_SC_PHYS_PAGES);
	//  long delta_pages = after-before;
	//  long delta_bytes = delta_pages * pagesize;
	//  qDebug() << "MEM LEAK: " << delta_bytes << " bytes ( in " << delta_pages << " pages )";
	if (debugWindow != NULL && debugMode) {
		debugWindow->appendScript("<b>pd_init.js</b>");
		debugWindow->appendScript(m_initScript);
	}
	if (result.isError()) {
		if (debugWindow != NULL && debugMode) {
			debugWindow->appendScript("<font color='red'>" + result.toString() + "</font>");
		}
		return false;
	} else {
		if (debugWindow != NULL && debugMode) {
			debugWindow->appendScript("<font color='green'>OK</font>");
		}
	}
	m_currentPosition = 0;
	for (int i = 0; i < m_scripts.count(); i++) {
		QPointer<Script> script_ptr = m_scripts[i];
		if (script_ptr.isNull()) {
			script_ptr = m_fixedScripts[i];
		}
		if (!script_ptr.isNull()) {
			Script *script = script_ptr;
			QString debugString;
			//          qDebug() << i << " : " << script->toString();
			if (debugWindow != NULL && debugMode) {
				debugWindow->appendScript("<b>" + script->ruleDesc + "</b>");
			}
			if (debugWindow != NULL && debugMode) {
				debugWindow->appendScript(script->toString());
			}
			if (!script->process(&m_scriptEngine, debugString)) {
				if (debugWindow != NULL && debugMode) {
					debugWindow->appendScript("<font color='red'>" + debugString + "</font>");
				}
				return false;
			} else {
				if (debugWindow != NULL && debugMode) {
					debugWindow->appendScript("<font color='green'>OK</font>");
				}
			}
		}
		m_currentPosition++;
	}
	if (currentModuleSize() > 0) {
		pushMainIsp();
	}
	return true;
}


QString PushdownAutomata::terminalByCode(Kumir::InstructionType code)
{
	switch (code) {
	case Kumir::VarDecl:
		return QString::fromUtf8("перем");
	case Kumir::AlgDecl:
		return QString::fromUtf8("алг");
	case Kumir::AlgBegin:
		return QString::fromUtf8("нач");
	case Kumir::AlgEnd:
		return QString::fromUtf8("кон");
	case Kumir::LoopBegin:
		return QString::fromUtf8("нц");
	case Kumir::LoopEnd:
		return QString::fromUtf8("кц");
	case Kumir::Else:
		return QString::fromUtf8("иначе");
	case Kumir::Exit:
		return QString::fromUtf8("выход");
	case Kumir::If:
		return QString::fromUtf8("если");
	case Kumir::Then:
		return QString::fromUtf8("то");
	case Kumir::Fin:
		return QString::fromUtf8("все");
	case Kumir::Switch:
		return QString::fromUtf8("выбор");
	case Kumir::Case:
		return QString::fromUtf8("при");
	case Kumir::Assign:
		return QString::fromUtf8("присваивание");
	case Kumir::Output:
		return QString::fromUtf8("вывод");
	case Kumir::Assert:
		return QString::fromUtf8("утв");
	case Kumir::Pre:
		return QString::fromUtf8("дано");
	case Kumir::Post:
		return QString::fromUtf8("надо");
	case Kumir::Input:
		return QString::fromUtf8("ввод");
	case Kumir::LoopEndCond:
		return QString::fromUtf8("кц_при");
	case Kumir::ModuleBegin:
		return QString::fromUtf8("исп");
	case Kumir::ModuleEnd:
		return QString::fromUtf8("кон_исп");
	case Kumir::UseModule:
		return QString::fromUtf8("использовать");
	case Kumir::FileInput:
		return QString::fromUtf8("ф_ввод");
	case Kumir::FileOutput:
		return QString::fromUtf8("ф_вывод");
	case Kumir::Doc:
		return QString::fromUtf8("строка_документации");
	case Kumir::CallAlg:
		return QString::fromUtf8("вызов_алгоритма");
	case Kumir::Undefined:
		return "begin";
	default:
		return QString(" ");
	}
}

bool PushdownAutomata::isCorrectTerminal(const QString &terminal)
{
	if (
		terminal == QString::fromUtf8("перем")
		||
		terminal == QString::fromUtf8("алг")
		||
		terminal == QString::fromUtf8("нач")
		||
		terminal == QString::fromUtf8("кон")
		||
		terminal == QString::fromUtf8("нц")
		||
		terminal == QString::fromUtf8("кц")
		||
		terminal == QString::fromUtf8("иначе")
		||
		terminal == QString::fromUtf8("выход")
		||
		terminal == QString::fromUtf8("если")
		||
		terminal == QString::fromUtf8("то")
		||
		terminal == QString::fromUtf8("все")
		||
		terminal == QString::fromUtf8("выбор")
		||
		terminal == QString::fromUtf8("при")
		||
		terminal == QString::fromUtf8("присваивание")
		||
		terminal == QString::fromUtf8("вывод")
		||
		terminal == QString::fromUtf8("утв")
		||
		terminal == QString::fromUtf8("дано")
		||
		terminal == QString::fromUtf8("надо")
		||
		terminal == QString::fromUtf8("ввод")
		||
		terminal == QString::fromUtf8("кц_при")
		||
		terminal == QString::fromUtf8("исп")
		||
		terminal == QString::fromUtf8("использовать")
		||
		terminal == QString::fromUtf8("кон_исп")
		||
		terminal == QString::fromUtf8("ф_ввод")
		||
		terminal == QString::fromUtf8("ф_вывод")
		||
		terminal == QString::fromUtf8("строка_документации")
		||
		terminal == QString::fromUtf8("вызов_алгоритма")
		||
		terminal == "end"
		||
		terminal == "begin"
	) {
		return true;
	} else {
		return false;
	}
}

void AutomataData::setError(int no, int errorCode)
{
	K_ASSERT(no < m_text.count());
	if (no >= m_text.count()) {
		return;
	}
	ProgaText pt = m_text[no];
	pt.Error = errorCode;
	pt.errstart = 0;
	pt.errlen = pt.Text.length();
	m_text[no] = pt;
	if (m_translationsMap.contains(no)) {
		QPoint point = m_translationsMap[no];
		if (point.x() == -1) {
			m_currentModule[point.y()].error = errorCode;
		} else {
			m_modules[point.x()][point.y()].error = errorCode;
		}
	}
	i_errorsCount ++;
}

int AutomataData::getError(int no) const
{
	K_ASSERT(no < m_text.count());
	if (no >= m_text.count()) {
		return 0;
	}
	ProgaText pt = m_text[no];
	int result = pt.Error;
	return result;
}



void AutomataData::changeError(int pos, int offset, int err)
{
	K_ASSERT(pos + offset < m_text.count());
	if (pos + offset >= m_text.count()) {
		return;
	}
	K_ASSERT(pos < m_currentModule.count());
	if (pos >= m_currentModule.count()) {
		return;
	}
	m_text[pos + offset + 1].Error = err;
	m_text[pos + offset + 1].errstart = 0;
	m_text[pos + offset + 1].errlen = m_text[pos + offset + 1].Text.length();
	m_currentModule[pos].error = err;
}

void AutomataData::addToCurrentModule(int no)
{
	K_ASSERT(no < m_text.count());
	if (no >= m_text.count()) {
		return;
	}
	proga result;
	ProgaText pt = m_text[no];
	result.line = pt.Text;
	//  qDebug() << "ASD GF DFG DFS GDSF GS Sdfg fgSDFG DFG FG ---- Text " << pt.Text << " Line: " << pt.stroka;
	result.str_type = pt.Base_type;
	result.counter = 0;
	result.else_pos = 0;
	result.real_line.lineNo = pt.stroka;
	result.real_line.termNo = pt.position;
	result.real_line.fileName = m_fileName;
	result.real_line.tab = m_tab;
	result.error = pt.Error;
	result.source = pt.source;
	result.is_break = false;
	//  qDebug() << "ASD GF DFG DFS GDSF GS Sdfg fgSDFG DFG FG ---- Text " << result.line << " Line: " << result.real_line.lineNo;
	m_translationsMap[no] = QPoint(-1, m_currentModule.size());
	m_currentModule << result;
	//  qDebug() << "Current module: ";
	//  for ( int i=0; i<m_currentModule.count(); i++ ) {
	//      qDebug() << "ASD GF DFG DFS GDSF GS Sdfg fgSDFG DFG FG ---- Text " << m_currentModule[i].line << " Line: " << m_currentModule[i].real_line.lineNo;
	//  }
}

void AutomataData::addLoad(int no)
{
	proga result;
	result.line = m_text[no].Text;
	result.str_type = m_text[no].Base_type;
	result.counter = 0;
	result.else_pos = 0;
	result.real_line.lineNo = m_text[no].stroka;
	result.real_line.termNo = m_text[no].position;
	result.real_line.fileName = m_fileName;
	result.real_line.tab = m_tab;
	result.error = m_text[no].Error;
	result.source = m_text[no].source;
	result.is_break = false;
	m_loads << result;
}

void AutomataData::setJump(int elem, int value)
{
	//  int debug = m_currentModule.count();
	K_ASSERT(m_currentModule.count() > elem);
	K_ASSERT(m_currentModule.count() > value);
	if (elem >= m_currentModule.count()) {
		return;
	}
	m_currentModule[elem].else_pos = value;
}

void AutomataData::changeInstructionType(int elem, const QString &newInstr)
{
	K_ASSERT(m_currentModule.count() > elem);
	if (elem >= m_currentModule.count()) {
		return;
	}
	Kumir::InstructionType type = m_currentModule[elem].str_type;
	if (newInstr == QString::fromUtf8("кон")) {
		type = m_currentModule[elem].str_type = Kumir::AlgEnd;
	} else if (newInstr == QString::fromUtf8("кц")) {
		type = m_currentModule[elem].str_type = Kumir::LoopEnd;
	} else if (newInstr == QString::fromUtf8("все")) {
		type = m_currentModule[elem].str_type = Kumir::Fin;
	}
	m_currentModule[elem].str_type = type;
}

void AutomataData::pushIsp()
{
	//  int debug = m_currentModule.count();
	m_modules << m_currentModule;
	m_currentModule.clear();
	foreach (int key, m_translationsMap.keys()) {
		QPoint point = m_translationsMap[key];
		if (point.x() == -1) {
			point.setX(m_modules.size());
			m_translationsMap[key] = point;
		}
	}
}

void AutomataData::pushMainIsp()
{
	pushIsp();
	////    int debug = m_currentModule.count();
	////    for ( int i=0; i<m_currentModule.count(); i++ ) {
	//      //qDebug() << "ASD GF DFG DFS GDSF GS Sdfg fgSDFG DFG FG ---- Text " << m_currentModule[i].line << " Line: " << m_currentModule[i].real_line.lineNo;
	////    }
	//  if (m_mainModule.isEmpty()) {
	//      m_mainModule = m_currentModule;
	//      m_currentModule.clear();
	//  }
	//  else {
	//      pushIsp();
	//  }
	////    for ( int i=0; i<m_mainModule.count(); i++ ) {
	//      //qDebug() << "ASD GF DFG DFS GDSF GS Sdfg fgSDFG DFG FG ---- Text " << m_mainModule[i].line << " Line: " << m_mainModule[i].real_line.lineNo;
	////    }
}

void AutomataData::setIndentRank(int no, int start, int end)
{
	K_ASSERT(no < m_text.count());
	if (no >= m_text.count()) {
		return;
	}
	ProgaText txt = m_text[no];
	txt.indentRank = QPoint(start, end);
	m_text[no] = txt;
}

void AutomataData::clearAll()
{
	m_currentPosition = 0;
	m_currentModule.clear();
	m_modules.clear();
	m_mainModule.clear();
	m_incompleteModules.clear();
	m_brokenModules.clear();
	m_dummyNumber = 0;
	m_loads.clear();
}

void AutomataData::beginBrokenIsp()
{
	m_incompleteModules.push(m_currentModule);
	m_currentModule.clear();
}

void AutomataData::endBrokenIsp()
{
	m_brokenModules << m_currentModule;
	m_currentModule = m_incompleteModules.pop();
}

void AutomataData::addDummyAlgHeader()
{
	proga pv;
	pv.str_type = Kumir::AlgDecl;
	pv.line = QString(QChar(KS_ALG)) + " " + QString(QChar(KS_DUMMY_ALG_NAME)) + " " + QString::number(m_dummyNumber);
	pv.real_line.fileName = m_fileName;
	pv.real_line.tab = NULL;
	pv.real_line.lineNo = -2;
	pv.real_line.termNo = -1;
	m_currentModule << pv;
	m_dummyNumber++;
}

class Runner : public QThread
{
public:
	inline Runner(QObject *parent, PushdownAutomata *automata) : QThread(parent)
	{
		m_automata = automata;
	}
	inline uint result()
	{
		return m_result;
	}
	inline bool scriptResult()
	{
		return m_scriptResult;
	}
protected:
	inline void run()
	{
		m_result = m_automata->process(true);
		if (m_result == 0) {
			m_scriptResult = m_automata->postProcess(true);
		} else  {
			m_automata->debugWindow->status->setText(QString::fromUtf8("Аварийное завершение: не хватает правил описания ошибочных ситуаций."));
		}
	}
	PushdownAutomata *m_automata;
	uint m_result;
	bool m_scriptResult;
};

bool PDDebugWindow::isProcessing()
{
	return m_runner->isRunning();
}
bool PDDebugWindow::result()
{
	return m_runner->result();
}

PDDebugWindow::PDDebugWindow(PushdownAutomata *automata, QWidget *parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), Ui::PDDebugWindowBase()
{
	Q_UNUSED(automata);
	//  setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint );
	setupUi(this);
	m_automata = new PushdownAutomata(this, this);
	m_runner = new Runner(this, m_automata);
	m_stepWaiter = new QMutex();
	m_treeUpdateLocker = new QMutex();
	m_scUpdateLocker = new QMutex();
	m_currentRuleItem = NULL;
	//  treeWidget->header()->setVisible(false);
	connect(btnNextStep, SIGNAL(clicked()), this, SLOT(step()));
	connect(btnContinue, SIGNAL(clicked()), this, SLOT(play()));
	connect(this, SIGNAL(updateStep()), this, SLOT(updateState()));
	connect(this, SIGNAL(updateTree()), this, SLOT(updateRulesListSlot()));
	connect(m_runner, SIGNAL(finished()), this, SLOT(processFinished()));
	connect(this, SIGNAL(updateTreeItem()), this, SLOT(highlightCurrentRule()));
	connect(this, SIGNAL(treeRestored()), this, SLOT(fallbackToRestoredItem()));

	connect(this, SIGNAL(sctiptAppendRequest(const QString &)), this, SLOT(doAppendScript(const QString &)));
	connect(this, SIGNAL(scriptClearRequest()), this, SLOT(doClearScript()));
	connect(this, SIGNAL(errorCounterChanged(int)), this, SLOT(updateErrorCounter(int)));

	btnBreakMode->setChecked(false);

	rulesBrowser = new RulesBrowser();
	rulesBrowser->resize(size());
	rulesBrowser->setWindowTitle(QString::fromUtf8("Список нормализованных правил"));
	rulesBrowser->setLineWrapMode(QTextEdit::NoWrap);
	rulesBrowser->show();
	//  connect ( treeWidget, SIGNAL(itemChanged ( QTreeWidgetItem * , int )), rulesBrowser, SLOT(setSelectedRule(QTreeWidgetItem*, int)) );

	connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), rulesBrowser, SLOT(setSelectedRule(QTreeWidgetItem *, int)));


	QDir rulesDir = QDir(QApplication::applicationDirPath() + "/Kumir/PDScripts/");

	QStringList rulesFileList = rulesDir.entryList(QStringList() << "*.rules");
	QBuffer normalizedRules;
	QList<int> priorities;
	for (int i = 0; i < rulesFileList.count(); i++) {
		QString absName = rulesDir.absoluteFilePath(rulesFileList[i]);
		//      char *debug_mac = absName.toUtf8().data();
		rulesFileList[i] = absName;
		priorities << i;
	}
	//  qDebug() << rulesFileList;
	//  qDebug() << priorities;
	PWRulesNormalizer::process(rulesFileList, &normalizedRules, priorities);
	normalizedRules.open(QIODevice::ReadOnly);
	QPoint errCode = m_automata->loadRules(&normalizedRules);
	normalizedRules.close();
	normalizedRules.open(QIODevice::ReadOnly);
	QTextStream ts(&normalizedRules);
	rulesBrowser->setText(ts.readAll());
	rulesBrowser->show();
	normalizedRules.close();
	if (errCode.x() > 0) {
		normalizedRules.open(QIODevice::ReadOnly);
		QTextStream ts(&normalizedRules);
		ts.setCodec("UTF-8");
		QStringList lines = ts.readAll().split("\n", QString::KeepEmptyParts);
		normalizedRules.close();
		qDebug() << "Error processing rules at line " << errCode.y() << ". Error code: " << errCode.x() << ", line: " << lines[errCode.y() - 1];
	}


}

void PDDebugWindow::setCurrentRulesList(const PDStackElem &nonTerminal, const QString &terminal, const QList<RuleRightPart> &rules)
{
	m_currentTerminal = terminal;
	m_currentStackElem = nonTerminal;
	m_currentRules = rules;
	bool treeUpdated = false;
	m_treeUpdated = false;
	emit updateTree();

	while (!treeUpdated) {
		m_treeUpdateLocker->lock();
		treeUpdated = m_treeUpdated;
		m_treeUpdateLocker->unlock();
#ifndef Q_OS_WIN32
#ifndef Q_OS_SOLARIS
		usleep(500);
#endif
#endif
	}
}

void PDDebugWindow::init(const QList<ProgaText> &source, const QString &fileName, ProgramTab *tab)
{
	m_automata->init(source, fileName, tab);
	reset();
	setProgaText(source);
	emit clearScripts();
}

void PDDebugWindow::waitForNextStep()
{
	bool needStop = btnBreakMode->isChecked();
	if (needStop && !m_continuePressed) {
		emit updateStep();
		btnNextStep->setEnabled(true);
		btnContinue->setEnabled(true);
		m_stepWaiter->lock();
	}
}

void PDDebugWindow::updateRulesListSlot()
{

	QStringList labels;
	for (int i = 0; i < m_currentRules.count(); i++) {
		QString label = m_currentStackElem.nonTerminal + " -> ";
		if (m_currentRules[i].isEpsilon) {
			label += "0";
		} else {
			label += m_currentTerminal + " " + m_currentRules[i].nonTerminals.join(" ");
		}
		labels << label;
	}
	for (int i = 0; i < labels.count(); i++) {
		if (m_currentRuleItem == NULL) {
			treeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << labels[i], 0));
		} else {
			m_currentRuleItem->addChild(new QTreeWidgetItem(QStringList() << labels[i], 0));
		}
	}
	if (m_currentRuleItem != NULL) {
		m_currentRuleItem->setExpanded(true);
	}
	treeWidget->resizeColumnToContents(0);
	m_treeUpdateLocker->lock();
	m_treeUpdated = true;
	m_treeUpdateLocker->unlock();
}

void PDDebugWindow::step()
{
	m_stepWaiter->unlock();
}

void PDDebugWindow::play()
{
	m_continuePressed = true;
	m_stepWaiter->tryLock();
	m_stepWaiter->unlock();
}

void PDDebugWindow::reset()
{
	m_runner->terminate();
#ifndef Q_OS_WIN32
#ifndef Q_OS_SOLARIS
	usleep(500);
#endif
#endif
	m_continuePressed = false;
	m_stepWaiter->tryLock();
	treeWidget->clear();
	m_savedRuleItem.clear();
	status->setText("");
	errorCounter->setText("");
	m_currentRuleItem = NULL;
	btnNextStep->setEnabled(false);
	btnContinue->setEnabled(false);
	clearScripts();
}

void PDDebugWindow::processFinished()
{
	btnNextStep->setEnabled(false);
	btnContinue->setEnabled(false);
	btnBreakMode->setChecked(false);
	updateState();
	updateTreeItem();
}

void PDDebugWindow::startProcess()
{
	doClearScript();
	m_runner->start();
}

void PDDebugWindow::setProgaText(const QList<ProgaText> &source)
{
	QString line = QString::fromUtf8("начало ");
	foreach (const ProgaText &pt, source) {
		line += PushdownAutomata::terminalByCode(pt.Base_type) + " ";
	}
	line += QString::fromUtf8("конец");
	textBrowser->setText(line);
}

void PDDebugWindow::updateState()
{
	setCurrentPosition(m_automata->m_currentPosition);
	QStack<QString> stack;
	PDStackElem se;
	for (int i = 0; i < m_automata->m_stack.count(); i++) {
		se = m_automata->m_stack.at(i);
		stack.push(se.nonTerminal + " (" + QString::number(se.iterateStart) + ")");
	}
	setStack(stack);
}

void PDDebugWindow::setCurrentPosition(int position)
{
	QString line = textBrowser->toPlainText();
	QTextDocument *doc = textBrowser->document();
	QTextCursor c(doc);
	QTextCharFormat empty = c.charFormat();
	empty.setFontWeight(QFont::Normal);
	empty.setFontUnderline(false);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.setCharFormat(empty);
	c.movePosition(QTextCursor::StartOfBlock);
	c.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, position);
	c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
	QTextCharFormat fmt;
	fmt.setFontWeight(QFont::Bold);
	fmt.setFontUnderline(true);
	c.mergeCharFormat(fmt);
	c.movePosition(QTextCursor::PreviousWord);
	textBrowser->setTextCursor(c);
	textBrowser->ensureCursorVisible();

}

void PDDebugWindow::setStack(const QStack<QString> &stack)
{
	listWidget->clear();
	QListWidgetItem *current = new QListWidgetItem(listWidget);
	current->setText(m_currentStackElem.nonTerminal + " (" + QString::number(m_currentStackElem.iterateStart) + ")");
	QFont fnt = current->font();
	fnt.setBold(true);
	current->setFont(fnt);
	listWidget->addItem(current);
	for (int i = stack.count() - 1; i >= 0; i--) {
		listWidget->addItem(stack[i]);
	}
}

PDDebugWindow::~PDDebugWindow()
{
}


void PDDebugWindow::setCurrentRuleIndex(int index)
{
	if (m_currentRuleItem == NULL) {
		m_currentRuleItem = treeWidget->topLevelItem(index);
	} else {
		m_currentRuleItem = m_currentRuleItem->child(index);
	}
	if (m_currentRuleItem != NULL) {
		m_nextRuleItem = m_currentRuleItem->child(index);
	}
	emit updateTreeItem();
}

void PDDebugWindow::highlightCurrentRule()
{
	if (m_currentRuleItem == NULL) {
		return;
	}
	QFont fnt = m_currentRuleItem->font(0);
	fnt.setBold(false);
	fnt.setUnderline(true);
	m_currentRuleItem->setFont(0, fnt);
	fnt.setBold(true);
	fnt.setUnderline(false);
	if (m_currentRuleItem->parent() != NULL) {
		m_currentRuleItem->parent()->setFont(0, fnt);
	}
}

void PDDebugWindow::save()
{

	m_savedRuleItem.push(m_currentRuleItem);
}

void PDDebugWindow::restore()
{
	m_restoredRuleItem = m_savedRuleItem.pop();
	bool a = false;
	m_treeUpdated = false;
	emit treeRestored();
	while (!a) {
#ifndef Q_OS_WIN32
#ifndef Q_OS_SOLARIS
		usleep(500);
#endif
#endif
		m_treeUpdateLocker->lock();
		a = m_treeUpdated;
		m_treeUpdateLocker->unlock();
	}
	emit updateStep();


}

void PDDebugWindow::fallbackToRestoredItem()
{
	while (m_currentRuleItem != NULL) {
		QFont fnt = m_currentRuleItem->font(0);
		fnt.setBold(false);
		fnt.setUnderline(false);
		m_currentRuleItem->setFont(0, fnt);
		if (m_currentRuleItem == NULL || m_currentRuleItem == m_restoredRuleItem) {
			break;
		} else {
			m_currentRuleItem = m_currentRuleItem->parent();
		}
	}
	m_treeUpdateLocker->lock();
	m_treeUpdated = true;
	m_treeUpdateLocker->unlock();
}

void PDDebugWindow::doClearScript()
{
	scriptBrowser->clear();
	m_scUpdateLocker->lock();
	m_scUpdated = true;
	m_scUpdateLocker->unlock();
}


void PDDebugWindow::doAppendScript(const QString &txt)
{
	scriptBrowser->append(txt);
	m_scUpdateLocker->lock();
	m_scUpdated = true;
	m_scUpdateLocker->unlock();
}

void RulesBrowser::setSelectedRule(QTreeWidgetItem *item, int)
{
	QString rule = item->text(0).trimmed();
	QString txt = toPlainText();
	QStringList lines = txt.split("\n");
	int lineNo = -1;
	for (int i = 0; i < lines.count(); i++) {
		int sp = lines[i].indexOf(":");
		if (sp == -1) {
			sp = lines[i].indexOf("[");
		}
		QString cr = lines[i].left(sp).trimmed();
		if (rule == cr) {
			lineNo = i;
			break;
		}
	}
	if (lineNo != -1) {
		QTextCursor c(document());
		c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNo);
		c.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
		c.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		setTextCursor(c);
		ensureCursorVisible();
	}
}

void PushdownAutomata::setTooManyErrors()
{
	for (int i = 0; i < m_modules.size(); i++) {
		for (int j = 0; j < m_modules.at(i).size(); j++) {
			if (m_modules[i][j].error > 0) {
				m_modules[i][j].error = PUSHDOWN_AUTOMATA_TOO_MANY_ERRORS;
			}
		}
	}
	for (int j = 0; j < m_mainModule.size(); j++) {
		if (m_mainModule[j].error > 0) {
			m_mainModule[j].error = PUSHDOWN_AUTOMATA_TOO_MANY_ERRORS;
		}
	}
	for (int j = 0; j < m_text.size(); j++) {
		if (m_text[j].Error > 0) {
			m_text[j].Error = PUSHDOWN_AUTOMATA_TOO_MANY_ERRORS;
		}
	}
}
