/* Example code showing how to use dynamic regexps and user-defined assertions
 * with Boost.Xpressive. This is so cool.
 *
 * The input is a set of tokens, and the first match is searching for a token
 * looking like an IP address. The additional check after an IP is found is to
 * test whether it is a private (RFC1918) address
 *
 * License is BSD.
 * Author: Jordan Sissel
 * http://www.semicomplete.com
 */

#include <string>
#include <iostream>
#include <map>
#include <exception>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
using namespace std;
using namespace boost::xpressive;

typedef map<string, string> ssmap_t;

struct is_private {
  bool operator()(ssub_match const &sub) const {
    /* try to match 192.168/16, 10/8, and 172.16/12 */
    sregex rfc1918re = \
       sregex::compile("^(?:192\\.168|10|172\\.(?:1[6-9]|2[0-9]|3[01]))\\.");
    smatch m;
    bool ret;
    ret = regex_search(sub.str(), m, rfc1918re);
    cout << "RFC1918 test on '" << sub << "': " << (ret ? "pass" : "fail") << endl;
    return ret;
  }
};

int main() {
  smatch m;
  string test1 = "hello 1.2.3.4 4.5.6.7 192.168.0.5 bar";
  string test2 = "a.b.c.d 129.21.60.0 172.17.44.25 pants";
  sregex ip_re = sregex::compile("(?:[0-9]+\\.){3}(?:[0-9]+)");
  sregex priv_ip_re = ip_re[ check(is_private()) ];

  if (regex_search(test1, m, priv_ip_re))
    cout << "Match on test1: " << m[0] << endl;
  if (regex_search(test2, m, priv_ip_re))
    cout << "Match on test2: " << m[0] << endl;
  return 0;
}
