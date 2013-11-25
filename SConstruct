import os

cflags = {"debug": ["-g", "-D_DEBUG"],
          "release": ["-O3", "-DNDEBUG"]}

try:
    mode = cflags[ARGUMENTS.get("mode", "debug")]
except:
    mode = cflags["debug"]

# macro-like function to build a list of
# directories
def build(dirs):
    SConscript([os.path.join(d, "SConscript") for d in dirs])

def add_include_path():
    Import("env")
    env.Append(CPPPATH=["%s/include" %Dir(".").abspath])
    Export("env")

def add_library_path():
    Import("env")
    env.Append(LIBPATH=["%s" %Dir(".").abspath])
    Export("env")

env = Environment(CCFLAGS = ["-fopenmp"] + mode)
Export("env", "os", "build", "add_include_path", "add_library_path", "mode")

SConscript("SConscript", variant_dir="build")
