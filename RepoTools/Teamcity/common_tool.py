#!/usr/bin/env python
import sys

def __print( str ):
    sys.stdout.write("{0}\n".format(str))
    sys.stdout.flush()

def __print_teamcity_message( text, type, errorDetails = '' ):
    __print('##teamcity[message text=\'{}\' errorDetails=\'{}\' status=\'{}\']'.format( text, errorDetails, type) )

def __print_teamcity_set_parameter( name, value ):
    __print('##teamcity[setParameter name=\'{}\' value=\'{}\']'.format( name, value ) )

def get_pull_requests_number( brunch ):
    brunch     = brunch.split('/')
    brunch_len = len( brunch )

    pull_requests_number = None

    if brunch_len == 1:
        if brunch[0].isdigit():
            pull_requests_number = brunch[0]
    else:
        pull_requests_number = brunch[ brunch_len - 2 ]

    return pull_requests_number


