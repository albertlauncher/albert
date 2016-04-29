#include "player.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

namespace MPRIS {

Player::Player(QString &busid) : busid_(busid), name_(busid)
{
    QDBusInterface iface(busid, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
    QVariant prop = iface.property("Identity");
    QString name = prop.toString();
    if (!name.isNull() && !name.isEmpty()) {
        name_ = name;
    } else {
        qWarning("DBus: Name is either empty or null");
    }
}

QString& Player::getBusId() {
    return busid_;
}

QString& Player::getName() {
    return name_;
}

} // namespace MPRIS
