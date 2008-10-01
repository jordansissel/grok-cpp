#ifndef __FILEOBSERVER_HPP
#define __FILEOBSERVER_HPP

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>

#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct DataInput {
  FILE *fd;
  string data;

  bool is_command; /* is the 'filename' actually a command? */
  string shell;    /* only set if is_command == true */
  bool follow;     /* Only valid on files (when is_command == false) */
  struct timeval ignore_until_time; /* Ignore this file until ...
                                     *   ignore_until_time >= time_in_epoch */

  void SetIgnoreDuration(float duration) {
    struct timeval now;

    /* Add 'duration' to the current time */
    gettimeofday(&now, NULL);
    this->ignore_until_time.tv_sec = now.tv_sec + (long)duration;
    this->ignore_until_time.tv_usec = \
      now.tv_usec + ((long)(duration - (long)duration) * 1000000L);

    //cerr << " Ignoring: " << this->ignore_until_time.tv_sec << "." << this->ignore_until_time.tv_usec << endl;
  }

  bool IsValid() {
    return this->fd != NULL;
  }

  bool CanRead() {
    /* XXX: It's possible we'll be calling gettimeofday() lots of times.
     * Maybe we should pass the timeval into this method? */
    struct timeval now;
    gettimeofday(&now, NULL);
    //cerr << "CanRead: " << (timerisset(&(this->ignore_until_time))) 
      //<< " / " << timercmp(&now, &(this->ignore_until_time), <) << endl;
    //cerr << " --> " << this->ignore_until_time.tv_sec << "." << this->ignore_until_time.tv_usec << endl;
    if (timerisset(&(this->ignore_until_time)) &&
        timercmp(&now, &(this->ignore_until_time), <)) {
      return false;
    }

    return true;
  }

  void clear() {
    this->fd = NULL;
    this->data = "";
    this->is_command = false;
    this->shell = "";
    this->follow = false;
    timerclear(&this->ignore_until_time);
  }
};

class FileObserver {
  public:
    typedef pair < DataInput, string > data_pair_type;
    typedef vector < data_pair_type > data_input_vector_type;

    FileObserver() : inputs(), old_buffers(), buffer_size_limit(5U<<20)
      { }

    void SetBufferLimit(string::size_type size_limit) {
      this->buffer_size_limit = size_limit;
    }

    void AddCommand(string command) {
      DataInput di;
      di.clear();
      di.data = command;
      di.is_command = true;
      di.shell = "/bin/sh";

      cerr << "Adding cmd: " << command << endl;
      this->inputs.push_back(di);
    }

    void AddFile(string filename, bool follow=true) {
      DataInput di;
      di.clear();
      di.data = filename;
      di.fd = NULL;
      di.is_command = false;
      di.follow = follow;
      cerr << "Adding file: " << filename << endl;

      this->inputs.push_back(di);
    }

    void AddFileCommand(string command) {
      FILE *fd = popen(command.c_str(), "r");
      char buf[4096];

      while (fgets(buf, 4096, fd) != NULL) {
        buf[ strlen(buf) - 1 ] = '\0';
        string filename(buf);
        this->AddFile(buf);
      }
    }

    void Merge(FileObserver fo) {
      vector<DataInput>::iterator di_iter;
      for (di_iter = fo.inputs.begin(); di_iter != fo.inputs.end(); di_iter++) {
        this->inputs.push_back(*di_iter);
      }

      map <int, string>::const_iterator buffer_iter;
      for (buffer_iter = fo.old_buffers.begin(); 
           buffer_iter != fo.old_buffers.end();
           buffer_iter++) {
        this->old_buffers[ (*buffer_iter).first ] = (*buffer_iter).second;
      }
    }

    void OpenAll() {
      vector<DataInput>::iterator iter;
      vector<DataInput> inputs_copy = this->inputs;
      this->inputs.clear();
      for (iter = inputs_copy.begin(); iter != inputs_copy.end(); iter++) {
        DataInput di = *iter;
        if (di.is_command) {
          this->OpenCommand(di);
        } else {
          this->OpenFile(di);
        }
        cerr << "openall fd: " << di.fd << endl;
        /* Hack to store the modified DataInput object back */
        this->inputs.push_back(di);
      }
    }

    void OpenFile(DataInput &di) {
      di.fd = fopen(di.data.c_str(), "r");
      if (di.fd == NULL) {
        cerr << "OpenFile failed on file: '" << di.data << "'" << endl;
        cerr << "Error was: " << strerror(errno) << endl;
        return;
      }

      cerr << "OpenFile success on file: '" << di.data << "'" << endl;
    }

    void OpenCommand(DataInput &di) {
      di.fd = popen(di.data.c_str(), "r");
      if (di.fd == NULL) {
        cerr << "OpenCommand failed on command: '" << di.data << "'" << endl;
        cerr << "Error was: " << strerror(errno) << endl;
        return;
      }

      cerr << "OpenCommand success on command: '" << di.data << "'" << endl;
    }

