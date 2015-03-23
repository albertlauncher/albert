#ifndef EXTENSIONINTERFACE_H
#define EXTENSIONINTERFACE_H
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extensioninterface"

#include <QtPlugin>
#include <QString>
#include <QIcon>

class Query;
class QueryResult;

class ExtensionInterface
{
public:
    ExtensionInterface() {}
    virtual ~ExtensionInterface() {}

    virtual QString  name() const = 0;
    virtual QString  abstract() const = 0;

    virtual void     initialize() = 0;
    virtual void     finalize() = 0;
    virtual void     setupSession() = 0;
    virtual void     teardownSession() = 0;
    virtual void     handleQuery(Query *q) = 0;

    virtual const QIcon  &icon     (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) = 0;
    virtual void         action    (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) = 0;
    virtual QString      titleText (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;
    virtual QString      infoText  (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;
    virtual QString      actionText(const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;

    virtual QWidget* widget() = 0;
};
Q_DECLARE_INTERFACE(ExtensionInterface, ALBERT_EXTENSION_IID)
#endif // EXTENSIONINTERFACE_H
