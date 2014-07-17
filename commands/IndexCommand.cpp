#include "IndexCommand.h"

#include <QCommandLineParser>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "QuickModReader.h"

IndexCommand::IndexCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool IndexCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	if (!cmd.isSet("base"))
	{
		out << "You'll need to set --base" << endl;
		return false;
	}

	const QString base = cmd.value("base");

	QStringList filenames = QDir::current().entryList(QStringList() << "*.json", QDir::Files);

	QStringList errors;
	QList<QuickMod> mods = QuickModReader().read(filenames, &errors);
	if (!errors.isEmpty())
	{
		out << "There where some errors trying to read the QuickMod file:" << endl << errors.join("\n") << endl;
		return false;
	}

	QJsonObject root;
	QJsonArray items;
	for (auto mod : mods)
	{
		if (mod.uid.isEmpty())
		{
			continue;
		}
		QJsonObject modObj;
		modObj.insert("uid", mod.uid);
		modObj.insert("url", mod.updateUrl.remove(base));
		items.append(modObj);
	}
	root.insert("index", items);
	root.insert("baseUrl", base);
	if (cmd.isSet("repo"))
	{
		root.insert("repo", cmd.value("repo"));
	}

	QFile file(QDir::current().absoluteFilePath("index.json"));
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
	{
		out << "Couldn't open file for writing: " << file.errorString();
		return false;
	}
	file.write(QJsonDocument(root).toJson());
	file.close();

	return true;
}

void IndexCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	cmd->addOption(QCommandLineOption("base", "The baseUrl", "<URL>"));
	cmd->addOption(QCommandLineOption("repo", "The value for the 'repo' field", "<REPO>"));
}
