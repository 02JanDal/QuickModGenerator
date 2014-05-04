#include "MainWidget.h"
#include "ui_MainWidget.h"

#include <QFileSystemModel>
#include <QProcess>
#include <QTableView>
#include <QDebug>

#include "GraphWidget.h"
#include "FixupWidget.h"

MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
	ui->setupUi(this);

	m_model = new QFileSystemModel(this);
	m_model->setRootPath(QDir::currentPath());
	m_model->setFilter(QDir::Files);
	m_model->setNameFilters(QStringList() << "*.json"
										  << "*.quickmod"
										  << "*.qm");
	ui->filesView->setModel(m_model);
	ui->filesView->setRootIndex(m_model->index(QDir::currentPath()));
	ui->filesView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->filesView->setSelectionMode(QAbstractItemView::MultiSelection);

	connect(m_model, &QFileSystemModel::modelReset, [this]()
	{
		for (int i = 0; i < m_model->rowCount(ui->filesView->rootIndex()); ++i)
		{
			ui->filesView->selectionModel()->select(m_model->index(i, 0, ui->filesView->rootIndex()),
													QItemSelectionModel::Select);
		}
	});
	connect(m_model, &QFileSystemModel::rowsInserted,
			[this](const QModelIndex &parent, const int start, const int end)
	{
		for (int i = start; i < (end + 1); ++i)
		{
			ui->filesView->selectionModel()->select(m_model->index(i, 0, ui->filesView->rootIndex()),
													QItemSelectionModel::Select);
		}
	});

	connect(ui->setupBtn, &QPushButton::clicked, this, &MainWidget::setupClicked);
	connect(ui->updateBtn, &QPushButton::clicked, this, &MainWidget::updateClicked);
	connect(ui->graphBtn, &QPushButton::clicked, this, &MainWidget::graphClicked);
	connect(ui->fixupBtn, &QPushButton::clicked, this, &MainWidget::fixupClicked);
	connect(ui->createIndex, &QPushButton::clicked, this, &MainWidget::createIndexClicked);
	connect(ui->formatBtn, &QPushButton::clicked, this, &MainWidget::formatClicked);
	connect(ui->verifyBtn, &QPushButton::clicked, this, &MainWidget::verifyClicked);
}

MainWidget::~MainWidget()
{
	delete ui;
}

QProcess *MainWidget::createProcess()
{
	QProcess *process = new QProcess(this);
	process->setProgram(qApp->applicationFilePath());
	return process;
}

void MainWidget::startProcess(QProcess *process)
{
	process->setProcessChannelMode(QProcess::MergedChannels);
	ui->log->clear();
	ui->log->append(
		QString("Starting %1 %2").arg(process->program(), process->arguments().join(' ')));
	connect(process, &QProcess::readyRead, [this, process]()
	{ ui->log->append(process->readAll()); });
	process->start(QProcess::ReadWrite);
}

QStringList MainWidget::files() const
{
	QStringList list;
	for (auto selected : ui->filesView->selectionModel()->selectedIndexes())
	{
		const QString file = m_model->filePath(selected);
		if (!file.endsWith(".json") && !file.endsWith(".quickmod") && !file.endsWith(".qm"))
		{
			continue;
		}
		if (file.endsWith("index.json"))
		{
			continue;
		}
		list.append(file);
	}
	return list;
}

void MainWidget::setupClicked()
{
	if (ui->setupName->text().isEmpty())
	{
		return;
	}
	QProcess *process = createProcess();
	QStringList args;
	args << "setup";
	if (!ui->setupNem->text().isEmpty())
	{
		args << "--nem" << ui->setupNem->text();
	}
	if (!ui->setupCurse->text().isEmpty())
	{
		args << "--curse" << ui->setupCurse->text();
	}
	if (!ui->setupServer->text().isEmpty())
	{
		args << "--server" << ui->setupServer->text();
	}
	args << ui->setupName->text();
	process->setArguments(args);
	startProcess(process);
}
void MainWidget::updateClicked()
{
	if (files().isEmpty())
	{
		return;
	}
	QProcess *process = createProcess();
	QStringList args;
	args << "update";
	if (ui->updateNetworkBtn->isChecked())
	{
		args << "--no-network";
	}
	args << files();
	process->setArguments(args);
	startProcess(process);
}
void MainWidget::graphClicked()
{
	GraphWidget graph(files());
	graph.exec();
}
void MainWidget::fixupClicked()
{
	FixupWidget widget(files());
	widget.showMaximized();
	widget.exec();
}
void MainWidget::createIndexClicked()
{
	if (files().isEmpty())
	{
		return;
	}
	QProcess *process = createProcess();
	QStringList args;
	args << "create-index";
	if (!ui->createIndexBaseUrl->text().isEmpty())
	{
		args << "--base" << ui->createIndexBaseUrl->text();
	}
	args << files();
	process->setArguments(args);
	startProcess(process);
}
void MainWidget::formatClicked()
{
	if (files().isEmpty())
	{
		return;
	}
	QProcess *process = createProcess();
	process->setArguments(QStringList() << "format" << files());
	startProcess(process);
}
void MainWidget::verifyClicked()
{
	if (files().isEmpty())
	{
		return;
	}
	QProcess *process = createProcess();
	process->setArguments(QStringList() << "verify" << files());
	startProcess(process);
}
