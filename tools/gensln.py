import subprocess, sys

import globals

# Captures the failure of the subprocess calls
ret = 0

if globals.IsWindows():
    ret = subprocess.call(["cmd.exe", "/c", "premake\\premake5", "vs2019"])

# TODO: mac and linux support

sys.exit(ret)