#include "QuickModReader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QuickModReader::QuickModReader(QObject *parent) :
	QObject(parent)
{

}

QuickMod QuickModReader::read(const QByteArray &data)
{
	return jsonToMod(data, QString(), new QStringList());
}

QuickMod QuickModReader::read(const QString &fileName, QStringList *errorStrings)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
	{
		errorStrings->append(tr("Cannot read QuickMod file %1: %2").arg(file.fileName(), file.errorString()));
		return QuickMod(true);
	}
	return jsonToMod(file.readAll(), fileName, errorStrings);
}
QList<QuickMod> QuickModReader::read(const QStringList &fileNames, QStringList *errorStrings)
{
	QStringList tmp = fileNames;
	tmp.removeDuplicates();
	QList<QuickMod> res;
	foreach (const QString &fileName, tmp)
	{
		if (fileName == "index.json" || fileName.endsWith(".verify.json"))
		{
			continue;
		}
		QuickMod mod = read(fileName, errorStrings);
		if (!mod.isInvalid())
		{
			res.append(mod);
		}
	}
	return res;
}
QList<QuickMod> QuickModReader::read(const QDir &dir, QStringList *errorStrings)
{
	return read(dir.entryList(QStringList() << "*.json", QDir::Files), errorStrings);
}

QuickMod QuickModReader::jsonToMod(const QByteArray &json, const QString &filename, QStringList *errorStrings)
{
	QJsonParseError error;
	QJsonObject obj = QJsonDocument::fromJson(json, &error).object();
	if (error.error != QJsonParseError::NoError)
	{
		errorStrings->append(tr("JSON parse error in %1: %2").arg(filename, error.errorString()));
		return QuickMod();
	}

	QuickMod mod;
	mod.name = obj.value("name").toString();
	mod.nemName = obj.value("nemName").toString();
	mod.modId = obj.value("modId").toString();
	mod.description = obj.value("description").toString();
	mod.categories = jsonToStringList(obj.value("categories"));
	mod.tags = jsonToStringList(obj.value("tags"));
	mod.updateUrl = obj.value("updateUrl").toString();
	mod.authors = jsonToStringStringListMap(obj.value("authors"));
	mod.references = jsonToStringStringMap(obj.value("references"));
	mod.uid = obj.value("uid").toString();
	mod.repo = obj.value("repo").toString();
	for (auto repo : obj.value("mavenRepos").toObject())
	{
		mod.mavenRepos.append(repo.toString());
	}
	auto urls = obj.value("urls").toObject();
	for (auto it = urls.begin(); it != urls.end(); ++it)
	{
		QStringList list;
		for (auto url : it.value().toArray())
		{
			list.append(url.toString());
		}
		mod.urls[it.key()] = list;
	}
	jsonToVersion(obj.value("versions").toArray(), mod);

	return mod;
}

void QuickModReader::jsonToVersion(const QJsonArray &array, QuickMod &mod)
{
	for (auto val : array)
	{
		QJsonObject obj = val.toObject();
		QuickModVersion version;
		version.name = obj.value("name").toString();
		version.version = obj.contains("version") ? obj.value("version").toString() : QString();
		version.type = obj.value("type").toString("Release");
		version.mcCompat = jsonToStringList(obj.value("mcCompat"));
		version.references.clear();
		for (auto item : obj.value("references").toArray())
		{
			const QJsonObject refObj = item.toObject();
			version.references.insert(refObj.value("uid").toString(),
									  QuickModVersion::Reference(refObj.value("version").toString(),
																 refObj.value("type").toString(),
																 refObj.value("isSoft").toBool(false)));
		}
		version.forgeCompat = obj.value("forgeCompat").toString();
		version.liteloaderCompat = obj.value("liteloaderCompat").toString();
		version.sha1 = obj.value("sha1").toString();
		for (auto val : obj.value("urls").toArray())
		{
			const QJsonObject obj = val.toObject();
			QuickModDownload download;
			download.url = obj.value("url").toString();
			download.downloadType = obj.value("downloadType").toString();
			download.priority = obj.value("priority").toInt(0);
			download.hint = obj.value("hint").toString();
			download.group = obj.value("group").toString();
			version.urls.append(download);
		}
		const QString installType = obj.value("installType").toString();
		if (installType == "forgeMod")
		{
			version.installType = QuickModVersion::ForgeMod;
		}
		else if (installType == "forgeCoreMod")
		{
			version.installType = QuickModVersion::ForgeCoreMod;
		}
		else if (installType == "liteloaderMod")
		{
			version.installType = QuickModVersion::LiteLoaderMod;
		}
		else if (installType == "extract")
		{
			version.installType = QuickModVersion::Extract;
		}
		else if (installType == "configPack")
		{
			version.installType = QuickModVersion::ConfigPack;
		}
		else if (installType == "group")
		{
			version.installType = QuickModVersion::Group;
		}
		mod.versions.append(version);
	}
}

QMap<QString, QString> QuickModReader::jsonToStringStringMap(const QJsonValue &json)
{
	QJsonObject obj = json.toObject();
	QMap<QString, QString> out;
	foreach (const QString &key, obj.keys())
	{
		out.insert(key, obj.value(key).toString());
	}
	return out;
}

QMap<QString, QStringList> QuickModReader::jsonToStringStringListMap(const QJsonValue &json)
{
	QJsonObject obj = json.toObject();
	QMap<QString, QStringList> out;
	foreach (const QString &key, obj.keys())
	{
		QStringList list;
		foreach (const QJsonValue &val, obj.value(key).toArray())
		{
			list.append(val.toString());
		}
		out.insert(key, list);
	}
	return out;
}

QStringList QuickModReader::jsonToStringList(const QJsonValue &json)
{
	QJsonArray obj = json.toArray();
	QStringList out;
	foreach (const QJsonValue &val, obj)
	{
		out.append(val.toString());
	}
	return out;
}
