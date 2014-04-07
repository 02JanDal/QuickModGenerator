#pragma once

#include "AbstractCommand.h"

class FormatCommand : public AbstractCommand
{
	Q_OBJECT
public:
	FormatCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("format"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
