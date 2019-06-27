# this is a script to create the feature map
import re
from string import Template
from subprocess import call

if __name__ == "__main__":
  
  funcs = []
  with open("features.hpp", "r") as infile:
    for line in infile.readlines():
      if re.match(r'.*std::unique_ptr<DISCRETE_DIST>.*(.*Piece.*)', line):
        funcs.append( 
          line.split("std::unique_ptr<DISCRETE_DIST>")[1].split("(")[0].strip() )
  
  s = ",\n\t".join(['{ "' + name + '", &' + name + '}' for name in funcs])

  with open("feature_map_template.hpp") as tmpfile:
    src = Template( tmpfile.read() )
      
  with open("feature_map.hpp", "w") as outfile:
    outfile.write(src.substitute({"FEATURE_MAP" : s}))