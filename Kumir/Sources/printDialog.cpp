//
// C++ Implementation: printDialog
//
// Description:
//
//
// Author: Denis Khachko <mordol@mail.ru>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "printDialog.h"
#include "kumfiledialog.h"
#include "application.h"
#include "kumrobot.h"
#include <QPrinter>
#include <QMessageBox>
#include <QDateTime>

PrintPreview::PrintPreview(QWidget *parent, Qt::WindowFlags) : QWidget(parent)
{

}


PrintDialog::PrintDialog(QWidget *parent, Qt::WindowFlags fl) :
	QDialog(parent, fl), Ui::PrintDialog()
{
	setupUi(this);
//connect(pushButton,SIGNAL(clicked()),this,SLOT(selectFileName()));
	sizeX = 0;
	sizeY = 0;
	view = NULL;
	curDir = app()->settings->value("Directories/IO", "").toString();
	PrintFileName = app()->settings->value("LastPDF", "").toString();
	roboMode = false;
}

void PrintDialog::accept()
{
	if (!view) {
		qDebug() << "PrintDialog:no view to print!";
		return;
	}

	selectFileName();
	QPrinter printer(QPrinter::HighResolution);
	if (!radioButton->isChecked()) {
		printer.setColorMode(QPrinter::Color);
		qDebug() << "COLOR";
	} else {
		printer.setColorMode(QPrinter::GrayScale);

	};
	printer.setPageSize(QPrinter::A4);
// if(lineEdit->text().isEmpty()){QMessageBox::information( 0, "", QString::fromUtf8("Нет имени файла!"), 0,0,0);};
	printer.setOutputFileName(PrintFileName);
	printer.setFullPage(false);
	QPainter painter(&printer);

	if (roboMode) {
		drawRobot(&painter, &printer);
	} else {
		drawDraw(&painter, &printer);
	}

	setResult(1);
	close();
}

void PrintDialog::drawDraw(QPainter *painter, QPrinter *printer)
{
	int curPenY = 10;
	QDateTime now = QDateTime::currentDateTime();
	if (radioButton->isChecked()) {
		view->setBackgroundBrush(QBrush(QColor("white")));
	}
	if (checkBoxDate->isChecked()) {
		painter->drawText(10, curPenY, now.toString("dd.MM.yyyy hh:mm"));
		curPenY = curPenY + printer->height() / 80;
	};
	if (checkBoxName->isChecked()) {
		painter->drawText(10, curPenY, PrintFileName);
		curPenY = curPenY + printer->height() / 80;
	};





	if (sizeY > sizeX) {
		view->render(painter, QRectF(0, curPenY,
				printer->width(), printer->height() / 1.4),
			QRect(0, 0, sizeX, sizeY));
		curPenY = curPenY + printer->height() / 1.4 + printer->height() / 80;

	} else {
		view->render(painter, QRectF(0, curPenY,
				printer->width(), printer->height() / 2),
			QRect(0, 0, sizeX, sizeY));
		curPenY = curPenY + printer->height() / 2 - printer->height() / 13;
	};
	if (checkBoxComment->isChecked()) {
		QString comment = textEdit->toPlainText();
		QStringList lines = comment.split('\n');
		for (int i = 0; i < lines.count(); i++) {
			painter->drawText(10, curPenY + i * (printer->height() / 80), lines[i]);
		}
	};
	app()->settings->setValue("LastPDF", PrintFileName);
	view->setBackgroundBrush(QBrush(QColor("lightgreen")));
};

void PrintDialog::drawRobot(QPainter *painter, QPrinter *printer)
{
	int curPenY = 10;
	QDateTime now = QDateTime::currentDateTime();
	if (radioButton->isChecked()) {
		field->wbMode();

	};
	if (checkBoxDate->isChecked()) {
		painter->drawText(10, curPenY, now.toString("dd.MM.yyyy hh:mm"));
		curPenY = curPenY + printer->height() / 80;
	};
	if (checkBoxName->isChecked()) {
		painter->drawText(10, curPenY, PrintFileName);
		curPenY = curPenY + printer->height() / 80;
	};


	QRect from = QRect(0, 0, sizeX - 5, sizeY - 2);
	if (Cols < 3) {
		from = QRect(FIELD_SIZE_SMALL, FIELD_SIZE_SMALL, sizeX - 5, sizeY - 5);
	}

	view->render(painter, QRectF(0, curPenY,
			(printer->width() / 20) * (Cols + 1), (printer->width() / 20) * (Rows + 1)), from
	);
	curPenY = curPenY + (printer->width() / 20) * (Rows + 1) + printer->height() / 80;

	if (checkBoxComment->isChecked()) {
		QString comment = textEdit->toPlainText();
		QStringList lines = comment.split('\n');
		for (int i = 0; i < lines.count(); i++) {
			painter->drawText(10, curPenY + i * (printer->height() / 80), lines[i]);
		}
	};
	app()->settings->setValue("LastPDF", PrintFileName);
	if (radioButton->isChecked()) {
		field->colorMode();

	};
};


void PrintDialog::selectFileName()
{
	QFileDialog dialog(this, tr("Print to file"), curDir, "(*.pdf)");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	if (!dialog.exec()) {
		return ;
	}
	PrintFileName = dialog.selectedFiles().first();

	QDir dir = dialog.directory();
	curDir = dir.path();

	if (PrintFileName.contains("*") || PrintFileName.contains("?")) {
		QMessageBox::information(0, "", tr("Bad filename"), 0, 0, 0);
		return ;
	}

	if (PrintFileName.right(4) != ".pdf") {
		PrintFileName += ".pdf";
	}

};
