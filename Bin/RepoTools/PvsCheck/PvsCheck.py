import subprocess
import sys
import argparse

def teamcity_message(error, status):
    teamcity_error = "##teamcity[message text='{0}' status='{2}']\n" \
        .format(error, status)
    return teamcity_error

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

    if proc.returncode == 0:
        teamcity_message("analysis was successfully completed, no issues were found in the source code", "SUCCESS")
        sys.exit(proc.returncode)

    if proc.returncode == 1:
        teamcity_message("error (crash) during analysis of some source file(s)", "ERROR")

    if proc.returncode == 2:
        teamcity_message("general (nonspecific) error in the analyzer|'s operation, a possible handled exception", "ERROR")

    if proc.returncode == 4:
        teamcity_message("some of the command line arguments passed to the tool were incorrect", "ERROR")

    if proc.returncode == 8:
        teamcity_message("some of the analyzed source files or project files were not found", "ERROR")

    if proc.returncode == 16:
        teamcity_message("specified configuration and (or) platform were not found in a solution file", "ERROR")

    if proc.returncode == 32:
        teamcity_message("solution file is not supported", "ERROR")

    if proc.returncode == 64:
        teamcity_message("incorrect extension of analyzed project", "ERROR")

    if proc.returncode == 128:
        teamcity_message("incorrect or out-of-date analyzer license", "ERROR")

    if proc.returncode == 128:
        teamcity_message("some issues were found in the source code", "ERROR")


    proc = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe",
                             "-t", "Html",
                             "-a", "GA:1",
                             "-d", "V520",
                             args.output_log])
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

        
if "__main__" == __name__:
    main()
