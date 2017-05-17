#!/usr/bin/env python
import sys

def flush_print( str ):
    sys.stdout.write("{0}\n".format(str))
    sys.stdout.flush()

def flush_print_teamcity_message( text, type, errorDetails = '' ):
    flush_print('##teamcity[message text=\'{}\' errorDetails=\'{}\' status=\'{}\']'.format( text, errorDetails, type) )

def flush_print_teamcity_set_parameter( name, value ):
    flush_print('##teamcity[setParameter name=\'{}\' value=\'{}\']'.format( name, value ) )

def get_pull_requests_number( branch ):

    if  branch == None:
        return None;

    brunch     = branch.split('/')
    branch_len = len( brunch )

    pull_requests_number = None

    if branch_len == 1:
        if brunch[0].isdigit():
            pull_requests_number = brunch[0]
    else:
        pull_requests_number = brunch[ branch_len - 2 ]

    return pull_requests_number


