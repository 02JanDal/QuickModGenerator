#include "FixupCommand.h"

#include <QCommandLineParser>
#include <QTextStream>
#include <QMutableListIterator>
#include <QDebug>
#include <QDesktopServices>

#include "QuickMod.h"
#include "QuickModReader.h"
#include "QuickModWriter.h"

FixupCommand::FixupCommand(QObject *parent) :
	AbstractCommand(parent), in(stdin)
{

}

QString FixupCommand::getCommandLineInput(const QString &message)
{
	out << " " << message << " " << flush;
	return in.readLine();
}

bool FixupCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);

	out << "You will now be prompted for all missing info for QuickMod files in the current directory. Just leave a blank line to not set a value." << endl << flush;

	QuickModWriter writer(this);

	QListIterator<QuickMod> it(mods);
	while (it.hasNext())
	{
		QuickMod mod = it.next();

		out << "Fixing " << mod.name << endl << flush;

		if (cmd.isSet("browser") && !mod.urls["website"].isEmpty() && false)
		{
			QDesktopServices::openUrl(mod.urls["website"].first());
		}

		if (mod.description.isEmpty())
		{
			mod.description = getCommandLineInput("Description:");
		}
		if (mod.urls["website"].isEmpty())
		{
			mod.urls["website"] = QStringList() << getCommandLineInput("Website URL:");
		}
		if (mod.urls["icon"].isEmpty())
		{
			mod.urls["icon"] = QStringList() << getCommandLineInput("Icon URL:");
		}
		if (mod.urls["logo"].isEmpty())
		{
			mod.urls["logo"] = QStringList() << getCommandLineInput("Logo URL:");
		}
		if (mod.modId.isEmpty())
		{
			mod.modId = getCommandLineInput("Mod ID:");
		}
		if (mod.nemName.isEmpty())
		{
			mod.nemName = getCommandLineInput("NEM name:");
		}
		if (mod.tags.isEmpty())
		{
			mod.tags = getCommandLineInput("Tags (tag1,tag2...tagN):").split(',', QString::SkipEmptyParts);
		}
		if (mod.categories.isEmpty())
		{
			mod.categories = getCommandLineInput("Categories (cat1,cat2...catN):").split(',', QString::SkipEmptyParts);
		}
		if (mod.references.isEmpty())
		{
			const QStringList references = getCommandLineInput("References (name1[:url1],name2[:url2]...nameN[:urlN]):").split(',', QString::SkipEmptyParts);
			foreach (const QString &ref, references)
			{
				const QStringList r = ref.split(':', QString::SkipEmptyParts);
				const QString name = r.at(0);
				QString url;
				if (r.size() == 0)
				{
					continue;
				}
				else if (r.size() == 1)
				{
					url = "http://localhost/quickmod/" + name + ".json";
				}
				else
				{
					url = r.at(1);
				}
				mod.references.insert(name, url);
			}
		}
		if (mod.authors.isEmpty())
		{
			const QStringList authors = getCommandLineInput("Authors (title1:name1a,name1b;title2:name2a,name2b):").split(';', QString::SkipEmptyParts);
			foreach (const QString &authorsRole, authors)
			{
				const QStringList a = authorsRole.split(':');
				if (a.size() < 2)
				{
					continue;
				}
				mod.authors.insert(a.at(0), QString(authorsRole).remove(a.at(0) + ':').split(',', QString::SkipEmptyParts));
			}
		}
		QMutableListIterator<QuickModVersion> vit(mod.versions);
		while (vit.hasNext())
		{
			QuickModVersion version = vit.next();

			out << " Version " << version.name << endl << flush;

			if (version.md5.isEmpty() && !cmd.isSet("no-checksums"))
			{
				version.md5 = getCommandLineInput(" Checksum:");
			}

			vit.setValue(version);
		}

		QString error;
		if (!writer.write(mod, QDir::current(), &error))
		{
			out << "There was an error trying to write the QuickMod file:" << endl << error << endl;
			return false;
		}
	}

	return true;
}

void FixupCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	cmd->addOption(QCommandLineOption(tr("no-checksums"), tr("Don't fix missing checksums")));
	cmd->addOption(QCommandLineOption(tr("browser"), tr("Opens a browser for every mod")));
	addMultiFileArgument(cmd);
}
