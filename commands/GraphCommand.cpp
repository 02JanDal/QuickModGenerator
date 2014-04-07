#include "GraphCommand.h"

#include <QCommandLineParser>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

#include "QuickModReader.h"

GraphCommand::GraphCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool GraphCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);

	out << "digraph {" << endl;
	for (auto mod : mods)
	{
		out << "\"" << mod.uid << "\"[label=\"" << mod.name << "\"]" << endl;
		for (auto ref : mod.references.keys())
		{
			out << "\"" << mod.uid << "\" -> \"" << ref << "\"" << endl;
		}
	}
	out << "}" << endl;

	return true;
}

void GraphCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addMultiFileArgument(cmd);
}
