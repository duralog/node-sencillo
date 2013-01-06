#!/usr/bin/python
# Tool that produces the C++ code to export a set of
# enum symbols to the JS side, with appropiate names.
#
# First reads lines from stdin,
# then outputs the code to stdout.

# ADJUST SETTINGS:
strip = "GIT_REPOSITORY"
out_pattern = '  func->Set(Symbol("{js_sym}"), Int({c_sym}));'


import re
strip += '_'; out_pattern += '\n'
excl = re.compile('^\s*$')
patt = re.compile('^\s*(\w+)(\s*=.+)?\s*,\s*$')

output=str()
try:
  while True:
    line = raw_input()
    if re.match(excl, line): continue
    
    m = re.match(patt, line)
    if not m: raise Exception('bad line!')

    c_sym = m.group(1)
    if not c_sym.startswith(strip): raise Exception('bad strip!')

    js_sym = c_sym[len(strip):]
    output += out_pattern.format(c_sym=c_sym, js_sym=js_sym)

  print output
except EOFError, e:
  print output
#except Exception, e:
#  print 'Error: ' + e
