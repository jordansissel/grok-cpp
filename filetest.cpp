
#include <string>
using namespace std;

#include "grokregex.hpp"
#include "grokmatch.hpp"

#include "fileobserver.hpp"

int main() {
  FileObserver fo;
  string messages("../logs/messages");
  string auth("/var/log/auth.log");
  string lscmd("ls /var/log/*.log");
  string tailcmd("tail -f /var/log/auth.log");
  fo.AddFile(messages);
  //fo.AddFile(auth);
  //fo.AddFileCommand(lscmd);
  //fo.AddCommand(tailcmd);

  FileObserver::data_input_vector_type input_vector;
  FileObserver::data_pair_type input_pair;
  FileObserver::data_input_vector_type::iterator iter;
  GrokRegex<sregex> gre;
  GrokMatch<sregex> gm;
  GrokPatternSet<sregex> pset;
  pset.LoadFromFile("patterns");
  gre.AddPatternSet(pset);

  gre.SetRegex("%SYSLOGBASE%");

  fo.OpenAll();
  fo.ReadLines(30, input_vector);
  for (iter = input_vector.begin(); iter != input_vector.end(); iter++) {
    input_pair = *iter;
    if (gre.Search(input_pair.second, gm)) {
      GrokMatch<sregex>::match_map_type gmap = gm.GetMatches();
      cout << "(" << input_pair.first.data << ") " << input_pair.second << endl;
      cout << "date: " << gmap["SYSLOGDATE"] << endl;
      cout << "hostname: " << gmap["HOSTNAME"] << endl;
      cout << "prog: " << gmap["PROG"] << endl;
      cout << endl;
    }
  }

  return 0;
}
