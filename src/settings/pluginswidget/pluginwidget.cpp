// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include "plugininstance.h"
#include "pluginmetadata.h"
#include "pluginprovider.h"
#include "pluginwidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QLocale>
using namespace albert;
using namespace std;


PluginWidget::PluginWidget(const Plugin &p):
    plugin(p)
{
    layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addWidget(createPluginPageHeader());
    layout->addWidget(body = createPluginPageBody(), 1);  // Placeholder, Strech 1
    layout->addStretch();  // Strech 0
    layout->addWidget(createPluginPageFooter());
    setLayout(layout);

    connect(&plugin, &Plugin::stateChanged,
            this, &PluginWidget::onPluginStateChanged);
}

PluginWidget::~PluginWidget()
{

}

QWidget *PluginWidget::createPluginPageHeader()
{
    QString fmt = R"(<span style="font-size:16pt;">%1</span><br>)"
                  R"(<span style="font-size:11pt; font-weight:lighter; font-style:italic;">)"
                  "%2"
                  "</span>";

    return new QLabel(fmt.arg(plugin.metaData().name, plugin.metaData().description));
}

QWidget *PluginWidget::createPluginPageBody()
{
    // Config widget or error message
    if (plugin.state() == Plugin::State::Loaded) {
        if (auto *inst = plugin.instance(); inst) {
            if (auto *cw = inst->buildConfigWidget(); cw) {
                if (auto *cwl = cw->layout(); cwl)
                    cwl->setContentsMargins(0,0,0,0);
                return cw;
            }
        }
    }

    else if (!plugin.stateInfo().isEmpty()) {
        auto *lbl = new QLabel(plugin.stateInfo());
        lbl->setWordWrap(true);
        return lbl;
    }

    return new QWidget;  // Empty placeholder
}

QWidget *PluginWidget::createPluginPageFooter()
{
    QStringList meta;

    // Credits if any
    if (const auto &list = plugin.metaData().third_party_credits; !list.isEmpty())
        meta << tr("Credits: %1").arg(list.join(", "));

    // Required executables, if any
    if (const auto &list = plugin.metaData().binary_dependencies; !list.isEmpty())
        meta << tr("Required executables: %1", nullptr, list.size()).arg(list.join(", "));

    // Required libraries, if any
    if (const auto &list = plugin.metaData().runtime_dependencies; !list.isEmpty())
        meta << tr("Required libraries: %1", nullptr, list.size()).arg(list.join(", "));

    // Id, version, license, authors
    QStringList authors;
    for (const auto &author : plugin.metaData().authors)
        if (author.startsWith(QStringLiteral("@")))
            authors << QStringLiteral("<a href=\"https://github.com/%1\">%2</a>")
                           .arg(author.mid(1), author);
        else
            authors << author;

    meta << QString("<a href=\"%1\">%2 v%3</a>. %4. %5.")
                .arg(plugin.metaData().url,
                     plugin.metaData().id,
                     plugin.metaData().version,
                     tr("License: %1").arg(plugin.metaData().license),
                     tr("Authors: %1", nullptr, authors.size()).arg(authors.join(", ")));

    // Dependencies
    if (const auto &list = plugin.dependencies(); !list.empty())
    {
        QStringList names;
        for (const auto &d : list)
            names << d->metaData().name;
        meta << tr("Requires: %1").arg(names.join(", "));
    }

    // Dependees
    if (const auto &list = plugin.dependees(); !list.empty())
    {
        QStringList names;
        for (const auto &d : list)
            names << d->metaData().name;
        meta << tr("Required by: %1").arg(names.join(", "));
    }

    // Translations
    if (const auto &list = plugin.metaData().translations; !list.empty())
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
    meta << tr("%1, Interface: %2").arg(plugin.provider->name(), plugin.metaData().iid);

    // Path
    meta << plugin.path();

    QString fmt = R"(<span style="font-size:9pt; color:#808080;">%1</span>)";
    auto *l = new QLabel(fmt.arg(meta.join("<br>")));
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    return l;
}

void PluginWidget::onPluginStateChanged()
{
    QWidget *new_w = createPluginPageBody();
    auto layout_item = layout->replaceWidget(body, new_w, Qt::FindDirectChildrenOnly);
    Q_ASSERT(layout_item != nullptr);
    layout_item->widget()->deleteLater();
    body->deleteLater();
    body = new_w;
}
