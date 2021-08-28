# The script that interacts with everything in the tools directory

import os, sys
import subprocess # run external commands

TOOLS_DIR = "tools"

def RunCommand(cmd):
    ret = 0
    script = "{}/{}/{}.py".format(os.getcwd(), TOOLS_DIR, cmd)

    # Check for script's existance
    if os.path.exists(script):
        print("Executing command:", cmd)
        ret = subprocess.call(["python3", script])
    else:
        print("Invalid command:", cmd)
        ret = -1

    return ret

for i in range(1, len(sys.argv)):
    cmd = sys.argv[i]
    print("\n----------------------")

    if RunCommand(cmd) != 0:
        break