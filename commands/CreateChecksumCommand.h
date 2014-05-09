#pragma once

#include "AbstractCommand.h"

class CreateChecksumCommand : public AbstractCommand
{
	Q_OBJECT
public:
	CreateChecksumCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("create-checksum"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
