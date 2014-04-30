#include "UpdateCommand.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCryptographicHash>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QDesktopServices>

#include "QuickModReader.h"
#include "QuickMod.h"

#include "sequence.h"
#include "collection.h"
#include "threadweaver.h"
#include "thread.h"
#include "dependencypolicy.h"

using namespace ThreadWeaver;

/*

Wanted mod              Hosted at
Forestry                CurseForge http://minecraft.curseforge.com/mc-mods/forestry/files/
Applied Energistics     Custom http://ae1.ae-mod.info/Download-Archive/
Thermal Expansion       CurseForge http://minecraft.curseforge.com/mc-mods/thermalexpansion/
CoFHCore                CurseForge http://minecraft.curseforge.com/mc-mods/cofhcore/
Tinkers Construct       CurseForge http://minecraft.curseforge.com/mc-mods/tinkers-construct/
Natura                  CurseForge http://minecraft.curseforge.com/mc-mods/natura/
MFR                     CurseForge http://minecraft.curseforge.com/mc-mods/minefactory-reloaded/
PCC                     CurseForge http://minecraft.curseforge.com/mc-mods/powercrystalscore/
Ex Nihilo               None http://www.minecraftforum.net/topic/1981778-
Ex Aliquo               Bitly bundle http://bitly.com/bundles/zerokyuuni/1

TODOS:
1. Skeletons DONE
2. CurseForge DONE
3. Bitly bundle

*/

class BaseJob : public QObject, public Job
{
	Q_OBJECT
public:
	BaseJob() : QObject(), Job()
	{
	}
	virtual ~BaseJob()
	{
	}

signals:
	void output(const QString &msg);

protected:
	static QString filename(const QuickMod &mod, const QuickModVersion &version)
	{
		return QDir::current().absoluteFilePath("tmp/" + mod.name + "-" + version.name +
												".jar");
	}
	QString uidForModId(const QString &modid)
	{
		static QMap<QString, QString> mapping;
		if (mapping.isEmpty())
		{
			mapping["Forge"] = "Forge";
			mapping["IC2"] = "ic2";
			mapping["ForgeMultipart"] = "codechicken.multipart";
			mapping["AppliedEnergistics"] = "appeng";
			mapping["Buildcraft|Core"] = "buildcraft";
			mapping["ExtraBees"] = "binnie.extrabees";
			mapping["BiomesOPlenty"] = "biomesoplenty";
			mapping["Thaumcraft"] = "thaumcraft";
			mapping["NetherOres"] = "powercrystals.netherores";
			mapping["MiscPeripherals"] = "miscperipherals";
			mapping["Metallurgy"] = "rebelkeithy.mods.metallurgy";
			mapping["Mariculture"] = "mariculture";
			mapping["ExtrabiomesXL"] = "extrabiomesxl";
			mapping["arsmagica2"] = "arsmagica2";
			mapping["Growthcraft|Apples"] = "growthcraft.apples";
			mapping["Growthcraft|Bamboo"] = "growthcraft.bamboo";
			mapping["Growthcraft|Bees"] = "growthcraft.bees";
			for (auto mod : mods())
			{
				if (!mod.modId.isEmpty() && !mod.uid.isEmpty())
				{
					mapping.insert(mod.modId, mod.uid);
				}
			}
		}
		if (mapping.contains(modid))
		{
			return mapping[modid];
		}
		emit output("Cannot map modid " + modid + " to a uid. Please do so manually");
		return modid;
	}
	static QString updateUrlForUid(const QString &uid)
	{
		for (auto mod : mods())
		{
			if (mod.uid == uid)
			{
				return mod.updateUrl.toString();
			}
		}
		return QString();
	}
	static QList<QuickMod> mods()
	{
		static QList<QuickMod> m;
		if (m.isEmpty())
		{
			QuickModReader reader;
			m = reader.read(QDir::current(), new QStringList());
		}
		return m;
	}
};

