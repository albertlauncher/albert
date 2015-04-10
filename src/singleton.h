#pragma once

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
       if (_instance == nullptr)
           delete _instance;
      _instance = nullptr;
   }
private:
   static C* _instance;
protected:
   Singleton () { }
};
template <typename C> C* Singleton <C>::_instance = nullptr;
