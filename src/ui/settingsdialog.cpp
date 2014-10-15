#include "settingsdialog.h"
#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/appindex/appindex.h"
#include <QCloseEvent>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);


	/* APPEARANCE */

	QDir dir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/albert/skins");
	QStringList filters;
	filters << "*.qss";
	dir.setNameFilters(filters);

	// Add skins to list
	QFileInfoList list = dir.entryInfoList();
	for ( QFileInfo &dfi : list)
		ui.listWidget_skins->addItem(dfi.baseName());

	// Apply a skin if clicked
	connect(ui.listWidget_skins, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(onSkinClicked(QListWidgetItem*)));



	/* MODULES */

	QListWidgetItem *item = new QListWidgetItem(QIcon(":icon_websearch"),"Websearch");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(WebSearch::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_apps"),"Apps");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(AppIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_bookmarks"),"Bookmarks");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(BookmarkIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_files"),"Files");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(FileIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_calc"),"Calculator");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(Calculator::instance()->widget());

	ui.listWidget->setCurrentRow(0);

}

void SettingsDialog::onSkinClicked(QListWidgetItem *i)
{
	QFile styleFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/albert/skins/"+i->text()+".qss");
	if (styleFile.open(QFile::ReadOnly)) {
		qApp->setStyleSheet(styleFile.readAll());
		styleFile.close();
	}
}
