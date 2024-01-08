// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "hotkey.h"
#include <QCoreApplication>
#include <QHotkey>
#include <QMessageBox>
#include <QSettings>
static const char *CFG_NOTIFY_SUPPORT = "notifiedUnsupportedHotkey";
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";

Hotkey::Hotkey()
{
    auto s = albert::settings();
    if (isPlatformSupported())
        setHotkey(QKeySequence::fromString(s->value(CFG_HOTKEY, DEF_HOTKEY).toString())[0]);
    else {
        if (!s->value(CFG_NOTIFY_SUPPORT, false).toBool()){
            QMessageBox::warning(nullptr, tr("Hotkeys not supported"),
                                 tr("Hotkeys are not supported on this platform. Use your desktop "
                                    "environment to bind a hotkey to 'albert toggle'."));
            s->setValue(CFG_NOTIFY_SUPPORT, true);
        }
    }
}

Hotkey::~Hotkey() = default;

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

        albert::settings()->setValue(CFG_HOTKEY, ks.toString());

        QObject::connect(hotkey_.get(), &QHotkey::activated, this, &Hotkey::activated);

        INFO << "Hotkey set to" << ks.toString();
        return true;
    } else {
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to set hotkey '%1'").arg(ks.toString()));
        WARN << "Failed to set hotkey " << ks.toString();
        return false;
    }
}

bool Hotkey::isPlatformSupported()
{ return QHotkey::isPlatformSupported(); }
