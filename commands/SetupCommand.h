#pragma once

#include "AbstractCommand.h"

class SetupCommand : public AbstractCommand
{
	Q_OBJECT
public:
	SetupCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("setup"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
