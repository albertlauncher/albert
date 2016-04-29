#ifndef MPRIS_PLAYER_H
#define MPRIS_PLAYER_H

#include <QString>

namespace MPRIS {

class Player
{
public:
    Player(QString& busid);
    QString& getName();
    QString& getBusId();

private:
    QString busid_, name_;
};

} // namespace MPRIS

#endif // MPRIS_PLAYER_H
