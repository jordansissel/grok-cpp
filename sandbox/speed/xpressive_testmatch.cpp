#include <string>
#include <iostream>
#include <exception>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
using namespace std;
using namespace boost::xpressive;

int main(int argc, char **argv) {
  smatch m;
  string input;
  int count = 0;
  // using a 'static' version of the regex isn't much faster, so don't bother.
  //sregex ip_re = repeat<3>(repeat<1,3>digit >> '.' ) >> repeatdigit;

  sregex ip_re = sregex::compile("((?:[0-9]+\\.){3}(?:[0-9]+))");
  sregex ip_prefix_re = sregex::compile("^65\\.");

  int iterations = 0;
  int max_iterations = atoi(argv[1]);
  input = "- - [03/Oct/2006:18:37:42 -0400] 65.57.245.11 \"GET / HTTP/1.1\" 200 637 \"-\" \"Mozilla/5.0 (X11; U; Linux i686 (x86_64); en-US; rv:1.8.0.5) Gecko/20060731 Ubuntu/dapper-security Firefox/1.5.0.5\"";
  
  while (iterations < max_iterations) {
    if (regex_search(input, m, ip_re)) {
      string ip = m[1].str();
      smatch m2;
      if (regex_search(ip, m2, ip_prefix_re)) {
        count++;
      }
    }

    iterations++;
  }
  cout << "Matches: " << count << endl;
  return 0;
}
