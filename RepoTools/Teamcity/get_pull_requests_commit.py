#!/usr/bin/env python
import sys
import os
import common_tool
import subprocess
import re

def main():

    commit = 0
    commit_log = subprocess.check_output("git log -1", shell=True)

    commit_log =  commit_log.split("\n")

    commit_log = [x.lstrip() for x in commit_log if x != '']

    auto_merge = [s for s in commit_log if 'Automatic merge' in s]

    if auto_merge:
        commit = [s for s in commit_log if '* commit' in s]
        commit = ''.join(commit).split('\'')[1]
    else:
        commit = [s for s in commit_log if 'commit' in s]
        commit = ''.join(commit).split(' ')[1]

    common_tool.flush_print_teamcity_set_parameter( 'env.from_commit', commit )

if __name__ == '__main__':
    main()
