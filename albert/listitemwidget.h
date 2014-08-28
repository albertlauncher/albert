#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class ListItemWidget : public QWidget
{

	QHBoxLayout *_horizontalLayout;
	QLabel      *_icon;
	QVBoxLayout *_verticalLayout;
	QLabel      *_title;
	QLabel      *_auxInfo;

public:
	ListItemWidget(QWidget *p = 0) : QWidget(p)
	{
		_horizontalLayout = new QHBoxLayout(p);
		_horizontalLayout->setSpacing(0);
		_horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		_horizontalLayout->setContentsMargins(0, 0, 0, 0);

		_icon = new QLabel(p);
		_icon->setObjectName(QStringLiteral("icon"));
		_icon->setFixedHeight(48);
		_icon->setFixedWidth(48);

		_horizontalLayout->addWidget(_icon);

		_verticalLayout = new QVBoxLayout(p);
		_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

		_title = new QLabel(p);
		_title->setObjectName(QStringLiteral("title"));

		_auxInfo = new QLabel(p);
		_auxInfo->setObjectName(QStringLiteral("auxInfo"));

		_verticalLayout->addWidget(_title);
		_verticalLayout->addWidget(_auxInfo);

		_horizontalLayout->addLayout(_verticalLayout);

		this->setObjectName(QStringLiteral("listItemWidget"));
		this->setLayout(_horizontalLayout);
	}

	~ListItemWidget()
	{
		delete _auxInfo;
		delete _title;
		delete _verticalLayout;
		delete _icon;
		delete _horizontalLayout;
	}

	inline QString title() const { return _title->text(); }
	inline QString auxInfo() const { return _auxInfo->text(); }
	inline void setTitle(const QString& t) { _title->setText(t); }
	inline void setAuxInfo(const QString& a) { _auxInfo->setText(a); }
};

#endif // LISTITEMWIDGET_H
