#include "SetupCommand.h"

#include <QCommandLineParser>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

#include "QuickModReader.h"
#include "QuickModWriter.h"

SetupCommand::SetupCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool SetupCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	if (command != "setup")
	{
		return false;
	}

	if (cmd.positionalArguments().size() < 2)
	{
		out << "You need to specify a name" << endl;
		return false;
	}
	const QString name = cmd.positionalArguments().at(1);

	QuickMod mod;
	if (QDir::current().exists(name + ".json"))
	{
		QuickModReader reader(this);
		QStringList errors;
		mod = reader.read(QDir::current().absoluteFilePath(name + ".json"), &errors);
		if (!errors.isEmpty())
		{
			out << "There where some errors trying to read the QuickMod file:" << endl << errors.join("\n") << endl;
			return false;
		}
	}
	else
	{
		mod.name = name;
	}

	if (cmd.isSet("nem"))
	{
		mod.nemName = cmd.value("nem");
	}
	if (cmd.isSet("curse"))
	{
		mod.websiteUrl = QUrl(QString("http://www.curse.com/mc-mods/minecraft/%1").arg(cmd.value("curse")));
	}
	if (cmd.isSet("server"))
	{
		mod.updateUrl = QUrl(cmd.value("server")).resolved(name + ".json");
	}
	else
	{
		mod.updateUrl = QUrl("http://localhost/quickmod/").resolved(name + ".json");
	}

	QuickModWriter writer(this);
	QString error;
	if (!writer.write(mod, QDir::current(), &error))
	{
		out << "There was an error trying to write the QuickMod file:" << endl << error << endl;
		return false;
	}

	return true;
}

void SetupCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	if (command != "setup")
	{
		return;
	}

	cmd->addOption(QCommandLineOption(tr("nem"), tr("Sets the NEM name to use when updating"), tr("NEM-NAME")));
	cmd->addOption(QCommandLineOption(tr("curse"), tr("Sets the Curse id to use when updating"), tr("CURSE-ID")));
	cmd->addOption(QCommandLineOption(tr("server"), tr("Sets the server root. Used for the updateUrl property"), tr("URL")));
	cmd->addPositionalArgument(tr("name"), tr("The name used when creating a new QuickMod file, or to select the existing QuickMod file"));
}