    void ReadLines(float timeout, data_input_vector_type &data) {
      int ret;
      vector<DataInput>::iterator iter;
      struct timeval tv;
      fd_set in_fdset;
      fd_set err_fdset;

      tv.tv_sec = (long)timeout;
      tv.tv_usec = (long)(timeout - (tv.tv_sec)) * 1000000L;

      /* Come up with a descriptor set that includes all of the inputs */
      FD_ZERO(&in_fdset);
      FD_ZERO(&err_fdset);
      for (iter = this->inputs.begin(); iter != this->inputs.end(); iter++) {
        DataInput di = *iter;
        
        if (di.CanRead()) {
          //cerr << "File: " << di.fd << "(" << di.data  << ")" << endl;
          FD_SET(fileno(di.fd), &in_fdset);
          FD_SET(fileno(di.fd), &err_fdset);
        } else {
          if (!di.IsValid() || !di.follow) {
            cerr << "Removing no-longer-valid file entry: " << di.data << endl;
            this->inputs.erase(iter);
          } else {
            cerr << "Ignoring file (can't read right now): " << di.data << endl;
          }
        }
      }

      ret = select(FD_SETSIZE, &in_fdset, NULL, &err_fdset, &tv);
      if (ret == -1) {
        cerr << "Error in select() call" << endl;
        return;
      } else if (ret == 0) {
        return; /* Nothing to read */
      }
      //cerr << "Select returned " << ret << endl;

      /* else, ret > 0, read data from the inputs */
      //cerr << "Input size: " << this->inputs.size() << endl;
      for (iter = this->inputs.begin(); 
           (!this->inputs.empty()) && (iter != this->inputs.end());
           iter++) {
        DataInput *di = &(*iter);

        cerr << "Checking for input: " << di->data << "(" << di->fd << ")" << endl;
        if (FD_ISSET(fileno(di->fd), &in_fdset)) {
          ReadLinesFromInput(data, *di);
        } else if (FD_ISSET(fileno(di->fd), &err_fdset)) {
          cerr << "Error condition on " << di->data << " (removing this input now)" << endl;
          fclose(di->fd);
          this->inputs.erase(iter);
        }

        if (feof(di->fd)) {
          if (di->follow) {
            /* Ignore this file for at least one iteration */
            cerr << "No data left in file; ignoring for " << timeout 
                 << " secs: " << di->data << endl;
            di->SetIgnoreDuration(timeout);
          } else {
            this->inputs.erase(iter);
          }
        } 
      }
    }

    void ReadLinesFromInput(data_input_vector_type &data, DataInput &di) {
      fd_set fdset;
      bool done = false;
      struct timeval tv;
      int ret;
      string buffer = this->old_buffers[fileno(di.fd)];
      char readbuf[4096];

      cerr << "Reading from: " << di.data << endl;
      //cerr << "fd: " << di.fd << endl;
      //cerr << "fn: " << fileno(di.fd) << endl;

      //setlinebuf(di.fd);

      while (!done) {
        FD_ZERO(&fdset);
        FD_SET(fileno(di.fd), &fdset);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        //cerr << "Waiting for data" << endl;
        ret = select(FD_SETSIZE, &fdset, NULL, &fdset, &tv);
        //cerr << "Finished waiting for data" << endl;

        /* XXX: Handle '< 0' and '== 0' cases separately */
        if (ret <= 0) {
          done = true;
          continue;
        }

        int bytes;
        char *fgets_ret;

        //cerr << "Starting read" << endl;
        //bytes = fread(readbuf, 1, 4096, di.fd);
        fgets_ret = fgets(readbuf, 4096, di.fd);
        bytes = strlen(readbuf);
        //cerr << "Finished read" << endl;
        if (fgets_ret == NULL)
          done = true;
        else
          buffer.append(readbuf, bytes);

        /* Cut this short if the buffer is big enough */
        if (buffer.size() > this->buffer_size_limit)
          done = true;
      }

      /* Look for full lines of text, push them into the data vector */
      string::size_type pos = 0;
      string::size_type last_pos = 0;
      while ((pos = buffer.find("\n", last_pos)) != string::npos) {
        data_pair_type p;
        p.first = di;
        p.second = buffer.substr(last_pos, (pos - last_pos));
        //cerr << "Found line: " << p.second.size() << endl;
        //cerr << "data: " << last_pos << " / " << buffer.length() << endl;
        data.push_back(p);
        last_pos = pos + 1;
      }
      
      string remainder;
      if (last_pos <= buffer.size() - 1)
         remainder = buffer.substr(last_pos, buffer.size() - last_pos);
      old_buffers[fileno(di.fd)] = remainder;

      if (data.size() > 0)
        cerr << "Read " << data.size() << " lines" << endl;
    }

    bool DoneReading() {
      return this->inputs.empty();
    }

    const vector<DataInput>& GetDataInputs() const {
      return this->inputs;
    }

  protected:
    vector<DataInput> inputs;
    map <int, string> old_buffers;
    string::size_type buffer_size_limit;
};

#endif /* ifdef __FILEOBSERVER_HPP */
