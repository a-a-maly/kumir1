#ifndef TABLEVIEWER_H
#define TABLEVIEWER_H

#include <QTabWidget>

class KumModules;

class TableViewer : public QTabWidget
{
	Q_OBJECT;
	friend class TableViewerPrivate;
public:
	explicit TableViewer(KumModules *modules, int modId, int varId, QWidget *parent = 0);
	int moduleId() const;
	int variableId() const;
	~ TableViewer();
public slots:
	void updateContent();
private:
	struct TableViewerPrivate *d;
};

#endif // TABLEVIEWER_H
