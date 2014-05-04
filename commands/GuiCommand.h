#pragma once

#include "AbstractCommand.h"

class GuiCommand : public AbstractCommand
{
	Q_OBJECT
public:
	GuiCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("gui"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
