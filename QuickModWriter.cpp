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
		const QString fileName = mod.updateUrl.path().mid(mod.updateUrl.path().lastIndexOf('/')+1);
		QFile file(fileName);
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
		{
			*errorString = tr("Cannot write QuickMod file %1: %2").arg(file.fileName(), file.errorString());
			return false;
		}
		file.write(modToJson(mod));
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

	obj.insert("name", mod.name);
	obj.insert("description", mod.description);
	obj.insert("nemName", mod.nemName);
	obj.insert("modId", mod.modId);
	obj.insert("websiteUrl", mod.websiteUrl.toString(QUrl::FullyEncoded));
	obj.insert("iconUrl", mod.iconUrl.toString(QUrl::FullyEncoded));
	obj.insert("logoUrl", mod.logoUrl.toString(QUrl::FullyEncoded));
	obj.insert("updateUrl", mod.updateUrl.toString(QUrl::FullyEncoded));
	obj.insert("categories", QJsonArray::fromStringList(mod.categories));
	obj.insert("tags", QJsonArray::fromStringList(mod.tags));
	obj.insert("references", stringStringMapToJson(mod.references));
	obj.insert("authors", stringStringListMapToJson(mod.authors));
	obj.insert("uid", mod.uid);
	obj.insert("versions", versionToJson(mod));

	return QJsonDocument(obj).toJson(QJsonDocument::Indented);
}

QJsonArray QuickModWriter::versionToJson(const QuickMod &mod)
{
	QJsonArray array;
	foreach (const QuickModVersion &ver, mod.versions)
	{
		QJsonObject obj;
		obj.insert("name", ver.name);
		obj.insert("url", ver.url.toString(QUrl::FullyEncoded));
		obj.insert("mcCompat", QJsonArray::fromStringList(ver.mcCompat));
		QJsonArray refs;
		for (auto ref : ver.references.keys())
		{
			QJsonObject refObj;
			refObj.insert("uid", ref);
			refObj.insert("version", ver.references[ref].first);
			refObj.insert("type", ver.references[ref].second);
			refs.append(refObj);
		}
		obj.insert("references", refs);
		if (!ver.forgeCompat.isEmpty())
		{
			obj.insert("forgeCompat", ver.forgeCompat);
		}
		if (!ver.md5.isEmpty())
		{
			obj.insert("md5", ver.md5);
		}
		obj.insert("downloadType", ver.downloadType);
		switch (ver.installType)
		{
		case QuickModVersion::ForgeMod:     obj.insert("installType", QString("forgeMod"));     break;
		case QuickModVersion::ForgeCoreMod: obj.insert("installType", QString("forgeCoreMod")); break;
		case QuickModVersion::Extract:      obj.insert("installType", QString("extract"));		break;
		case QuickModVersion::ConfigPack:   obj.insert("installType", QString("configPack"));   break;
		case QuickModVersion::Group:	    obj.insert("installType", QString("group"));		break;
		}
		array.append(obj);
	}
	return array;
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
