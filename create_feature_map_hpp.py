# this is a script to create the feature map
import re
from string import Template
from subprocess import call

def build_feature_map():
  funcs = []
  with open("features.hpp", "r") as infile:
    for m in re.findall(r'unique_ptr<DISCRETE_DIST>(.+?)Piece', infile.read()):
      funcs.append( "".join(m.split("(")[0].split()) )
  
  s = ",\n\t".join(['{ "' + name + '", &' + name + '}' for name in funcs])

  with open("feature_map_template.hpp") as tmpfile:
    src = Template( tmpfile.read() )
      
  with open("feature_map.hpp", "w") as outfile:
    outfile.write(src.substitute({"FEATURE_MAP" : s}))

if __name__ == "__main__":
  build_feature_map()