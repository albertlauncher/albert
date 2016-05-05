#ifndef MPRIS_COMMAND_H
#define MPRIS_COMMAND_H

#include <QString>
#include <QVariant>
#include <functional>
#include "query.h"
#include "objects.hpp"
#include "player.h"
using std::function;

namespace MPRIS {

class Command
{
public:
    /**
     * @brief Command   Constructs a DBus command to launch from albert with the given parameters.
     * @param label     An internal variable to query at a later point. Not needed for the DBus query.
     * @param title     The subtext of the StandardItem which will be created.
     * @param method    The DBus method to invoke when this command is performed.
     * @param iconpath  The path to the icon which the StandardItem will get.
     */
    Command(QString& label, QString& title, QString& method, QString& iconpath);
    Command(const char* label, const char* title, const char* method, QString iconpath);

    QString& getLabel();
    QString& getTitle();
    QString& getMethod();
    QString& getIconPath();

    /**
     * @brief applicableWhen    Configure this command to be only appicable under a certian (given) conditions.
     * @param path              The path to query the property from.
     * @param property          The name of the property. This property will be checked as condition.
     * @param expectedValue     The value of the property.
     * @param positivity        The result of the equality-check (queriedValue == expectedValue). Here you can negate the result.
     * @return                  Returns itself, but now configured for applicability-check
     */
    Command& applicableWhen(const char *path, const char* property, QVariant expectedValue, bool positivity);

    /**
     * @brief closeWhenHit  Configure this command to close the albert widget when activated.
     * @return              Returns itseft, but now configured to close albert.
     */
    Command& closeWhenHit();

    //Command& fireCallback(function<void()>);

    /**
     * @brief closesWhenHit Queries if the application should hide if the item was activated.
     * @return              True if so, false otherwise.
     */
    bool closesWhenHit();

    /**
     * @brief produceStandardItem   Produces an instance of AlbertItem for this command to invoke on a given Player.
     * @return                      Returns a shared_ptr on this AlbertItem.
     */
    std::shared_ptr<AlbertItem> produceAlbertItem(Player &);

    /**
     * @brief isApplicable  If configured, checks if the given property meets the expected criteria.
     * @return              True if not configured or match, false if the property is different than expected.
     */
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
