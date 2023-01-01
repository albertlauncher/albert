// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include <QLabel>
using namespace albert;

PluginLoader::PluginLoader(const QString &path) : state_(PluginState::Invalid), path(path) {}

PluginState PluginLoader::state() const { return state_; }              /// @See albert::PluginState

const QString &PluginLoader::stateInfo() const { return state_info_; }


PluginInfoWidget::PluginInfoWidget(const PluginLoader &loader)
{
    //    setStyleSheet("border: 1px solid red");
    //    setWindowFlags(Qt::Popup|Qt:: Dialog);
    //    setWindowModality(Qt::WindowModal);
    //    layout->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
    //    setContentsMargins(0,0,0,0);

    layout = new QFormLayout();
    setLayout(layout);  // The QWidget will take ownership of layout.

    layout->addRow("Provider:", new QLabel(loader.provider()->name(), this));

    layout->addRow("Path:", new QLabel(loader.path, this));

    const auto &metadata = loader.metaData();

    layout->addRow("Interface identifier:", new QLabel(metadata.iid, this));

    layout->addRow("Identifier:", new QLabel(metadata.id, this));

    layout->addRow("Version:", new QLabel(metadata.version, this));

    layout->addRow("Name:", new QLabel(metadata.name, this));

    layout->addRow("Brief description:", new QLabel(metadata.description, this));

    layout->addRow("License:", new QLabel(metadata.license, this));

    auto *label = new QLabel(this);
    label->setText(QString("<a href=\"%1\">%1</a>").arg(metadata.url));
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setFocusPolicy(Qt::NoFocus); // Remove ugly border
    label->setOpenExternalLinks(true);
    layout->addRow("Upstream:", label);

    label = new QLabel(this);
    label->setTextFormat(Qt::MarkdownText);
    label->setText(metadata.long_description);
    label->setFocusPolicy(Qt::NoFocus); // Remove ugly border
    layout->addRow("Description:", label);

    QString maintainers;
    if (metadata.maintainers.isEmpty())
        maintainers = "None! This plugin is looking for a maintainer.";
    else
        maintainers = metadata.maintainers.join(", ");
    layout->addRow("Maintainers:", new QLabel(maintainers, this));

    if (!metadata.runtime_dependencies.isEmpty())
        layout->addRow("Runtime dependencies:", new QLabel(metadata.runtime_dependencies.join(", "), this));

    if (!metadata.binary_dependencies.isEmpty())
        layout->addRow("Binary depencencies:", new QLabel(metadata.binary_dependencies.join(", "), this));

    if (!metadata.third_party_credits.isEmpty())
        layout->addRow("Third party:", new QLabel(metadata.third_party_credits.join('\n'), this));

    layout->addRow("User enableable:", new QLabel(metadata.user ? "True" : "False", this));
}




