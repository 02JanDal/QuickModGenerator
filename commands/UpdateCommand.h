#pragma once

#include "AbstractCommand.h"

class UpdateCommand : public AbstractCommand
{
	Q_OBJECT
public:
	UpdateCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("update"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
