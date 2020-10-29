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
#ifndef KUM_FILES_H
#define KUM_FILES_H
#include "kum_instrument.h"
#include "error.h"

class STFile
{
public:
	STFile()
	{
		ready = false;
		in_skobka = false;
		in_kavichka = false;
		in_comment = false;
		file = NULL;
		fileError = 0;
		byteRead = false;
		codec = QTextCodec::codecForName("KOI8-R");
		i_handle = -1;
	};
	~STFile()
	{
		if (ready) {
			CloseFile();
			delete file;
		} else {
			return;
		}
	};
	int OpenFile(QString fileName, bool readOrWrite); //true  - write false - read
	int OpenByteFile(QString fileName, bool readOrWrite); //true  - write false - read

	void CloseFile();
	bool SkipSpaces();//TODO impl.
	bool atEnd()
	{
		if (!ready) {
			qDebug("STFile not ready!");
		}
		if (byteRead) {
			return dataStream.device()->bytesAvailable() == 0;
		}
		int pos = fileData.pos();
		Q_UNUSED(pos);
		bool dataEnd = fileData.atEnd() && cur_symb == QChar(0);
		return (dataEnd);
	};
	int bytesAv()
	{
		return dataStream.device()->bytesAvailable();
	};
	QString name()
	{
		if (!ready) {
			return "NOFILE";
		};
		return file->fileName();
	};
	int handle()
	{
		if (!ready) {
			return -1;
		};
#ifdef WIN32
		if (i_handle == -1) {
			generateHandle();
		}
		return i_handle;
#else
		return file->handle();
#endif
	};
	bool forWrite()
	{
		if (!ready) {
			qDebug("forWrite? - STFile not ready!");
			return false;
		};
		return write;
	};
	bool WriteData(QString data)
	{
		if (!ready) {
			qDebug("forWrite? - STFile not ready!");
			return false;
		};
		fileData << data;
		return true;
	};
	bool Flush()
	{
		if (!ready) {
			qDebug("forWrite? - STFile not ready!");
			return false;
		};
		fileData.flush();
		return file->flush();
	};
	int error()
	{
		return fileError;
	};
	void ResetFile()
	{
		if (isByteRead()) {
			qDebug() << "Reset Byte file";
			file->setTextModeEnabled(false);
			file->reset();
			file->seek(0);
			ready = true;
		};
		if (!ready) {
			qDebug("STFile not ready!");
		}
		prepareFile();
	};
	bool isLexemBreak()
	{
		if (!ready) {
			qDebug("STFile not ready!");
		}
		if ((cur_symb == ',') || (cur_symb.unicode() == 10) || (cur_symb.isSpace())) {
			return true;    //PRAVKA SEPT09
		} else {
			return false;
		}
	};
	QString getIntLexem(int *err, int *pos);
	QString getStrLexem(int *err, int *pos);
	QString getRealLexem(int *err, int *pos);
	QString getBoolLexem(int *err, int *pos);
	QString getCharLexem(int *err, int *pos);
	int readByte();
	int writeByte(quint8 data)
	{
		qDebug() << "TOREC:" << data;
//	if((data<0)||(data>255))return INTFUN_NOT_KOI8_RANGE;
		dataStream << data;
		return 0;
	};
	bool isByteRead()
	{
		return byteRead;
	};
	void setByteRead(bool br)
	{
		byteRead = br;
	};
private:
	bool byteRead;//Читаем по байтам?
	int prepareFile();
	bool checkSymb(QChar symb);
	QFile *file;
	bool ready;
	bool write;
	bool in_skobka, in_kavichka, in_comment;
	QTextStream fileData;
	int fileError;
	QChar cur_symb;
	QTextCodec *codec;
	int i_handle;
	void generateHandle();
	void releaseHandle();
	static QList<int> usedHandles;
	QDataStream dataStream;
};
//Работа с файлами
class KumFiles
	: public KumInstrument

{
	Q_OBJECT
public:
	/**
	 * Конструктор
	 * @param parent сслыка на объект-владелец
	 */
	KumFiles(QWidget *parent = 0);

	bool isReady()
	{
		return ready;
	};
public:
	int getFunctionList(function_table *Functions, symbol_table *Symbols);
	QString readLexem(int key, int *err);
	bool CheckKey(int key)
	{
		if (getIdByKey(key) < 0) {
			return false;
		} else {
			return true;
		}
	};
	bool atEnd(int key, int *err)
	{
		if (getIdByKey(key) < 0) {
			*err = 7010;
			return false;
		};
		return fileTable[key]->atEnd();
	};

public slots:
	void runFunc(QString name, QList<KumValueStackElem> *aruments, int *err);

	QString getLexem(PeremType type, int key, int *err, int *pos);

	int writeText(int key, QString text, int *pos);
	void close();
	void reset(void)
	{
		qDebug("Files Reset");
		for (int i = 0; i < fileTable.count(); i++) {
			fileTable[i]->CloseFile();
		}
		fileTable.clear();
	};
	bool flushFile(int key)
	{
		int id = getIdByKey(key);

		if (id < 0) {
			return RT_FILE_NO_KEY;
		};
		return fileTable[id]->Flush();
	};

signals:
	void sync();
private:
	bool isAbsPath(QString FileName)
	{
#ifndef WIN32
		if (FileName.startsWith("/")) {
			return true;
		}
#else
		QRegExp diskName("[a-z]\\:\\\\", Qt::CaseInsensitive);
		if (diskName.indexIn(FileName) == 0) {
			return true;
		}
#endif
		return false;
	};

	bool isOpen(QString fileName);
	int getIdByKey(int key)
	{
		for (int i = 0; i < fileTable.count(); i++) {
			if (fileTable[i]->handle() == key) {
				return i;
			}
		};
		return -1;
	};


	function_table *functions;
	symbol_table *symbols;
	QTextCodec *codec;
	bool ready;
	QList<STFile *> fileTable;


};

#endif
