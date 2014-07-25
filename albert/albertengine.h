#ifndef ALBERTENGINE_H
#define ALBERTENGINE_H

#include <QObject>
#include <vector>
#include "item.h"

class AlbertEngine : public QObject
{
	Q_OBJECT
public:
	explicit AlbertEngine(QObject *parent = 0);
	~AlbertEngine();
	void buildIndex();
	void loadIndex();
	void saveIndex();

signals:

public slots:

private:
	std::vector<Items::AbstractItem*> _index;

};

#endif // ALBERTENGINE_H
