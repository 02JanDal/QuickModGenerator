#pragma once

#include "AbstractCommand.h"

#include <QStringList>

#include "QuickMod.h"

class QCommandLineParser;

class AbstractParser : public AbstractCommand
{
	Q_OBJECT
public:
	AbstractParser(QObject *parent = 0);

	virtual QString name() const = 0;

	virtual QStringList commands() const { return QStringList() << name().append("-list") << name().append("-search"); }
	virtual bool handleCommand(const QString &command, const QCommandLineParser &cmd) { return true; }
	virtual void populateParserForCommand(const QString &command, QCommandLineParser *cmd) {}

	virtual	QuickMod addInfo(const QuickMod &in) const { return in; }
};
