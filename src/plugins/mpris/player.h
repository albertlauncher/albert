
#pragma once

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

