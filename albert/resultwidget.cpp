#include "resultwidget.h"

ResultWidget::ResultWidget(QWidget *p) : QFrame(p)
{
	_horizontalLayout = new QHBoxLayout(p);
	_horizontalLayout->setSpacing(0);
	_horizontalLayout->setContentsMargins(0, 0, 0, 0);

	_icon = new QLabel(p);
	_icon->setObjectName(QStringLiteral("icon"));
	_icon->setFixedHeight(48);
	_icon->setFixedWidth(48);

	_horizontalLayout->addWidget(_icon);

	_verticalLayout = new QVBoxLayout(p);

	_title = new QLabel(p);
	_title->setObjectName(QStringLiteral("title"));

	_auxInfo = new QLabel(p);
	_auxInfo->setObjectName(QStringLiteral("auxInfo"));

	_verticalLayout->addWidget(_title);
	_verticalLayout->addWidget(_auxInfo);

	_horizontalLayout->addLayout(_verticalLayout);
	this->setLayout(_horizontalLayout);

	setObjectName(QStringLiteral("resultWidget"));
}

ResultWidget::~ResultWidget()
{
	delete _auxInfo;
	delete _title;
	delete _verticalLayout;
	delete _icon;
	delete _horizontalLayout;
}
