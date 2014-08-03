#pragma once

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>

struct QuickModDownload
{
	QuickModDownload(const QString &url, const QString &downloadType) : url(url), downloadType(downloadType) {}
	QuickModDownload() {}
	QString url;
	QString downloadType;
	int priority = -1;
	QString hint;
	QString group;
};

struct QuickModVersion
{
	QString name;
	QString version;
	QString type;
	QStringList mcCompat;
	QString forgeCompat;
	QString liteloaderCompat;
	struct Reference
	{
		Reference()
		{
		}
		Reference(const QString &version, const QString &type, const bool isSoft = false)
			: version(version), type(type), isSoft(isSoft)
		{
		}
		QString version;
		QString type;
		bool isSoft = false;
	};

	QMap<QString, Reference> references;
	QString sha1;
	QList<QuickModDownload> urls;
	enum { ForgeMod, ForgeCoreMod, LiteLoaderMod, ConfigPack, Extract, Group, Invalid } installType = Invalid;

	void sort();
	void tryAddUrl(const QuickModDownload &download);
	QuickModDownload getBestDownload();
};

struct QuickMod
{
	QuickMod(const bool invalid = false) : m_invalid(invalid) {}
	QString name;
	QString nemName;
	QString modId;
	QString description;
	QMap<QString, QStringList> urls;
	QMap<QString, QString> references;
	QString updateUrl;
	QStringList categories;
	QStringList tags;
	QList<QuickModVersion> versions;
	QMap<QString, QStringList> authors;
	QString uid;
	QString repo;
	QString license;
	QStringList mavenRepos;

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
