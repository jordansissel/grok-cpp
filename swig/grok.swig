%module grok

%{
#include "boost/xpressive/xpressive.hpp"
#include "../grokpatternset.hpp"
#include "../grokregex.hpp"
#include "../grokmatch.hpp"
#include "../grokconfig.hpp"
//include "grokwrap.hpp"
%}


namespace boost { namespace xpressive { }; };

%include "../grokregex.hpp"
%include "../grokmatch.hpp"
%template(SGrokRegex) GrokRegex<sregex>;
%template(SGrokMatch) GrokMatch<sregex>;
