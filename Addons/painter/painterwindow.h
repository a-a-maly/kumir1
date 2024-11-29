#ifndef PAINTERWINDOW_H
#define PAINTERWINDOW_H

#include <QMainWindow>
class QLabel;

namespace Ui
{
    class PainterWindow;
}
class PainterNewImageDialog;

class PainterWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit PainterWindow(QWidget *parent = 0);
	~PainterWindow();
	void setCanvas(QImage *canvas, QMutex *locker);
	QWidget *view();

public slots:
	void newImage();
	void loadImage();
	void saveImage();
	void saveImageAs();

signals:
	void newImageRequest(int w, int h, const QString &bg, bool useTemplate, const QString &templateFileName);
	void loadImageRequest(const QString &fileName);
	void resetRequest();

private slots:
	void handleColorTextModeChanged();
	void handleCursorMoved(int x, int y, const QColor &color);
	void saveImageToFile(const QString &fileName);
	void handleFullScreen();
	void handleScale();
	void reset();

private:
	void closeEvent(QCloseEvent *event);
	void hideEvent(QHideEvent *event);

	QString e_showColorMode; // values: "HTML", "RGB", "CMYK", "HSL", "HSV"
	Ui::PainterWindow *ui;
	QString s_fileName;
	QString s_templateName;
	QLabel *m_zoomLabel;
	PainterNewImageDialog *m_newImageDialog;
};

#endif // PAINTERWINDOW_H
