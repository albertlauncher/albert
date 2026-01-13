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

QWidget *PluginWidget::createPluginPageHeader() const
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *t = new QLabel(plugin.metadata.version.startsWith("0.") ? plugin.metadata.name + " ⚠️🚧👷"
                                                                  : plugin.metadata.name);
    auto f = w->font();
    f.setPointSize(f.pointSize() + 2);
    t->setFont(f);
    t->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto *d = new QLabel(plugin.metadata.description);
    f = w->font();
    f.setPointSize(f.pointSize() - 2);
    d->setForegroundRole(QPalette::PlaceholderText);
    d->setFont(f);

    l->setContentsMargins({});
    l->setSpacing(2);
    l->addWidget(t);
    l->addWidget(d);

    return w;
}

QWidget *PluginWidget::createPluginPageBody() const
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
        lbl->setAlignment(Qt::AlignTop);
        return lbl;
    }

    return new QWidget;  // Empty placeholder
}

QWidget *PluginWidget::createPluginPageFooter() const
{
    QStringList meta;

    if (!plugin.metadata.readme_url.isEmpty())
        meta << QString("<a href=\"%1\">README</a>").arg(plugin.metadata.readme_url);

    // Id, version
    meta << QString("<a href=\"%1\">%2 v%3</a>")
                .arg(plugin.metadata.url, plugin.metadata.id, plugin.metadata.version);

    // License
    meta << tr("License: %1").arg(plugin.metadata.license);

    // Authors
    QStringList authors;
    for (const auto &a : plugin.metadata.authors)
        if (a.startsWith(QStringLiteral("@")))
            authors << QStringLiteral("<a href=\"https://github.com/%1\">%2</a>")
                           .arg(a.mid(1), a);
        else
            authors << a;

    meta << tr("Authors: %1", nullptr, authors.size()).arg(authors.join(", "));

    // Maintainers
    QStringList maintainers;
    for (const auto &m : plugin.metadata.maintainers)
        if (m.startsWith(QStringLiteral("@")))
            maintainers << QStringLiteral("<a href=\"https://github.com/%1\">%2</a>")
                           .arg(m.mid(1), m);
        else
            maintainers << m;

    meta << tr("Maintainers: %1", nullptr, authors.size())
                //: Placeholder for empty maintainers
                .arg(maintainers.isEmpty() ? QStringLiteral("<b>%1</b>").arg(tr("Wanted!"))
                                           : maintainers.join(", "));

    // Dependencies
    if (const auto &list = plugin_registry.dependencies(&plugin);
        !list.empty())
    {
        auto names = list | views::transform([](const auto &p){ return p->metadata.name; });
        meta << tr("Required plugins: %1", nullptr, names.size())
                    .arg(QStringList(names.begin(), names.end()).join(", "));  // ranges::to
    }

    // Dependees
    if (const auto &list = plugin_registry.dependees(&plugin);
        !list.empty())
    {
        auto names = list | views::transform([](const auto &p){ return p->metadata.name; });
        meta << tr("Required by plugins: %1", nullptr, names.size())
                    .arg(QStringList(names.begin(), names.end()).join(", "));  // ranges::to
    }

    // Required executables, if any
    if (const auto &list = plugin.metadata.binary_dependencies; !list.isEmpty())
        meta << tr("Required executables: %1", nullptr, list.size()).arg(list.join(", "));

    // Required libraries, if any
    if (const auto &list = plugin.metadata.runtime_dependencies; !list.isEmpty())
        meta << tr("Required libraries: %1", nullptr, list.size()).arg(list.join(", "));

    // Translations
    if (const auto &list = plugin.metadata.translations; !list.empty())
    {
        QStringList displayList;
        for (const auto &lang : list)
        {
            auto split = lang.split(" ");
            auto language = QLocale(split[0]).nativeLanguageName();
            displayList << QString("%1 %2").arg(language, split[1]);
        }
        meta << tr("Translations: %1").arg(displayList.join(", "));
    }

    // Provider
    meta << tr("%1, Interface: %2").arg(plugin.provider.name(), plugin.metadata.iid);

    // Path
    meta << plugin.loader.path();

    // Credits if any
    if (const auto &list = plugin.metadata.third_party_credits; !list.isEmpty())
        meta << tr("Credits: %1").arg(list.join(", "));

    auto *l = new QLabel(meta.join("<br>"));
    auto font = l->font();
    font.setPointSize(font.pointSize() - 4);
    l->setForegroundRole(QPalette::PlaceholderText);
    l->setFont(font);
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    return l;
}

void PluginWidget::onPluginStateChanged(const QString &id)
{
    QWidget *new_body;
    if (plugin.id == id && plugin.state == Loaded)
        new_body = createPluginPageBody();
    else
        new_body = new QWidget;

    auto layout_item = layout->replaceWidget(body, new_body, Qt::FindDirectChildrenOnly);
    Q_ASSERT(layout_item != nullptr);

    // Do _not_ delete later
    delete layout_item;
    delete body;

    body = new_body;
}
