#include "GuiCommand.h"

#include <QApplication>

#include "MainWidget.h"

GuiCommand::GuiCommand(QObject *parent)
	: AbstractCommand(parent)
{
}

bool GuiCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	MainWidget widget;
	widget.show();
	return qApp->exec() == 0;
}

void GuiCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
}
