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
#include "files_isp.h"
#include "application.h"
#include "mainwindow.h"
#include "compiler.h"
#include <QSettings>

//______________________________________________
//          STFile
//_____________________________________________

int STFile::OpenFile(QString fileName, bool Write)
{
	file = new QFile(fileName);

	if (!file->exists()) {
		qDebug() << fileName;
		return RT_ERR_FILE_NO_EXISTS;
	};
	if (!Write) {

		if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
			if (file->handle() == -1) {
				qDebug("STFile error:handle == -1!");
			}
			write = false;

			return prepareFile();
		} else {
			return RT_ERR_FILE_NOT_OPENED;
		}

	} else {
		if (file->open(QIODevice::WriteOnly)) {
			if (file->handle() == -1) {
				qDebug("STFile error:handle == -1!");
			}
			write = true;
			ready = true;
			fileData.setCodec(codec);
			fileData.setDevice(file);
			fileData.setCodec(codec);
			return 0;
		} else {
			return RT_ERR_FILE_NOT_OPENED;
		}
	};
	return 0;
};


int STFile::OpenByteFile(QString fileName, bool Write)
{
	file = new QFile(fileName);

	if (!file->exists()) {
		qDebug() << fileName;
		return RT_ERR_FILE_NO_EXISTS;
	};
	if (!Write) {

		if (file->open(QIODevice::ReadOnly)) {
			if (file->handle() == -1) {
				qDebug("STFile error:handle == -1!");
			}
			write = false;
			byteRead = true;
			ready = true;
			dataStream.setDevice(file);
			qDebug() << "Bytes:" << dataStream.device()->bytesAvailable();
			return 0;
		} else {
			return RT_ERR_FILE_NOT_OPENED;
		}

	} else {
		if (file->open(QIODevice::WriteOnly)) {
			if (file->handle() == -1) {
				qDebug("STFile error:handle == -1!");
			}
			write = true;
			ready = true;
			byteRead = true;
			dataStream.setDevice(file);
			return 0;
		} else {
			return RT_ERR_FILE_NOT_OPENED;
		}
	};
	return 0;
};

void STFile::CloseFile()
{
	if (!ready) {
		qDebug("STFile:file not ready");
		return;
	} else {

		if (!file) {
			qDebug("STFile:NULL");
		}
		if (file->handle() == -1) {
			qDebug("STFile error:handle == -1!");
			return;
		};
		qDebug("STFile: %i closed.", file->handle());
		ready = false;
		file->close();
		releaseHandle();
		fileError = 0;
	};
};


void STFile::generateHandle()
{
	int h = -1;
	bool found = false;
	do {
		h++;
		found = false;
		for (int i = 0; i < usedHandles.count(); i++) {
			if (usedHandles[i] == h) {
				found = true;
			}
		}
	} while (found);
	i_handle = h;
	usedHandles << i_handle;
}

QList<int> STFile::usedHandles = QList<int>();

void STFile::releaseHandle()
{
	usedHandles.removeAll(i_handle);
}

int STFile::prepareFile()
{
	file->reset();

	fileData.setCodec(codec);
	fileData.setDevice(file);
	fileData.setCodec(codec);
	fileData.seek(0);
	fileData >> cur_symb;

	SkipSpaces();
	ready = true;
	return 0;
}


bool STFile::SkipSpaces()
{

	if (!checkSymb(cur_symb)) {
		return false;
	}
	while (((in_comment) || (cur_symb.isSpace()) || (cur_symb.unicode() == 10)) && (!atEnd())) {
		//TODO Check!!!!
		fileData >> cur_symb;
		if (!checkSymb(cur_symb)) {
			return false;
		}
		if (cur_symb == '\n') {
			in_comment = false;
		}

	};
	if (atEnd() && (in_kavichka || in_skobka)) {
		fileError = RT_INTERNAL_FILE_ERROR;
		return false;
	};

	return true;
	//TODO Ipliment
};
bool STFile::checkSymb(QChar symb)
{
	if (symb.isSpace()) {
		return true;
	}



	if (in_comment) {
		return true;
	}
	if (symb == '|') {
		in_comment = true;
	} else if (symb == '(') {

		if (!in_skobka) {
			in_skobka = true;
		} else {
			fileError = RT_INTERNAL_FILE_ERROR;
			return false;
		};
	} else if (symb == ')') {
		if (in_skobka) {
			in_skobka = false;
		} else {
			fileError = RT_INTERNAL_FILE_ERROR;
			return false;
		};
	} else if (symb == '\"') {
		if (in_kavichka) {
			in_kavichka = false;
		} else {
			in_kavichka = true;
		};
	} else if (symb == '\"') {
		if (in_skobka) {
			in_skobka = false;
		} else {
			fileError = RT_INTERNAL_FILE_ERROR;
			return false;
		};
	};

	return true;
};

