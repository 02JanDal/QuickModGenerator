#include "DumpCommand.h"

#include <QCommandLineParser>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "QuickModReader.h"

DumpCommand::DumpCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool DumpCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QuickMod mod = readSingleMod(cmd);

	out << "UID: " << mod.uid << endl
		<< "Name: " << mod.name << endl
		<< "NEM Name: " << mod.nemName << endl
		<< "Mod ID: " << mod.modId << endl
		<< "Description: " << mod.description << endl
		<< "Website: " << mod.urls["website"].join(", ") << endl
		<< "Icon: " << mod.urls["icon"].join(", ") << endl
		<< "Logo: " << mod.urls["logo"].join(", ") << endl
		<< "Categories: " << mod.categories.join(", ") << endl
		<< "Tags: " << mod.tags.join(", ") << endl
		<< "Update URL: " << mod.updateUrl << endl
		<< "Authors: " << QJsonDocument::fromVariant(QVariant::fromValue(mod.authors)).toJson() << endl
		<< "References: " << QJsonDocument::fromVariant(QVariant::fromValue(mod.references)).toJson() << endl
		   ;

	return true;
}

void DumpCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addSingleFileArgument(cmd);
}