class Fetcher : public QObject
{
	Q_OBJECT
public:
	Fetcher(const QUrl &url, QNetworkAccessManager *nam, const bool fullFollow = false,
			QObject *parent = 0)
		: QObject(parent), m_nam(nam), m_url(url), m_fullFollow(fullFollow)
	{
		m_reply = m_nam->get(QNetworkRequest(url));
		connect(m_reply, &QNetworkReply::finished, this, &Fetcher::networkFinished);
	}

signals:
	void done(QNetworkReply *reply);

private
slots:
	void networkFinished()
	{
		m_nam->cookieJar()->setCookiesFromUrl(
			QNetworkCookie::parseCookies(
				m_reply->header(QNetworkRequest::CookieHeader).toByteArray()),
			m_url);
		if (m_reply->error() == QNetworkReply::NoError && m_reply->hasRawHeader("Location"))
		{
			m_reply->deleteLater();
			m_url = m_url.resolved(QUrl(QString::fromUtf8(m_reply->rawHeader("Location"))));
			m_reply = m_nam->get(QNetworkRequest(m_url));
			connect(m_reply, &QNetworkReply::finished, this, &Fetcher::networkFinished);
			return;
		}
		emit done(m_reply);
	}

private:
	QNetworkAccessManager *m_nam;
	QUrl m_url;
	QNetworkReply *m_reply;
	bool m_fullFollow;
};

class BaseParseJob : public BaseJob
{
	Q_OBJECT
public:
	BaseParseJob() : BaseJob()
	{
		m_waiter.lock();
	}

