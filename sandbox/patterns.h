
#ifndef PATTERNS_H
#define PATTERNS_H

//typedef map< string, sregex, less<string> > sregex_map_t;
typedef map< string, string, less<string> > pattern_map_t;

pattern_map_t& DefaultPatterns();

#endif /* ifdndef  PATTERNS_H */
