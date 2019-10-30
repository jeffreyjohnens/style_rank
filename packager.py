# this is a script to package the contents
import os
from subprocess import call
from string import Template
from create_feature_map_hpp import build_feature_map

def create_from_template(template_path, output_path, content):
  with open(template_path) as template_file:
    src = Template( template_file.read() )
  with open(output_path, "w") as output_file:
    output_file.write(src.substitute(content))


if __name__ == "__main__":

  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument('--mode', type=str, default="develop")
  args = parser.parse_args()

  version_number = 19

  # delete old stuff
  call("pip3 uninstall style_rank -y", shell=True)
  call("rm -rf style_rank-1.0.{}".format(version_number), shell=True)

  # create the feature map
  cwd = os.getcwd()
  os.chdir("src/style_rank")
  build_feature_map()
  os.chdir(cwd)

  # get all the .cpp files
  all_paths = []
  root = "src/style_rank"
  for dirs, subdirs, paths in os.walk(root):
    for path in paths:
      all_paths.append( os.path.join(dirs, path) )

  cpp_paths = [p for p in all_paths if p.endswith(".cpp")]
  hpp_paths = [p for p in all_paths if p.endswith(".hpp") or p.endswith(".h")]

  # automatically get the requirements
  #call(["pipreqs", "./src/style_rank", "--force"])
  with open("./src/style_rank/requirements.txt", "r") as f:
    install_requires = [" ".join(l.split()).replace("==",">=") for l in f.readlines()]
  install_requires += ['pybind11>=2.3']

  content = {
    "URL" : "'https://github.com/jeffreyjohnens/style_rank'",
    "PACKAGE_NAME" : "'style_rank'",
    "VERSION" : "'1.0.{}'".format(version_number),
    "DESCRIPTION" : "''",
    "INSTALL_REQUIRES" : repr(install_requires),
    "CPP_PATHS" : repr(cpp_paths),
    "HPP_PATHS" : repr(hpp_paths)
  }

  create_from_template("setup_TEMPLATE.py", "setup.py", content)

  # clean dist and build
  call("rm -rf dist", shell=True)
  call("rm -rf build", shell=True)
  call(["python3", "setup.py", "sdist"])

  # test the build ... will show all warnings
  if args.mode == "develop":
    call(["pip3", "uninstall", "style_rank", "-y"])
    call(["tar", "-xf", "./dist/style_rank-1.0.{}.tar.gz".format(version_number)])
    call(["pip3", "install", "-vvv", "-e", "style_rank-1.0.{}".format(version_number)])
  
  if args.mode == "upload":
    call(["twine", "upload", "dist/*"])