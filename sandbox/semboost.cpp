#include <string>
#include <iostream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
using namespace boost::xpressive;
using namespace std;

void append(sregex **re, sregex &source) {
  if (*re == NULL)
    *re = new sregex;
  if ((*re)->regex_id() == 0)
    **re = source;
  else
    **re >>= source;
}

int main()
{
    smatch m;
    sregex *re = NULL;
    sregex tmp;

    tmp = sregex::compile("foo");
    append(&re, tmp);
    tmp = sregex::compile("bar");
    append(&re, tmp);

    cout << re->regex_id() << endl;
    cout << tmp.regex_id() << endl;
    
    string str("foobar");
    int ret = regex_search(str, m, *re);
    cout << "Match: " << ret << endl;
    cout << "Data: '" << m.str(0) << "'" << endl;
    return 0;
}

