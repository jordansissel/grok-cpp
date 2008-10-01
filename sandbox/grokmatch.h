
#ifndef GROKMATCH_H
#define GROKMATCH_H

#include <map>
#include <string>
using namespace std;

#include <oniguruma.h>

class GrokRegex; /* defined in grokregex.h */

class GrokMatch {
  public:
    GrokMatch(const GrokRegex *gre, OnigRegion * const region, const string text);
    ~GrokMatch();

  private:
    int name_callback(const OnigUChar *name, const OnigUChar *end, 
                      int ngroups, int *group_list, OnigRegex re, void *arg);

    OnigRegion *region;
    const GrokRegex *gre;
    const string text;
    map< string, string, less<string> > named_groups;
    map< int, string, less<int> > numbered_groups;
};

#endif /* ifdef GROKMATCH_H */