	void setDownload(const QByteArray &data, const QUrl &url)
	{
		m_data = data;
		m_url = url;
		m_waiter.unlock();
	}
	void setQuickMods(QList<QuickMod> *mods, QMutex *mutex)
	{
		m_mods = mods;
		m_mutex = mutex;
	}

protected:
	void run(JobPointer self, Thread *thread) override
	{
		m_waiter.lock();
		emit output("Processing " + m_url.toString());
		parse(m_data, m_url);
		emit output("Done processing " + m_url.toString());
	}
	virtual void parse(const QByteArray &data, const QUrl &url) = 0;

private:
	QByteArray m_data;
	QUrl m_url;
	QMutex m_waiter;

protected:
	QList<QuickMod> *m_mods;
	QMutex *m_mutex;
};
class ChickenbonesParser : public BaseParseJob
{
	Q_OBJECT
public:
	ChickenbonesParser() : BaseParseJob()
	{
	}

protected:
	void parse(const QByteArray &data, const QUrl &url) override
	{
		QString oneline = QString::fromUtf8(data).replace('\n', "");
		while (oneline.contains("  "))
		{
			oneline.replace("  ", " ");
		}

		//   name          version        mcver    url
		QMap<QString, QMap<QString, QPair<QString, QUrl>>> versions;
		if (data.contains("New Versions"))
		{
			QRegularExpression divExp("<div id=\"s_(?<mcver>.*?)\"(?<data>.*?)</div>");
			QRegularExpression itemExp(
				"<td><a href=\"(?<url>.*?)\">(?<name>.*?) (?<version>.*?)</a></td>");
			QRegularExpressionMatchIterator divIt = divExp.globalMatch(oneline);
			while (divIt.hasNext())
			{
				auto divMatch = divIt.next();
				const QString mcver = divMatch.captured("mcver");
				if (mcver.startsWith("1.0") || mcver.startsWith("1.1") ||
					mcver.startsWith("1.2") || mcver.startsWith("1.3") ||
					mcver.startsWith("1.4") || mcver.startsWith("1.5"))
				{
					continue;
				}
				QRegularExpressionMatchIterator itemIt =
					itemExp.globalMatch(divMatch.captured("data"));
				while (itemIt.hasNext())
				{
					auto itemMatch = itemIt.next();
					const QString name = itemMatch.captured("name");
					const QString version = itemMatch.captured("version");
					if (name.contains("-dev") || name.contains("-Bukkit") ||
						version.contains("Bukkit ") || version.contains("-dev") ||
						name.contains("-src") || version.contains("Client") ||
						version.contains("Server") || !version.at(0).isDigit())
					{
						continue;
					}
					versions[name].insert(
						version, qMakePair(mcver, url.resolved(itemMatch.captured("url"))));
				}
			}
		}
		else if (data.contains("Old Versions"))
		{
			QRegularExpression divExp("<div id=\"s_(?<mcver>.*?)\"(?<data>.*?)</div>");
			QRegularExpression itemExp(
				"<tr><td align=\"center\"><i>(?<name>.*?)</i></td></tr><tr><td><a "
				"href=\"(?<url>.*?)\">(?<version>.*?)</a></td></tr>");
			QRegularExpressionMatchIterator divIt = divExp.globalMatch(oneline);
			while (divIt.hasNext())
			{
				auto divMatch = divIt.next();
				const QString mcver = divMatch.captured("mcver");
				if (mcver.startsWith("1.0") || mcver.startsWith("1.1") ||
					mcver.startsWith("1.2") || mcver.startsWith("1.3") ||
					mcver.startsWith("1.4") || mcver.startsWith("1.5"))
				{
					continue;
				}
				QRegularExpressionMatchIterator itemIt =
					itemExp.globalMatch(divMatch.captured("data"));
				while (itemIt.hasNext())
				{
					auto itemMatch = itemIt.next();
					const QString name = itemMatch.captured("name");
					const QString version = itemMatch.captured("version");
					if (name.contains("-dev") || name.contains("-Bukkit") ||
						version.contains("Bukkit ") || version.contains("-dev") ||
						name.contains("-src") || version.contains("Client") ||
						version.contains("Server") || !version.at(0).isDigit())
					{
						continue;
					}
					versions[name].insert(
						version, qMakePair(mcver, url.resolved(itemMatch.captured("url"))));
				}
			}
		}
		else
		{
			emit output("Invalid chickenbones download");
			setStatus(Status_Failed);
		}
		QString stuff;
		for (auto name : versions.keys())
		{
			stuff += name + "\n";
			for (auto version : versions[name].keys())
			{
				stuff += "  " + version + " " + versions[name][version].first + " " +
						 versions[name][version].second.toString() + "\n";
			}
		}

		QMutexLocker locker(m_mutex);
		for (int i = 0; i < m_mods->size(); ++i)
		{
			QuickMod mod = m_mods->at(i);
			if (versions.contains(mod.name))
			{
				for (auto version : versions[mod.name].keys())
				{
					int versionIndex = mod.version(version);
					QuickModVersion v =
						versionIndex == -1 ? QuickModVersion() : mod.versions.at(versionIndex);
					auto pair = versions[mod.name][version];
					if (!v.mcCompat.contains(pair.first))
					{
						v.mcCompat.append(pair.first);
					}
					if (v.url.isEmpty() || (v.url.toString().contains("adf.ly") &&
											!pair.second.toString().contains("adf.ly")))
					{
						v.url = pair.second;
						v.md5.clear();
						if (pair.second.toString().contains("adf.ly"))
						{
							v.downloadType = "sequential";
						}
						else
						{
							v.downloadType = "direct";
						}
					}
					if (v.installType == QuickModVersion::Invalid)
					{
						v.installType = QuickModVersion::ForgeMod;
					}
					v.name = version;
					if (versionIndex == -1)
					{
						mod.versions.append(v);
					}
					else
					{
						mod.versions.replace(versionIndex, v);
					}
				}
			}
			m_mods->replace(i, mod);
		}
	}
};
class CurseParser : public BaseParseJob
{
	Q_OBJECT
public:
	CurseParser(const QString &mod) : BaseParseJob(), m_mod(mod)
	{
	}

protected:
	void parse(const QByteArray &data, const QUrl &url)
	{
		QJsonDocument doc = QJsonDocument::fromJson(data);
		QJsonObject root = doc.object();

		QMutexLocker locker(m_mutex);
		int index = -1;
		for (int i = 0; i < m_mods->size(); ++i)
		{
			if (m_mods->at(i).name == m_mod)
			{
				index = i;
				break;
			}
		}
		if (index < 0)
		{
			return;
		}
		QuickMod mod = m_mods->at(index);

		if (mod.name.isEmpty())
		{
			mod.name = root.value("title").toString();
		}
		for (auto cat : root.value("category").toString().split(QRegularExpression(" and |, ")))
		{
			mod.categories.append(cat);
		}
		mod.categories.removeDuplicates();
		if (mod.iconUrl.isEmpty())
		{
			mod.iconUrl = QUrl(root.value("thumbnail").toString());
		}
		if (mod.authors.isEmpty())
		{
			for (auto author : root.value("authors").toArray().toVariantList())
			{
				mod.authors["Developer"].append(author.toString());
			}
		}
		for (auto file : root.value("files").toArray())
		{
			QJsonObject obj = file.toObject();
			const QString name = cleanVersion(obj.value("name").toString());
			if (mod.version(name) >= 0)
			{
				continue;
			}
			QuickModVersion version;
			version.name = name;
			version.url = obj.value("url").toString();
			version.mcCompat += obj.value("version").toString();
			version.downloadType = "parallel";
			version.installType = QuickModVersion::ForgeMod;
			mod.versions.append(version);
		}

		m_mods->replace(index, mod);
	}

private:
	QString m_mod;

