#include "CreateChecksumCommand.h"

#include <QDebug>
#include <QFile>
#include <QCommandLineParser>
#include <QCryptographicHash>

#include "QuickMod.h"

CreateChecksumCommand::CreateChecksumCommand(QObject *parent) :
	AbstractCommand(parent)
{

}

bool CreateChecksumCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);

	QMutableListIterator<QuickMod> it(mods);
	while (it.hasNext())
	{
		QuickMod mod = it.next();
		mod.verifyUrl = QString(mod.updateUrl).replace(".json", ".verify.json");
		it.setValue(mod);
	}

	saveMultipleMods(mods);

	for (auto mod : mods)
	{
		const QString fileName = mod.updateUrl.mid(mod.updateUrl.lastIndexOf('/')+1);
		QFile modFile(fileName);
		if (!modFile.open(QFile::ReadOnly))
		{
			out << "Unable to open" << fileName << modFile.errorString();
			continue;
		}
		const QByteArray checksum = QCryptographicHash::hash(modFile.readAll(), QCryptographicHash::Sha512).toHex();
		QFile checksumFile(mod.verifyUrl.mid(mod.verifyUrl.lastIndexOf('/')+1));
		if (!checksumFile.open(QFile::WriteOnly | QFile::Truncate))
		{
			out << "Unable to open" << checksumFile.fileName() << checksumFile.errorString();
			continue;
		}
		checksumFile.write(checksum + ":" + cmd.value("shown-url").toUtf8());
		checksumFile.close();
	}

	return true;
}

void CreateChecksumCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addMultiFileArgument(cmd);
	cmd->addOption(QCommandLineOption("shown-url", "The URL that will be shown to the user", "URL"));
}
