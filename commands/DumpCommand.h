#pragma once

#include "AbstractCommand.h"

class DumpCommand : public AbstractCommand
{
	Q_OBJECT
public:
	DumpCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("dump"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
