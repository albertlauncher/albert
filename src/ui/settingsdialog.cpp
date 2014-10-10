#include "settingsdialog.h"
#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/appindex/appindex.h"
#include <QCloseEvent>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);


	QListWidgetItem *item = new QListWidgetItem(QIcon(":icon_websearch"),"Websearch");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	qDebug() << ui.stackedWidget->addWidget(WebSearch::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_apps"),"Apps");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	qDebug() << ui.stackedWidget->addWidget(AppIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_bookmarks"),"Bookmarks");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	qDebug() << ui.stackedWidget->addWidget(BookmarkIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_files"),"Files");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	qDebug() << ui.stackedWidget->addWidget(FileIndex::instance()->widget());

	item = new QListWidgetItem(QIcon(":icon_calc"),"Calculator");
	item->setTextAlignment(Qt::AlignHCenter);
	item->setSizeHint(QSize(96,72));
	ui.listWidget->addItem(item);
	qDebug() << ui.stackedWidget->addWidget(Calculator::instance()->widget());

	ui.listWidget->setCurrentRow(0);

}
