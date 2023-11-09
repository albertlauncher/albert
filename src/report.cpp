// Copyright (c) 2023 Manuel Schneider

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QStyle>
#include <QFont>
#include <QMetaEnum>
#include <QStyleFactory>
#include <QStringBuilder>
#include <iostream>

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

    // ENV
    sl << fn("$LANG",                 QString::fromLocal8Bit(qgetenv("LANG")));
    sl << fn("$QT_QPA_PLATFORMTHEME", QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME")));
    sl << fn("$PATH",                 QString::fromLocal8Bit(qgetenv("PATH")));
    sl << fn("$SHELL",                QString::fromLocal8Bit(qgetenv("SHELL")));

    // LINUX ENV
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    sl << fn("$XDG_SESSION_TYPE",     QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")));
    sl << fn("$XDG_CURRENT_DESKTOP",  QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")));
    sl << fn("$DESKTOP_SESSION",      QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION")));
    sl << fn("$XDG_SESSION_DESKTOP",  QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP")));
#endif

    return sl;
}

void printReportAndExit()
{
    for (const auto &line : report())
        std::cout << line.toStdString() << std::endl;
    ::exit(EXIT_SUCCESS);
}
