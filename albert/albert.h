#ifndef ALBERT_H
#define ALBERT_H


// aufräumemn
#include <QLineEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QKeyEvent>
#include <QAbstractNativeEventFilter>

//bleibtz
#include <QWidget>
#include "commandline.h"
#include "albertengine.h"

class AlbertWidget : public QWidget
{
    Q_OBJECT

public:
	AlbertWidget(QWidget *parent = 0);
    ~AlbertWidget();

private slots:
	void toggle();
	void onHotKeyPressed();

protected:
	void keyPressEvent(QKeyEvent * event) override;
	void focusOutEvent(QFocusEvent* e) override;
	bool eventFilter(QObject *obj, QEvent *event) override;
	virtual bool nativeEvent(const QByteArray &eventType, void *message, long *) override;


private:
	void hideAndClear();

	CommandLine * _commandLine;
	AlbertEngine  _engine;


};

#endif // ALBERT_H
