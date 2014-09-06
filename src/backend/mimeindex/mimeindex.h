#ifndef MIMEINDEX_H
#define MIMEINDEX_H

#include "abstractindexprovider.h"

class MimeIndex : public AbstractIndexProvider
{
public:
	class MimeIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		MimeIndexItem() = delete;
		MimeIndexItem(QString title, QString uri)
			: AbstractIndexItem(title, uri) {}
		~MimeIndexItem(){}

		inline  QString  title() const { return _title; }
		QString  iconPath() override;
		QString  complete() override;
		void     action(Action) override;
		QString  actionText(Action) override;
	};


	MimeIndex(){}
	~MimeIndex();

	void buildIndex() override;
	QWidget* configWidget() override;

};

#endif // MIMEINDEX_H
