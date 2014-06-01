#include "QuickMod.h"

void QuickModVersion::sort()
{
	std::sort(urls.begin(), urls.end(), [](const QuickModDownload &d1, const QuickModDownload &d2)
	{
		return d1.priority < d2.priority;
	});
}

void QuickModVersion::tryAddUrl(const QuickModDownload &download)
{
	if (urls.isEmpty())
	{
		urls.append(download);
		return;
	}
	sort();
	for (auto existing : urls)
	{
		if (existing.url == download.url || QUrl(existing.url) == QUrl(download.url))
		{
			return;
		}
	}
	auto it = urls.begin();
	QString last;
	while (true)
	{
		const QuickModDownload dl = *it;
		if (last != dl.downloadType && dl.downloadType == download.downloadType)
		{
			urls.insert(it, download);
			return;
		}
		last = dl.downloadType;
		++it;
	}
	// If all else fails, just append it
	urls.append(download);
}

QuickModDownload QuickModVersion::getBestDownload()
{
	if (urls.isEmpty())
	{
		return QuickModDownload();
	}
	sort();
	for (auto download : urls)
	{
		if (download.downloadType == "direct")
		{
			return download;
		}
	}
	for (auto download : urls)
	{
		if (download.url.contains("www.curse.com"))
		{
			return download;
		}
	}
	for (auto download : urls)
	{
		if (download.downloadType != "encoded")
		{
			return download;
		}
	}
	return urls.first();
}
