#include "Util.h"

#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>

namespace Util
{

QNetworkReply *blockNetworkReply(QNetworkReply *reply)
{
	while (!reply->isFinished())
	{
		if (reply->error() != QNetworkReply::NoError)
		{
			break;
		}
		qApp->processEvents();
	}
	return reply;
}

QByteArray getUsingExternal(const QUrl &url)
{
	QString outputFile = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).absoluteFilePath("quickmodtmp");
	QProcess proc;
	proc.setProgram("curl");
	proc.setArguments(QStringList() << "-o" << outputFile << url.toString(QUrl::FullyEncoded));
	proc.setProcessChannelMode(QProcess::MergedChannels);
	proc.start();
	proc.waitForStarted();
	proc.waitForFinished();
	qDebug() << proc.readAll();
	QFile f(outputFile);
	Q_ASSERT(f.open(QFile::ReadOnly));
	const QByteArray result = f.readAll();
	f.close();
	f.remove();
	return result;
}

}
