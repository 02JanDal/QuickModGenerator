#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
}

class QFileSystemModel;
class QProcess;

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	explicit MainWidget(QWidget *parent = 0);
	~MainWidget();

private:
	Ui::MainWidget *ui;
	QFileSystemModel *m_model;

	QProcess *createProcess();
	void startProcess(QProcess *process);
	QStringList files() const;

private slots:
	void setupClicked();
	void updateClicked();
	void graphClicked();
	void fixupClicked();
	void createIndexClicked();
	void formatClicked();
	void verifyClicked();
};

#endif // MAINWIDGET_H
