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
#ifndef KUMIR_TOOLS_H
#define KUMIR_TOOLS_H


#include "const.h"
#include "error.h"
//#include "strtypes.h"
#include "int_proga.h"
#include "lineprop.h"
#include "enums.h"

#include <QThread>


enum InputError {
	OK,
	unpairedQuote,
	quoteMisplace,
	differentQuotes,
	longCharect,
	notReal,
	notBool,
	notInteger
};

struct InputTerm {
	PeremType pType;
	QString text;
	bool valid;
	InputError error;
	bool allow_left;
	bool allow_right;
	int start;
	InputTerm operator+ (InputTerm other) {
		InputTerm result;
		QString sp;
		for ( int i=start+text.length(); i<other.start; i++ )
			sp += " ";
		result.text = text + sp + other.text;
		result.valid = valid && other.valid;
		result.allow_right = other.allow_right;
		result.allow_left = allow_left;
		result.start = start;
		return result;
	}
};


struct ErrorPosition
{
	// 	ErrorPosition() { start = -1; end = -1; start_incl = -1; end_incl = -1; }
	// 	ErrorPosition(int a, int b, int c, int d)
	// 	{
	// 		start = a;
	// 		end = b;
	// 		start_incl = c;
	// 		end_incl = d;
	// 	}
	// 	int start;
	// 	int end;
	// 	int start_incl;
	// 	int end_incl;
	ErrorPosition() { start = 0; len = 0; }
	ErrorPosition(int start, int len) { this->start = start; this->len = len; }
	int start;
	int len;
};

class UrlFetcher
	: public QThread
{
public:
	UrlFetcher(QObject *parent = 0);
	int GET(const QUrl &url, QByteArray &result);
private:
	void run();
	QUrl m_url;
	QByteArray *m_buffer;
	QNetworkAccessManager *m_accessManager;
	int i_result;
};

class KumTools: public QObject
{
	Q_OBJECT
public:
	KumTools();
	static KumTools* instance();
	void initNormalizator(const QString &fileName);
	void init();
	inline void setAllowHiddenNames(bool v) { m_allowHiddenNames = v; }
	int test_skobk ( QString stroka );
	Kumir::InstructionType getStringType ( const QString &stroka );
	int ks_count ( QString stroka,int begin,int end );
	int getPosByKS ( int ks_id,QString stroka );
	int getKSCount ( int pos,QString stroka );
	//        QString naznachitKostil(QString setLine,int *err);//Костыль переназначения вводв вывода
	QString *ispKwd;
	QString *useKwd;

	QString getKeyWordsHTMLList() const;

	bool isBooleanConstant(const QString &text) const;
	void setExtraCharset(const QString &charsetSpec);
	bool isValidChar(const QChar &ch) const;

	/** Выполняет АГК/МР-разбор строки ввода
	  * @param format форматная строка в C-виде (%i%f%c%b%s)
	  * @param txt строка, которую разбираем
	  * @return result список строк-лексем
	  * @return error тест ошибки, либо пустая строка
	  * @returns true, если успешно, false в противном случае
	  */
	bool parceInputString( const QString &format
						 , const QString &txt
						 , QStringList &result
						 , QString &error
						 );


	int test_name ( QString stroka,int &pos, int &len );
	QString normalizeString ( QString stroka,int * err, int *err_start, int *err_end );
	QString SplitPrisvBlock ( QStringList p_List, QStringList *p_Result, int p_Num, bool p_FuncParam );
//	int splitUserStr ( QString stroka,QList<ProgaText>* result,int str );
	int splitUserText ( KumirEdit* textEdit,QList<ProgaText>* result,int BASE );

	int splitUserTextV2 ( QString text, QList<LineProp> *textProp,QList<ProgaText>* result,int BASE );//16.07.07-Mordol
	int splitFStringList ( QStringList FSpisok,QList<ProgaText>* result,int str,int ID,int offset,QString file, const QList<QList<int> > &PP, const QList<QList<int> > &LL, const QString &sourceText ); //16.07.07-Mordol

	int splitUserIsp ( QStringList *stroki,QList<ProgaText>* result,QString file );
	QStringList splitElse ( QString normal_else_string );
	QStringList splitTo ( QString normal_to_string );
	bool allowIndent( const int str_type, const int err_code );
	QPoint getIndentRang( const ProgaText &ptl);
	QPoint getIndentRang( const QList<ProgaText> &ptl);

