import os
from subprocess import call

cpp_paths = []
for dirs,_,paths in os.walk("../src/style_rank"):
    for path in paths:
        if path.endswith(".cpp") and not path == "bindings.cpp":
            cpp_paths.append(os.path.join(dirs,path))

print("compiling c++ test program ...")
call("g++ -o test -I../src/style_rank test.cpp " + " ".join(cpp_paths) + " -std=c++14", shell=True)

print("running c++ test program ...")
call("./test", shell=True)

