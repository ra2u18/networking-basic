import globals
import sys, os
import subprocess

config = "Debug"
exepath = "{}/bin/{}/{}/".format(os.getcwd(), config, globals.PROJECT_NAME) 

# Captures the failure of the subprocess calls
ret = 0

if globals.IsWindows():
    # Run the .exe build from the directory bin/%{cfg}/%{prj.name}
    ret = subprocess.call(["cmd.exe", "/c", "{}\\run.bat".format(globals.TOOLS_DIR),
            config, globals.PROJECT_NAME], cwd=os.getcwd())
else:
    ret = subprocess.call(["{}{}".format(exepath, globals.PROJECT_NAME)], cwd=exepath)

sys.exit(ret)