#include "GraphWidget.h"
#include "ui_GraphWidget.h"

#include <QFileDialog>
#include <QProcess>
#include <QDebug>

GraphWidget::GraphWidget(const QStringList &files, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GraphWidget), m_buffer(new QBuffer(this)), m_tempFile(new QTemporaryFile(this))
{
	ui->setupUi(this);

	m_buffer->open(QFile::ReadWrite);
	m_tempFile->open();

	QProcess *graph = new QProcess;
	QProcess *dot = new QProcess;
	graph->setProgram(qApp->applicationFilePath());
	graph->setArguments(QStringList() << "graph" << files);
	graph->setStandardOutputProcess(dot);
	dot->setProgram("dot");
	dot->setArguments(QStringList() << "-Tpng");
	dot->setStandardOutputFile(m_tempFile->fileName());
	connect(dot, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [this](int, QProcess::ExitStatus)
	{
		m_image = QPixmap(m_tempFile->fileName());
		m_image = m_image.scaled(m_image.size() / 2);
		ui->image->setPixmap(m_image);
	});
	graph->start();
	dot->start();
}

GraphWidget::~GraphWidget()
{
	delete ui;
}

void GraphWidget::saveClicked()
{
	const QString filename = QFileDialog::getSaveFileName(this, tr("Save graph..."), QDir::currentPath(), tr("Images (*.png)"));
	if (filename.isEmpty())
	{
		return;
	}
	m_image.save(filename);
}
