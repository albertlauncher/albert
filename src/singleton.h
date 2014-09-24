#ifndef SINGLETON_H
#define SINGLETON_H

template <typename C>
class Singleton
{
public:
	static C* instance () {
		if (!_instance)
			_instance = new C ();
		return _instance;
	}
	virtual ~Singleton () {
		_instance = 0;
	}
private:
	static C* _instance;
protected:
	Singleton () { }
};
template <typename C> C* Singleton <C>::_instance = nullptr;

#endif // SINGLETON_H