	QString cleanVersion(const QString &in)
	{
		QRegularExpression exp("[0-9]\\.[0-9](\\.[0-9])*");
		return exp.match(in).captured();
	}
};
class BitlyBundleParser : public BaseParseJob
{
	Q_OBJECT
public:
	BitlyBundleParser(const QString &mod) : BaseParseJob(), m_mod(mod)
	{
	}

protected:
	void parse(const QByteArray &data, const QUrl &url)
	{
		QJsonObject root = QJsonDocument::fromJson(data).object();
		if (!root.value("status_code").toInt() == 200)
		{
			emit output("Invalid bitly bundle: " + url.toString());
			return;
		}
		QJsonArray links =
			root.value("data").toObject().value("bundle").toObject().value("links").toArray();
		QMap<QString, QUrl> linkList;
		for (auto link : links)
		{
			QJsonObject linkObj = link.toObject();
			const QString title = linkObj.value("title").toString();
			QUrl url = QUrl(linkObj.value("long_url").toString());
			if (url.host() == "www.dropbox.com")
			{
				url.setHost("dl.dropboxusercontent.com");
				url.setQuery("dl=1");
			}
			linkList.insert(
				QRegularExpression("([0-9a-z\\.]*)\\.[a-z]*").match(title).captured(1), url);
		}
		QMutexLocker locker(m_mutex);
		for (int i = 0; i < m_mods->size(); ++i)
		{
			if (m_mods->at(i).name == m_mod)
			{
				QuickMod mod = m_mods->at(i);
				for (auto version : linkList.keys())
				{
					if (mod.version(version) == -1)
					{
						QuickModVersion v;
						v.name = version;
						v.url = linkList[version];
						v.downloadType = "direct";
						mod.versions.append(v);
					}
				}
				m_mods->replace(i, mod);
				break;
			}
		}
	}

private:
	QString m_mod;
};
class ForgeFilesParser : public BaseParseJob
{
	Q_OBJECT
public:
	ForgeFilesParser(const QString &mod) : BaseParseJob(), m_mod(mod)
	{
	}

protected:
	void parse(const QByteArray &data, const QUrl &url)
	{
		QMap<QString, QPair<QUrl, QString> > linkList;
		const QJsonArray builds = QJsonDocument::fromJson(data).object().value("builds").toArray();
		for (auto build : builds)
		{
			const QJsonArray files = build.toObject().value("files").toArray();
			for (auto file : files)
			{
				const QJsonObject fileObj = file.toObject();
				if (fileObj.value("buildtype").toString() == "universal")
				{
					linkList[fileObj.value("jobbuildver").toString()] = qMakePair(QUrl(fileObj.value("url").toString()), fileObj.value("mcver").toString());
				}
			}
		}
		QMutexLocker locker(m_mutex);
		for (int i = 0; i < m_mods->size(); ++i)
		{
			if (m_mods->at(i).name == m_mod)
			{
				QuickMod mod = m_mods->at(i);
				for (auto version : linkList.keys())
				{
					if (mod.version(version) == -1)
					{
						QuickModVersion v;
						v.name = version;
						v.url = linkList[version].first;
						v.mcCompat += linkList[version].second;
						v.downloadType = "direct";
						mod.versions.append(v);
					}
				}
				m_mods->replace(i, mod);
				break;
			}
		}
	}

private:
	QString m_mod;
};

