#include <string>
#include <algorithm>
#include <iostream>
#include "Directory.h"
#include "Path.h"

#ifdef WIN32
  #include <windows.h>
  #include <tchar.h>
#else
  #include <sys/stat.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>
#endif

using namespace log4cxx;

namespace Directory
{
  LoggerPtr Iterator::logger(Logger::getLogger("Directory.Iterator"));

  bool exists(const std::string & path)
  {
    return Path::exists(path);
  }
  
  std::string getCWD()
  {
    char cwd[1024];
    cwd[0] = '\0';
  #ifdef WIN32
    ::GetCurrentDirectoryA(1024, cwd);
  #else
    ::getcwd(cwd, 1024);
    
  #endif
    return std::string(cwd);
  }
  
  void setCWD(const std::string & path)
  {
  #ifdef WIN32
    ::SetCurrentDirectoryA(path.c_str());
  #else
    ::chdir(path.c_str());
  #endif
  }

  Iterator::Iterator(const Path & path)
  {
    init(std::string(path));
  }
      
  Iterator::Iterator(const std::string & path) : handle_(NULL)
  {
    init(path);
  }

  void Iterator::init(const std::string & path)
  {
    std::string absolutePath = Path::makeAbsolute(path);
  #ifndef WIN32
    handle_ = ::opendir(absolutePath.c_str());
    if (!handle_) {
    	LOG4CXX_ERROR(logger, "Can't open directory " << path << ". Error code: " << errno);
    }

  #else
    handle_ = 0;

        if((handle_ = static_cast<DIR*>(malloc(sizeof *handle_))) != 0 &&
           (handle_->name = (char *) malloc(absolutePath.length() + 2 + 1)) != 0)
        {
            strcpy(handle_->name, (absolutePath + "/*").c_str());

            if((handle_->handle = _findfirst(handle_->name, &handle_->info)) != -1)
            {
                handle_->result.d_name = 0;
            }
            else /* rollback */
            {
                free(handle_->name);
                free(handle_);
                handle_ = 0;
            }
        }
        else /* rollback */
        {
            free(handle_);
            handle_   = 0;
        }

  #endif
  }
  
  Iterator::~Iterator()
  {
  #ifndef WIN32
    int res = ::closedir(handle_);
    if (!res == 0) {
    	LOG4CXX_ERROR(logger, "Couldn't close directory. Error code: " << errno);
    }
  #else
    if(handle_) {
        if(handle_->handle != -1) {
            _findclose(handle_->handle);
        }

        free(handle_->name);
        free(handle_);
    }
  #endif
  }
  
  void Iterator::reset()
  {
  #ifndef WIN32
    ::rewinddir(handle_);
  #else
    if(handle_ && handle_->handle != -1) {
        _findclose(handle_->handle);
        handle_->handle = _findfirst(handle_->name, &handle_->info);
        handle_->result.d_name = 0;
    }
  #endif
  }

#ifndef WIN32
  Entry * Iterator::next(Entry & e)
  {
    errno = 0;
    struct dirent * p = ::readdir(handle_);
    if (errno) {
    	LOG4CXX_ERROR(logger, "Couldn't read next dir entry. Error code: " << errno);
    }
    
    if (!p)
      return NULL;

    e.path = std::string(p->d_name);                               

    // Skip '.' and '..' directories
    if (
       (e.path == std::string(".") || e.path == std::string("..")))
      return next(e);
    else
      return &e;
  }
  
#else  
  Entry * Iterator::next(Entry & e)
  {
	    struct dirent *p = 0;

	    if(handle_ && handle_->handle != -1) {
	        if(!handle_->result.d_name || _findnext(handle_->handle, &handle_->info) != -1) {
	            p         = &handle_->result;
	            p->d_name = handle_->info.name;
	        }
	    }

	    if (!p)
	      return NULL;

	    e.path = std::string(p->d_name);

	    // Skip '.' and '..' directories
	    if (e.path == std::string(".") || e.path == std::string(".."))
	      return next(e);
	    else
	      return &e;
  }
#endif
  
}

