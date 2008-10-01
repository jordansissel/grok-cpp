
#ifndef GROKREGEX_H
#define GROKREGEX_H

#include <string>
#include <exception>
#include <iostream>
using namespace std;

#include <oniguruma.h>

#include "grokmatch.h"

class GrokRegex {
  public:
    GrokRegex(string pattern);
    ~GrokRegex();
    
    GrokMatch* match(string pattern);
    const OnigRegex regex;
    const string pattern;
    
  private:
    OnigErrorInfo errinfo;
};  

class OnigurumaException: public exception {
  public:
    OnigurumaException(int errorcode) {
      char s[ONIG_MAX_ERROR_MESSAGE_LEN];
      onig_error_code_to_str((UChar *)s, errorcode);
      errormsg = "Error compiling expression: ";
      errormsg += s;
    } 
    
    /* Why is this necessary? */
    virtual ~OnigurumaException() throw() {
    }
    
    virtual const char* what() const throw() {
      return errormsg.c_str(); 
    } 
    
  private:
    string errormsg;
};  


#endif /* ifndef GROKREGEX_H */

