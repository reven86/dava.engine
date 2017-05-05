#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api

# The script runs the teamcity-build or executes the command if the
# changes in the pull-request affect the specified folder list


def __print( str ):
    sys.stdout.write("{0}\n".format(str))
    sys.stdout.flush()


def __print_teamcity_message( text, type, errorDetails = '' ):
    __print('##teamcity[message text=\'{}\' errorDetails=\'{}\' status=\'{}\']'.format( text, errorDetails, type) )


def __print_teamcity_set_parameter( name, value ):
    __print('##teamcity[setParameter name=\'{}\' value=\'{}\']'.format( name, value ) )


def __parser_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument( '--teamcity_url', required = True )

    arg_parser.add_argument( '--stash_url', required = True )
    arg_parser.add_argument( '--stash_api_version', default = '1.0' )
    arg_parser.add_argument( '--stash_project', default = 'DF' )
    arg_parser.add_argument( '--stesh_repo_name', default = 'dava.framework' )

    arg_parser.add_argument( '--teamcity_freq_requests', default = 60, type=int  )

    arg_parser.add_argument( '--framework_brunch', required = True )
    arg_parser.add_argument( '--client_brunch' )

    arg_parser.add_argument( '--check_folders', required = True  )

    arg_parser.add_argument( '--configuration_id'  )
    arg_parser.add_argument( '--run_command'  )


    arg_parser.add_argument( '--login', required = True )
    arg_parser.add_argument( '--password', required = True )

    return arg_parser.parse_args()

def __run_build( args ):
    teamcity = team_city_api.TeamCityRequest( args.teamcity_url,
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

        teamcity_build_status =  teamcity.get_build_status( teamcity_start_result['id'] )

        build_status          = teamcity_build_status['state']

        build_status_text_old = build_status_text
        build_status_text     = teamcity_build_status['statusText']

        if build_status_text != build_status_text_old:
            __print( "{} ..".format( build_status_text ) )

        time.sleep( args.teamcity_freq_requests )



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

    stash = stash_api.StashRequest( args.stash_url,
                                    args.stash_api_version,
                                    args.stash_project,
                                    args.stesh_repo_name,
                                    args.login,
                                    args.password )


    brunch_info = stash.get_pull_requests_info( framework_brunch )

    merged_brunch = brunch_info['toRef']['id'].split('/').pop()

    if merged_brunch != 'development' :
        __print( "Build is required, because brunch_toRef == {}".format( framework_brunch ) )
        return True

    #changes folders check
    brunch_changes =  stash.get_pull_requests_changes( framework_brunch )[ 'values' ]

    depends_folders = args.check_folders.split(';')

    for path_dep_folder in depends_folders:
        for path_brunch_folder in brunch_changes:
            path                =  path_brunch_folder['path']['parent']
            path                = os.path.realpath( path )
            realpath_dep_folder = os.path.realpath(path_dep_folder)

            if realpath_dep_folder in path:
                __print( "Build is required because changes affect folders {}".format( depends_folders ) )
                return True


    if args.configuration_id != None :
        __print( "Build [{}] it is possible not to launch".format( args.configuration_id ) )

    if args.run_command != None :
        __print( "Command [{}] it is possible not to launch".format( args.run_command ) )

    if args.configuration_id == None and args.run_command != None :
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

        __print_teamcity_set_parameter( 'env.build_required', 'true' )

    else:
        __print_teamcity_set_parameter( 'env.build_required', 'false' )



if __name__ == '__main__':
    main()