class NetworkJob : public BaseJob
{
	Q_OBJECT
public:
	NetworkJob(const QMap<QString, BaseParseJob *> &parsers) : BaseJob(), m_parsers(parsers)
	{
	}

protected:
	void run(JobPointer self, Thread *thread)
	{
		QEventLoop eventLoop;
		int left = m_parsers.size();
		QNetworkAccessManager *nam = new QNetworkAccessManager;
		for (auto it = m_parsers.begin(); it != m_parsers.end(); ++it)
		{
			auto parseJob = it.value();
			auto url = QUrl(it.key());
			Fetcher *fetcher = new Fetcher(url, nam);
			emit output("Fetching " + url.toString() + "...");
			connect(fetcher, &Fetcher::done,
					[parseJob, url, &left, &eventLoop, this](QNetworkReply *reply)
			{
				if (reply->error() != QNetworkReply::NoError)
				{
					emit output("Unable to fetch " + url.toString() + ": " +
								reply->errorString());
				}
				else
				{
					emit output("Fetched " + url.toString());
					parseJob->setDownload(reply->readAll(), reply->url());
				}

				--left;
				if (left == 0)
				{
					eventLoop.quit();
				}
			});
		}
		eventLoop.exec();
		delete nam;
	}

private:
	QMap<QString, BaseParseJob *> m_parsers;
};
class ChecksumJob : public BaseJob
{
	Q_OBJECT
public:
	ChecksumJob(QuickMod *mod) : BaseJob(), m_mod(mod)
	{
	}

protected:
	void run(JobPointer self, Thread *thread)
	{
		for (int i = 0; i < m_mod->versions.size(); ++i)
		{
			QuickModVersion version = m_mod->versions.at(i);
			QFile f(filename(*m_mod, version));
			if (f.open(QFile::ReadOnly))
			{
				version.md5 = QString::fromLatin1(
					QCryptographicHash::hash(f.readAll(), QCryptographicHash::Md5).toHex());
				m_mod->versions.replace(i, version);
			}
		}
	}

private:
	QuickMod *m_mod;
};
class DownloadVersionsJob : public BaseJob
{
	Q_OBJECT
public:
	DownloadVersionsJob(const QuickMod &mod) : BaseJob(), m_mod(mod)
	{
		if (!QDir::current().exists("tmp"))
		{
			QDir::current().mkdir("tmp");
		}
	}

protected:
	void run(JobPointer self, Thread *thread)
	{
		QEventLoop eventLoop;
		QNetworkAccessManager *nam = new QNetworkAccessManager;
		int left = 0;
		QTextStream in(stdin);
		for (auto version : m_mod.versions)
		{
			auto url = version.url;
			QFile *f = new QFile(filename(m_mod, version));
			if (f->exists())
			{
				continue;
			}
			if (version.downloadType != "direct" && url.host() != "www.curse.com")
			{
				QDesktopServices::openUrl(url);
				output(m_mod.name + " " + version.name + ">");
				QString i;
				in >> i;
				if (i.isEmpty())
				{
					continue;
				}
				url = QUrl::fromUserInput(i);
			}
			Fetcher *fetcher = new Fetcher(url, nam);
			emit output("Fetching " + url.toString() + "...");
			++left;
			connect(fetcher, &Fetcher::done,
					[this, url, &left, &eventLoop, f, &nam](QNetworkReply *reply)
			{
				if (reply->error() != QNetworkReply::NoError)
				{
					emit output("Error fetching " + url.toString() + ": " +
								reply->errorString());
				}
				else
				{
					if (url.host() == "www.curse.com")
					{
						auto newUrl = QUrl(QRegularExpression("data-href=\"(.*?)\"")
											   .match(QString::fromUtf8(reply->readAll()))
											   .captured(1));
						Fetcher *fetcher = new Fetcher(newUrl, nam);
						connect(fetcher, &Fetcher::done,
								[this, &eventLoop, &left, newUrl, f](QNetworkReply *reply)
						{
							if (reply->error() != QNetworkReply::NoError)
							{
								emit output("Error fetching " + newUrl.toString() + ": " +
											reply->errorString());
							}
							else
							{
								emit output("Fetched " + newUrl.toString());
								if (f->open(QFile::WriteOnly | QFile::Truncate))
								{
									f->write(reply->readAll());
								}
							}

							--left;
							if (left <= 0)
							{
								eventLoop.quit();
							}
						});
						return;
					}
					else
					{
						emit output("Fetched " + url.toString());
						if (f->open(QFile::WriteOnly | QFile::Truncate))
						{
							f->write(reply->readAll());
						}
					}
				}

				--left;
				if (left <= 0)
				{
					eventLoop.quit();
				}
			});
		}
		if (left > 0)
		{
			eventLoop.exec();
		}
		delete nam;
	}

private:
	QuickMod m_mod;
};
class JavaProbeJob : public BaseJob
{
	Q_OBJECT
public:
	JavaProbeJob(QuickMod *mod) : BaseJob(), m_mod(mod)
	{
	}

protected:
	void run(JobPointer self, Thread *thread)
	{
		QMap<QString, QString> files;
		for (int i = 0; i < m_mod->versions.size(); ++i)
		{
			QuickModVersion version = m_mod->versions.at(i);
			const QString fileName = filename(*m_mod, version);
			if (QDir::current().exists(fileName))
			{
				files.insert(version.name, fileName);
			}
		}
		const QDir appDir = QDir(qApp->applicationDirPath());
		const QStringList args = QStringList() << "-jar"
											   << appDir.absoluteFilePath("ModProbe.jar")
											   << appDir.absoluteFilePath("fml.jar");
		QProcess proc;
		proc.setProgram("java");
		proc.setArguments(args + files.values());
		proc.start();
		proc.waitForStarted();
		proc.waitForFinished();
		const QByteArray error = proc.readAllStandardError();
		const QByteArray output = proc.readAllStandardOutput();
		if (!error.isEmpty())
		{
			emit this->output(QString::fromUtf8(error));
		}
		const QJsonObject obj = QJsonDocument::fromJson(output).object();
		QStringList modids;
		QStringList packages;
		QMap<QString, QMap<QString, QMap<QString, QPair<QString, QString>>>> references;
		QMap<QString, QMap<QString, QString>> forge;
		for (auto it = obj.begin(); it != obj.end(); ++it)
		{
			const QString versionName = files.key(it.key());
			if (versionName.isEmpty())
			{
				continue;
			}
			QuickModVersion v = m_mod->versions.at(m_mod->version(versionName));
			const QJsonArray array = it.value().toArray();
			for (auto val : array)
			{
				const QJsonObject atmod = val.toObject();
				const QString modid = atmod.value("modid").toString();
				modids += modid;
				packages += atmod.value("package").toString();
				if (!atmod.value("acceptedMinecraftVersions").toString().isEmpty())
				{
					QRegularExpression exp("[0-9\\.]*");
					v.mcCompat.append(exp.match(atmod.value("acceptedMinecraftVersions")
													.toString()).captured());
					v.mcCompat.removeDuplicates();
					v.mcCompat.removeAll("");
				}
				QStringList deps =
					atmod.value("dependencies").toString().split(';', QString::SkipEmptyParts);
				QRegularExpression depExp("(?<type>[^:]*?):(?<modid>[^@]*)(@(?<version>.*))?");
				for (auto dep : deps)
				{
					QRegularExpressionMatch match = depExp.match(dep);
					const QString type = match.captured("type");
					const QString uid = uidForModId(match.captured("modid"));
					const QString version = match.captured("version");
					if (uid == "Forge")
					{
						forge[modid].insert(v.name, version);
					}
					else
					{
						references[modid][v.name].insert(
							uid, qMakePair(version, type.contains("required") ? "depends"
																			  : "recommends"));
					}
				}
			}
			m_mod->versions.replace(m_mod->version(versionName), v);
		}
		modids.removeAll("");
		modids.removeDuplicates();
		packages.removeAll("");
		packages.removeDuplicates();
		QString package;
		if (!packages.isEmpty() && m_mod->uid.isEmpty())
		{
			if (packages.size() > 1)
			{
				emit this->output("Several different packages found for " + m_mod->name +
								  ", available: " + packages.join(", ") +
								  ". The first one got selected, you may change it.");
			}
			package = packages.first();
			if (package.startsWith("mods."))
			{
				package = package.remove(0, 4);
			}
		}
		if (modids.size() > 1)
		{
			QStringList cores;
			for (auto m : modids)
			{
				if (m.contains("core", Qt::CaseInsensitive))
				{
					cores.append(m);
				}
			}
			if (!cores.isEmpty())
			{
				modids = cores;
			}
		}
		QString modid;
		if (!modids.isEmpty())
		{
			if (modids.size() > 1 && m_mod->modId.isEmpty())
			{
				emit this->output("Several different modids found for " + m_mod->name +
								  ", available: " + modids.join(", ") +
								  ". The first one got selected, you may change it.");
			}
			modid = modids.first();
		}
		if (m_mod->modId.isEmpty() && !modid.isEmpty())
		{
			m_mod->modId = modid;
		}
		if (m_mod->uid.isEmpty() && !package.isEmpty())
		{
			m_mod->uid = package;
			if (!m_mod->modId.isEmpty())
			{
				m_mod->uid += '.' + m_mod->modId.split('|').first();
			}
		}

		// finally references and forge
		QMap<QString, QMap<QString, QPair<QString, QString>>> refs = references[modid];
		QMap<QString, QString> forges = forge[modid];
		QMutableListIterator<QuickModVersion> it(m_mod->versions);
		while (it.hasNext())
		{
			QuickModVersion v = it.next();
			if (v.forgeCompat.isEmpty() && forges.contains(v.name))
			{
				v.forgeCompat = forges[v.name];
			}
			for (auto ref : refs[v.name].keys())
			{
				if (!v.references.contains(ref) && ref != m_mod->uid)
				{
					v.references.insert(ref, refs[v.name][ref]);
					const QString updateUrl = updateUrlForUid(ref);
					if (!updateUrl.isEmpty())
					{
						m_mod->references.insert(ref, updateUrl);
					}
				}
			}
			it.setValue(v);
		}
	}

private:
	QuickMod *m_mod;
};