	int splitUserLine ( const QString &text, QPoint &errorPosition, QList<ProgaText> *result );
//	int splitUserFile ( QFile &file, QList< QList<ProgaText> > *result );

	QVector<bool> recalculateErrorPositions ( const QString &text, const QList<ProgaText> &ptl );
	bool inLit ( QString s, int pos );

	/**
		 * Нормализация строки
		 * @param src IN: строка исходного Кумир-текста
		 * @param P OUT: вектор начальных позиций нормализованных символов в исходной строке
		 * @param L OUT: вектор длин нормализованных символов в исходной строке
		 * @param error OUT: код ошибки при нормализации
		 * @param err_start OUT: позиция начала ошибки в исходной строке
		 * @param err_len OUT: длина ошибочной области в исходной строке
		 * @return нормализованная строка
		 */
	QString normalize(const QString & src, QList<int> &P, QList<int> &L, int &error, int &err_start, int &err_len);
	int includeTextOrBytecode(const QString &src, QList<ProgaText> &data);
	void trim(QString &s, QList<int> &P, QList<int> &L ) ;
	QRegExp *rxKeyWords;
	QRegExp *rxTypes;
	QRegExp *rxConst;
	QStringList getKeyWords();
	QStringList getTypeNames();
	QStringList getConstNames();

	inline void setProgramFileName(const QString &v) { s_fileName = v; }
	inline QString programFileName() const { return s_fileName; }

private:
	// copyed from KumIOArea!!!
	QStringList finishInput();
	bool tryFinishInput(const QString &txt, QStringList &result, QString &error);
	bool validateInput(QString txt);
	void input(const QString &format);
	QList<InputTerm> words;
	QList<PeremType> types;
	QStringList formats;
	int inputGarbageStart;
	// end copy from KumIOArea


	QHash<QString,QString> *kwdMap;
	QRegExp *rxCompound;
	void initNormalizator();
	QRegExp rxLitBreaks;
	QString Breaks;
	QRegExp brks;
	QString zamena ( QString string );
	QRegExp rx_NOT;
	QRegExp rx_OR;
	QRegExp rx_AND;
	QRegExp rx_CEL_TAB;
	QRegExp rx_LIT_TAB;
	QRegExp rx_VES_TAB;
	QRegExp rx_SIM_TAB;
	QRegExp rx_LOG_TAB;
	QRegExp rx_CEL;
	QRegExp rx_LIT;
	QRegExp rx_VES;
	QRegExp rx_SIM;
	QRegExp rx_LOG;
	QRegExp rx_ESLI;
	QRegExp rx_TO;
	QRegExp rx_INACHE;
	QRegExp rx_VSE;
	QRegExp rx_VIBOR;
	QRegExp rx_PRI;
	QRegExp rx_ALG;
	QRegExp rx_ARG;
	QRegExp rx_NACH;
	QRegExp rx_KON;
	QRegExp rx_NC;
	QRegExp rx_KC;
	QRegExp rx_DLYA;
	QRegExp rx_RAZ;
	QRegExp rx_POKA;
	QRegExp rx_NS;
	QRegExp rx_ISP;
	QRegExp rx_ENDISP;
	QRegExp rx_DANO;
	QRegExp rx_NADO;
	QRegExp rx_EXIT;

	QRegExp rx_OT;
	QRegExp rx_DO;
	QRegExp rx_SHAG;
	QRegExp rx_REZ;
	QRegExp rx_ARGREZ;
	QRegExp rx_VIVOD;
	QRegExp rx_DA;
	QRegExp rx_NET;
	QRegExp rx_UTV;

	QRegExp *KSIMS;//10-08-Mordol
	QString st_CEL;
	QString st_LIT;
	QString st_LOG;
	QString st_SIM;
	QString st_VES;

	QStringList compounds;
	QStringList compounds2;
	QStringList keyWords;
	QStringList typeNames;
	QStringList constNames;

	static KumTools* m_instance;
	QList<ushort> m_extraChars;
	QList< QPair<ushort,ushort> > m_extraCharRanges;
	bool m_allowHiddenNames;

	UrlFetcher *m_urlFetcher;
	QString  s_fileName;

};
#endif
