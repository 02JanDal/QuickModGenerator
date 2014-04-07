#pragma once

#include <QString>
#include <QUrl>
#include <QMap>

struct QuickModVersion
{
	QString name;
	QUrl url;
	QStringList mcCompat;
	QString forgeCompat;
	QMap<QString, QPair<QString, QString> > references;
	QString md5;
	QString downloadType;
	enum { ForgeMod, ForgeCoreMod, ConfigPack, Extract, Group } installType = ForgeMod;
};

struct QuickMod
{
	QuickMod(const bool invalid = false) : m_invalid(invalid) {}
	QString name;
	QString nemName;
	QString modId;
	QString description;
	QUrl websiteUrl;
	QUrl iconUrl;
	QUrl logoUrl;
	QMap<QString, QString> references;
	QUrl updateUrl;
	QStringList categories;
	QStringList tags;
	QList<QuickModVersion> versions;
	QMap<QString, QStringList> authors;
	QUrl versionsUrl;
	QString uid;

	bool isInvalid() const { return m_invalid; }

	bool containsVersion(const QString &name) const
	{
		foreach (const QuickModVersion &version, versions)
		{
			if (version.name == name)
			{
				return true;
			}
		}
		return false;
	}

	int version(const QString &name) const
	{
		for (int i = 0; i < versions.size(); ++i)
		{
			if (versions.at(i).name == name)
			{
				return i;
			}
		}
		return -1;
	}

private:
	bool m_invalid;
};
