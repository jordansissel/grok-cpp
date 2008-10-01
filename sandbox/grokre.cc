#include <string>
#include <map>
#include <iostream>
#include <exception>
using namespace std;

#include <oniguruma.h>
#include "grokregex.h"
#include "grokmatch.h"

int main(int argc, char **argv) {
  GrokRegex gre(".*");
  GrokMatch *match;

  match = gre.match("Hello");
  if (match != NULL) {
    printf("match!\n");
  }

  return 0;
}
