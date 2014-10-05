#include "settingsdialog.h"
#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/applicationindex/appindex.h"
#include <QCloseEvent>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);
//	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::WindowStaysOnTopHint
					|Qt::Dialog );

	ui.tabAppIndex->layout()->addWidget(ApplicationIndex::instance()->widget());
	ui.tabBookmarkIndex->layout()->addWidget(BookmarkIndex::instance()->widget());
	ui.tabFileIndex->layout()->addWidget(FileIndex::instance()->widget());
	ui.tabCalculator->layout()->addWidget(Calculator::instance()->widget());
	ui.tabWebSearch->layout()->addWidget(WebSearch::instance()->widget());
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
	event->ignore();
}
