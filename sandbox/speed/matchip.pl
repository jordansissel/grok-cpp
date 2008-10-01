#!/usr/bin/perl

use strict;
use warnings;

my $re = qr/((?:[0-9]+\.){3}(?:[0-9]+))/;
my $count = 0;
my $iterations = 0;
my $max_iterations = $ARGV[0];

my $str = "- - [03/Oct/2006:18:37:42 -0400] 65.57.245.11 \"GET / HTTP/1.1\" 200 637 \"-\" \"Mozilla/5.0 (X11; U; Linux i686 (x86_64); en-US; rv:1.8.0.5) Gecko/20060731 Ubuntu/dapper-security Firefox/1.5.0.5\"";  

while ($iterations < $max_iterations) {
  if ($str =~ m/$re/) {
    $count++;
  }
  $iterations++;
}

print "Matches: $count\n";
