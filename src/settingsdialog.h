#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "singleton.h"
#include "ui_settingsdialog.h"
#include <QWidget>

class SettingsDialog : public QDialog, public Singleton<SettingsDialog>
{
	friend class Singleton<SettingsDialog>;

	Ui::SettingsDialog ui;

	Q_OBJECT
public:
	explicit SettingsDialog(QWidget *parent = 0);

signals:

public slots:

};

#endif // SETTINGSDIALOG_H
