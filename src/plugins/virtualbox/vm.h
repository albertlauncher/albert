
#pragma once

#include <QString>
#include "vmitem.h"

namespace VirtualBox {

class VM
{
public:
    VM(QString &listVmsLine);
    VMItem* produceItem();
    bool startsWith(QString other);

private:
    QString name_;
    QString uuid_;
    QString state_;
};

} // namespace VirtualBox

