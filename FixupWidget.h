#ifndef FIXUPWIDGET_H
#define FIXUPWIDGET_H

#include <QDialog>
#include <QAbstractTableModel>

#include "QuickMod.h"

namespace Ui {
class FixupWidget;
}

class QuickModModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	QuickModModel(const QStringList &files, QObject *parent = 0);

	enum {
		NameCol = 0,
		NemNameCol,
		ModIdCol,
		DescriptionCol,
		WebsiteUrlCol,
		IconUrlCol,
		LogoUrlCol,
		UpdateUrlCol,
		CategoriesCol,
		TagsCol,
		UidCol,
		RepoCol
	};

	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
	void save();

signals:
	void changesChanged(bool hasChanges);

private:
	QList<QuickMod> m_mods;

	QMap<QString, QPixmap> m_icons;
	QMap<QString, QPixmap> m_logos;

	QString cleanUrl(const QString &in) const;
};

class FixupWidget : public QDialog
{
	Q_OBJECT

public:
	explicit FixupWidget(const QStringList &files, QWidget *parent = 0);
	~FixupWidget();

private:
	Ui::FixupWidget *ui;
	QuickModModel *m_model;
};

#endif // FIXUPWIDGET_H
