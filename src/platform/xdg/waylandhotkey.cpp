// Copyright (c) 2026-2026 Aurelien Brabant

#include "qwayland-vicinae-hotkey-v1.h"
#include "waylandhotkey.h"
#include <QEventLoop>
#include <QGuiApplication>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>
#include <QWaylandClientExtensionTemplate>
#include <xkbcommon/xkbcommon.h>
using namespace Qt::StringLiterals;
using namespace std;

namespace {

static const int bind_reply_timeout_ms = 10000;

class Manager : public QWaylandClientExtensionTemplate<Manager>,
                public QtWayland::vicinae_hotkey_manager_v1
{
public:
    Manager() : QWaylandClientExtensionTemplate(1) { initialize(); }
    ~Manager() { if (isInitialized()) destroy(); }
};

static Manager *manager()
{
    static QPointer<Manager> manager = []() -> Manager * {
        if (!qGuiApp || QGuiApplication::platformName() != u"wayland"_s)
            return nullptr;
        auto *m = new Manager;
        m->setParent(qGuiApp);
        return m;
    }();
    return manager;
}

static uint32_t toNativeModifiers(Qt::KeyboardModifiers modifiers)
{
    using M = QtWayland::vicinae_hotkey_manager_v1;
    uint32_t native_modifiers = 0;
    if (modifiers & Qt::ShiftModifier)
        native_modifiers |= M::modifiers_shift;
    if (modifiers & Qt::ControlModifier)
        native_modifiers |= M::modifiers_ctrl;
    if (modifiers & Qt::AltModifier)
        native_modifiers |= M::modifiers_alt;
    if (modifiers & Qt::MetaModifier)
        native_modifiers |= M::modifiers_super;
    return native_modifiers;
}

static xkb_keysym_t toKeysym(Qt::Key key)
{
    switch (key)
    {
    case Qt::Key_Escape: return XKB_KEY_Escape;
    case Qt::Key_PageUp: return XKB_KEY_Prior;
    case Qt::Key_PageDown: return XKB_KEY_Next;
    case Qt::Key_Insert: return XKB_KEY_Insert;
    case Qt::Key_Delete: return XKB_KEY_Delete;
    case Qt::Key_Print: return XKB_KEY_Print;
    case Qt::Key_MediaLast:
    case Qt::Key_MediaPrevious: return XKB_KEY_XF86AudioPrev;
    case Qt::Key_MediaNext: return XKB_KEY_XF86AudioNext;
    case Qt::Key_MediaPause:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaTogglePlayPause: return XKB_KEY_XF86AudioPlay;
    case Qt::Key_MediaRecord: return XKB_KEY_XF86AudioRecord;
    case Qt::Key_MediaStop: return XKB_KEY_XF86AudioStop;
    default: break;
    }

    const auto string = QKeySequence(key).toString(QKeySequence::PortableText);
    if (string.isEmpty())
        return XKB_KEY_NoSymbol;
    else if (string.size() == 1)
        return xkb_utf32_to_keysym(string.front().toLower().unicode());
    else
        return xkb_keysym_from_name(string.toUtf8().constData(), XKB_KEYSYM_CASE_INSENSITIVE);
}

}

class WaylandHotkey::Private : public QtWayland::vicinae_hotkey_v1
{
public:
    enum class State { Pending, Bound, Denied, Revoked };

    WaylandHotkey &q;
    State state = State::Pending;
    QString denial_message;
    function<void()> on_settled;

    Private(WaylandHotkey &hotkey, struct ::vicinae_hotkey_v1 *object) :
        QtWayland::vicinae_hotkey_v1(object),
        q(hotkey)
    {}

    ~Private() { destroy(); }

protected:

    void vicinae_hotkey_v1_bound() override
    {
        state = State::Bound;
        if (on_settled)
            on_settled();
    }

    void vicinae_hotkey_v1_denied(uint32_t reason, const QString &message) override
    {
        state = State::Denied;
        if (!message.isEmpty())
            denial_message = message;
        else if (reason == deny_reason_already_bound)
            denial_message = u"Already bound by another application."_s;
        else if (reason == deny_reason_invalid)
            denial_message = u"Invalid key combination."_s;
        else
            denial_message = u"Not permitted by compositor policy."_s;
        if (on_settled)
            on_settled();
    }

    void vicinae_hotkey_v1_revoked(uint32_t, const QString &message) override
    {
        state = State::Revoked;
        emit q.revoked(message);
    }

    void vicinae_hotkey_v1_pressed(uint32_t, uint32_t) override { emit q.activated(); }
};

WaylandHotkey::WaylandHotkey() = default;

WaylandHotkey::~WaylandHotkey() = default;

bool WaylandHotkey::isPlatformSupported()
{
    const auto *m = manager();
    return m && m->isActive();
}

expected<unique_ptr<WaylandHotkey>, QString> WaylandHotkey::grab(QKeyCombination key_combination)
{
    auto *m = manager();
    if (!m || !m->isActive())
        return unexpected(u"Compositor does not support vicinae-hotkey-v1."_s);

    const auto keysym = toKeysym(key_combination.key());
    if (keysym == XKB_KEY_NoSymbol)
        return unexpected(u"Key has no keysym representation."_s);

    unique_ptr<WaylandHotkey> hotkey(new WaylandHotkey);
    hotkey->d = make_unique<Private>(
        *hotkey,
        m->QtWayland::vicinae_hotkey_manager_v1::bind(
            keysym,
            toNativeModifiers(key_combination.keyboardModifiers()),
            nullptr,
            QGuiApplication::applicationName(),
            tr("Show/hide Albert")));

    QEventLoop loop;
    hotkey->d->on_settled = [&loop] { loop.quit(); };
    QTimer::singleShot(bind_reply_timeout_ms, &loop, &QEventLoop::quit);
    if (hotkey->d->state == Private::State::Pending)
        loop.exec();
    hotkey->d->on_settled = nullptr;

    switch (hotkey->d->state)
    {
    case Private::State::Bound:
        return hotkey;
    case Private::State::Denied:
        return unexpected(hotkey->d->denial_message);
    default:
        return unexpected(u"Compositor did not answer the bind request."_s);
    }
}
