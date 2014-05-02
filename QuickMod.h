#pragma once

#include <QString>
#include <QUrl>
#include <QMap>

struct QuickModVersion
{
	QString name;
	QString type;
	QString url;
	QStringList mcCompat;
	QString forgeCompat;
	QMap<QString, QPair<QString, QString> > references;
	QString md5;
	QString downloadType;
	enum { ForgeMod, ForgeCoreMod, ConfigPack, Extract, Group, Invalid } installType = Invalid;
};

struct QuickMod
{
	QuickMod(const bool invalid = false) : m_invalid(invalid) {}
	QString name;
	QString nemName;
	QString modId;
	QString description;
	QString websiteUrl;
	QString iconUrl;
	QString logoUrl;
	QMap<QString, QString> references;
	QString updateUrl;
	QStringList categories;
	QStringList tags;
	QList<QuickModVersion> versions;
	QMap<QString, QStringList> authors;
	QString uid;
	QString repo;

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
