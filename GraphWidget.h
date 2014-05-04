#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QDialog>
#include <QBuffer>
#include <QTemporaryFile>

namespace Ui {
class GraphWidget;
}

class GraphWidget : public QDialog
{
	Q_OBJECT

public:
	explicit GraphWidget(const QStringList &files, QWidget *parent = 0);
	~GraphWidget();

private:
	Ui::GraphWidget *ui;

	QBuffer *m_buffer;
	QTemporaryFile *m_tempFile;

	QPixmap m_image;

private slots:
	void saveClicked();
};

#endif // GRAPHWIDGET_H