//� азбор лексем
QString STFile::getStrLexem(int *err, int *pos)
{
	qDebug() << "Symb" << cur_symb;
	SkipSpaces();

	QChar kav;
	//fileData>>cur_symb;
	QString dbg;
	dbg += cur_symb;
	qDebug() << "Symb" << cur_symb;
	if (isLexemBreak()) {
		fileData >> cur_symb;
		SkipSpaces();
	};
	if ((cur_symb != '"') && ((cur_symb != '\''))) {
		*err = RT_INTERNAL_FILE_ERROR;
		*pos = file->pos();
		return "WRONG TYPE";
	};
	kav = cur_symb;
	in_kavichka = true;

	if (atEnd() && cur_symb == QChar(0)) {
		*err = RT_INTERNAL_FILE_ERROR;
		return "";
	};
	int fpos = fileData.pos();
	if (fpos < 1) {
		fileData >> cur_symb;
	}
	fileData >> cur_symb;
	QString toret = "";
	//toret+=cur_symb;
	while ((cur_symb != kav) && (!fileData.atEnd())) {
		if (fileData.pos() > 0) {
			toret += cur_symb;
		}
		fileData >> cur_symb;
		qDebug() << "CurStrToRet" << toret;
	};

	in_kavichka = false;
	fileData >> cur_symb;
	if (cur_symb == '|') {
		in_comment = true;
	}
	if (!in_comment && !isLexemBreak() && !atEnd() && !cur_symb.isSpace()) {
		*err = RT_INTERNAL_FILE_ERROR;
		return "";
	};
	SkipSpaces();

	return toret;
};
QString STFile::getIntLexem(int *err, int *pos)
{
	Q_UNUSED(pos);
	QString toret;
	if (atEnd() && cur_symb == QChar(0)) {
		*err = RT_ERR_READ_AT_EOF;
		return "";
	};

	SkipSpaces();
	if (isLexemBreak()) {
		fileData >> cur_symb;
		SkipSpaces();
	};

	while (((cur_symb != ',') && (cur_symb != '|')) && (!cur_symb.isSpace()) && (cur_symb != QChar(0) || !atEnd())) {
		if (fileData.pos() > 0 && cur_symb != QChar(0)) {
			toret += cur_symb;
		}
		fileData >> cur_symb;
	};
	if (atEnd() && cur_symb != QChar(0)) {
		toret += cur_symb;
	}
	if (cur_symb == '|') {
		in_comment = true;
	}
	SkipSpaces();
	return toret;
};

QString STFile::getRealLexem(int *err, int *pos)
{
	Q_UNUSED(pos);
	QString toret;
	if (atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return "";
	};

	SkipSpaces();
	int fpos = fileData.pos();
	if (isLexemBreak()) {
		fileData >> cur_symb;
		SkipSpaces();
	};
	fpos = fileData.pos();
	while (((cur_symb != ',') && (cur_symb != '|')) && (!cur_symb.isSpace()) && (cur_symb != QChar(0) || !atEnd())) {
		if (fpos != 0) {
			toret += cur_symb;
		}
		fileData >> cur_symb;
		fpos = fileData.pos();
	};
	if (atEnd() && cur_symb != QChar(0)) {
		toret += cur_symb;
	}
	if (cur_symb == '|') {
		in_comment = true;
	}

	qDebug() << "CurSymb" << cur_symb;
	SkipSpaces();
	qDebug() << "CurSymb" << cur_symb;
	return toret;
};
QString STFile::getBoolLexem(int *err, int *pos)
{
	Q_UNUSED(pos);
	QString toret;
	if (atEnd() && cur_symb == QChar(0)) {
		*err = RT_ERR_READ_AT_EOF;
		return "";
	};

	SkipSpaces();
	if (isLexemBreak()) {
		fileData >> cur_symb;
		SkipSpaces();
	};

	while (((cur_symb != ',') && (cur_symb != '|')) && (!cur_symb.isSpace()) && (cur_symb != QChar(0) || !atEnd())) {
		if (fileData.pos() > 0) {
			toret += cur_symb;
		}
		fileData >> cur_symb;
	};
	if (atEnd() && cur_symb != QChar(0)) {
		toret += cur_symb;
	}
	if (cur_symb == '|') {
		in_comment = true;
	}

	SkipSpaces();
	return toret;
};
QString STFile::getCharLexem(int *err, int *pos)
{
	Q_UNUSED(pos);
	if (file->atEnd()) {
		*err = RT_ERR_READ_AT_EOF;
		return "";
	};
	QChar cur_symb;

	fileData >> cur_symb;
	return "";

};
int STFile::readByte()
{
	char t;
	if (file->size() == file->pos()) {
		return -257;
	}
	int d = dataStream.readRawData(&t, 1);
	qDebug() << "Size" << file->size() << "Pos" << file->pos();

	if (d < 0) {
		qDebug() << "CANT READ";
	}
	return t;
};

