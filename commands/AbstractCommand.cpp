#include "AbstractCommand.h"
#include <cstdio>

#include <QNetworkAccessManager>
#include <QCommandLineParser>

#include "QuickModReader.h"
#include "QuickModWriter.h"

AbstractCommand::AbstractCommand(QObject *parent) :
	QObject(parent), out(stdout), m_nam(new QNetworkAccessManager(this))
{

}

void AbstractCommand::addSingleFileArgument(QCommandLineParser *cmd)
{
	cmd->addPositionalArgument("file", tr("Specifies the file to read"), tr("FILE"));
}

void AbstractCommand::addMultiFileArgument(QCommandLineParser *cmd)
{
	cmd->addPositionalArgument("files", tr("Specifies the files to read"), tr("FILES"));
}

QuickMod AbstractCommand::readSingleMod(const QCommandLineParser &cmd)
{
	if (cmd.positionalArguments().size() < 2)
	{
		out << "You need to specify a file" << endl;
		throw Exception();
	}
	const QString file = cmd.positionalArguments().at(1);

	QuickModReader reader(this);
	QStringList errors;
	QuickMod mod = reader.read(QDir::current().absoluteFilePath(file), &errors);
	if (!errors.isEmpty())
	{
		out << "There where some errors trying to read the QuickMod file:" << endl << errors.join("\n") << endl;
		throw Exception();
	}

	return mod;
}

QList<QuickMod> AbstractCommand::readMultipleMods(const QCommandLineParser &cmd)
{
	QuickModReader reader(this);
	QList<QuickMod> mods;
	QStringList errors;
	if (cmd.positionalArguments().size() > 1)
	{
		mods = reader.read(cmd.positionalArguments().mid(1), &errors);
	}
	else
	{
		mods = reader.read(QDir::current(), &errors);
	}

	if (!errors.isEmpty())
	{
		out << "There where some errors trying to read the QuickMod files:" << endl << errors.join("\n") << endl;
		throw Exception();
	}
	return mods;
}

void AbstractCommand::saveMultipleMods(const QList<QuickMod> &mods)
{
	QuickModWriter writer(this);
	QStringList errors;
	for (auto mod : mods)
	{
		QString error;
		if (!writer.write(mod, QDir::current(), &error))
		{
			errors += error;
		}
	}
	if (!errors.isEmpty())
	{
		out << "There where some errors while trying to save the QuickMod files:" << endl << errors.join("\n") << endl;
		throw Exception();
	}
}
