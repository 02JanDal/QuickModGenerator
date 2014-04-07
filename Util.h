#pragma once

#include <QNetworkReply>

namespace Util
{
QNetworkReply *blockNetworkReply(QNetworkReply *reply);
QByteArray getUsingExternal(const QUrl &url);
}
