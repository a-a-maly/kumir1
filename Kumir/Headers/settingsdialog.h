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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_SettingsDialog.h"

const QString strRobotStartField = "10x16.fil";

/** Диалог настроек */
class SettingsDialog : public QDialog, private Ui::SettingsDialog
{
	Q_OBJECT

public:
	/**
	 * Конструктор
	 * @param parent ссылка на объект-владелец
	 * @param fl флаги окна
	 */
	SettingsDialog(QWidget *parent = 0, Qt::WindowFlags fl = 0);
	/**
	 * Деструктор
	 */
	~SettingsDialog();
//		static QString askForPDFReader(QWidget *parent = 0);
	static QString defaultTextEditor();

public slots:
	/**
	 * Выполнение диалога. Предварительно инициализирует значения настраиваемых параметров
	 * @return код возврата QDialog
	 */
	int exec();
	/**
	 * Нажатие кнопки OK
	 */
	virtual void accept();
	/**
	 * Нажатие кнопки Cancel
	 */
	virtual void reject();



protected:
	/** Флаг "язык изменен" */
	bool languageChanged;
	/** Флаг "изменен путь стандартных исполнителей" */
	bool stdLibPathChanged;
	/**
	 * Возвращает цвет кнопки. Используется при настройке цветов
	 * @param btn ссылка на кнопку
	 * @return цвет
	 */
	QColor btnColor(QToolButton *btn);
	/**
	 * Устанавливает цвет кнопки. Используется при настройке цветов
	 * @param btn ссылка на кнопку
	 * @param color цвет
	 */
	void setBtnColor(QToolButton *btn, QColor color);
	void setLabels();
	QStringList m_pdfNames, m_pdfPaths;
	void loadSettingsToDialog();

protected slots:
	/**
	 * Устанавливает шрифт в облати просмотра шрифта
	 * @param font шрифт
	 */
	void fontFamilyChanged(const QFont &font);
	/**
	 * Устанавливает размер шрифта в области просмотра шрифта
	 * @param size размер шрифта
	 */
	void fontSizeChanged(int size);
	void fontBoldChanged(int state);
	/**
	 * Открывает диалог выбора каталога и устанавливает каталог стандартных исполнителей
	 */
	void browseStdLib();
	void browseExternalEditor();
	/**
	 * Открывает диалог выбора каталога и устанавливает каталог ввода-вывода
	 */
	void browseIO();
	/**
	 * Открывает диалог выбора каталога и устанавливает каталог с файлами гипертекстов
	 */
	void browseHtml();
	/**
	 * Открывает диалог выбора файла и устанавливает файл со стартовой обстановкой Робота
	 */
//      void browseStartField();
	/**
	 * Берет текст из текущего редактора и вставляет его в "Текст программы при запуске"
	 */
	void getTextFromEditor();
	/**
	 * Открывает диалог выбора цвета и устанавливает цвет кнопки
	 */
	void chooseColor();
	/**
	 * Делает доступным или недоступным поле ввода размера фикс. ширины ввода-вывода
	 * @param v значение флага ( см. константы Qt )
	 */
	void fixedSizeChanged(int v);
	/**
	 * Делает доступным или недоступным поле ввода цвета подсветки ввода
	 * @param v значение флага ( см. константы Qt )
	 */
	void highlightInputChanged(int v);

	void setDefaultSettings();

};

#endif
