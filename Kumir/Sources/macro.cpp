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
#include "macro.h"
#include "textedit.h"
#include "application.h"
#include <QtGlobal>
#include <QAction>
#include <QDomNode>

Macro::Macro(QObject *parent) : QObject(parent)
{
	m_action = NULL;
	editable = true;
}

void Macro::setKeySequence(const QString &sk)
{
	m_keySequence = sk;
}

void Macro::setName(QString name)
{
	m_name = name;
}

QString Macro::keySequence()
{
	if (m_nativeKeySequence.isEmpty()) {
		return m_keySequence;
	} else {
		return m_nativeKeySequence;
	}
}

QString Macro::name()
{
	return m_name;
}

Macro::~Macro()
{
	if (m_action != NULL) {
		delete m_action;
	}
}

void Macro::addCommand(QKeyEvent *e)
{
	QKeyEvent ee(e->type(),
		e->key(),
		e->modifiers(),
		e->text(),
		e->isAutoRepeat(),
		e->count());
	// TODO make events filtering
	events << ee;
}

QList<QKeyEvent> Macro::commands()
{
	return events;
}

void Macro::setAction(QAction *a)
{
	m_action = a;
}

QAction *Macro::action()
{
	return m_action;
}

void Macro::save(QDomDocument &parent)
{
	QDomNode root = parent.createElement("macro");
	root.toElement().setAttribute("name", m_name);
	root.toElement().setAttribute("keySequence", m_keySequence);
	foreach (QKeyEvent event, events) {
		QDomElement node = parent.createElement("event");
		node.setAttribute("type", (uint)(event.type()));
		node.setAttribute("key", event.key());
		node.setAttribute("modifiers", (uint)(event.modifiers()));
		node.setAttribute("text", event.text());
		node.setAttribute("autoRepeat", event.isAutoRepeat() ? "1" : "0");
		node.setAttribute("count", event.count());
		root.appendChild(node);
	}
	parent.appendChild(root);
}

bool Macro::load(const QDomNode &data)
{
	QDomNode root = data.toDocument().firstChild();
	if (root.isElement() && (root.nodeName() == "macro")) {
		m_name = root.toElement().attribute("name", "");
		if (m_name.isEmpty()) {
			return false;
		}
#ifdef Q_OS_MAC
		m_name.replace("ESC, ", "Control+");
#endif
		QString sequence = root.toElement().attribute("keySequence", "");
		QString macSequence = root.toElement().attribute("macSequence", "");
		QString winSequence = root.toElement().attribute("winSequence", "");
		QString x11Sequence = root.toElement().attribute("x11Sequence", "");
		QString winceSequence = root.toElement().attribute("winceSequence", "");
		QString symbianSequence = root.toElement().attribute("symbianSequence", "");
		QString maemoSequence = root.toElement().attribute("maemoSequence", "");
		QString nativeSequence;

#ifdef Q_OS_UNIX
		nativeSequence = x11Sequence;
#endif

#ifdef Q_OS_MAC
		nativeSequence = macSequence;
#endif

#ifdef Q_OS_WIN32
		nativeSequence = winSequence;
#endif

#ifdef Q_OS_WINCE
		nativeSequence = winceSequence;
#endif

		// TODO Symbian and Maemo detection

		if (nativeSequence != "") {
			m_nativeKeySequence = nativeSequence;
		}


#ifdef Q_OS_MAC
		char *debug = sequence.toUtf8().data();
		sequence.replace("Esc, ", "Meta+");
		debug = sequence.toUtf8().data();
#endif
		if (sequence.isEmpty()) {
			return false;
		}
		for (int i = 0; i < app()->macros.count(); i++) {
			if (app()->macros[i]->name() == m_name) {
				return false;
			}
			if (app()->macros[i]->keySequence() == sequence) {
				return false;
			}
		}

		m_keySequence = sequence;
		QDomNode node = root.firstChild();
		while (!node.isNull()) {
			if (node.isElement() && (node.nodeName() == "event")) {
				uint type = node.toElement().attribute("type", "0").toUInt();
				int key = node.toElement().attribute("key", "-1").toInt();
				uint modifiers = node.toElement().attribute("modifiers", "0").toUInt();
				QString txt = node.toElement().attribute("text", "");

				uint autoRepeat = node.toElement().attribute("autoRepeat", "0").toUInt();
				int count = node.toElement().attribute("count", "1").toInt();
				QKeyEvent event = QKeyEvent((QEvent::Type)(type),
						key,
						(Qt::KeyboardModifiers)(modifiers),
						txt,
						autoRepeat == 1,
						count);
				events << event;
			}
			node = node.nextSibling();
		}
		return true;
	} else {
		return false;
	}

}

bool Macro::isEditable()
{
	return editable;
}

void Macro::setEditable(bool v)
{
	editable = v;
}

void Macro::setFileName(const QString &s)
{
	m_fileName = s;
}

QString Macro::fileName()
{
	return m_fileName;
}
