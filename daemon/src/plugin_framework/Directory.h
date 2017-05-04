#ifndef DIRECTORY_H
#define DIRECTORY_H

//----------------------------------------------------------------------

#include <string>
#ifndef WIN32
#include <dirent.h>
#include <sys/stat.h>
#else
#include <io.h>
#endif
#include <log4cxx/logger.h>

//----------------------------------------------------------------------

class Path;
  
namespace Directory
{
  // check if a directory exists
  bool exists(const std::string & path);

  // get current working directory
  std::string getCWD();

  // set current working directories
  void setCWD(const std::string & path);

  struct Entry  
  {
    enum Type { FILE, DIRECTORY, LINK };

    std::string path;
  };

  class Iterator
  {
  public:

    Iterator(const Path & path);    
    Iterator(const std::string & path);
    ~Iterator();

    // Resets directory to start. Subsequent call to next() 
    // will retrieve the first entry
    void reset();
    // get next directory entry
    Entry * next(Entry & e);

#ifdef WIN32
    struct dirent
    {
        char *d_name;
    };

    struct DIR {
        intptr_t         	handle; /* -1 for failed rewind */
        struct _finddata_t  info;
        struct dirent       result; /* d_name null iff first time */
        char             *name;  /* null-terminated char string */
    };
#endif


  private:
    Iterator();
    Iterator(const Iterator &);

    void init(const std::string & path);
  private:
    std::string path_;
    DIR * handle_;

    static log4cxx::LoggerPtr logger;
  };
}


#endif // DIRECTORY_H


