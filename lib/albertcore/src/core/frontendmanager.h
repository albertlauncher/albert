// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>

namespace Core {

class Frontend;
class PluginSpec;
class FrontendManagerPrivate;

class FrontendManager : public QObject
{
    Q_OBJECT

public:

    FrontendManager(QStringList pluginroots);
    ~FrontendManager();

    const std::vector<std::unique_ptr<PluginSpec> > &frontendSpecs() const;

    Frontend *currentFrontend();
    bool setCurrentFrontend(QString id);

private:

    std::unique_ptr<FrontendManagerPrivate> d;

signals:

    void frontendChanged(Frontend*);

};

}

