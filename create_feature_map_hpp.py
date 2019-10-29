# this is a script to create the feature map
# we also group the features by function
import re
from string import Template
from subprocess import call

def build_feature_map():
  funcs = []
  tag_to_func = {}
  with open("features.hpp", "r") as infile:
    src = str(infile.read())

    # extract feature names from the src
    #for m in re.findall(r'unique_ptr<DISCRETE_DIST>(.+?)Piece', src):
    #  funcs.append( "".join(m.split("(")[0].split()) )
    
    # extract feature names and tag_to_func from the src
    # raise exception if feature is not tagged
    for m in re.findall(r'unique_ptr<DISCRETE_DIST>(.+?)\{', src):
      if len(m.split("/*"))<2 or len(m.split("/*")[1].split("*/"))==0:
        raise RuntimeError("ERROR : %s does not have a tag." % m)
      tags = m.split("/*")[1].split("*/")[0].split(",") + ["ALL"]
      func = "".join(m.split("(")[0].split())
      
      for tag in tags:
        if not tag in tag_to_func:
          tag_to_func[tag] = []
        tag_to_func[tag].append( func )
      funcs.append( func )

  feature_map = ",\n\t".join(
    ['{ "' + name + '", &' + name + '}' for name in funcs])

  feature_tag_map = ",\n\t".join(
    ['{ "' + k + '", {' + ",\n\t\t".join(['"' + vv + '"' for vv in v]) + '}}' for k,v in tag_to_func.items()])
  
  feature_tags = ",".join(['"{}"'.format(k) for k in list(tag_to_func.keys())])

  with open("feature_map_template.hpp") as tmpfile:
    src = Template( tmpfile.read() )
      
  with open("feature_map.hpp", "w") as outfile:
    outfile.write(src.substitute(
      {"FEATURE_MAP" : feature_map, "FEATURE_TAG_MAP" : feature_tag_map, "FEATURE_TAGS" : feature_tags}))

if __name__ == "__main__":
  build_feature_map()