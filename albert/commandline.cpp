#include "commandline.h"

CommandLine::CommandLine(QWidget *parent) :
	QLineEdit(parent)
{
	setObjectName("commandline");
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}
