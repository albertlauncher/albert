#ifndef EXTENSIONINTERFACE_H
#define EXTENSIONINTERFACE_H

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

	virtual QString      text(const QueryResult&) const = 0;
	virtual QString      subtext(const QueryResult&) const = 0;
	virtual const QIcon  &icon(const QueryResult&) = 0;
	virtual void         action(const Query&, const QueryResult&) = 0;

	virtual QWidget* widget() = 0;

//	virtual void queryFallback(const QString&, QVector<Item*>*) const = 0;
};

#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extensioninterface"
Q_DECLARE_INTERFACE(ExtensionInterface, ALBERT_EXTENSION_IID)

#endif // EXTENSIONINTERFACE_H
