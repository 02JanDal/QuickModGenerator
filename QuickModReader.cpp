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
	return jsonToMod(data, new QStringList());
}

QuickMod QuickModReader::read(const QString &fileName, QStringList *errorStrings)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
	{
		errorStrings->append(tr("Cannot read QuickMod file %1: %2").arg(file.fileName(), file.errorString()));
		return QuickMod(true);
	}
	return jsonToMod(file.readAll(), errorStrings);
}
QList<QuickMod> QuickModReader::read(const QStringList &fileNames, QStringList *errorStrings)
{
	QStringList tmp = fileNames;
	tmp.removeDuplicates();
	QList<QuickMod> res;
	foreach (const QString &fileName, tmp)
	{
		if (fileName == "index.json")
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

QuickMod QuickModReader::jsonToMod(const QByteArray &json, QStringList *errorStrings)
{
	QJsonParseError error;
	QJsonObject obj = QJsonDocument::fromJson(json, &error).object();
	if (error.error != QJsonParseError::NoError)
	{
		errorStrings->append(error.errorString());
		return QuickMod();
	}

	QuickMod mod;
	mod.name = obj.value("name").toString();
	mod.nemName = obj.value("nemName").toString();
	mod.modId = obj.value("modId").toString();
	mod.description = obj.value("description").toString();
	mod.categories = jsonToStringList(obj.value("categories"));
	mod.tags = jsonToStringList(obj.value("tags"));
	mod.websiteUrl = obj.value("websiteUrl").toString();
	mod.iconUrl = obj.value("iconUrl").toString();
	mod.logoUrl = obj.value("logoUrl").toString();
	mod.updateUrl = obj.value("updateUrl").toString();
	mod.authors = jsonToStringStringListMap(obj.value("authors"));
	mod.references = jsonToStringStringMap(obj.value("references"));
	mod.uid = obj.value("uid").toString();
	mod.repo = obj.value("repo").toString();
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
		version.type = obj.value("type").toString("Release");
		version.url = obj.value("url").toString();
		version.mcCompat = jsonToStringList(obj.value("mcCompat"));
		version.references.clear();
		for (auto item : obj.value("references").toArray())
		{
			const QJsonObject refObj = item.toObject();
			version.references.insert(refObj.value("uid").toString(), qMakePair(refObj.value("version").toString(), refObj.value("type").toString()));
		}
		version.forgeCompat = obj.value("forgeCompat").toString();
		version.md5 = obj.value("md5").toString();
		version.downloadType = obj.value("downloadType").toString("parallel");
		if (version.downloadType.isEmpty())
		{
			version.downloadType = "parallel";
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
