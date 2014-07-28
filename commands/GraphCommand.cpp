#include "GraphCommand.h"

#include <QCommandLineParser>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

#include "QuickModReader.h"

#include "include/modutils.h"

GraphCommand::GraphCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool GraphCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);
	const QString mcversion = cmd.value("mcver");

	QMap<QString, QString> referenceStyles;
	referenceStyles["depends"] = "solid";
	referenceStyles["recommends"] = "dashed";
	referenceStyles["suggests"] = "dotted";
	referenceStyles["breaks"] = "bold";
	QMap<QString, QString> categoryStyles;
	categoryStyles["API"] = "dashed";
	categoryStyles["Library"] = "dashed";

	out << "digraph {" << endl;
	for (auto mod : mods)
	{
		if (mod.uid.isEmpty())
		{
			continue;
		}
		const QString firstCategory = mod.categories.isEmpty() ? "" : mod.categories.first();
		const QString style = categoryStyles.contains(firstCategory) ? categoryStyles[firstCategory] : "solid";
		QuickModVersion version = mod.versions.first();
		for (auto v : mod.versions)
		{
			if (v.name == version.name || !v.mcCompat.contains(mcversion))
			{
				continue;
			}
			if (Util::Version(v.name) > Util::Version(version.name))
			{
				version = v;
			}
		}
		out << "\"" << mod.uid << "\" [label=\"" << mod.name << "\\n" << version.name << "\",style=\"" << style << "\"]" << endl;
		for (auto it = version.references.begin(); it != version.references.end(); ++it)
		{
			out << "\"" << mod.uid << "\" -> \"" << it.key() << "\" [style=\"" << referenceStyles[it.value().type] << "\"]" << endl;
		}
	}
	out << "}" << endl;

	return true;
}

void GraphCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addMultiFileArgument(cmd);
	cmd->addOption(QCommandLineOption("mcver", "The version of Minecraft", "VERSION", "1.6.4"));
}
