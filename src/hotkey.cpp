#include "hotkey.h"
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>
#include <QHotkey>
#include "albert/albert.h"
#include "albert/logging.h"
static const char *CFG_NOTIFY_SUPPORT = "notifiedUnsupportedHotkey";
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";

Hotkey::Hotkey()
{
    QSettings s(qApp->applicationName());
    if (isPlatformSupported())
        setHotkey(QKeySequence::fromString(s.value(CFG_HOTKEY, DEF_HOTKEY).toString())[0]);
    else {
        if (!s.value(CFG_NOTIFY_SUPPORT, false).toBool()){
            QMessageBox::warning(nullptr, "Hotkey not supported",
                                 "Hotkeys are not supported on this platform. Use your desktop "
                                 "environment to run bind a hotkey to 'albertctl toggle'");
            s.setValue(CFG_NOTIFY_SUPPORT, true);
        }
    }
}

QKeyCombination Hotkey::hotkey() const
{
    if (hotkey_)
        return hotkey_->shortcut()[0];
    else
        return Qt::Key_unknown;
}

bool Hotkey::setHotkey(QKeyCombination keycode)
{
    QKeySequence ks(keycode);

    if (auto hotkey = std::make_unique<QHotkey>(ks, true, qApp); hotkey->isRegistered()){
        if (hotkey_)
            hotkey_->disconnect();

        hotkey_ = std::move(hotkey);

        QSettings(qApp->applicationName()).setValue(CFG_HOTKEY, ks.toString());

        QObject::connect(hotkey_.get(), &QHotkey::activated,
                         qApp, [](){ albert::toggle(); });

        INFO << "Hotkey set to" << ks.toString();
        return true;
    } else {
        QMessageBox::warning(nullptr, "Error", QString("Failed to set hotkey '%1'").arg(ks.toString()));
        WARN << "Failed to set hotkey " << ks.toString();
        return false;
    }
}

bool Hotkey::isPlatformSupported()
{
    return QHotkey::isPlatformSupported();
}
