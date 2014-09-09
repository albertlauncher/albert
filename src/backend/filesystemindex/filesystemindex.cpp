#include "filesystemindex.h"
#include "settings.h"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include <functional>
#include <sstream>

//REMOVE
#include <iostream>
#include <boost/timer/timer.hpp>

FileSystemIndex* FileSystemIndex::_instance = nullptr;


/**************************************************************************//**
 * @brief FileSystemIndex::instance
 * @return
 */
FileSystemIndex *FileSystemIndex::instance(){
	if (_instance == nullptr)
		_instance = new FileSystemIndex;
	return _instance;
}

/**************************************************************************//**
 * @brief FileSystemIndex::FileSystemIndex
 */
FileSystemIndex::FileSystemIndex()
{
//	magic_t _magic_cookie = magic_open(MAGIC_MIME);
//	if (_magic_cookie == NULL) {
//		std::cout << "Unable to initialize magic library" << std::endl;
//	}

//	printf("Loading default magic database\n");
//	if (magic_load(_magic_cookie, NULL) != 0) {
//		std::cout << "Cannot load magic database: " << magic_error(_magic_cookie) << std::endl;
//		magic_close(_magic_cookie);
//	}
}

/**************************************************************************//**
 * @brief FileSystemIndex::~FileSystemIndex
 */
FileSystemIndex::~FileSystemIndex()
{
//	magic_close(_magic_cookie);

}
/**************************************************************************//**
 * @brief FileSystemIndex::buildIndex
 */

void FileSystemIndex::buildIndex()
{
	std::string paths = Settings::instance()->get("file_index_paths");
	std::cout << paths << std::endl;

	std::vector<std::string> pathList;
	boost::split(pathList, paths, boost::is_any_of(","), boost::token_compress_on);

	// Define a lambda for recursion
	std::function<void(const boost::filesystem::path &p)> rec_dirsearch = [&] (const boost::filesystem::path &p)
	{
		boost::filesystem::path path(p);
		boost::filesystem::directory_iterator end_iterator;
		if ( boost::filesystem::exists(path))
		{
			if (boost::filesystem::is_regular_file(path))
				_index.push_back(new FileIndexItem(path));
			if (boost::filesystem::is_directory(path))
			{
				_index.push_back(new FileIndexItem(path));
				for( boost::filesystem::directory_iterator d(path); d != end_iterator; ++d)
					rec_dirsearch(*d);
			}
		}
	};

	// Finally do this recursion for all paths

	boost::timer::auto_cpu_timer *t = new boost::timer::auto_cpu_timer;
	for ( std::string &p : pathList)
		rec_dirsearch(boost::filesystem::path(p));
	delete t;

	t = new boost::timer::auto_cpu_timer;
	std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare());
	delete t;

//	for ( auto &i : _index)
//		std::cout << i->title() << std::endl;
	std::cout << "Indexing done. Found " << _index.size() << " items." << std::endl;
}

/*****************************************************************************/
/*****************************************************************************/
/******************************* FileIndexItem *******************************/
/*****************************************************************************/
/**************************************************************************//**
 * @brief FileSystemIndex::FileIndexItem::action
 * @param a
 */
void FileSystemIndex::FileIndexItem::action(Action a)
{
	_lastAccess = std::chrono::system_clock::now();

	if (a == Action::Enter)
		return startDetached("xdg-open", _path.string());

	if (a == Action::Ctrl)
		return startDetached("xdg-open", _path.parent_path().string());

	// else Action::Alt
	fallbackAction(a);
}

/**************************************************************************//**
 * @brief FileSystemIndex::FileIndexItem::actionText
 * @param a
 * @return
 */
std::string FileSystemIndex::FileIndexItem::actionText(Action a) const
{
	std::ostringstream stringStream;

	if (a == Action::Enter){
		stringStream << "Open '" << _title << "' with default application.";
		return stringStream.str();
	}

	if (a == Action::Ctrl){
		stringStream << "Open '" << _title << "' in default file browser.";
		return stringStream.str();
	}

	// else Action::Alt
	stringStream << "Search for '" << _title << "' in web.";
	return stringStream.str();
}

/**************************************************************************//**
 * @brief FileSystemIndex::FileIndexItem::mimeType
 * @return
 */
std::string FileSystemIndex::FileIndexItem::iconName() const
{
#ifdef FRONTEND_QT
	return FileSystemIndex::instance()->mimeDb.mimeTypeForFile(QString::fromStdString(_path.string())).iconName().toStdString();
#endif

//	std::string s("xdg-mime query filetype ");
//	s.append(_path.string());
//	FILE* pipe = popen(s.c_str(), "r");
//	if (!pipe)
//		return "ERROR";

//	s.clear();
//	while(!feof(pipe)) {
//		char buffer[128];
//		if(fgets(buffer, 128, pipe) != NULL)
//			s += buffer;
//	}
//	pclose(pipe);
//	return s
//--------------------------------------------------------------------------...
	//	std::string s(magic_file(FileSystemIndex::instance()->_magic_cookie, _path.c_str()));
//	return s;
//--------------------------------------------------------------------------...
}
