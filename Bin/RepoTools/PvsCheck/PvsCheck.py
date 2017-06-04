import subprocess
import sys
import argparse

ERROR_LEVEL_1 = 1    # FilesFail
ERROR_LEVEL_2 = 2    # GeneralExeption
ERROR_LEVEL_3 = 4    # IncorrectArguments
ERROR_LEVEL_4 = 8    # FileNotFound
ERROR_LEVEL_5 = 16   # IncorrectConfiguration
ERROR_LEVEL_6 = 32   # IncorrectCfg
ERROR_LEVEL_7 = 64   # IncorrectExtension
ERROR_LEVEL_8 = 128  # IncorrectLicense
ERROR_LEVEL_9 = 256  # AnalysisDiff

def teamcity_message(error, status):
    teamcity_error = "##teamcity[message text='{0}' status='{2}']\n" \
        .format(error, status)
    return teamcity_error

def main():
    parser = argparse.ArgumentParser(description='Analyze .sln with PVS')
    parser.add_argument('--output', dest='output_log', default='log.plog', help="path to PVS log output file")
    parser.add_argument("--sln", dest="sln_path", help="path to a project solution", required=True)

    args = parser.parse_args()

    PvsProcess = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PVS-Studio_Cmd.exe",
        "--progress",
        "--target", args.sln_path, 
        "--output", args.output_log,
        "--settings", "Settings.xml"])
    PvsProcess.communicate()

    if PvsProcess.returncode & ERROR_LEVEL_1:
        teamcity_message("error (crash) during analysis of some source file(s)", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_2:
        teamcity_message("general (nonspecific) error in the analyzer|'s operation, a possible handled exception", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_3:
        teamcity_message("some of the command line arguments passed to the tool were incorrect", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_4:
        teamcity_message("some of the analyzed source files or project files were not found", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_5:
        teamcity_message("specified configuration and (or) platform were not found in a solution file", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_6:
        teamcity_message("solution file is not supported", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_7:
        teamcity_message("incorrect extension of analyzed project", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_8:
        teamcity_message("incorrect or out-of-date analyzer license", "ERROR")

    if PvsProcess.returncode & ERROR_LEVEL_9:
        teamcity_message("some issues were found in the source code", "ERROR")

    if PvsProcess.returncode == 0:
        teamcity_message("analysis was successfully completed, no issues were found in the source code", "SUCCESS")
        sys.exit(PvsProcess.returncode)

    ConverterProcess = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe",
                             "-t", "Html",
                             "-a", "GA:1",
                             "-d", "V520",
                             args.output_log])
    ConverterProcess.communicate()

    if PvsProcess.returncode != 0:
        sys.exit(PvsProcess.returncode)

        
if "__main__" == __name__:
    main()
