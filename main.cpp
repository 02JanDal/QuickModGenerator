#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTextStream>
#include <cstdio>
#include <QDebug>

#include "QuickMod.h"
#include "QuickModReader.h"
#include "QuickModWriter.h"

#include "AbstractCommand.h"
#include "DumpCommand.h"
#include "FixupCommand.h"
#include "FormatCommand.h"
#include "GraphCommand.h"
#include "IndexCommand.h"
#include "SetupCommand.h"
#include "VerifyCommand.h"
#include "UpdateCommand.h"
#include "GuiCommand.h"
#include "CreateChecksumCommand.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QTextStream out(stdout);

	QList<AbstractCommand *> commandProviders;
	commandProviders << new DumpCommand << new FixupCommand << new FormatCommand
					 << new GraphCommand << new IndexCommand << new SetupCommand
					 << new VerifyCommand << new UpdateCommand << new GuiCommand << new CreateChecksumCommand;

	QMap<QString, AbstractCommand *> commands;
	foreach(AbstractCommand * commandProvider, commandProviders)
	{
		// commandProvider->setup(parsers);
		foreach(const QString & command, commandProvider->commands())
		{
			commands.insert(command, commandProvider);
		}
	}

	QCommandLineParser args;
	args.setApplicationDescription(
		QCoreApplication::tr("Creates and manages QuickMod files using several webservices "
							 "like Not Enough Mods, Curse etc."));
	auto helpOption = args.addHelpOption();
	args.addPositionalArgument(QCoreApplication::tr("command"),
							   QCoreApplication::tr("Execute <command>. Possible values: %1")
								   .arg(QStringList(commands.keys()).join(", ")));
	args.addOption(QCommandLineOption(
		QStringList() << QCoreApplication::tr("p") << QCoreApplication::tr("page"),
		QCoreApplication::tr("Page for paged queries"), QCoreApplication::tr("page")));

	args.parse(app.arguments());
	if (args.isSet(helpOption) && args.positionalArguments().isEmpty())
	{
		args.showHelp();
		return 0;
	}

	const QString cmd = args.positionalArguments().first();

	if (cmd == "help")
	{
		args.showHelp();
		return 0;
	}
	else if (commands.contains(cmd))
	{
		AbstractCommand *commandProvider = commands[cmd];
		commandProvider->populateParserForCommand(cmd, &args);
		args.process(app);
		try
		{
			commandProvider->handleCommand(cmd, args);
		}
		catch (std::exception &e)
		{
			out << "Caught exception: " << e.what() << endl;
		}

		return 0;
	}

	out << args.errorText() << endl << flush;
	args.showHelp();

	return 0;
}
