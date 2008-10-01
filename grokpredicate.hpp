#ifndef __GROKPREDICATE_HPP
#define __GROKPREDICATE_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

#include "stringutils.hpp"

using namespace std;
using namespace boost::xpressive;

#define GROK_P_EQUAL  (1<<1)
#define GROK_P_NOT  (1<<2)
#define GROK_P_LESS (1<<3)
#define GROK_P_GREATER (1<<4)
#define GROK_P_REGEX (1<<5)

#define GROK_T_INT (1<<0)
#define GROK_T_STR (1<<1)

template <typename regex_type>
class GrokPredicate {
  public:
    typedef sub_match<typename regex_type::iterator_type> sub_match_t;

    GrokPredicate(string predicate) {
      local<unsigned int> flags(0);
      match_results<typename regex_type::iterator_type> match;

      /* XXX: Should this just be a dispatch table? */
      regex_type op_re = 
        ( /* Test for < > <= >= */
          (as_xpr('>')  [ flags |= GROK_P_GREATER ]
           | as_xpr('<') [ flags |= GROK_P_LESS ]
          )
          >> !(as_xpr('=') [ flags |= GROK_P_EQUAL ]
          )
        ) | ( /* Test for '==' */
          as_xpr("==") [ flags |= GROK_P_EQUAL ]
        ) | ( /* Test for != */
          as_xpr("!=") [ flags |= GROK_P_NOT | GROK_P_EQUAL ]
        ) | ( /* Test for =~ and !~ */
          ((as_xpr('=') [ flags |= GROK_P_EQUAL ]
            | as_xpr('!') [ flags |= GROK_P_NOT ] 
           ) >> as_xpr('~')) [ flags |= GROK_P_REGEX ]
        ); /* end 'op' regex */

      regex_search(predicate, match, op_re);
      this->flags = flags.get();

      if (this->flags == 0) {
        /* Throw an error that this was an invalid predicate */
        cerr << "Invalid predicate: '" << predicate << "'" << endl;
      }

      string remainder_string = predicate.substr(match.length(), 
                                                 predicate.size() - match.length());
      stringstream remainder(remainder_string, stringstream::in);

      this->type = GROK_T_STR;
      this->value_string = remainder_string;
      StringUtils::Unescape(this->value_string);

      /* If not a regex, consider using integer comparisons */
      if (this->flags & GROK_P_REGEX) {
        this->value_regex = regex_type::compile(this->value_string);
      } else {
        /* Try to see if this is an integer predicate 
         * (ie, try to convert it to an int) */
        int tmp = 0;
        remainder >> tmp;
        if (!remainder.fail()) {
          this->type = GROK_T_INT;
          this->value_int = tmp;
        }
      }

      //cerr << "Predicate type: " << (this->type == GROK_T_INT ? "int" : "string") << endl;
      //cerr << "Flags: " << this->flags << endl;
      //cerr << "String: " << this->value_string << endl;
      //cerr << "Int: " << this->value_int << endl;
    }

    bool operator()(const sub_match_t &match) const {
      bool result;
      if (this->flags & GROK_P_REGEX) 
        result =  this->call_regex(match);
      else if (this->type == GROK_T_STR)
        result = this->call_string(match);
      else
        result = this->call_int(match);

      if (this->flags & GROK_P_NOT)
        result = !result;

      return result;
    }

    bool call_regex(const sub_match_t &match) const {
      return regex_search(match.str(), this->value_regex);
    }

    bool call_string(const sub_match_t &match) const {
      int cmp = this->compare_string(match);
      return this->result(cmp);
    }

    bool call_int(const sub_match_t &match) const {
      int val = 0;
      int ret;
      stringstream ss(match.str(), stringstream::in);
      ss >> val;

      ret = this->result(val - this->value_int);
      //cerr << val << " vs " << this->value_int << " == " << ret << endl;
      /* Throw an exception if ss.fail() ? */
      return ret;
    }

    int compare_string(const sub_match_t &match) const {
      return match.compare(this->value_string);
    }

    int compare_int(const sub_match_t &match) const {
      return this->value_int - as<int>(_);
    }

    bool result(int compare) const {
      int flags = this->flags;
      switch (flags) {
        case GROK_P_LESS:
          return compare < 0; break;
        case GROK_P_LESS | GROK_P_EQUAL:
          return compare <= 0; break;
        case GROK_P_GREATER:
          return compare > 0; break;
        case GROK_P_GREATER | GROK_P_EQUAL:
          return compare >= 0; break;
        case GROK_P_EQUAL:
          return compare == 0; break;
        case GROK_P_EQUAL | GROK_P_NOT:
          return compare != 0; break;
        default:
          /* Should not get here */
          cerr << "SHOULD NOT GET HERE. flags = " << this->flags << endl;
      }
      cerr << "SHOULD(2) NOT GET HERE. flags = " << this->flags << endl;
      return false;
    }

  private:
    unsigned int flags;
    unsigned int type;
    int value_int;
    string value_string;
    regex_type value_regex;
};
#endif /* ifndef __GROKPREDICATE_HPP */
