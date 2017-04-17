#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api

def __parser_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument( '--teamcity_url', required = True )
    arg_parser.add_argument( '--stash_url', required = True )
    arg_parser.add_argument( '--brunch', required = True )
    arg_parser.add_argument( '--check_folders', required = True  )

    arg_parser.add_argument( '--configuration_id'  )
    arg_parser.add_argument( '--run_command'  )


    arg_parser.add_argument( '--login', required = True )
    arg_parser.add_argument( '--password', required = True )

    return arg_parser.parse_args()

def __run_build( args ):
    teamcity = team_city_api.TeamCity( args.teamcity_url,
                                       args.login,
                                       args.password )

    teamcity_start_result = teamcity.run_build( args.configuration_id, args.brunch  )

    build_status = ''
    build_status_text = ''
    while  build_status != 'finished':
        time.sleep( 5 )

        teamcity_build_status =  teamcity.get_build_status( teamcity_start_result['id'] )
        build_status          = teamcity_build_status['state']

        build_status_text_old = build_status_text
        build_status_text     = teamcity_build_status['statusText']

        if build_status_text != build_status_text_old:
            print build_status_text, ' ..'


def __check_depends_of_folders( args ):

    print 'Check depends'

    #brunch check
    brunch     =  args.brunch.split('/')
    brunch_len = len(brunch)

    if brunch_len == 1  :
        if brunch[0].isdigit() :
            brunch = brunch[0]
        else :
            print 'Build is required, because brunch == ', brunch
            return True
    else :
        brunch = brunch[ brunch_len - 2 ]

    stash = stash_api.Stash( args.stash_url,
                             args.login,
                             args.password )

    brunch_info = stash.get_pull_requests_info( brunch )

    brunch_info_toRef = brunch_info['toRef']['id'].split('/').pop()

    if brunch_info_toRef != 'development' :
        print 'Build is required, because brunch_toRef == ', brunch
        return True

    #changes folders check
    brunch_changes = stash.get_pull_requests_changes( brunch )
    branch_changes_values =  brunch_changes[ 'values' ]

    depends_folders = args.check_folders.split(';')

    print depends_folders

    for path_dep_folder in depends_folders:
        for path_brunch_folder in branch_changes_values:
            path =  path_brunch_folder['path']['parent']
            if path_dep_folder in path:
                print 'Build is required because changes affect folders', depends_folders
                return True

    print 'Build it is possible not to launch'
    return False

def main():

    args = __parser_args()

    check_depends = __check_depends_of_folders(args)
    if check_depends == True:
        if args.run_command != None :
            os.system( args.run_command )

        if args.configuration_id != None :
            __run_build( args )


if __name__ == '__main__':
    main()
