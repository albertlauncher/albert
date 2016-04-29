#ifndef MPRIS_COMMAND_H
#define MPRIS_COMMAND_H

#include <QString>
#include <functional>
#include "item.h"
#include "query.h"
#include "objects.hpp"
#include "player.h"
using std::function;

namespace MPRIS {

class Command
{
public:
    Command(QString& label, QString& title, QString& method, QString& iconpath);
    Command(const char* label, const char* title, const char* method, QString iconpath);
    QString& getLabel();
    QString& getTitle();
    QString& getMethod();
    QString& getIconPath();

    Command& applicableWhen(const char *path, const char* property, QVariant expectedValue, bool positivity);
    Command& closeWhenHit();
    //Command& fireCallback(function<void()>);
    bool closesWhenHit();

    std::shared_ptr<StandardItem> produceStandardItem(Player&);
    bool isApplicable(Player&);

private:
    QString label_, title_, method_, iconpath_;
    bool applicableCheck_;
    QString path_;
    QString property_;
    QVariant expectedValue_;
    bool positivity_;
    bool closeOnEnter_;
    //function<void()> fireCallback_;
};

} // namespace MPRIS

#endif // MPRIS_COMMAND_H
