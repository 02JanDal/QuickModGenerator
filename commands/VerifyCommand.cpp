#include "VerifyCommand.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QJsonDocument>
#include <QFile>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>

#include "sequence.h"
#include "collection.h"
#include "threadweaver.h"
#include "resourcerestrictionpolicy.h"

class VerifyJob : public QObject, public ThreadWeaver::Job
{
	Q_OBJECT
public:
	VerifyJob(const QString &filename) : ThreadWeaver::Job(), m_filename(filename) {}

signals:
	void output(const QString &message);

protected:
	void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
	{
		QFile f(m_filename);
		if (!f.open(QFile::ReadOnly))
		{
			emit output("Cannot open QuickMod file " + f.fileName() + ": " + f.errorString());
			setStatus(Status_Failed);
			return;
		}

		QJsonParseError err;
		QJsonDocument::fromJson(f.readAll(), &err);
		if (err.error != QJsonParseError::NoError)
		{
			emit output("Error parsing QuickMod file " + f.fileName() + ": " + err.errorString());
			setStatus(Status_Failed);
			return;
		}

		QProcess proc;
		proc.setArguments(QStringList()
						  << QDir(qApp->applicationDirPath()).absoluteFilePath("../validator/schemavalidate.php")
						  << QDir::current().absoluteFilePath(m_filename)
						  << (m_filename.endsWith(".versions.json")
							  ? QDir(qApp->applicationDirPath()).absoluteFilePath("../validator/QuickModVersionSchema.json")
							  : QDir(qApp->applicationDirPath()).absoluteFilePath("../validator/QuickModSchema.json")));
		proc.setProgram("php");
		proc.setWorkingDirectory(QDir(qApp->applicationDirPath()).absoluteFilePath(".."));
		proc.start();
		proc.waitForStarted();
		proc.waitForFinished();
		const QByteArray response = proc.readAll();
		if (!response.isEmpty())
		{
			emit output("Error validating against schema in " + QFileInfo(f.fileName()).fileName() + ": " + response);
			setStatus(Status_Failed);
			return;
		}
	}

private:
	QString m_filename;
};

VerifyCommand::VerifyCommand(QObject *parent) : AbstractCommand(parent)
{
}

bool VerifyCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QStringList filenames;
	if (cmd.positionalArguments().size() > 1)
	{
		filenames += cmd.positionalArguments();
		filenames.removeFirst();
	}
	else
	{
		filenames += QDir::current().entryList(QStringList() << "*.json", QDir::Files);
	}
	filenames.removeAll("index.json");
	filenames.removeDuplicates();

	ThreadWeaver::ResourceRestrictionPolicy *policy = new ThreadWeaver::ResourceRestrictionPolicy(10);

	ThreadWeaver::Collection *jobs = new ThreadWeaver::Collection;
	foreach(const QString & filename, filenames)
	{
		auto job = new VerifyJob(filename);
		job->assignQueuePolicy(policy);
		connect(job, &VerifyJob::output, [this](const QString &msg){out << msg << endl << flush;});
		*jobs << ThreadWeaver::make_job_raw(job);
	}

	ThreadWeaver::enqueue(jobs);
	ThreadWeaver::Queue::instance()->finish();

	return true;
}

void VerifyCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addMultiFileArgument(cmd);
}

#include "VerifyCommand.moc"
