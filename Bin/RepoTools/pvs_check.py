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
        "--output", args.output_log])
    proc.communicate()
    hadErrors = false
    if proc.returncode == 0:
        ok
    elif proc.returncode == 7:
        hadErrors = true
    else:
        return proc.returncode

    proc = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe", 
        "-t", "Html",
        "-a", "GA:1",
        args.output_log])
    proc.communicate()
    if proc.returncode != 0:
        return proc.returncode
        
    if hadErrors:
        print "PVS found some issues, see .plog.html for details"
        return 7

if "__main__" == __name__:
    main()
