#include "FixupWidget.h"
#include "ui_FixupWidget.h"

#include <QMessageBox>
#include <QDebug>

#include "QuickModReader.h"
#include "QuickModWriter.h"

#include "include/modutils.h"

FixupWidget::FixupWidget(const QStringList &files, QWidget *parent)
	: QDialog(parent), ui(new Ui::FixupWidget), m_model(new QuickModModel(files, this))
{
	ui->setupUi(this);
	ui->tableView->setModel(m_model);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->resizeColumnToContents(QuickModModel::NameCol);
	ui->tableView->resizeColumnToContents(QuickModModel::NemNameCol);
	ui->tableView->resizeColumnToContents(QuickModModel::ModIdCol);
	ui->saveBtn->setEnabled(false);

	connect(ui->closeBtn, &QPushButton::clicked, [this]()
	{
		if (ui->saveBtn->isEnabled())
		{
			int ret = QMessageBox::question(this, tr("Discard"), tr("You have unsaved changes"), QMessageBox::Save, QMessageBox::Discard, QMessageBox::Cancel);
			if (ret == QMessageBox::Cancel)
			{
				return;
			}
			else if (ret == QMessageBox::Save)
			{
				m_model->save();
			}
			accept();
		}
		else
		{
			accept();
		}
	});
	connect(ui->saveBtn, &QPushButton::clicked, m_model, &QuickModModel::save);
	connect(m_model, &QuickModModel::changesChanged, ui->saveBtn, &QPushButton::setEnabled);
}

FixupWidget::~FixupWidget()
{
	delete ui;
}

QuickModModel::QuickModModel(const QStringList &files, QObject *parent)
	: QAbstractTableModel(parent)
{
	QStringList errors;
	m_mods = QuickModReader().read(files, &errors);
	if (!errors.isEmpty())
	{
		QMessageBox::critical(qApp->activePopupWidget(), tr("Error"), errors.join("\n"));
		m_mods.clear();
	}
}

int QuickModModel::rowCount(const QModelIndex &parent) const
{
	return m_mods.size();
}
int QuickModModel::columnCount(const QModelIndex &parent) const
{
	return 12;
}

QVariant QuickModModel::data(const QModelIndex &index, int role) const
{
	QuickMod mod = m_mods.at(index.row());

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (index.column())
		{
		case NameCol: return mod.name;
		case NemNameCol: return mod.nemName;
		case ModIdCol: return mod.modId;
		case WebsiteUrlCol: return mod.urls["website"];
		case IconUrlCol: return mod.urls["icon"];
		case LogoUrlCol: return mod.urls["logo"];
		case UpdateUrlCol: return mod.updateUrl;
		case CategoriesCol: return mod.categories.join(',');
		case TagsCol: return mod.tags.join(',');
		case UidCol: return mod.uid;
		case RepoCol: return mod.repo;
		}
	}
	else if (role == Qt::ToolTipRole)
	{
		switch (index.column())
		{
		case IconUrlCol:
			if (!mod.urls["icon"].isEmpty())
			{
				return "<img src=\"" + Util::expandQMURL(mod.urls["icon"].first()).toString() + "\"/>";
			}
		case LogoUrlCol:
			if (!mod.urls["logo"].isEmpty())
			{
				return "<img src=\"" + Util::expandQMURL(mod.urls["logo"].first()).toString() + "\"/>";
			}
		}
	}
	else if (role == Qt::DecorationRole)
	{
		switch (index.column())
		{
		case IconUrlCol:
			return m_icons[mod.repo + mod.uid];
		case LogoUrlCol:
			return m_logos[mod.repo + mod.uid];
		}
	}
	return QVariant();
}

bool QuickModModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	QuickMod mod = m_mods[index.row()];

	if (role == Qt::EditRole)
	{
		switch (index.column())
		{
		case NameCol: mod.name = value.toString(); break;
		case NemNameCol: mod.nemName = value.toString(); break;
		case ModIdCol: mod.modId = value.toString(); break;
		case WebsiteUrlCol: mod.urls["website"] = QStringList() << cleanUrl(value.toString()); break;
		case IconUrlCol: mod.urls["icon"] = QStringList() << cleanUrl(value.toString()); break;
		case LogoUrlCol: mod.urls["logo"] = QStringList() << cleanUrl(value.toString()); break;
		case UpdateUrlCol: mod.updateUrl = cleanUrl(value.toString()); break;
		case CategoriesCol: mod.categories = value.toString().split(','); break;
		case TagsCol: mod.tags = value.toString().split(','); break;
		case UidCol: mod.uid = value.toString(); break;
		case RepoCol: mod.repo = value.toString(); break;
		default:
			return false;
		}
		m_mods[index.row()] = mod;
		emit dataChanged(index, index, QVector<int>() << role);
		emit changesChanged(true);
		return true;
	}

	return false;
}

QVariant QuickModModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case NameCol:
			return tr("Name");
		case NemNameCol:
			return tr("NEM Name");
		case ModIdCol:
			return tr("Mod ID");
		case DescriptionCol:
			return tr("Description");
		case WebsiteUrlCol:
			return tr("Website");
		case IconUrlCol:
			return tr("Icon");
		case LogoUrlCol:
			return tr("Logo");
		case UpdateUrlCol:
			return tr("Update URL");
		case CategoriesCol:
			return tr("Categories");
		case TagsCol:
			return tr("Tags");
		case UidCol:
			return tr("Uid");
		case RepoCol:
			return tr("Repo");
		}
		return QVariant();
	}
	return QVariant();
}

Qt::ItemFlags QuickModModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void QuickModModel::save()
{
	QString error;
	QuickModWriter().write(m_mods, QDir::current(), &error);
	if (error.isEmpty())
	{
		emit changesChanged(false);
	}
	else
	{
		QMessageBox::critical(qApp->activeModalWidget(), tr("Error"), tr("Error saving QuickMod files: %1").arg(error));
	}
}

QString QuickModModel::cleanUrl(const QString &in) const
{
	QRegularExpression ghExp("https://raw\\.github\\.com/(?<user>[^/]*)/(?<repo>[^/]*)/(?<branch>[^/]*)/(?<file>.*)");
	QRegularExpressionMatch ghMatch = ghExp.match(in);
	if (ghMatch.hasMatch())
	{
		return QString("github://%1@%2/%3").arg(ghMatch.captured("user"), ghMatch.captured("repo"), ghMatch.captured("file")) + (ghMatch.captured("branch") == "master" ? QString() : "#" + ghMatch.captured("branch"));
	}
	QRegularExpression mcfExp("http://www\\.minecraftforum\\.net/topic/(?<id>[0-9]*)-.*");
	QRegularExpressionMatch mcfMatch = mcfExp.match(in);
	if (mcfMatch.hasMatch())
	{
		return QString("mcf:") + mcfMatch.captured("id");
	}
	QRegularExpression curseExp("http://www.curse.com/mc-mods/minecraft/(?<id>.*)");
	QRegularExpressionMatch curseMatch = curseExp.match(in);
	if (curseMatch.hasMatch())
	{
		return QString("curse:") + curseMatch.captured("id");
	}
	return in;
}
