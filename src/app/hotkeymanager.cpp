// Copyright (c) 2026-2026 Manuel Schneider


#include "app.h"
#include "hotkey.h"
#include "hotkeymanager.h"
#include "logging.h"
#include "messagebox.h"
#include <QKeySequence>
#include <QSettings>
using namespace albert;
using namespace std;

namespace {
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";
}

HotkeyManager::HotkeyManager(const QSettings &settings)
{
    if (Hotkey::isPlatformSupported())
    {
        if (auto s_hk = settings.value(CFG_HOTKEY, DEF_HOTKEY).toString();
            s_hk.isEmpty())
        {
            DEBG << "Hotkey explicitly unset.";
            return;
        }
        else if (auto hk = Hotkey::grab(s_hk))
        {
            hotkey_ = ::move(hk.value());
            connect(hotkey_.get(), &Hotkey::activated, this, &HotkeyManager::activated);
            connect(hotkey_.get(), &Hotkey::revoked,
                    this, &HotkeyManager::onRevoked, Qt::QueuedConnection);
            INFO << "Hotkey set to" << s_hk;
        }
        else
        {
            auto *t = QT_TR_NOOP("Failed to set the hotkey '%1'");
            WARN << QString::fromUtf8(t).arg(s_hk);
            warning(tr(t).arg(QKeySequence(s_hk).toString(QKeySequence::NativeText)));
            app().showSettings();
        }
    }
    else
        INFO << "Hotkeys are not supported on this platform.";
}

HotkeyManager::~HotkeyManager() {}

Hotkey *HotkeyManager::hotkey() const { return hotkey_.get(); }

void HotkeyManager::setHotkey(unique_ptr<Hotkey> hotkey)
{
    if (hotkey_ = ::move(hotkey);
        hotkey_)
    {
        app().settings()->setValue(CFG_HOTKEY, hotkey_->portableString());
        connect(hotkey_.get(), &Hotkey::activated, this, &HotkeyManager::activated);
        connect(hotkey_.get(), &Hotkey::revoked,
                this, &HotkeyManager::onRevoked, Qt::QueuedConnection);
        INFO << "Hotkey set to" << hotkey_->nativeString();
    }
    else
    {
        app().settings()->setValue(CFG_HOTKEY, QString());
        INFO << "Hotkey disabled";
    }
}

void HotkeyManager::onRevoked(const QString &message)
{
    if (!hotkey_)
        return;

    const auto native_string = hotkey_->nativeString();
    hotkey_.reset();

    auto *t = QT_TR_NOOP("The hotkey '%1' has been revoked by the system.");
    WARN << QString::fromUtf8(t).arg(native_string) << message;
    warning(message.isEmpty()
                ? tr(t).arg(native_string)
                : QString("%1\n\n%2").arg(tr(t).arg(native_string), message));
    app().showSettings();
}
