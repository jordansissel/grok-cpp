
#include <iostream>
#include <fstream>
#include <string>
#include <boost/xpressive/xpressive.hpp>

#include <popt.h>

#include "grokpatternset.hpp"
#include "grokregex.hpp"
#include "grokmatch.hpp"
#include "grokconfig.hpp"

using namespace std;
using namespace boost::xpressive;

#define CONFIG_BUFSIZE 4096

typedef map < string, WatchFileEntry > watch_map_type;
FILE *shell_fp;

char *flag_match = NULL;
char *flag_result = "%=MATCH%";
char *flag_config_file = NULL;
int flag_json = 0;

struct poptOption options_table[] = {
  /* longName, shortName, argInfo, arg, val, descrip, argDescrip */
  { NULL, 'm', POPT_ARG_STRING, &flag_match, 0, 
   "Match string", "Match string" },
  { NULL, 'r', POPT_ARG_STRING, &flag_result, 0, 
   "Result string", "Result string" },
  { "json", '\0', POPT_ARG_NONE, &flag_json, 0, 
   "Enable json output (overrides -r)", NULL },
  { "config_file", 'f', POPT_ARG_STRING, &flag_config_file, 0, 
   "Config file", NULL },
  POPT_TABLEEND
};

void grok_line(const FileObserver::data_pair_type &input_pair, 
               watch_map_type &watchmap) {
  const DataInput &di = input_pair.first;
  watch_map_type::iterator map_entry = watchmap.find(di.data);
  WatchFileEntry &wfe = (*map_entry).second;
  const string &line = input_pair.second;

  //cerr << "(" << di.data << ") " << line << endl;

  vector<WatchMatchType>::iterator wmt_iter;

  for (wmt_iter = wfe.match_types.begin();
       wmt_iter != wfe.match_types.end(); 
       wmt_iter++) {
    WatchMatchType &wmt = (*wmt_iter);
    WatchMatchType::grok_regex_vector_type &gre_vector = wmt.match_strings;
    WatchMatchType::grok_regex_vector_type::iterator gre_iter;

    if (gre_vector.size() == 0) {
      cerr << "Match by default." << endl;
      continue;
    }

    for (gre_iter = gre_vector.begin();
         gre_iter != gre_vector.end();
         gre_iter++) {
      /* XXX: gre.Search() modifies self, so we can't be const... blah */
      GrokRegex<sregex> &gre = (*gre_iter);
      GrokMatch<sregex> gm;
      bool success = gre.Search(line, gm);
      //cerr << "Line: " << line << endl;
      //cerr << "Regex: " << gre.GetOriginalPattern() << " : " 
           //<< gre.GetExpandedPattern() << endl;

      //cerr << "Match return: " << success << endl;
      if (!success)
        continue;

      if (wmt.reaction.size() == 0 
          && wmt.reaction_type != WatchMatchType::JSON) {
        cerr << "No reaction specified for type section '" << wmt.type_name 
             << "'" <<  endl;
        continue;
      }

      gm.SetMatchMetaValue("TYPE", wmt.type_name);
      gm.SetMatchMetaValue("DATASOURCE", wfe.name);
      /* XXX: keys not implemented yet */
      //gm.SetMatchMetaValue("KEY", wmt.key);
      string data;
      switch (wmt.reaction_type) {
        case WatchMatchType::SHELL:
          gm.ExpandString(wmt.reaction, data);
          //cerr << "Reaction: " << data << endl;
          data += "\n";
          fwrite(data.c_str(), data.size(), 1, shell_fp);
          fflush(shell_fp);
          break;
        case WatchMatchType::JSON:
          gm.ToJSON(data);
          cout << data << endl;
          break;
        case WatchMatchType::PRINT:
          gm.ExpandString(wmt.reaction, data);
          cout << data << endl;
          break;
        default:
          cerr << "UNKNOWN REACTION TYPE FOUND: " << wmt.reaction_type << endl;
      } /* switch wmt.reaction_type */
    } /* for ... gre_vector.begin() to .end() */
  } /* for ... wfe.match_types.begin() to .end()  */
}

int main(int argc, const char **argv) {
  GrokConfig config;
  string config_data = "";
  char buffer[CONFIG_BUFSIZE];
  int bytes = 0;

  int popt_ret;
  poptContext popts_context;

  popts_context = poptGetContext(NULL, argc, argv, options_table, 0);
  popt_ret = poptGetNextOpt(popts_context);
  if (popt_ret < -1) { /* -1 means we're done parsing, less than that means error */
    cerr << "Error parsing arguments." << endl;
    cerr << "-> " << poptStrerror(popt_ret) << endl;
    return 1;
  }

  if (flag_config_file != NULL) {
    ifstream in(flag_config_file);
    while (!(in.eof() || in.fail())) {
      in.read(buffer, CONFIG_BUFSIZE);
      bytes = in.gcount();
      buffer[bytes] = '\0';
      config_data += buffer;
    }
  } else if (flag_match != NULL) {
    config_data += "file \"/dev/stdin\" {";
    config_data += "  type \"all\" {";
    config_data += "    match = \"";
    config_data += flag_match;
    config_data += "\";";
    if (flag_json) {
      config_data += "    reaction = json_output;";
    } else {
      config_data += "    reaction_print = \"";
      config_data += flag_result;
      config_data += "\";";
    }
    config_data += "  };";
    config_data += "};";
    cerr << config_data << endl;
  } else {
    cout << "No work to do. (No config file or -m flag specified)" << endl;
    return 1;
  }

  shell_fp = popen("/bin/sh", "w");

  config.parse(config_data);

  GrokConfig::watch_file_vector_type::iterator watch_iter;
  GrokConfig::watch_file_vector_type watches;

  watches = config.GetFileEntries();
  map < string, WatchFileEntry > watchmap;
  FileObserver fo;
  for (watch_iter = watches.begin(); watch_iter != watches.end(); watch_iter++) {
    const vector<DataInput> &di_vector = (*watch_iter).fo.GetDataInputs();
    vector<DataInput>::const_iterator di_iter;
    cerr << "FileObserver name: " << (*watch_iter).name << endl;
    for (di_iter = di_vector.begin(); di_iter != di_vector.end(); di_iter++) {
      cerr << "File name: " << (*di_iter).data << endl;
      watchmap[(*di_iter).data] = *watch_iter;
    }
    fo.Merge((*watch_iter).fo);
  }

  FileObserver::data_input_vector_type input_vector;
  FileObserver::data_pair_type input_pair;
  FileObserver::data_input_vector_type::iterator input_iter;

  fo.OpenAll();
  while (!fo.DoneReading()) {
    fo.ReadLines(30, input_vector);
    for (input_iter = input_vector.begin(); 
         input_iter != input_vector.end();
         input_iter++) {
      input_pair = *input_iter;
      grok_line(input_pair, watchmap);
      //cerr << "(" << input_pair.first.data << ") " << input_pair.second << endl;
      //cerr << "watchmap: " << watchmap[input_pair.first.data].name << endl;
    }
    input_vector.clear();
  }
  return 0;
}
