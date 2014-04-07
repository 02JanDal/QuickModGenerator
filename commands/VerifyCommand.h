#pragma once

#include "AbstractCommand.h"

class VerifyCommand : public AbstractCommand
{
	Q_OBJECT
public:
	VerifyCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("verify"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);
};
