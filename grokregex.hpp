#ifndef __GROKREGEX_HPP
#define __GROKREGEX_HPP

#include <iostream>
#include <string>
#include <boost/xpressive/xpressive.hpp>

#include "grokpatternset.hpp"
#include "grokmatch.hpp"
#include "grokpredicate.hpp"

using namespace std;
using namespace boost::xpressive;

template <typename regex_type>
class GrokRegex {
  public:
    typedef map <string, typename regex_type::string_type> capture_map_t;

    GrokRegex(const string grok_pattern) 
    : pattern(grok_pattern) {
      this->generated_regex = NULL;
      this->generated_string = NULL;
      this->re_compiler = NULL;
      this->track_matches = true;
      this->GenerateRegex();
    }

    GrokRegex() { 
      this->generated_regex = NULL;
      this->generated_string = NULL;
      this->pattern = "";
      this->re_compiler = NULL;
      this->track_matches = true;
    }
    ~GrokRegex() { /* Nothing to do */ }

    void SetRegex(const string grok_pattern) {
      this->pattern = grok_pattern;
      this->GenerateRegex();
    }

    void SetRegex(const char *char_grok_pattern) {
      string grok_pattern = char_grok_pattern;
      this->SetRegex(grok_pattern);
    }

    const string GetExpandedPattern() const {
      return *(this->generated_string);
    }

    const regex_type GetRegex() const {
      return *(this->generated_regex);
    }

    const string& GetOriginalPattern() const {
      return this->pattern;
    }

    const void SetTrackMatches(bool value) {
      this->track_matches = value;
    }

    void AddPatternSet(const GrokPatternSet<regex_type> &pattern_set) {
      this->pattern_set.Merge(pattern_set);
      this->GenerateRegex();
    }

    bool Search(const typename regex_type::string_type data, 
                GrokMatch<regex_type> &gm) {
      match_results<typename regex_type::iterator_type> match;
      int ret;

      /* Late binding with Boost.Xpressive. 
       * Inject capture_map for placeholder_map. */
      match.let(this->placeholder_map = this->capture_map);
      ret = regex_search(data.begin(), data.end(), match, *(this->generated_regex));
      if (!ret)
        return false;

      gm.init(data, match, this->capture_map);

      return true;
    }

    string Replace(string source, string replacement, bool replace_all=false) {
      string result = source;
      //cerr << "Pattern: " << pattern << endl;
      //cerr << "Pattern:: " << *this->generated_string << endl;
      regex_iterator<typename regex_type::iterator_type> 
        iter(source.begin(), source.end(), (*this->generated_regex));
      regex_iterator<typename regex_type::iterator_type> end;
      int offset = 0;

      for (; iter != end; iter++) {
        match_results<typename regex_type::iterator_type> match(*iter);
        //result = match.format(replacement);
        int len;
        int pos;
        len = match.length();
        pos = match.position();

        result.replace(pos, len, replacement);

        if (replace_all == false)
          break;
      }
      cerr << "Result: " << result << endl;
      return result;
    }

  private:
    GrokPatternSet<regex_type> pattern_set;
    string pattern;
    regex_compiler<typename regex_type::iterator_type> *re_compiler;
    regex_type *generated_regex;
    string *generated_string;
    capture_map_t capture_map;
    placeholder< capture_map_t > placeholder_map;
    bool track_matches;

    void GenerateRegex() {
      int backref = 0;

      if (this->generated_regex != NULL)
        delete this->generated_regex;
      this->generated_regex = new regex_type;

      if (this->generated_string != NULL)
        delete this->generated_string;
      this->generated_string = new string;

      /* make a new compiler */
      if (this->re_compiler != NULL)
        delete this->re_compiler;
      this->re_compiler = new regex_compiler<typename regex_type::iterator_type>;

      /* XXX: Enforce a max recursion depth */
      this->RecursiveGenerateRegex(this->pattern, backref, 
                                   &this->generated_regex, 
                                   *this->generated_string);
      //cerr << "Regex str: " << *(this->generated_string) << endl;
    }

