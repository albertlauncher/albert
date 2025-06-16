// Copyright (c) 2022-2025 Manuel Schneider

#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginprovider.h"
#include "pluginregistry.h"
#include "pluginwidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QLocale>
#include <ranges>
using enum Plugin::State;
using namespace albert;
using namespace std;


PluginWidget::PluginWidget(const PluginRegistry &r, const Plugin &p):
    plugin_registry(r), plugin(p)
{
    layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addWidget(createPluginPageHeader());
    layout->addWidget(body = createPluginPageBody(), 1);  // Placeholder, Strech 1
    layout->addStretch();  // Strech 0
    layout->addWidget(createPluginPageFooter());
    setLayout(layout);

    connect(&plugin_registry, &PluginRegistry::pluginStateChanged,
            this, &PluginWidget::onPluginStateChanged);
}

PluginWidget::~PluginWidget() = default;

QWidget *PluginWidget::createPluginPageHeader()
{
    QString fmt = R"(<span style="font-size:16pt;">%1</span><br>)"
                  R"(<span style="font-size:11pt; font-weight:lighter; font-style:italic;">)"
                  "%2"
                  "</span>";

    return new QLabel(fmt.arg(plugin.metadata.name, plugin.metadata.description));
}

QWidget *PluginWidget::createPluginPageBody()
{
    if (auto *inst = plugin.loader.instance(); inst)
    {
        if (auto *cw = inst->buildConfigWidget(); cw)
        {
            if (auto *cwl = cw->layout(); cwl)
                cwl->setContentsMargins(0,0,0,0);
            return cw;
        }
    }

    else if (const auto info = plugin.state_info;
             !info.isEmpty())
    {
        auto *lbl = new QLabel(info);
        lbl->setWordWrap(true);
        return lbl;
    }

    return new QWidget;  // Empty placeholder
}

QWidget *PluginWidget::createPluginPageFooter()
{
    QStringList meta;

    // Credits if any
    if (const auto &list = plugin.metadata.third_party_credits; !list.isEmpty())
        meta << tr("Credits: %1").arg(list.join(", "));

    // Required executables, if any
    if (const auto &list = plugin.metadata.binary_dependencies; !list.isEmpty())
        meta << tr("Required executables: %1", nullptr, list.size()).arg(list.join(", "));

    // Required libraries, if any
    if (const auto &list = plugin.metadata.runtime_dependencies; !list.isEmpty())
        meta << tr("Required libraries: %1", nullptr, list.size()).arg(list.join(", "));

    // Id, version, license, authors
    QStringList authors;
    for (const auto &author : plugin.metadata.authors)
        if (author.startsWith(QStringLiteral("@")))
            authors << QStringLiteral("<a href=\"https://github.com/%1\">%2</a>")
                           .arg(author.mid(1), author);
        else
            authors << author;

    meta << QString("<a href=\"%1\">%2 v%3</a>. %4. %5.")
                .arg(plugin.metadata.url,
                     plugin.metadata.id,
                     plugin.metadata.version,
                     tr("License: %1").arg(plugin.metadata.license),
                     tr("Authors: %1", nullptr, authors.size()).arg(authors.join(", ")));


    // Dependencies
    if (const auto &list = plugin_registry.dependencies(&plugin);
        !list.empty())
    {
        auto names = list | views::transform([](const auto &p){ return p->metadata.name; });
        meta << tr("Requires: %1").arg(QStringList(names.begin(), names.end()).join(", "));  // ranges::to
    }

    // Dependees
    if (const auto &list = plugin_registry.dependees(&plugin);
        !list.empty())
    {
        auto names = list | views::transform([](const auto &p){ return p->metadata.name; });
        meta << tr("Required by: %1").arg(QStringList(names.begin(), names.end()).join(", "));  // ranges::to
    }

    // Translations
    if (const auto &list = plugin.metadata.translations; !list.empty())
    {
        QStringList displayList;
        for (const auto &lang : list)
        {
            auto split = lang.split(" ");
            ;
            auto language = QLocale(split[0]).nativeLanguageName();
            displayList << QString("%1 %2").arg(language, split[1]);
        }
        meta << tr("Translations: %1").arg(displayList.join(", "));
    }

    // Provider
    meta << tr("%1, Interface: %2").arg(plugin.provider.name(), plugin.metadata.iid);

    // Path
    meta << plugin.loader.path();

    QString fmt = R"(<span style="font-size:9pt; color:#808080;">%1</span>)";
    auto *l = new QLabel(fmt.arg(meta.join("<br>")));
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    return l;
}

void PluginWidget::onPluginStateChanged(const QString &id)
{
    if (plugin.id == id)
    {
        QWidget *new_body = createPluginPageBody();

        auto layout_item = layout->replaceWidget(body, new_body, Qt::FindDirectChildrenOnly);
        Q_ASSERT(layout_item != nullptr);

                // Do not! delete later
        delete layout_item;
        delete body;

        body = new_body;
    }
}
