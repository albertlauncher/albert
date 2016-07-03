
#pragma once

#include "abstractitem.h"

namespace VirtualBox {

typedef vector<shared_ptr<AbstractAction>> ActionSPtrVec;

class VMItem : public AbstractItem
{
public:
    VMItem(QString &name, QString &uuid, int &mainAction, ActionSPtrVec actions, QString &state);


    /*
     * Implementation of AlbertItem interface
     */

    QString id() const override { return idstring_; }
    QString text() const override { return name_; }
    QString subtext() const override;
    QString iconPath() const override { return iconPath_; }
    ActionSPtrVec actions() override { return actions_; }

    /*
     * Item specific members
     */

    static QString iconPath_;
    static const int VM_START;
    static const int VM_PAUSE;
    static const int VM_RESUME;
    static const int VM_STATE_CHANGING;

private:
    QString name_;
    QString uuid_;
    QString idstring_;
    ActionSPtrVec actions_;
    int mainAction_;
};

} // namespace VirtualBox
