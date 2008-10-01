
#include "grokregex.h"
#include "grokmatch.h"

GrokMatch::GrokMatch(const GrokRegex *gre, OnigRegion * const region, const string text) {
  this->gre = gre;
  this->region = region;
  //this->text = "foo";

  //reinterpret_cast< int (*)(const OnigUChar*, const OnigUChar*, int, int*, OnigRegexType*, void*) >(&this->name_callback),
  onig_foreach_name(gre->regex, 
                    this->name_callback,
                    (void *)this);
}

GrokMatch::~GrokMatch() {
  onig_region_free(this->region, 1);
}

int GrokMatch::name_callback(const OnigUChar *name, const OnigUChar *end, 
                             int ngroups, int *group_list, OnigRegex re, void *arg) {
  GrokMatch *match = (GrokMatch *)arg;
  int capture_num;
  string key;
  string value;

  capture_num = onig_name_to_backref_number(match->gre->regex, name, end, match->region);
  return 0;
}
