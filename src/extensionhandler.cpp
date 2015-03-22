#include "extensionhandler.h"
#include <QDirIterator>
#include <QDebug>
#include <QJsonArray>
#include "settings.h"

/****************************************************************************///
void ExtensionHandler::loadExtensions()
{
	QStringList blacklist = gSettings->value("blacklist").toStringList();
	QDirIterator plgnIt("/usr/share/albert/plugins/", QDir::Files);
	while (plgnIt.hasNext())
	{
		QString path = plgnIt.next();

		// Check if this is a lib
		if (!QLibrary::isLibrary(path)) {
			qDebug() << "Not a library:" << path;
			continue;
		}

		// Check if this lib is an albert extension plugin
		QPluginLoader loader(path);
		QString name = loader.metaData()["MetaData"].toObject()["name"].toString();

		if (loader.metaData()["IID"].toString().compare(ALBERT_EXTENSION_IID) != 0) {
			qDebug() << "Extension incompatible:" << path
					 << loader.metaData()["IID"].toString()
					 << ALBERT_EXTENSION_IID;
			continue;
		}

		// METADATA ACCESS
//		// Populate Extension
//		Extension *ret = new Extension;
//		ret->_info.path        = path;
//		ret->_info.name        = metaData["name"].toString();
//		ret->_info.version     = metaData["version"].toString();
//		ret->_info.description = metaData["description"].toString();
//		ret->_info.url         = metaData["url"].toString();
//		ret->_info.copyright   = metaData["copyright"].toString();
//		for(const auto &v : metaData["dependencies"].toArray())
//			ret->_info.deps << v.toString();
//		ret->_loader.setFileName(path);
//		ret->_interface = nullptr;
//		return ret;

		// Check if this extension is blacklisted
		if (blacklist.contains(name)){
			qWarning() << "WARNING: Extension blacklisted:" << path;
			continue;
		}

		QObject *rootComponent = loader.instance();
		if (rootComponent == nullptr){
			qWarning() << "WARNING: Loading extension failed:" << path << loader.errorString();
			if (loader.isLoaded())
				loader.unload();
			continue;
		}

		ExtensionInterface *extension = qobject_cast<ExtensionInterface*>(rootComponent);
		if (!extension) {
			qWarning() << "Interface cast failed:" << path << loader.errorString();
			if (loader.isLoaded())
				loader.unload();
			continue;
		}

		// Store the extensions
		_extensions.insert(name, extension);
		qDebug() << "Extension loaded:" <<  path;
	}
}

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
	qDebug() << "Initialize extenstions.";
	loadExtensions();
	for (ExtensionInterface *e : _extensions)
		e->initialize();
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

