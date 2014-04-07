#pragma once

#include "AbstractCommand.h"

class GraphCommand : public AbstractCommand
{
	Q_OBJECT
public:
	GraphCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("graph"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
