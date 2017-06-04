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
    teamcity_error = "##teamcity[message text='{0}' status='{1}']\n".format(error, status)
    print (teamcity_error)

def main():
    parser = argparse.ArgumentParser(description='Analyze .sln with PVS')
    parser.add_argument('--output', dest='output_log', default='log.plog', help="path to PVS log output file")
    parser.add_argument("--sln", dest="sln_path", help="path to a project solution", required=True)

    args = parser.parse_args()

    pvs_process = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PVS-Studio_Cmd.exe",
                             "--progress",
                             "--target", args.sln_path,
                             "--output", args.output_log,
                             "--settings", "Settings.xml"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    while pvs_process.poll() is None:
        try:
            line = pvs_process.stdout.readline()
            sys.stdout.write(line)
            sys.stdout.flush()

        except IOError as err:
            sys.stdout.write(err.message)

    return_code = pvs_process.returncode

    if return_code & ERROR_LEVEL_1:
        teamcity_message("error (crash) during analysis of some source file(s)", "ERROR")

    if return_code & ERROR_LEVEL_2:
        teamcity_message("general (nonspecific) error in the analyzer|'s operation, a possible handled exception",
                         "ERROR")

    if return_code & ERROR_LEVEL_3:
        teamcity_message("some of the command line arguments passed to the tool were incorrect", "ERROR")

    if return_code & ERROR_LEVEL_4:
        teamcity_message("some of the analyzed source files or project files were not found", "ERROR")

    if return_code & ERROR_LEVEL_5:
        teamcity_message("specified configuration and (or) platform were not found in a solution file", "ERROR")

    if return_code & ERROR_LEVEL_6:
        teamcity_message("solution file is not supported", "ERROR")

    if return_code & ERROR_LEVEL_7:
        teamcity_message("incorrect extension of analyzed project", "ERROR")

    if return_code & ERROR_LEVEL_8:
        teamcity_message("incorrect or out-of-date analyzer license", "ERROR")

    if return_code & ERROR_LEVEL_9:
        teamcity_message("some issues were found in the source code", "ERROR")

    if return_code == 0:
        teamcity_message("analysis was successfully completed, no issues were found in the source code", "SUCCESS")
        sys.exit(return_code)


    converter_process = subprocess.Popen(["C:\Program Files (x86)\PVS-Studio\PlogConverter.exe",
                             "-t", "Html",
                             "-a", "GA:1",
                             "-d", "V520",
                             args.output_log], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    while converter_process.poll() is None:
        try:
            line = converter_process.stdout.readline()
            sys.stdout.write(line)
            sys.stdout.flush()

        except IOError as err:
            sys.stdout.write(err.message)

    sys.exit(return_code)

if "__main__" == __name__:
    main()
