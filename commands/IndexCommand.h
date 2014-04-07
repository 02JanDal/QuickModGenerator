#pragma once

#include "AbstractCommand.h"

class IndexCommand : public AbstractCommand
{
	Q_OBJECT
public:
	IndexCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("create-index"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
