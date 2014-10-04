#include "settingsdialog.h"
#include <QCloseEvent>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);
//	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::WindowStaysOnTopHint
					|Qt::Dialog );
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
	event->ignore();
}
