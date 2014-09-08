#ifndef FILESYSTEMINDEX_H
#define FILESYSTEMINDEX_H

#include "abstractindexprovider.h"
#include "boost/filesystem.hpp"
#include <string>


#include <QFileInfo>
#include <QDir>
#include <QMimeType>
#include <QMimeDatabase>

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
		void               action(Action) override;
		std::string        actionText(Action) const override;
		std::string        mimeType() const override;

	protected:
		boost::filesystem::path _path;
	};


	FileSystemIndex() {}
	~FileSystemIndex() {}

	void buildIndex() override;
};

#endif // FILESYSTEMINDEX_H
