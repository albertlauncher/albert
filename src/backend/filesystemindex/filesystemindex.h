#ifndef FILESYSTEMINDEX_H
#define FILESYSTEMINDEX_H

#include "abstractindexprovider.h"
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
		FileIndexItem(QFileInfo fi) : AbstractIndexItem(fi.fileName()), _fi(fi) {}
		~FileIndexItem(){}

		inline  QString iconName() const override { return QMimeDatabase().mimeTypeForFile(_fi.canonicalFilePath()).iconName(); }
		inline  QString complete() const override { return _fi.fileName(); }
		inline  QString infoText() const override { return _fi.canonicalFilePath(); }
		inline  QString uri() const override { return _fi.canonicalFilePath(); }
		void    action(Action) override;
		QString actionText(Action)  const override ;
	protected:
		QFileInfo _fi;
	};

	class DirIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		DirIndexItem() = delete;
		DirIndexItem(QDir dir) : AbstractIndexItem(dir.dirName()), _dir(dir) {}
		~DirIndexItem(){}

		inline  QString iconName() const override { return QMimeDatabase().mimeTypeForFile(_dir.canonicalPath()).iconName(); }
		inline  QString complete() const override { return _dir.dirName(); }
		inline  QString infoText() const override { return _dir.canonicalPath(); }
		inline  QString uri() const override { return _dir.canonicalPath(); }
		void    action(Action) override;
		QString actionText(Action)  const override ;
	protected:
		QDir _dir;
	};


	FileSystemIndex() {}
	~FileSystemIndex() {}

	void buildIndex() override;
	QWidget* configWidget() override;

};

#endif // FILESYSTEMINDEX_H
