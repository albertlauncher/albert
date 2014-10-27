// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "albert.h"
#include "albertengine.h"
#include "settingsdialog.h"
#include "globalhotkey.h"
#include <QEvent>
#include <QSettings>
#include <QLabel>
#include <QFile>
#include <QStandardPaths>

/**************************************************************************/
AlbertWidget::AlbertWidget(QWidget *parent)
	: QWidget(parent)
{
	/* Stuff concerning the UI and windowing */


	// Window properties
	setObjectName(QString::fromLocal8Bit("albert"));
	setWindowTitle(QString::fromLocal8Bit("Albert"));
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool
					);

	QVBoxLayout *l2 = new QVBoxLayout;
	l2->setMargin(0);
	l2->setSizeConstraint(QLayout::SetFixedSize);
	this->setLayout(l2);

	// Layer 2
	_frame2 = new QFrame;
	_frame2->setObjectName(QString::fromLocal8Bit("bottomframe"));
	_frame2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	l2->addWidget(_frame2,0,0);

	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	_frame2->setLayout(l1);

	// Layer 1
	_frame1 = new QFrame;
	_frame1->setObjectName(QString::fromLocal8Bit("topframe"));
	_frame1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	l1->addWidget(_frame1,0,0);

	QVBoxLayout *contentLayout = new QVBoxLayout();
	contentLayout->setMargin(0);
	_frame1->setLayout(contentLayout);

	// ContentLayer
	_inputLine = new InputLine;
	contentLayout->addWidget(_inputLine);

	_proposalListView = new ProposalListView;
	_proposalListView->setFocusPolicy(Qt::NoFocus);
	_proposalListView->setFocusProxy(_inputLine);
	_proposalListView->hide();
    contentLayout->addWidget(_proposalListView);



	/* MISC */

	// Listview intercepts inputline (Navigation with keys, pressed modifiers, etc)
	_inputLine->installEventFilter(_proposalListView);

	// A change in text triggers requests
	connect(_inputLine, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited(QString)));

	// Inputline listens if proposallistview tells it to change text (completion)
	connect(_proposalListView, SIGNAL(completion(QString)), _inputLine, SLOT(setText(QString)));

	// Start the engine :D
	_engine = new AlbertEngine;
	_proposalListView->setModel(_engine);

	deserialize();

	// React to hotkeys
	connect(GlobalHotkey::instance(), SIGNAL(hotKeyPressed()), this, SLOT(toggleVisibility()));

	GlobalHotkey::instance()->setHotkey({Qt::AltModifier, Qt::Key_Space});

	_t.start();
    this->show();
}

/**************************************************************************/
AlbertWidget::~AlbertWidget()
{
	serialize();
}

/**************************************************************************/
void AlbertWidget::serialize() const
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
	QFile f(path);
	if (!f.open(QIODevice::ReadWrite| QIODevice::Text)){
		qWarning() << "[Albert]\tCould not open file" << path;
	}

	qDebug() << "[Albert]\tSerializing to " << path;
	QDataStream out( &f );
	out << _skinName;
	_engine->serialize(out);
	f.close();
}

/**************************************************************************/
void AlbertWidget::deserialize()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
	QFile f(path);
	if (f.open(QIODevice::ReadOnly| QIODevice::Text))
	{
		qDebug() << "[Albert]\t\tDeserializing from" << path;
		QDataStream in( &f );
		in >> _skinName;
		_engine->deserialize(in);
		f.close();

		QFile styleFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/albert/skins/"+_skinName+".qss");
		if (styleFile.open(QFile::ReadOnly)) {
			qApp->setStyleSheet(styleFile.readAll());
			styleFile.close();
		}
		else
		{
			qWarning() << "[Albert]\t\tCould not open style file" << _skinName;
			qWarning() << "[Albert]\t\tFallback to basicskin";
			qApp->setStyleSheet(QString::fromLocal8Bit("file:///:/resources/basicskin.qss"));
		}
	}
	else
	{
		qWarning() << "[Albert]\t\tCould not open file" << path;
		_engine->initialize();
		qApp->setStyleSheet(QString::fromLocal8Bit("file:///:/resources/basicskin.qss"));
	}

}

/*****************************************************************************/
/********************************* S L O T S *********************************/
/**************************************************************************/
void AlbertWidget::show()
{
	_engine->clear();
	_proposalListView->hide();
	QWidget::show();
	_inputLine->clear();
	updateGeometry();
	if (QSettings().value(QString::fromLocal8Bit("show_centered"), QString::fromLocal8Bit("true")).toBool())
		this->move(QApplication::desktop()->screenGeometry().center() - QPoint(rect().right()/2,192 ));
	this->raise();
	this->activateWindow();
	_inputLine->setFocus();
}

/**************************************************************************/
void AlbertWidget::toggleVisibility()
{
	// Delay the hiding, since this may have happened because HK was pressed
	// and X11 sent a shitty focus while grab, which could not be catched in a
	// native way (nativeEvent), since qt does bullshit on focuses after that.
	// I hate workarounds...
	qDebug("Time elapsed: %d ms", _t.elapsed());
	if (_t.elapsed() > 50)
	{
		qDebug("Toggled");
		_t.restart();
		this->isVisible() ? this->hide() : this->show();
	}
}

/**************************************************************************/
void AlbertWidget::onTextEdited(const QString & text)
{
	QString t = text.trimmed();
	if (!t.isEmpty()){
		_engine->query(t);
		if (_engine->rowCount() > 0){
			if (!_proposalListView->currentIndex().isValid())
				_proposalListView->setCurrentIndex(_engine->index(0, 0));
		}
		_proposalListView->show();
		return;
	}
	_engine->clear();
	_proposalListView->hide();
}
