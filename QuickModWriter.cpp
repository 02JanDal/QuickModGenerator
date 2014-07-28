#include "QuickModWriter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QuickModWriter::QuickModWriter(QObject *parent) :
	QObject(parent)
{

}

bool QuickModWriter::write(const QuickMod &mod, const QDir &dir, QString *errorString)
{
	// mod
	{
		const QString fileName = mod.updateUrl.mid(mod.updateUrl.lastIndexOf('/')+1);
		QFile file(fileName);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			*errorString = tr("Cannot write QuickMod file %1 (%2): %3").arg(file.fileName(), mod.name, file.errorString());
			return false;
		}
		const QByteArray data = modToJson(mod);
		file.write(data);
		file.close();
	}
	return true;
}
bool QuickModWriter::write(const QList<QuickMod> &mods, const QDir &dir, QString *errorString)
{
	foreach (const QuickMod &mod, mods)
	{
		if (!write(mod, dir, errorString))
		{
			return false;
		}
	}

	return true;
}

QByteArray QuickModWriter::modToJson(const QuickMod &mod)
{
	QJsonObject obj;

	obj.insert("formatVersion", 1);
	obj.insert("name", mod.name);
	obj.insert("description", mod.description);
	obj.insert("nemName", mod.nemName);
	obj.insert("modId", mod.modId);
	QJsonObject urls;
	for (auto it = mod.urls.begin(); it != mod.urls.end(); ++it)
	{
		urls.insert(it.key(), QJsonArray::fromStringList(it.value()));
	}
	obj.insert("urls", urls);
	obj.insert("updateUrl", mod.updateUrl);
	obj.insert("categories", QJsonArray::fromStringList(mod.categories));
	obj.insert("tags", QJsonArray::fromStringList(mod.tags));
	obj.insert("references", stringStringMapToJson(mod.references));
	obj.insert("authors", stringStringListMapToJson(mod.authors));
	obj.insert("uid", mod.uid);
	obj.insert("repo", mod.repo);
	obj.insert("versions", versionToJson(mod));
	obj.insert("license", mod.license);

	return QJsonDocument(obj).toJson(QJsonDocument::Indented);
}

QJsonArray QuickModWriter::versionToJson(const QuickMod &mod)
{
	QJsonArray array;
	foreach (const QuickModVersion &ver, mod.versions)
	{
		QJsonObject obj;
		obj.insert("name", ver.name);
		obj.insert("type", ver.type);
		obj.insert("mcCompat", QJsonArray::fromStringList(ver.mcCompat));
		QJsonArray refs;
		for (auto ref : ver.references.keys())
		{
			QJsonObject refObj;
			refObj.insert("uid", ref);
			refObj.insert("version", ver.references[ref].version);
			refObj.insert("type", ver.references[ref].type);
			if (ver.references[ref].isSoft)
			{
				refObj.insert("isSoft", true);
			}
			refs.append(refObj);
		}
		obj.insert("references", refs);
		if (!ver.forgeCompat.isEmpty())
		{
			obj.insert("forgeCompat", ver.forgeCompat);
		}
		if (!ver.liteloaderCompat.isEmpty())
		{
			obj.insert("liteloaderCompat", ver.liteloaderCompat);
		}
		obj.insert("sha1", ver.sha1);
		switch (ver.installType)
		{
		case QuickModVersion::ForgeMod:      obj.insert("installType", QStringLiteral("forgeMod"));      break;
		case QuickModVersion::ForgeCoreMod:  obj.insert("installType", QStringLiteral("forgeCoreMod"));  break;
		case QuickModVersion::LiteLoaderMod: obj.insert("installType", QStringLiteral("liteloaderMod")); break;
		case QuickModVersion::Extract:       obj.insert("installType", QStringLiteral("extract"));	    break;
		case QuickModVersion::ConfigPack:    obj.insert("installType", QStringLiteral("configPack"));    break;
		case QuickModVersion::Group:	     obj.insert("installType", QStringLiteral("group"));	        break;
		default: obj.insert("installType", QString("forgeMod")); break;
		}
		QJsonArray urls;
		for (auto download : ver.urls)
		{
			urls.append(downloadToJson(download, ver.urls.size() == 1));
		}
		obj.insert("urls", urls);
		array.append(obj);
	}
	return array;
}

QJsonObject QuickModWriter::downloadToJson(const QuickModDownload &download, const bool isOnly)
{
	QJsonObject obj;
	obj.insert("url", download.url);
	obj.insert("downloadType", download.downloadType);
	if (!isOnly)
	{
		obj.insert("priority", download.priority);
	}
	if (!download.hint.isEmpty())
	{
		obj.insert("hint", download.hint);
	}
	if (!download.group.isEmpty())
	{
		obj.insert("group", download.group);
	}
	return obj;
}

QJsonObject QuickModWriter::stringStringMapToJson(const QMap<QString, QString> &map)
{
	QJsonObject out;
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		out.insert(it.key(), it.value());
	}
	return out;
}

QJsonObject QuickModWriter::stringStringListMapToJson(const QMap<QString, QStringList> &map)
{
	QJsonObject out;
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		out.insert(it.key(), QJsonArray::fromStringList(it.value()));
	}
	return out;
}
