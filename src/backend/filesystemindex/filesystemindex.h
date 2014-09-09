#ifndef FILESYSTEMINDEX_H
#define FILESYSTEMINDEX_H

#include "abstractindexprovider.h"
#include "boost/filesystem.hpp"
#include <string>
#include <magic.h>

#ifdef FRONTEND_QT
#include <QMimeDatabase>
#endif

class FileSystemIndex : public AbstractIndexProvider
{
public:
	class FileIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		FileIndexItem() = delete;
		FileIndexItem(boost::filesystem::path p) : AbstractIndexItem(p.filename().string()), _path(p) {}
		~FileIndexItem(){}

		inline std::string complete() const override {return _path.filename().string();}
		inline std::string infoText() const override {return _path.string();}
		inline std::string uri() const override {return _path.string();}
		std::chrono::system_clock::time_point lastAccess() const override {return _lastAccess;}
		void               action(Action) override;
		std::string        actionText(Action) const override;
		std::string        iconName() const override;

	protected:
		boost::filesystem::path _path;
		std::chrono::system_clock::time_point _lastAccess;
	};


	static FileSystemIndex* instance();

private:
	FileSystemIndex();
	~FileSystemIndex();

	void buildIndex() override;

	static FileSystemIndex *_instance;
//	magic_t _magic_cookie;

#ifdef FRONTEND_QT
	QMimeDatabase mimeDb;
#endif
};
#endif // FILESYSTEMINDEX_H
