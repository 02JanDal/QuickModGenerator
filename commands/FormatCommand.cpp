#include "FormatCommand.h"

#include <QCommandLineParser>

#include "QuickMod.h"
#include "QuickModReader.h"
#include "QuickModWriter.h"

FormatCommand::FormatCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool FormatCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);

	QuickModWriter writer(this);

	QListIterator<QuickMod> it(mods);
	while (it.hasNext())
	{
		QuickMod mod = it.next();

		out << "Formatting " << mod.name << "..." << endl << flush;

		QString error;
		if (!writer.write(mod, QDir::current(), &error))
		{
			out << "There was an error trying to write the QuickMod file:" << endl << error << endl;
			return false;
		}
	}

	return true;
}

void FormatCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	cmd->addOption(QCommandLineOption(tr("no-checksums"), tr("Don't fix missing checksums")));
	cmd->addOption(QCommandLineOption(tr("browser"), tr("Opens a browser for every mod")));
	addMultiFileArgument(cmd);
}
