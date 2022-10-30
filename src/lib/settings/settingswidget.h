// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
#include <QListWidget>
#include <QStackedWidget>
#include <vector>
class App;

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(App &albert);

private:
    void resetUI();

    App &albert;
    QListWidget list_widget;
    QStackedWidget stacked_widget;

};

