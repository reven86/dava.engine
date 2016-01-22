import subprocess
import sys
from optparse import OptionParser


def main():
    options = OptionParser()
    options.add_option("--vs", dest="vs_version", type=float, default=12.0, help="Visual studio compiler version.")
    options.add_option("--o", dest="output_log", default="Log/Log.plog", help="Path to PVS log output file.")
    options.add_option("--sln", dest="sln_path", help="Path to a project solution.")

    (options, args) = options.parse_args(sys.argv)
    if options.vs_version < 12.0:
        print "Use 12 or newer visual studio compiler." + str(options.vs_version) + " used."
        exit(1)

    if None == options.sln_path:
        print "Unknown solution. Use --sln [path] to specify project solution."
        exit(1)

    vs = "\"C:\Program Files (x86)\Microsoft Visual Studio " + str(options.vs_version) + "\Common7\IDE\devenv.exe\""
    cmd = "/command \"PVSStudio.CheckSolution Win32|Debug|" + options.output_log + "\""

    call = vs + " \"" + options.sln_path + "\"" + cmd
    print subprocess.call(call, shell=False)


if "__main__" == __name__:
    main()
