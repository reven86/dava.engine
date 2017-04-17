#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api


def __print( str ):
    sys.stdout.write("{0}\n".format(str))
    sys.stdout.flush()


def __print_teamcity_message( text, type, errorDetails = '' ):
    __print('##teamcity[message text=\'{}\' errorDetails=\'{}\' status=\'{}\']'.format( text, errorDetails, type) )


def __parser_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument( '--teamcity_url', required = True )
    arg_parser.add_argument( '--stash_url', required = True )
    arg_parser.add_argument( '--framework_brunch', required = True )
    arg_parser.add_argument( '--client_brunch' )

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

    client_brunch = {}
    if args.client_brunch and args.client_brunch != '<default>':
        client_brunch = {'client_branch': args.client_brunch }

    teamcity_start_result = teamcity.run_build( args.configuration_id, args.framework_brunch, client_brunch  )

    build_status = ''
    build_status_text = ''

    teamcity_build_status = {}

    while  build_status != 'finished':
        time.sleep( 5 )

        teamcity_build_status =  teamcity.get_build_status( teamcity_start_result['id'] )

        build_status          = teamcity_build_status['state']

        build_status_text_old = build_status_text
        build_status_text     = teamcity_build_status['statusText']

        if build_status_text != build_status_text_old:
            __print( "{} ..".format( build_status_text ) )

    if( teamcity_build_status[ 'status' ] != 'SUCCESS' ):
        __print_teamcity_message( 'Build failed !!!', 'ERROR', teamcity_build_status['webUrl'] )


def __check_depends_of_folders( args ):
    __print( "Check depends" )

    #brunch check
    framework_brunch     =  args.framework_brunch.split('/')
    framework_brunch_len = len(framework_brunch)

    if framework_brunch_len == 1  :
        if framework_brunch[0].isdigit() :
            framework_brunch = framework_brunch[0]
        else :
            __print( "Build is required, because brunch == {}".format( framework_brunch ) )
            return True
    else :
        framework_brunch = framework_brunch[ framework_brunch_len - 2 ]

    stash = stash_api.Stash( args.stash_url,
                             args.login,
                             args.password )

    brunch_info = stash.get_pull_requests_info( framework_brunch )

    brunch_info_toRef = brunch_info['toRef']['id'].split('/').pop()

    if brunch_info_toRef != 'development' :
        __print( "Build is required, because brunch_toRef == {}".format( framework_brunch ) )
        return True

    #changes folders check
    brunch_changes = stash.get_pull_requests_changes( framework_brunch )
    branch_changes_values =  brunch_changes[ 'values' ]

    depends_folders = args.check_folders.split(';')

    for path_dep_folder in depends_folders:
        for path_brunch_folder in branch_changes_values:
            path =  path_brunch_folder['path']['parent']
            if path_dep_folder in path:
                __print( "Build is required because changes affect folders {}".format( depends_folders ) )
                return True

    __print( "Build it is possible not to launch" )

    return False

def main():

    args = __parser_args()

    check_depends = __check_depends_of_folders( args )
    if check_depends == True:
        if args.run_command != None :
            os.system( args.run_command )

        if args.configuration_id != None :
            __run_build( args )


if __name__ == '__main__':
    main()