    /* XXX: Split this into smaller functions */
    void RecursiveGenerateRegex(string pattern, int &backref, regex_type **pregex, string &expanded_regex) {
      //sregex not_percent = (!+(+~(as_xpr('%') | '\\') | "\\."))
      /* not_percent == /([^\\%]+|\\.)+?/ */
      sregex not_percent = -+(~(boost::xpressive::set='%','\\')
                              | (as_xpr('\\') >> _));
      int last_pos = 0;
      string re_string;
      //regex_type *re;

      mark_tag mark_name(1), mark_alias(2), mark_predicate(3);
      /* Match %foo(:bar)?% */
      sregex pattern_expr_re(
         as_xpr('%')
         /* Pattern name and alias (FOO and FOO:BAR) */
         >> (mark_alias = 
             (mark_name = +(alnum | as_xpr('_')))
             >> !(as_xpr(':') >> +(alnum | as_xpr('_')))
            )
         /* Predicate conditional, optional. */
         >> !(mark_predicate = /* predicates are optional */
              ((boost::xpressive::set= '<', '>', '=') >> !as_xpr('=')
               | (!as_xpr('!') >> as_xpr('~')))
              >> not_percent
             )
         >> '%'
       );

      //cerr << "pattern: " << pattern << endl;

      /* probaly should use regex_iterator<regex_type> here */
      sregex_iterator cur(pattern.begin(), pattern.end(), pattern_expr_re);
      sregex_iterator end;

      for (; cur != end; cur++) {
        smatch const &match = *cur;
        string pattern_name = match[mark_name].str();
        string pattern_alias = match[mark_alias].str();
        string pattern_predicate = match[mark_predicate].str();
        //cerr << "P: " << pattern_name << " / " << pattern_alias << endl;

        if (match.position() > last_pos) {
          string substr = pattern.substr(last_pos, match.position() - last_pos);
          //cerr << "Appending regex '" << substr << "'" << endl;
          re_string += substr;
          expanded_regex += substr;
        }

        last_pos = match.position() + match.length();

        if (pattern_set.patterns.count(pattern_name) > 0) {
          string sub_pattern = pattern_set.patterns[pattern_name].regex_str;
          stringstream re_name(stringstream::out);

          backref++;

          //cerr << "Appending pattern [" << backref << "] '" << pattern_name << "'" << endl;
          //cerr << "--> " << sub_pattern << endl;
          //cerr << "Setting backref of '" << pattern_name << "' to " << backref << endl;

          /* Recurse deep until we run out of patterns to expand */
          re_string += "(";
          expanded_regex += "(";

          regex_type *ptmp_re;
          regex_type backref_re;
          this->RecursiveGenerateRegex(sub_pattern, backref, &ptmp_re, expanded_regex);
          mark_tag backref_tag(backref);

          /* Append predicate regex if we have one */
          if (pattern_predicate.size() > 0) {
            /* Need to track this memory and delete it when we destroy this object. */
            GrokPredicate<regex_type> *pred = 
              new GrokPredicate<regex_type>(pattern_predicate);
            if (this->track_matches)
              backref_re = (*ptmp_re) [ check(*pred) ] [ (this->placeholder_map)[pattern_alias] = as<string>(_) ];
            else
              backref_re = (*ptmp_re) [ check(*pred) ];
          } else {
            if (this->track_matches)
              backref_re = (*ptmp_re) [ (this->placeholder_map)[pattern_alias] = as<string>(_) ];
            else
              backref_re = (*ptmp_re);
          }

          /* Generate the named regex name for (?$foo)  as 'pattern' + 'backref' */
          re_name << pattern_name;
          re_name << backref;

          (*this->re_compiler)[re_name.str()] = backref_re;
          re_string += "(?$"+ re_name.str() + ")";

          re_string += ")";
          expanded_regex += ")";
          delete ptmp_re;
        } else {
          //cerr << "Appending nonpattern '" << pattern_name << "'" << endl;
          string str = "%" + pattern_name + "%";
          re_string += str;
          expanded_regex += str;
        }
      }

      if (last_pos < pattern.size()) {
        /* XXX: Make this a function */
        string substr = pattern.substr(last_pos, pattern.size() - last_pos);
        //cerr << "Appending regex '" << substr << "'" << endl;
        re_string += substr;
        expanded_regex += substr;
      }

      //cerr << "String: " << re_string << endl;
      *pregex = new regex_type(this->re_compiler->compile(re_string));
    }
};
#endif /* ifndef __GROKREGEX_HPP */