UpdateCommand::UpdateCommand(QObject *parent) : AbstractCommand(parent)
{
}

bool UpdateCommand::handleCommand(const QString &command, const QCommandLineParser &cmd)
{
	QList<QuickMod> mods = readMultipleMods(cmd);
	QMutex modsMutex;

	QMap<QString, BaseParseJob *> parsers;

	const QString bitlyAccessToken = QString::fromLocal8Bit(qgetenv("BITLY_ACCESS_TOKEN"));

	Queue::instance()->setMaximumNumberOfThreads(1);

	// parsers
	parsers.insert("http://www.chickenbones.craftsaddle.org/Files/Old_Versions/index.php",
				   new ChickenbonesParser);
	parsers.insert("http://www.chickenbones.craftsaddle.org/Files/New_Versions/links.php",
				   new ChickenbonesParser);
	parsers.insert("http://files.minecraftforge.net/CodeChickenLib/json", new ForgeFilesParser("CodeChickenLib"));
	parsers.insert("https://api-ssl.bitly.com/v3/bundle/"
				   "contents?bundle_link=http%3A%2F%2Fbitly.com%2Fbundles%2Fzerokyuuni%2F1&"
				   "access_token=" +
					   bitlyAccessToken,
				   new BitlyBundleParser("ExAliquo"));
	for (auto mod : mods)
	{
		if (mod.websiteUrl.host() == "www.curse.com")
		{
			parsers.insert(mod.websiteUrl.toString().replace("www.curse.com", "widget.mcf.li").append(".json"), new CurseParser(mod.name));
		}
	}

	auto outputter = [this](const QString &msg)
	{ out << msg << endl << flush; };

	auto sequence = new Sequence;

	if (!cmd.isSet("no-network"))
	{
		auto parserCollection = new Collection;
		for (auto parser : parsers.values())
		{
			parser->setQuickMods(&mods, &modsMutex);
			connect(parser, &BaseParseJob::output, outputter);
			parserCollection->addJob(make_job_raw(parser));
		}

		auto networkJob = new NetworkJob(parsers);
		connect(networkJob, &NetworkJob::output, outputter);

		sequence->addJob(make_job_raw(networkJob));
		sequence->addJob(make_job_raw(parserCollection));
	}

	auto versionsCollection = new Collection;
	for (auto &mod : mods)
	{
		auto versionSequence = new Sequence;
		{
			auto downloadJob = new DownloadVersionsJob(mod);
			connect(downloadJob, &DownloadVersionsJob::output, outputter);
			versionSequence->addJob(make_job_raw(downloadJob));
		}
		{
			auto checksumJob = new ChecksumJob(&mod);
			connect(checksumJob, &ChecksumJob::output, outputter);
			versionSequence->addJob(make_job_raw(checksumJob));
		}
		{
			auto javaJob = new JavaProbeJob(&mod);
			connect(javaJob, &JavaProbeJob::output, outputter);
			versionSequence->addJob(make_job_raw(javaJob));
		}
		versionsCollection->addJob(make_job_raw(versionSequence));
	}
	sequence->addJob(make_job_raw(versionsCollection));

	enqueue_raw(sequence);
	Queue::instance()->finish();

	saveMultipleMods(mods);

	return true;
}

void UpdateCommand::populateParserForCommand(const QString &command, QCommandLineParser *cmd)
{
	addMultiFileArgument(cmd);
	cmd->addOption(QCommandLineOption("no-network", "Don't fetch remote stuff"));
}

#include "UpdateCommand.moc"
