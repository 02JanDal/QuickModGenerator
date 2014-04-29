#pragma once

#include <QObject>
#include <QDir>

#include "QuickMod.h"

class QJsonObject;

class QuickModWriter : public QObject
{
	Q_OBJECT
public:
	QuickModWriter(QObject *parent = 0);

	bool write(const QuickMod &mod, const QDir &dir, QString *errorString);
	bool write(const QList<QuickMod> &mods, const QDir &dir, QString *errorString);

private:
	static QByteArray modToJson(const QuickMod &mod);
	static QJsonArray versionToJson(const QuickMod &mod);
	static QJsonObject stringStringMapToJson(const QMap<QString, QString> &map);
	static QJsonObject stringStringListMapToJson(const QMap<QString, QStringList> &map);
};
