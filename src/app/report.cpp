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

QStringList report()
{
    auto fn = [](const QString &label, const QString &value)
    { return QString("%1: %2").arg(label, 21).arg(value); };

    QStringList sl;

    // BUILD
    sl << fn("Albert version",        QApplication::applicationVersion());
    sl << fn("Build date",            __DATE__ " " __TIME__);
    sl << fn("Qt version",            qVersion());
    sl << fn("Build ABI",             QSysInfo::buildAbi());
    sl << fn("Build architecture",    QSysInfo::buildCpuArchitecture());

    // SYSTEM
    sl << fn("CPU architecture",      QSysInfo::currentCpuArchitecture());
    sl << fn("Kernel type",           QSysInfo::kernelType());
    sl << fn("Kernel version",        QSysInfo::kernelVersion());
    sl << fn("OS",                    QSysInfo::prettyProductName());
    sl << fn("OS type",               QSysInfo::productType());
    sl << fn("OS version",            QSysInfo::productVersion());

    // PLATFORM, QT CONFIG
    sl << fn("Platform name",         QGuiApplication::platformName());
    sl << fn("Style name",            QApplication::style()->objectName());
    sl << fn("Available styles",      QStyleFactory::keys().join(", "));
    sl << fn("Icon theme",            QIcon::themeName());
    sl << fn("Font",                  QGuiApplication::font().toString());
    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    QLocale loc;
    sl << fn("Language",              metaEnum.valueToKey(loc.language()));
    sl << fn("Locale",                loc.name());

    // APP
    sl << fn("Binary location",       QApplication::applicationFilePath());
    sl << fn("Working dir",           QDir::currentPath());
    sl << fn("Arguments",             QApplication::arguments().join(" "));

    // ENVIRONMENT
    sl << "ENVIRONMENT:";
    auto env = QProcessEnvironment::systemEnvironment();
    for (const auto &key : env.keys())
        sl << fn(key, env.value(key));

    return sl;
}

