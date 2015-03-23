#include "extensionhandler.h"
#include <QDirIterator>
#include <QDebug>
#include <QJsonArray>
#include "settings.h"
#include "pluginhandler.h"

/****************************************************************************///
void ExtensionHandler::startQuery(const QString &term)
{
	_lastSearchTerm = term.trimmed();
	qDebug() << "Query started" << _lastSearchTerm;

//	if (_lastSearchTerm.isEmpty()) return; // Todo: How to clear proposallist

	Query *q;
	if (!_recentQueries.contains(_lastSearchTerm)) {
		q = new Query(_lastSearchTerm);
		_recentQueries.insert(_lastSearchTerm, q);
		// TODO INTRODUCE MULTITHREADING HERE
		for (ExtensionInterface *e : _extensions) {
			e->handleQuery(q);
		}
	} else {
		q = _recentQueries.value(_lastSearchTerm);
	}
	emit currentQueryChanged(q);
}

/****************************************************************************///
void ExtensionHandler::initialize()
{
    QList<ExtensionInterface*> extensions =
            PluginHandler::instance()->getLoadedPlugins<ExtensionInterface*>();

	qDebug() << "Initialize extenstions.";
    for (ExtensionInterface *e : extensions){
        if (_extensions.contains(e))
            continue;
        _extensions.insert(e);
        e->initialize();
    }
}

/****************************************************************************///
void ExtensionHandler::finalize()
{
	qDebug() << "Finalize extenstions.";
	for (ExtensionInterface *e : _extensions)
		e->finalize();
}

/****************************************************************************///
void ExtensionHandler::setupSession(){
	for (ExtensionInterface *e : _extensions)
		e->setupSession();
	qDebug() << "Session set up.";
}

/****************************************************************************///
void ExtensionHandler::teardownSession(){
	_recentQueries.clear();
	for (Query *q : _recentQueries)
		delete q;
	for (ExtensionInterface *e : _extensions)
		e->teardownSession();
	qDebug() << "Session teared down.";
}




//		// DEBUG
//		qDebug() <<pluginInfo.path;
//		qDebug() <<pi.name;
//		qDebug() <<pi.deps;
//		qDebug() <<pi.authors;
//		qDebug() << "keys" << pluginLib.metaData().keys();
//		qDebug() << "IID" << pluginLib.metaData().value("IID").toString();
//		qDebug() << "className" << pluginLib.metaData().value("className").toString();
//		qDebug() << "debug" << pluginLib.metaData().value("debug").toString();
//		qDebug() << "version" << pluginLib.metaData().value("version").toString();
//		qDebug() << "version" << meta.value("version").toString();
//		qDebug() << "authors" ;
//		for (const QJsonValue &v : meta.value("authors").toArray())
//			qDebug() << v.toString("error");
//		qDebug() << "dependencies" ;
//		for (const QJsonValue &v : meta.value("dependencies").toArray())
//			qDebug() << v.toString("error");