//___________________________________________
//               KumFiles
//___________________________________________

KumFiles::KumFiles(QWidget *parent)
	: KumInstrument(parent)
{
	ready = false;
};

/**
 * Добавляет в таблицы функции и переменные,устанавливает таблицы класса
 *  function_table и symbol_table
 * @param Functions Таблица функций
 * @param Symbols Таблица переменных
 * @return количество функций
 */
int KumFiles::getFunctionList(function_table *Functions, symbol_table *Symbols)
{
	ready = true;
	symbols = Symbols;

	functions = Functions;
	int count = 0;
	Functions->addInternalFunction(trUtf8("открыть на чтение"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;

	Functions->addInternalFunction(trUtf8("открыть на запись"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;

	Functions->addInternalFunction(trUtf8("открыть на чтение байтов"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;

	Functions->addInternalFunction(trUtf8("открыть на запись байтов"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 1, true);
	count++;

	Functions->addInternalFunction(trUtf8("закрыть"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;

	Functions->addInternalFunction(trUtf8("начать чтение"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	count++;

	Functions->addInternalFunction(trUtf8("прочесть байт"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);
	Functions->setRes(Functions->size() - 1, 0, true);
	count++;

	Symbols->add_symbol(trUtf8("конец файла"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("конец файла"), kumBoolean, 1);
	Functions->setArgType(Functions->size() - 1, 0, integer);

	Symbols->add_symbol(trUtf8("существует файл"), -1, kumBoolean, "global", true);
	Symbols->new_symbol(Symbols->symb_table.size() - 1);
	Functions->addInternalFunction(trUtf8("существует файл"), kumBoolean, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);

	Functions->addInternalFunction(trUtf8("создать файл"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);

	Functions->addInternalFunction(trUtf8("записать байт"), none, 2);
	Functions->setArgType(Functions->size() - 1, 0, integer);
	Functions->setArgType(Functions->size() - 1, 1, integer);

	Functions->addInternalFunction(trUtf8("удалить файл"), none, 1);
	Functions->setArgType(Functions->size() - 1, 0, kumString);
	count++;

	return count;
};

/**
 * Выполнение функций
 * @param name Имя функции
 * @param aruments список аргументов
 */
void KumFiles::runFunc(QString name, QList<KumValueStackElem> *args, int *err)
{
	*err = 0;
	QList<KumValueStackElem> arguments = *args;
	if (name == trUtf8("открыть на запись байтов")) {

		QString fileName = arguments.first().asString();
		PeremPrt link;
		link.perem_id = arguments[1].asInt();
		link.mod_id = arguments[2].asInt();
		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		if (isOpen(fileName)) {
			*err = RT_ERR_FILE_REOPEN;
			return;
		};
		STFile *file = new STFile();
		*err = file->OpenByteFile(fileName, true);
		if (*err != 0) {
			return;
		}
		file->setByteRead(true);
		//TODO Kontril Oshibok
		fileTable.append(file);
		qDebug() << "FILE HND:" << file->handle();

		app()->compiler->modules()->setIntegerValue(link, file->handle());
		app()->compiler->modules()->setUsed(link, true);
		if (file->error() > 0) {
			*err = file->error();
		}
		return;
	} else if (name == trUtf8("открыть на чтение байтов")) {

		QString fileName = arguments.first().asString();
		PeremPrt link;
		link.perem_id = arguments[1].asInt();
		link.mod_id = arguments[2].asInt();
		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		if (isOpen(fileName)) {
			*err = RT_ERR_FILE_REOPEN;
			return;
		};
		STFile *file = new STFile();
		*err = file->OpenByteFile(fileName, false);
		if (*err != 0) {
			return;
		}
		file->setByteRead(true);
		//TODO Kontril Oshibok
		fileTable.append(file);
		qDebug() << "FILE HND:" << file->handle();
		app()->compiler->modules()->setIntegerValue(link, file->handle());
		app()->compiler->modules()->setUsed(link, true);
		if (file->error() > 0) {
			*err = file->error();
		}
		return;
	} else if (name == trUtf8("открыть на чтение")) {

		QString fileName = arguments.first().asString();
		PeremPrt link;
		link.perem_id = arguments[1].asInt();
		link.mod_id = arguments[2].asInt();
		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		if (isOpen(fileName)) {
			*err = RT_ERR_FILE_REOPEN;
			return;
		};
		STFile *file = new STFile();
		*err = file->OpenFile(fileName, false);
		if (*err != 0) {
			return;
		}
		//TODO Kontril Oshibok
		fileTable.append(file);
		app()->compiler->modules()->setIntegerValue(link, file->handle());
		app()->compiler->modules()->setUsed(link, true);
		if (file->error() > 0) {
			*err = file->error();
		}
		return;
	} else if (name == trUtf8("открыть на запись")) {


		QString fileName = arguments.first().asString();
		PeremPrt link;
		link.perem_id = arguments[1].asInt();
		link.mod_id = arguments[2].asInt();


		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		if (isOpen(fileName)) {
			*err = RT_ERR_FILE_REOPEN;
			return;
		};
		STFile *file = new STFile();
		*err = file->OpenFile(fileName, true);
		if (*err != 0) {
			return;
		}
		//TODO Kontril Oshibok
		fileTable.append(file);
		app()->compiler->modules()->setIntegerValue(link, file->handle());
		app()->compiler->modules()->setUsed(link, true);
		if (file->error() > 0) {
			*err = file->error();
		}
		return;
	} else if (name == trUtf8("закрыть")) {
		int key = arguments.last().asInt();
		int id = getIdByKey(key);

		if (id == -1) {
			qDebug("fileTable id<1 key %i", key);
			*err = RT_FILE_NO_KEY;
			return;
		};
		if (fileTable[id]->error() > 0) {
			*err = fileTable[id]->error();
		}
		fileTable[id]->CloseFile();
		fileTable.removeAt(id);
		return;

	} else if (name == trUtf8("начать чтение")) {
		int key = arguments.last().asInt();
		int id = getIdByKey(key);

		if (id == -1) {
			qDebug("fileTable id<1 key %i", key);
			*err = RT_FILE_NO_KEY;
			return;
		};
		if (fileTable[id]->error() > 0) {
			*err = fileTable[id]->error();
		}
		if (fileTable[id]->forWrite()) {

			*err = F_CANT_RESET_WRITE;
			return;
		};
		fileTable[id]->ResetFile();
		return;

	} else if (name == trUtf8("существует файл")) {

		int perem_id = symbols->inTable(trUtf8("существует файл"), "global");

		QString fileName = arguments.first().asString();

		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		QFile file(fileName);
		symbols->symb_table[perem_id].value.setBoolValue(file.exists());
		return;
	} else if (name == trUtf8("создать файл")) {
		int perem_id = symbols->inTable(trUtf8("создать файл"), "global");
		Q_UNUSED(perem_id);

		QString fileName = arguments.first().asString();

		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		QFile file(fileName);
		if (file.exists()) {
			*err = F_FILE_ALREADY_EXIST;
			return;
		};
		if (!file.open(QIODevice::WriteOnly)) {
			*err = F_FILE_CANT_CREATE;
			return;
		};
		file.close();
		return;
	}
	if (name == trUtf8("удалить файл")) {


		QString fileName = arguments.first().asString();

		if (!isAbsPath(fileName)) {
			fileName = QDir(app()->settings->value("Directories/IO", "").toString()).absoluteFilePath(fileName);
		}
		QFile file(fileName);
		if (isOpen(fileName)) {
			app()->mainWindow->textMessage(trUtf8("Невозможно удалить файл открытый файл"));
			return;
		};
		if (!file.exists()) {
			return;
		};

		if (!file.remove()) {
			app()->mainWindow->textMessage(trUtf8("Невозможно удалить файл"));
		}


		return;
	}
	if (name == trUtf8("конец файла")) {
		int perem_id = symbols->inTable(trUtf8("конец файла"), "global");
		int key = arguments.last().asInt();
		int id = getIdByKey(key);

		if (id == -1) {
			qDebug("fileTable id<1 key %i", key);
			*err = RT_FILE_NO_KEY;
			return;
		};
		STFile *file = fileTable[id];
		Q_UNUSED(file);
		//if(file->isByteRead()){*err=F_FILE_WRONG_TYPE;
		//      return;};
		symbols->symb_table[perem_id].value.setBoolValue(fileTable[id]->atEnd());
		return;
	};
	if (name == trUtf8("прочесть байт")) {

		int key = arguments.last().asInt();
		int id = getIdByKey(key);
		PeremPrt link;
		link.perem_id = arguments[0].asInt();
		link.mod_id = arguments[1].asInt();
		if (id == -1) {
			qDebug("fileTable id<1 key %i", key);
			*err = RT_FILE_NO_KEY;
			return;
		};
		if (fileTable[id]->forWrite()) {
			*err = RT_ERR_OPENED_FOR_WRITE_ONLY;
			return ;
		};

		STFile *file = fileTable[id];
		if (!file->isByteRead()) {
			*err = F_FILE_WRONG_TYPE;
			return;
		};
		int byte = file->readByte();
		if (byte > 255) {
			qDebug("BYTE>255 !");
		};
		if (byte < 0) {
			byte = 256 + byte;
		};
		qDebug() << "Byte:" << byte;
		// if(file->atEnd())byte=-1;
		qDebug() << "BytesAV:" << file->bytesAv();
		app()->compiler->modules()->setIntegerValue(link, byte);
		app()->compiler->modules()->setUsed(link, true);
		return;
	};
	if (name == trUtf8("записать байт")) {

		int key = arguments.last().asInt();
		int id = getIdByKey(key);

		int data = arguments[0].asInt();

		if (id == -1) {
			qDebug("fileTable id<1 key %i", key);
			*err = RT_FILE_NO_KEY;
			return;
		};
		if (!fileTable[id]->forWrite()) {
			*err = RT_ERR_OPENED_FOR_READ_ONLY;
			return ;
		};

		STFile *file = fileTable[id];
		if (!file->isByteRead()) {
			*err = F_FILE_WRONG_TYPE;
			return;
		};
		*err = file->writeByte((quint8)data);

		return;
	};
	*err = 2;
};
bool KumFiles::isOpen(QString fileName)
{
	for (int i = 0; i < fileTable.count(); i++) {
		if (fileTable[i]->name() == fileName) {
			return true;
		}
	};
	return false;
};
QString KumFiles::getLexem(PeremType type, int key, int *err, int *pos)
{
	Q_UNUSED(pos);
	int id = getIdByKey(key);

	if (id < 0) {
		*err = RT_FILE_NO_KEY;
		return "NO SUCH KEY";
	};
	if (fileTable[id]->forWrite()) {
		*err = RT_ERR_OPENED_FOR_WRITE_ONLY;
		return "WRITE ONLY FILE";
	};
	if (fileTable[id]->isByteRead()) {
		*err = F_FILE_WRONG_TYPE;
		return "FILE OPENED FOR BYTE READ";
	};
	if ((type == kumString) || (type == mass_string)) {
		return fileTable[id]->getStrLexem(err, pos);
	} else if ((type == integer) || (type == mass_integer)) {
		return fileTable[id]->getIntLexem(err, pos);
	} else if ((type == real) || (type == mass_real)) {
		return fileTable[id]->getRealLexem(err, pos);
	} else if ((type == charect) || (type == mass_charect)) {
		return fileTable[id]->getStrLexem(err, pos);
	} else if ((type == kumBoolean) || (type == mass_bool)) {
		return fileTable[id]->getBoolLexem(err, pos);
	};
	return "NO SUCH TYPE";
};

int KumFiles::writeText(int key, QString text, int *pos)
{
	Q_UNUSED(pos);
	int id = getIdByKey(key);

	if (id < 0) {
		return RT_FILE_NO_KEY;
	};
	if (fileTable[id]->isByteRead()) {
		return F_FILE_WRONG_TYPE;

	};
	if (!fileTable[id]->forWrite()) {
		return RT_ERR_OPENED_FOR_READ_ONLY;
	}

	if (!fileTable[id]->WriteData(text)) {
		return 2;
	}
	return 0;
};

void KumFiles::close()
{
	reset();
}
