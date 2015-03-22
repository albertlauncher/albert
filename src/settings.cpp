#include "settings.h"


/****************************************************************************///
Settings *Settings::_instance = nullptr;
/****************************************************************************///
Settings *Settings::instance() {
	if( _instance == nullptr ) _instance = new Settings();
	return _instance;
}
