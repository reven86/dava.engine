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
    if proc.returncode != 0:
        print "Failed with return code %s" % proc.returncode
        raise

    proc = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe", 
        "-t", "Html",
        "-a", "GA:1",
        args.output_log])
    proc.communicate()
    if proc.returncode != 0:
        print "Failed with return code %s" % proc.returncode
        raise

if "__main__" == __name__:
    main()
