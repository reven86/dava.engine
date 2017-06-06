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

PVS_STUDIO_PATH = 'C:\Program Files (x86)\PVS-Studio'

def teamcity_message(error, errorDetails, status):
    sys.stdout.write("##teamcity[message text='{0}' errorDetails='{1}' status='{2}']\n".format(error,
                                                                                               errorDetails,
                                                                                               status))
    sys.stdout.flush()

def main():
    sys.stdout.write("##teamcity[progressStart 'PVS analyze started...']\n")
    sys.stdout.flush()

    parser = argparse.ArgumentParser(description='Analyze .sln with PVS')
    parser.add_argument('--pvsPath',
                        dest='pvs_path',
                        default=PVS_STUDIO_PATH,
                        help="path to PVS studio, default: " + PVS_STUDIO_PATH)
    parser.add_argument('--output', dest='output_log', default='log.plog', help="path to PVS log output file")
    parser.add_argument("--sln", dest="sln_path", help="path to a project solution", required=True)

    args = parser.parse_args()

    pvs_process = subprocess.Popen([args.pvs_path + "\PVS-Studio_Cmd.exe",
                             "--progress",
                             "--target", args.sln_path,
                             "--output", args.output_log,
                             "--settings", "Settings.xml"])

    pvs_process.communicate()

    return_code = pvs_process.returncode

    if return_code & ERROR_LEVEL_1:
        teamcity_message("PVS found some issues",
                         "Error (crash) during analysis of some source file(s)",
                         "ERROR")

    if return_code & ERROR_LEVEL_2:
        teamcity_message("PVS found some issues",
                         "General (nonspecific) error in the analyzer|'s operation, a possible handled exception",
                         "ERROR")

    if return_code & ERROR_LEVEL_3:
        teamcity_message("PVS found some issues",
                         "Some of the command line arguments passed to the tool were incorrect",
                         "ERROR")

    if return_code & ERROR_LEVEL_4:
        teamcity_message("PVS found some issues",
                         "Some of the analyzed source files or project files were not found",
                         "ERROR")

    if return_code & ERROR_LEVEL_5:
        teamcity_message("PVS found some issues",
                         "Specified configuration and (or) platform were not found in a solution file",
                         "ERROR")

    if return_code & ERROR_LEVEL_6:
        teamcity_message("PVS found some issues",
                         "Solution file is not supported",
                         "ERROR")

    if return_code & ERROR_LEVEL_7:
        teamcity_message("PVS found some issues",
                         "incorrect extension of analyzed project",
                         "ERROR")

    if return_code & ERROR_LEVEL_8:
        teamcity_message("PVS found some issues",
                         "Incorrect or out-of-date analyzer license",
                         "ERROR")

    if return_code & ERROR_LEVEL_9:
        teamcity_message("PVS found some issues",
                         "Some issues were found in the source code",
                         "ERROR")

    if return_code == 0:
        teamcity_message("PVS not found any issues",
                         "Analysis was successfully completed, no issues were found in the source code",
                         "SUCCESS")

        sys.exit(return_code)

    sys.stdout.write("##teamcity[progressFinish 'PVS analyze finished...']\n")
    sys.stdout.flush()

    if return_code != 0:
        converter_process = subprocess.Popen([args.pvs_path + "\PlogConverter.exe",
                                              "-t", "Html",
                                              "-a", "GA:1",
                                              "-d", "V520",
                                              args.output_log])

        converter_process.communicate()

        if converter_process.returncode != 0:
            sys.exit(converter_process.returncode)
    else:
        sys.exit(return_code)

if "__main__" == __name__:
    main()
