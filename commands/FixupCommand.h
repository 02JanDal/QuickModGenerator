#pragma once

#include "AbstractCommand.h"

class FixupCommand : public AbstractCommand
{
	Q_OBJECT
public:
	FixupCommand(QObject *parent = 0);

	QStringList commands() const { return QStringList() << tr("fixup"); }
	bool handleCommand(const QString &command, const QCommandLineParser &cmd);
	void populateParserForCommand(const QString &command, QCommandLineParser *cmd);

private:
	QTextStream in;
	QString getCommandLineInput(const QString &message);
};
