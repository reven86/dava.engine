import subprocess
import sys
import argparse


def main():
    parser = argparse.ArgumentParser(description='Analyze .sln with PVS')
    parser.add_argument('--output', dest='output_log', default='log.plog', help="path to PVS log output file")
    parser.add_argument("--sln", dest="sln_path", help="path to a project solution", required=True)

    args = parser.parse_args()

    proc = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PVS-Studio_Cmd.exe",
                             "--progress",
                             "--target", args.sln_path,
                             "--output", args.output_log,
                             "--settings", "Settings.xml"])
    proc.communicate()
    if proc.returncode != 0 and proc.returncode != 7:
        sys.exit(proc.returncode)

    proc = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe",
                             "-t", "Html",
                             "-a", "GA:1",
                             "-d", "V520",
                             args.output_log])
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

    file = open("log.plog.html", "r")
    content = file.read()
    if content.find("No Messages Generated") == -1:
        errorMsg = "##teamcity[message text=\'PVS found some issues, see .plog.html for details\' errorDetails=\'\' status=\'" + "ERROR" + "\']\n"
        print errorMsg
        sys.exit(7)


if "__main__" == __name__:
    main()
