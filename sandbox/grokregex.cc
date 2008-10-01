
#include "grokregex.h"

GrokRegex::GrokRegex(string pattern) {
  int ret;
  this->pattern = pattern;
  ret = onig_new(&regex, 
                 (const OnigUChar *) (pattern.c_str()), 
                 (const OnigUChar *) (pattern.c_str() + pattern.size()),
                 ONIG_OPTION_DEFAULT,
                 ONIG_ENCODING_UTF8,
                 ONIG_SYNTAX_DEFAULT,
                 &errinfo);
  if (ret != ONIG_NORMAL) 
    throw OnigurumaException(ret);

}

GrokRegex::~GrokRegex() { 
  // onig_free?
}

GrokMatch* GrokRegex::match(string pattern) {
  OnigRegion *region;
  int ret;
  region = onig_region_new();
  ret = onig_search(this->regex,
                    (UChar *) (this->pattern.c_str()), /* Start of string */
                    (UChar *) (this->pattern.c_str() + pattern.size()), /* End of string */
                    (UChar *) (this->pattern.c_str()), /* Start of match */
                    (UChar *) (this->pattern.c_str() + pattern.size()) /* End of match */,
                    region, 
                    ONIG_OPTION_NONE);
  if (ret < ONIG_MISMATCH) {
    cout << "No match found" << endl;
    return NULL;
  } else { 
    return new GrokMatch(this, region);
  } 
} 



