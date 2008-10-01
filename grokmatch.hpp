#ifndef __GROKMATCH_HPP
#define __GROKMATCH_HPP

#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <boost/xpressive/xpressive.hpp>
using namespace boost::xpressive;

void StringSlashEscape(string &value, const string &chars) {
  sregex re_chars = sregex::compile("[" + chars + "]");
  string format = "\\$&";
  value = regex_replace(value, re_chars, format);
}

#include <string>
#include <vector>
#include <iostream>
using namespace std;

void ParseFuncArgs(const string &func, vector<string> &args) {
  args.clear();
  string::size_type pos;
  string::size_type last_pos;

  last_pos = func.find("(");

  /* If no arguments were given, leave the args vector empty */
  if (last_pos == string::npos)
    return;

  last_pos++; /* skip '(' */
  while ((pos = func.find(",", last_pos)) != string::npos) {
    args.push_back(func.substr(last_pos, (pos - last_pos)));
    last_pos = pos + 1; /* skip past comma */
  }

  pos = func.find(")", last_pos);
  if (pos == string::npos) {
    cerr << "Missing expected closing ')' in filter '" << func << "'" << endl;
    args.clear();
    /* XXX: Really we should throw an exception */
    exit(2); 
  } else { /* Closing paren found */
    args.push_back(func.substr(last_pos, (pos - last_pos)));
  }
}

template <typename regex_type>
class GrokMatch {

  public:
    typedef map<string, typename regex_type::string_type> match_map_type;
    sregex pattern_expand_re;
    mark_tag mark_pattern_name;
    mark_tag mark_filters;

    GrokMatch() : mark_pattern_name(1), mark_filters(2) { 
      /* nothing to do */
    }

    void init(const typename regex_type::string_type &data, 
              const match_results<typename regex_type::iterator_type> &match,
              const match_map_type &backref_map) {
      stringstream strconv(stringstream::in | stringstream::out);
      string tmp;

      this->match_string = match.str(0);
      this->matches = backref_map;
      this->length = match.length();
      this->position = match.position();

      /* Set some default values */
      //string match_key = "=MATCH";
      //string line_key = "=LINE";
      //this->matches[match_key] = this->match_string;
      //this->matches[line_key] = data;
      this->SetMatchMetaValue("MATCH", this->match_string);
      this->SetMatchMetaValue("LINE", data);

      strconv << this->length;
      this->SetMatchMetaValue("LENGTH", strconv.str());
      strconv.str(string());

      strconv << this->position;
      this->SetMatchMetaValue("POSITION", strconv.str());
      strconv.str(string());

      this->pattern_expand_re = 
        as_xpr('%')
          >> (GrokMatch::mark_pattern_name =
              !(as_xpr('=')) >> +(alnum | as_xpr('_'))
              >> !(as_xpr(':') >> +(alnum | as_xpr('_')))
             )
          >> (GrokMatch::mark_filters =
              *('|' >> +(+alnum | '(' | ')' | ','))
             )
        >> as_xpr('%');
    }

    ~GrokMatch() {
      /* Nothing to do */
    };

    const match_map_type& GetMatches() const {
      return this->matches;
    };

    typename regex_type::string_type GetMatchString() const {
      return this->match_string;
    }

    int GetLength() {
      return this->length;
    }

    int GetPosition() {
      return this->position;
    }

    void ExpandString(const string &src, string &dst) {
      regex_iterator<typename regex_type::iterator_type> cur(
           src.begin(), src.end(), pattern_expand_re);
      regex_iterator<typename regex_type::iterator_type> end;
      unsigned int last_pos = 0;
      typename match_map_type::const_iterator map_iter;
      
      dst = "";

      for (; cur != end; cur++) {
        match_results<typename regex_type::iterator_type> match = *cur;
        string pattern_name = match[this->mark_pattern_name].str();
        string filter_str = match[this->mark_filters].str();

        if (match.position() > last_pos)
          dst += src.substr(last_pos, match.position() - last_pos);

        last_pos = match.position() + match.length();

        map_iter = this->matches.find(pattern_name);
        if (map_iter != this->matches.end()) {
          string value = (*map_iter).second;
          this->Filter(value, filter_str);
          dst += value;
        } else {
          dst += "%" + pattern_name + "%";
        }
      }

      if (last_pos < src.size())
        dst += src.substr(last_pos, src.size() - last_pos);
    }

