#include "Path.h"
#include "Directory.h"
#include <iterator>
#include <cstring>

#ifdef WIN32
  #include <windows.h>
  #include <sys/stat.h>

  const char * Path::sep = "\\";
#else
  #include <sys/stat.h>
  #include <fstream>

  const char * Path::sep = "/";
#endif

Path::Path(const std::string & path) : path_(path)
{
}

bool Path::exists(const std::string & path)
{
  if (path.empty())
    return false;

	struct stat s;
	if( stat(path.c_str(), &s) == 0 ) {
		return true;
	}
	return false;
}


bool Path::isDirectory(const std::string & path)
{
	struct stat s;
	if( stat(path.c_str(), &s) == 0 ) {
	    if( s.st_mode & S_IFDIR ) {
	        return true;
	    }
	}
	return false;
}
  
bool Path::isAbsolute(const std::string & path)
{
  if (path.empty()) return false;
#ifdef WIN32
  if (path.size() < 2)
    return false;
  else
  return path[1] == ':';
#else
  return path[0] == '/';
#endif
}

bool Path::isSymbolicLink(const std::string & path)
{
#ifndef WIN32
	struct stat s;
	if( lstat(path.c_str(), &s) == 0 ) {
	    if( S_ISLNK(s.st_mode) ) {
	        return true;
	    }
	}
#endif
	return false;
}

std::string Path::getBasename(const std::string & path)
{
  std::string::size_type index = path.find_last_of(Path::sep);
  
  if (index == std::string::npos)
    return path;
  
  return std::string(path.c_str() + index + 1, path.size() - index);
}

std::string Path::getExtension(const std::string & path)
{
  std::string filename = Path::getBasename(path);
  std::string::size_type index = filename.find_last_of('.');
  
  // If its a  regular or hidden filenames with no extension
  // return an empty string
  if (index == std::string::npos ||  // regular filename with no ext 
      index == 0                 ||  // hidden file (starts with a '.')
      index == path.size() -1) {     // filename ends with a dot
    return "";
  }
  
  // Don't include the dot, just the extension itself (unlike Python)
  return filename.substr(index + 1);
}

std::string Path::normalize(const std::string & path)
{
  return path;
}

std::string Path::makeAbsolute(const std::string & path)
{
  if (Path::isAbsolute(path))
    return path;
    
  std::string cwd = Directory::getCWD();
  // If its already absolute just return the original path
  if (::strncmp(cwd.c_str(), path.c_str(), cwd.length()) == 0) 
    return path;
  
  // Get rid of trailing separators if any
  if (path.find_last_of(Path::sep) == path.length() - 1)
  {
    cwd = std::string(cwd.c_str(), cwd.length()-1);
  }
  // join the cwd to the path and return it (handle duplicate separators) 
  if (path.find_first_of(Path::sep) == 0)
  {
    return cwd + path;
  }
  else
  {
    return cwd + Path::sep + path;
  }
  
  return "";
}

std::string Path::join(StringVec::iterator begin, StringVec::iterator end)
{
  // Need to get rid of redundant separators

  if (begin == end)
    return "";
    
  std::string path(*begin++);
  
  while (begin != end)
  {
    path += Path::sep;
    path += *begin++;
  };
  
  return path;
}

Path::operator const char *() const
{
  return path_.c_str();
}

Path & Path::operator+=(const Path & path)
{
  Path::StringVec sv;
  sv.push_back(std::string(path_));
  sv.push_back(std::string(path.path_));
  path_ = Path::join(sv.begin(), sv.end());
  return *this;
}

Path Path::getBasename() const
{
  return Path::getBasename(path_);
}

Path Path::getExtension() const
{
  return Path::getExtension(path_); 
}
  
Path & Path::normalize()
{
  path_ = Path::normalize(path_);
  return *this;
}

Path & Path::makeAbsolute()
{
  if (!isAbsolute())
    path_ = Path::makeAbsolute(path_);
  return *this;
}

bool Path::exists() const
{
  return Path::exists(path_);
}

bool Path::isAbsolute() const
{
  return Path::isAbsolute(path_);
}

bool Path::isSymbolicLink() const
{
  return Path::isSymbolicLink(path_);
}

bool Path::isDirectory() const
{
  return Path::isDirectory(path_);
}

Path operator+(const Path & p1, const Path & p2)
{
  Path::StringVec sv;
  sv.push_back(std::string(p1));
  sv.push_back(std::string(p2));
  return Path::join(sv.begin(), sv.end());
}


