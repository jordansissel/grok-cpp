#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <exception>
using namespace std;

class GrokFileException : public Exception {
  virtual const char *what() const throw() {
    return strerror(errno);
  }
}

class GrokFile {
  public:
    GrokFile(const char *filename) throw(GrokFileException) {
      GrokFileException file_error();
      int stat_result;
      this->filename = filename;
      this->StatFile();
      this->Open();
    }

    void StatFile() throw(GrokFileException) {
      stat_result = stat(this->filename, &(this->metadata));
      if (stat_result == -1)
        throw this->file_error;
    }

    void Open() throw(GrokFileException) {
      this->fd = fopen(this->filename, "r");
      if (fd == NULL)
        throw this->file_error:
    }

    int GetFileNo() {
      return fileno(this->fd);
    }

  private:
    const char *filename;
    struct stat metadata;
    FILE *fd;
}
