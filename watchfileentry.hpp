
#ifndef __WATCHFILEENTRY_HPP
#define __WATCHFILEENTRY_HPP

#include "fileobserver.hpp"

struct WatchMatchType {
  typedef vector < GrokRegex<sregex> > grok_regex_vector_type;
  enum reaction_type { SHELL, PRINT, JSON };
  string type_name; /* "foo" from 'type "foo" {' in grok.conf  */
  grok_regex_vector_type match_strings;
  float threshold;
  float interval;
  string key;
  string reaction;
  enum reaction_type reaction_type;
  bool match_syslog;
  bool follow;
  string syslog_prog;
  string syslog_host;
  string shell; /* Not supported yet */

  void clear() {
    this->threshold = 0.0;
    this->interval = 0.0;
    this->key = "";
    this->reaction = "";
    this->match_syslog = false;
    this->syslog_prog = "";
    this->syslog_host = "";
    this->shell = "";
    this->match_strings.clear();
    this->follow = false;
  }
};

class WatchFileEntry {
  public:
    FileObserver fo;
    string name;
    vector < WatchMatchType > match_types;

    void clear() {
      this->name = "";
      this->match_types.clear();
    }
};

#endif /* ifndef __WATCHFILEENTRY_HPP */