    void ToJSON(string &dst) {
      typename match_map_type::const_iterator map_iter;
      dst = "{";

      for (map_iter = this->matches.begin();
           map_iter != this->matches.end();
           /* no increment here, see bottom of loop */) {
        string key = (*map_iter).first;
        string val = (*map_iter).second;
        StringSlashEscape(key, "\"");
        StringSlashEscape(val, "\"");
        dst += "\"" + key + "\": ";
        dst += "\"" + val + "\"";

        map_iter++;
        if (map_iter != this->matches.end())
          dst += ", ";
      }
      dst += "}";

    }
      
    void Filter(string &value, const string &filter_str) {
      string::size_type pos = 0;
      string::size_type last_pos = 0;
      string filter_func;
      vector<string> filters;
      while ((pos = filter_str.find("|", last_pos)) != string::npos) {
        filter_func = filter_str.substr(last_pos, (pos - last_pos));
        last_pos = pos + 1;
        if (filter_func.size() > 0)
          filters.push_back(filter_func);
      }
      /* Capture the last one, too */
      filter_func = filter_str.substr(last_pos, (filter_str.size() - last_pos));
      if (filter_func.size() > 0)
        filters.push_back(filter_func);

      vector<string>::iterator filter_iter;
      for (filter_iter = filters.begin(); filter_iter != filters.end(); filter_iter++) {
        cerr << "Filter: " << *filter_iter << endl;
        if (*filter_iter == "shellescape")
          this->Filter_ShellEscape(*filter_iter, value);
        else if ((*filter_iter).substr(0, 3) == "dns")
          this->Filter_DNS(*filter_iter, value);
        else if (*filter_iter == "stripquotes")
          this->Filter_StripQuotes(*filter_iter, value);
      }
    }

    void Filter_ShellEscape(const string &func, string &value) {
      StringSlashEscape(value, "(){}\\[\\]\"'!$^~;<>?\\\\");
    }

    void Filter_StripQuotes(const string &func, string &value) {
      if (value[0] = '"' && value[value.size() - 1] == '"') {
        value = value.substr(1, value.size() - 2);
      } else if (value[0] = '\'' && value[value.size() - 1] == '\'') {
        value = value.substr(1, value.size() - 2);
      }
    }

    void Filter_DNS(const string &func, string &value) {
      vector<string> args;
      vector<string>::const_iterator arg_iter;
      enum { Q_A, Q_AAAA, Q_PTR, Q_MX, Q_TXT } qtype;
      ParseFuncArgs(func, args);

      if (args.empty()) {
        string default_query = "A";
        args.push_back(default_query);
      }

      for (arg_iter = args.begin(); arg_iter != args.end(); arg_iter++) {
        if (*arg_iter == "A")
          qtype = Q_A;
        else if (*arg_iter == "AAAA") 
          qtype = Q_AAAA;
        else if (*arg_iter == "PTR")
          qtype = Q_PTR;
        else {
          cerr << "Unknown query type: " << *arg_iter << endl;
          qtype = Q_A;
        }

        const char *addr = value.c_str();
        struct addrinfo *res;
        struct addrinfo hints;
        int dns_error;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = (qtype == Q_AAAA ? PF_INET6 : PF_INET);
        if (qtype == Q_PTR)
          hints.ai_flags = AI_NUMERICHOST;
        cerr << "DNS: " << addr << endl;
        dns_error = getaddrinfo(addr, NULL, &hints, &res);
        if (dns_error) {
          value = "error_in_dns";
          cerr << "dns error: " << dns_error << endl;
          cerr << "query was " << addr << "(type: " << qtype  << endl;
          return;
        }
        char hostaddr[255];
        int getname_flags = 0;
        memset(hostaddr, 0, 255);
        if (qtype != Q_PTR)
          getname_flags = NI_NUMERICHOST;
        getnameinfo(res->ai_addr, res->ai_addrlen, hostaddr, 255,
	NULL, 0, getname_flags);
        value = hostaddr;
        freeaddrinfo(res);
      }
    }

    void SetMatchMetaValue(string name, string value) {
      this->matches["=" + name] = value;
    }

    void SetMatchMetaValue(const char * name, string value) {
      string name_str(name);
      this->SetMatchMetaValue(name_str, value);
    }

  private:
    match_map_type matches;
    typename regex_type::string_type match_string;
    int length;
    int position;
};

#endif /* ifndef __GROKMATCH_HPP */
