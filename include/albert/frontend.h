// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>
class QWidget;

namespace albert
{
class Query;

///
/// Convention on the item roles passed around
///
enum class ALBERT_EXPORT ItemRoles
{
    TextRole = Qt::DisplayRole,  ///< QString, The text
    SubTextRole = Qt::UserRole,  ///< QString, The subtext
    InputActionRole,  ///< QString, The tab action text
    IconUrlsRole,  ///< QStringList, Urls for icon lookup
    ActionsListRole,  ///< QStringList, List of action names
    // Dont change these without changing ItemsModel::roleNames
};


///
/// The interface for albert frontends.
///
class ALBERT_EXPORT Frontend : public QObject
{
    Q_OBJECT

public:

    /// Visibility of the frontend
    virtual bool isVisible() const = 0;

    /// Set the visibility state of the frontend
    virtual void setVisible(bool visible) = 0;

    /// Input line text
    virtual QString input() const = 0;

    /// Input line text setter
    virtual void setInput(const QString&) = 0;

    /// The native window id. Used to apply platform quirks.
    virtual unsigned long long winId() const = 0;

    /// The config widget show in the window settings tab
    virtual QWidget *createFrontendConfigWidget() = 0;

    /// The query setter
    virtual void setQuery(Query *query) = 0;

signals:

    void inputChanged(QString);
    void visibleChanged(bool);

protected:

    ~Frontend() override;

};

}
