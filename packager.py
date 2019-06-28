# this is a script to package the contents
import os
from subprocess import call
from string import Template

def create_from_template(template_path, output_path, content):
  with open(template_path) as template_file:
    src = Template( template_file.read() )
  with open(output_path, "w") as output_file:
    output_file.write(src.substitute(content))


# get all the .cpp files
all_paths = []
root = "src/style_rank"
for dirs, subdirs, paths in os.walk(root):
  for path in paths:
    all_paths.append( os.path.join(dirs, path) )

cpp_paths = [p for p in all_paths if p.endswith(".cpp")]


# automatically get the requirements
call(["pipreqs", "./src/style_rank", "--force"])
with open("./src/style_rank/requirements.txt", "r") as f:
  install_requires = [" ".join(l.split()) for l in f.readlines()]
install_requires += ['pybind11>=2.3']

content = {
  "PACKAGE_NAME" : "'style_rank'",
  "VERSION" : "'1.0.1'",
  "DESCRIPTION" : "''",
  "INSTALL_REQUIRES" : repr(install_requires),
  "SRC_PATHS" : repr(cpp_paths)
}

create_from_template("setup_TEMPLATE.py", "setup.py", content)

# then we need to do some more