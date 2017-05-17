#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api
import common_tool

# The script runs the teamcity-build or executes the command if the
# changes in the pull-request affect the specified folder list

def __parser_args():
    arg_parser = argparse.ArgumentParser()

    arg_parser.add_argument( '--stash_api_version', default = '1.0' )
    arg_parser.add_argument( '--stash_project', default = 'DF' )
    arg_parser.add_argument( '--stash_repo_name', default = 'dava.framework' )

    arg_parser.add_argument( '--stash_url', required = True )
    arg_parser.add_argument( '--teamcity_url', required = True )

    arg_parser.add_argument( '--login', required = True )
    arg_parser.add_argument( '--password', required = True )
##
    arg_parser.add_argument( '--convert_to_merge_requests', default = 'false', choices=[ 'true', 'false' ] )
    arg_parser.add_argument( '--framework_branch', required = True )
    arg_parser.add_argument( '--client_brunch' )
##
    arg_parser.add_argument( '--check_folders', required = True  )
##
    arg_parser.add_argument( '--configuration_name'  )
    arg_parser.add_argument( '--root_configuration_id', required = True )
    arg_parser.add_argument( '--request_configuration_id'  )

    arg_parser.add_argument( '--request_stash_mode', default = 'false', choices=[ 'true', 'false' ] )
    arg_parser.add_argument( '--teamcity_freq_requests', default = 60, type = int  )
##
    arg_parser.add_argument( '--run_command'  )

    return arg_parser.parse_args()


def __run_build( args, triggering_options = [] ):
    teamcity = team_city_api.ptr()

    client_brunch    = args.client_brunch
    framework_branch = args.framework_branch

    if args.convert_to_merge_requests == 'true':
        if client_brunch and '/from' in client_brunch:
            client_brunch = client_brunch.replace('/from', '/merge')

        if framework_branch and '/from' in framework_branch:
            framework_branch = framework_branch.replace('/from', '/merge')

    properties = {}
    if client_brunch and client_brunch != '<default>':
        properties = {'config.client_branch': client_brunch}

    run_build_result = teamcity.run_build( args.configuration_name, framework_brunch, properties, triggering_options  )

    return run_build_result


def __wait_end_build( args, build_id ):#run_build_result['id']
    teamcity = team_city_api.ptr()

    build_status = ''
    build_status_text = ''

    teamcity_build_status = {}

    while  build_status != 'finished':

        teamcity_build_status =  teamcity.get_build_status( build_id  )

        build_status          = teamcity_build_status['state']

        build_status_text_old = build_status_text
        build_status_text     = teamcity_build_status['statusText']

        if build_status_text != build_status_text_old:
            common_tool.flush_print( "{} ..".format( build_status_text ) )

        time.sleep( args.teamcity_freq_requests )

    if( teamcity_build_status[ 'status' ] != 'SUCCESS' ):
        common_tool.flush_print_teamcity_message( 'Build failed !!!', 'ERROR', teamcity_build_status['webUrl'] )


def __check_depends_of_folders( args ):
    common_tool.flush_print( "Check depends" )

    stash = stash_api.ptr()

    pull_requests_number = common_tool.get_pull_requests_number( args.framework_brunch )

    if pull_requests_number == None  :
        common_tool.flush_print( "Build is required, because brunch == {}".format( args.framework_brunch ) )
        return True,None

    brunch_info = stash.get_pull_requests_info( pull_requests_number )

    merged_brunch = brunch_info['toRef']['id'].split('/').pop()

    if merged_brunch != 'development' :
        common_tool.flush_print( "Build is required, because brunch_toRef == {}".format( pull_requests_number ) )
        return True, brunch_info

    #changes folders check
    brunch_changes =  stash.get_pull_requests_changes( pull_requests_number )[ 'values' ]

    depends_folders = args.check_folders.split(';')

    for path_dep_folder in depends_folders:
        for path_brunch_folder in brunch_changes:
            path                =  path_brunch_folder['path']['parent']
            path                = os.path.realpath( path )
            realpath_dep_folder = os.path.realpath(path_dep_folder)

            if realpath_dep_folder in path:
                common_tool.flush_print( "Build is required because changes affect folders {}".format( depends_folders ) )
                return True, brunch_info

    if args.configuration_name != None :
        common_tool.flush_print( "Build [{}] it is possible not to launch".format( args.configuration_name ) )

    if args.run_command != None :
        common_tool.flush_print( "Command [{}] it is possible not to launch".format( args.run_command ) )

    if args.configuration_name == None and args.run_command != None :
        common_tool.flush_print( "Build it is possible not to launch" )

    return False, brunch_info

def main():

    args = __parser_args()

    stash_api.init(     args.stash_url,
                        args.stash_api_version,
                        args.stash_project,
                        args.stash_repo_name,
                        args.login,
                        args.password )

    team_city_api.init( args.teamcity_url,
                        args.login,
                        args.password )

    stash    = stash_api.ptr()

    teamcity = team_city_api.ptr()

    request_configuration_id = args.request_configuration_id

    if request_configuration_id == None:
        request_configuration_id = args.configuration_name

    request_configuration_info = None
    if args.request_stash_mode == 'true' and request_configuration_id :
        request_configuration_info = teamcity.get_build_status( request_configuration_id )

    check_depends, brunch_info = __check_depends_of_folders( args )
    if check_depends == True:
        if args.run_command != None :
            os.system( args.run_command )

        if args.configuration_name != None :
            run_build_result = __run_build( args, ['queueAtTop'] )

            if args.request_stash_mode == 'true' :
                if common_tool.get_pull_requests_number(args.framework_brunch) != None :
                    stash.report_build_status('INPROGRESS',
                                              request_configuration_id,
                                              request_configuration_info['config_path'],
                                              run_build_result['webUrl'],
                                              brunch_info['fromRef']['latestCommit'],
                                              description="runing")
            else:
                __wait_end_build( args, run_build_result['id'] )

        common_tool.flush_print_teamcity_set_parameter( 'env.build_required', 'true' )

    else:

        if brunch_info != None and args.request_stash_mode == 'true' and request_configuration_id:

            build_status = teamcity.get_build_status( args.root_configuration_id )

            root_build_url = build_status['webUrl']

            stash.report_build_status('SUCCESSFUL',
                                      request_configuration_id,
                                      request_configuration_info['config_path'],
                                      root_build_url,
                                      brunch_info['fromRef']['latestCommit'],
                                      description="Tests were ignored due to changed files")

        common_tool.flush_print_teamcity_set_parameter( 'env.build_required', 'false' )


if __name__ == '__main__':
    main()
