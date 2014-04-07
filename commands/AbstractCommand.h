#pragma once

#include <QObject>
#include <QStringList>
#include <QTextStream>

class QNetworkAccessManager;
class QCommandLineParser;
class AbstractParser;
class QuickMod;

class Exception : public std::exception
{
public:
	Exception(const QString &what = QString()) : std::exception(), m_what(what) {}
	~Exception() noexcept {}
	QString cause() const { return m_what; }
	const char *what() const noexcept override { return m_what.toLocal8Bit().constData(); }

private:
	QString m_what;
};

class AbstractCommand : public QObject
{
	Q_OBJECT
public:
	AbstractCommand(QObject *parent = 0);

	virtual QStringList commands() const { return QStringList(); }
	virtual bool handleCommand(const QString &command, const QCommandLineParser &cmd) = 0;
	virtual void populateParserForCommand(const QString &command, QCommandLineParser *cmd) = 0;

	void setup(const QList<AbstractParser *> &parsers) { m_parsers = parsers; }

protected:
	QList<AbstractParser *> m_parsers;
	mutable QTextStream out;
	QNetworkAccessManager *m_nam;

	void addSingleFileArgument(QCommandLineParser *cmd);
	void addMultiFileArgument(QCommandLineParser *cmd);
	QuickMod readSingleMod(const QCommandLineParser &cmd);
	QList<QuickMod> readMultipleMods(const QCommandLineParser &cmd);
	void saveMultipleMods(const QList<QuickMod> &mods);
};
