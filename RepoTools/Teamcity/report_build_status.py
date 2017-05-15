#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import stash_api
import team_city_api
import stash_api

import common_tool


def __parser_args():
    arg_parser = argparse.ArgumentParser()

    arg_parser.add_argument( '--teamcity_url', required = True )

    arg_parser.add_argument( '--stash_api_version', default = '1.0' )
    arg_parser.add_argument( '--stash_project', default = 'DF' )
    arg_parser.add_argument( '--stesh_repo_name', default = 'dava.framework' )

    arg_parser.add_argument( '--stash_url', required = True )

    arg_parser.add_argument( '--login', required = True )
    arg_parser.add_argument( '--password', required = True )

    arg_parser.add_argument( '--brunch', required = True )
    arg_parser.add_argument( '--status', required = True, choices=[ 'INPROGRESS', 'SUCCESSFUL', 'FAILED' ] )
    arg_parser.add_argument( '--configuration_id', required = True )

    arg_parser.add_argument( '--build_url' )
    arg_parser.add_argument( '--root_configuration_id' )

    arg_parser.add_argument( '--description', default = 'auto' )


    return arg_parser.parse_args()


def main():

    args = __parser_args()


    stash_api.init(     args.stash_url,
                        args.stash_api_version,
                        args.stash_project,
                        args.stesh_repo_name,
                        args.login,
                        args.password )

    team_city_api.init( args.teamcity_url,
                        args.login,
                        args.password )


    stash    = stash_api.ptr()
    teamcity = team_city_api.ptr()

    pull_requests_number = common_tool.get_pull_requests_number( args.brunch )

    if pull_requests_number != None :

        build_url = args.build_url
        if args.root_configuration_id :
            build_status = teamcity.get_build_status(args.root_configuration_id)
            build_url = build_status['webUrl']


        assert (build_url != None ), "build_url == None"

        brunch_info = stash.get_pull_requests_info( pull_requests_number )

        stash.report_build_status( args.status,
                                   args.configuration_id,
                                   args.configuration_id,
                                   build_url,
                                   brunch_info['fromRef']['latestCommit'],
                                   description=args.description )
    else:
        common_tool.__print( 'Is not a pull requests [{}]'.format( args.brunch ) )

if __name__ == '__main__':
    main()
