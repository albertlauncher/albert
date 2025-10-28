// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "logging.h"
#include "messagebox.h"
#include "query.h"
#include "queryengine.h"
#include "queryresults.h"
using namespace albert;
using namespace std;

QueryResults::QueryResults(const Query &q) : query(q){}

QueryResults::~QueryResults()
{
    for (auto &result_item : results)
        result_item.item->removeObserver(this);
}

bool QueryResults::activate(uint item_idx, uint action_idx) const
{
    try {
        auto &[e, i] = results.at(item_idx);

        try {
            auto a = i->actions().at(action_idx);

            INFO << QString("Activating action %1 > %2 > %3 (%4 > %5 > %6) ")
                        .arg(e->id(), i->id(), a.id, e->name(), i->text(), a.text);

            // Order is cumbersome here

            App::instance()->queryEngine().storeItemActivation(query, e->id(), i->id(), a.id);

            // May delete the query, due to hide()
            // Note to myself:
            // - QTimer::singleShot(0, this, [a]{ a.function(); });
            //   Disconnects on query deletion.

            try {
                a.function();  // May delete the query, due to hide()
            } catch (const exception &exc) {
                const auto msg = QT_TR_NOOP("Exception in action");
                const auto fmt = QString("%1:\n\n%2 → %3 → %4\n\n%5");
                CRIT << fmt.arg(msg, e->id(), i->id(), a.id, exc.what());
                critical(fmt.arg(tr(msg), e->name(), i->text(), a.text, exc.what()));
            } catch (...) {
                const auto msg = QT_TR_NOOP("Unknown exception in action");
                const auto fmt = QString("%1:\n\n%2 → %3 → %4");
                CRIT << fmt.arg(msg, e->id(), i->id(), a.id);
                critical(fmt.arg(tr(msg), e->name(), i->text(), a.text));
            }
            return a.hide_on_activation;
        }
        catch (const out_of_range&) {
            WARN << "Activated action index is invalid:" << action_idx;
        }
    }
    catch (const out_of_range&) {
        WARN << "Activated item index is invalid:" << item_idx;
    }
    return false;
}

void QueryResults::notify(const Item *item)
{
    // O(n) but since the results are populated lazy this should be okay.
    if (auto it = find_if(results.begin(), results.end(),
                          [=](const auto &ri){ return ri.item.get() == item; });
        it != results.end())
        emit resultChanged(distance(results.begin(), it));
}
