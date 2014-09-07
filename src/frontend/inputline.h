#ifndef INPUTLINE_H
#define INPUTLINE_H

#include <QLineEdit>
#include <QString>

class InputLine : public QLineEdit
{
	Q_OBJECT
public:
	explicit InputLine(QWidget *parent = 0);

signals:

public slots:
	void onCompletion(QString);

};

#endif // INPUTLINE_H
