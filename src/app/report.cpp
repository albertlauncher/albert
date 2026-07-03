// Copyright (c) 2023-2024 Manuel Schneider

#include <QApplication>
#include <QDir>
#include <QFont>
#include <QIcon>
#include <QMetaEnum>
#include <QProcessEnvironment>
#include <QStringBuilder>
#include <QStyle>
#include <QStyleFactory>

static QString fmt (const QString &label, const QString &value)
{ return QString("%1: %2").arg(label, 21).arg(value); };

QStringList report()
{
    QStringList sl;

    // BUILD
    sl << fmt("Albert version",        QApplication::applicationVersion());
    sl << fmt("Build date",            __DATE__ " " __TIME__);
    sl << fmt("Qt version",            qVersion());
    sl << fmt("Build ABI",             QSysInfo::buildAbi());
    sl << fmt("Build architecture",    QSysInfo::buildCpuArchitecture());

    // SYSTEM
    sl << fmt("CPU architecture",      QSysInfo::currentCpuArchitecture());
    sl << fmt("Kernel type",           QSysInfo::kernelType());
    sl << fmt("Kernel version",        QSysInfo::kernelVersion());
    sl << fmt("OS",                    QSysInfo::prettyProductName());
    sl << fmt("OS type",               QSysInfo::productType());
    sl << fmt("OS version",            QSysInfo::productVersion());

    // PLATFORM, QT CONFIG
    sl << fmt("Platform name",         QGuiApplication::platformName());
    sl << fmt("Style name",            QApplication::style()->objectName());
    sl << fmt("Available styles",      QStyleFactory::keys().join(", "));
    sl << fmt("Icon theme",            QIcon::themeName());
    sl << fmt("Font",                  QGuiApplication::font().toString());
    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    QLocale loc;
    sl << fmt("Language",              metaEnum.valueToKey(loc.language()));
    sl << fmt("Locale",                loc.name());
    sl << fmt("Ui languages",          loc.uiLanguages().join(", "));

    // APP
    sl << fmt("Binary location",       QApplication::applicationFilePath());
    sl << fmt("Working dir",           QDir::currentPath());
    sl << fmt("Arguments",             QApplication::arguments().join(" "));

    // ENVIRONMENT
    sl << "ENVIRONMENT:";
    for (auto env = QProcessEnvironment::systemEnvironment();
         const auto &key : env.keys())
        sl << fmt(key, env.value(key));

    return sl;
}
