/****************************************************************************
**
** Copyright (C) 2004-2008 IMPB RAS. All rights reserved.
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
#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>

class text_analiz;
class KumirEdit;
class KumirMargin;
class PushdownAutomata;

#include "kum_tables.h"
#include "int_proga.h"
#include "lineprop.h"
#include "files_isp.h"
#include "files_p_isp.h"
#include "extstrings.h"
#include "kummodules.h"


#define USER_USE 0
#define ROBO_USE -2
#define ROBO_ST_USE -3
#define DRAW_USE -5

#define STRINGS_USE -4
#define FILES_P_USE -7
/** Нагрузка текста из редактора */
typedef QList<LineProp> TextProp;
struct IspLine {
	int type;
	QString line;
	int error;
	proga p;
};

/** Класс потока-компилятора */
class Compiler : public QObject
{
	Q_OBJECT
	struct ModuleError {
		int code;
		QString message;
		int from;
		int length;
	};

	struct RawModule {
		QString name;
		int id;
		QUrl url;
		QList<ModuleError> errors;
		QList<proga> content;
		QStringList uses;
	};

public:
	/**
	 * Конструктор
	 * @param parent сслыка на объект-владелец
	 */
	Compiler(QObject *parent = 0);
	void loadKumirModules();
	QString firstAlgorhitmText() const;
	/**
	     * Инициализирует компилятор перед выполнение компиляции.
	     * @param tabNo номер вкладки редактора с текстом или -1, если нужно компилировать весь текст
	     */
	void init(int tabNo);

	/**
	     * Выполняет инициализацию втроенных функций
	     */
	void initInternalFunctions();
	inline PushdownAutomata *getPdAutomata() const
	{
		return this->pdAutomata ;
	}
	inline text_analiz *getAnalizer() const
	{
		return this->analizer;
	}
	KumSingleModule *mainModule();
	/** Деструктор класса */
	~Compiler();
	/** Выполняет компиляцию в основном потоке. */
	inline void start()
	{
		run();
		updateEditor();
	}

	//    /** Ссылка на таблицу символов */
	//    symbol_table *symbols;//TODO Modules
	//    /** Ссылка на таблицу функций */
	//    function_table *functions;
	//    /** Ссылка на таблицу исполнителей */

	inline KumirEdit *curEditor()
	{
		return currentEditor;
	}
	QList<proga> *ProgaValue()
	{
		return &Proga_Value;
	};
	KumModules *modules()
	{
		return &Modules;
	};

	bool isEmpty(int moduleNo);

	proga compiledLine(int str, int module_id);


	inline int usesListCount() const
	{
		return uses_list.count();
	}

	int StartPos();

	/**
	     * Выполняет замену имен
	     * @param source исходный текст
	     * @param dict словарь замен
	     * @return новый текст
	     */
	QString refactorText(const QString &source, QMap<QString, QString> dict);

	/**
	     * Возвращает список имен идентификаторов в тексте
	     * @param source текст программы
	     * @return список имен
	     */
	QStringList extractNames(const QString &source);


	/**
	     * загружает исполнители. И записывает их в ispTable
	     * @return здвиг
	     */
	int scanIsps();

	/**
	     * Возвращает список имен идентификаторов в тексте
	     * @param source текст программы
	     * @return здвиг
	     */
	int compileIsp(QStringList *source, QString file_name, int *err);

	QStringList getIspText(QString *FileName, int *err, int use_isp_no);

	/**
	     * Отчистка таблиц symbols fuctions IspFiles Proga_Text
	     * Proga_Value
	     * @param isp_id Номер файла в таблице файлов исполнителей
	     * @return сколько элементов Proga_Text удалено
	     */
	//int cleanTables(int isp_id); //Отчистка ТАБЛИЦ

	/** Скомпилированный байт-код */
	QList<proga> Proga_Value;
	int newCleanTables(int isp_id);

	/** Внутреннее представление */
	KumModules Modules;
	int compileModule(int module_id);
	int loadModulesInfo();
	void compileAlgList(int module_id, QStringList algList);



signals:
	void changeCountErrors(int count);

	void kumInstrumentLoaded(int isp_id);
	void kumInstrumentUnLoaded(int isp_id);
	void updateEditorFinished();
	void firstAlgorhitmAvailable(bool);
protected slots:
	/**
	     * Перерисовывает редактор(ы) в соответствии с результатом компиляции.
	     * @param nowait если true, то не ждет освобождения блокировки редактора
	     */
	void updateEditor();

	/** Подлючение исполнителей **/
	void ispReady();
protected:

	/** Основная функция потока. Выполняет разбор текста */
	void run();
	/** Возвращает список испольовать */
	QList<IspLine> UsesList();
	/** Добавляет встроенную функцию в таблицы */
	void addInternalFunct(QString description, QString ns = "internal");
	/** Ссылка на анализатор программ */
	text_analiz *analizer;
	/** Флаг разбора текста только в текущей вкладке */
	bool singleEditor;
	/** Ссылка на редактор с разбираемым текстом */
	KumirEdit *currentEditor;
	/** Ссылка на поля редактора с разбираемым текстом */
	KumirMargin *currentMargin;
	/** Ссылка на вкладку с разбираемым текстом */
	ProgramTab *currentTab;
	QList<proga> useIspErrors;


	/** Парсировка исп файла*/
	int parceModulesInfoFile(QIODevice *source);
	/** Нормализованный текст программы */
	//QList<ProgaText> *Proga_Text;


	/** Текст разбираемой программы */
	QString text;
	/** Нагрузка разбираемой программы */
	TextProp textProp;
	QTextEdit *outputWindow;
	QList<QFile *> IspFiles;
	QList<IspLine> uses_list;
	int oldTabNo;
	int fileTabNo(QString FileName);
	void UnloadByUseType(int Use_Type, QString fileName);
	void appendProgaValue();
	void removeIsp(int first_str_id);
	/**
	     * Полная перегрузка исполнителей если изменились встроенные.
	     * @return -1 если исполнители перегружены 0 если нет;
	     */
	int compareLists(QList<IspLine> *uses_list, QList<IspLine> *cur_list);
	void compileUserModules(QList< QList<proga> > rawModules);

	//  KumSingleModule* main;
	//  int main_id;
	QList< QList< ProgaText > > normalizedText;
	//  QList<int> userModIds;

	PushdownAutomata *pdAutomata;
	QString modulesInfoPath;

	void scanForModules(const QString &modulesPath);
	QList<RawModule> extractModulesFromFile(const QUrl &url, const QByteArray &data) const;
	QList<ModuleError> buildTablesForModules(QList<RawModule> &modules);
	QList<ModuleError> analizeModules(QList<RawModule> &modules);

};



#endif
