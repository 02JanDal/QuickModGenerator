#pragma once

#include <QObject>
#include <QDir>

#include "QuickMod.h"

class QuickModReader : public QObject
{
	Q_OBJECT
public:
	QuickModReader(QObject *parent = 0);

	QuickMod read(const QByteArray &data);
	QuickMod read(const QString &fileName, QStringList *errorStrings);
	QList<QuickMod> read(const QStringList &fileNames, QStringList *errorStrings);
	QList<QuickMod> read(const QDir &dir, QStringList *errorStrings);

private:
	QuickMod jsonToMod(const QByteArray &json, const QString &filename, QStringList *errorStrings);
	void jsonToVersion(const QJsonArray &array, QuickMod &mod);
	QMap<QString, QString> jsonToStringStringMap(const QJsonValue &json);
	QMap<QString, QStringList> jsonToStringStringListMap(const QJsonValue &json);
	QStringList jsonToStringList(const QJsonValue &json);
};
