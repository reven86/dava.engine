
import sys
import requests
import json

class StashRequest:
    def __init__(self, stash_url,stash_api_version, stash_project, stesh_repo_name, login, password ):

        self.__headers      = {'Content-Type': 'application/json'}
        self.__session      = requests.Session()
        self.__session.auth = (login, password)
        self.__base_url     = ''.join((stash_url, "/rest/api/{}/projects/{}/repos/{}/".format( stash_api_version, stash_project, stesh_repo_name ) ) )

    def __request(self, uri, data=None):

        try:
            url = ''.join((self.__base_url, uri))
            request_method = 'GET'
            if data:
                request_method = 'POST'
            response = self.__session.request(request_method, url, headers=self.__headers, data = data )
            response.raise_for_status()
            return response

        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

    def get_pull_requests_info(self, pull_requests ):
        response = self.__request("pull-requests/{}".format(pull_requests))
        return json.loads( response.content )

    def get_pull_requests_changes(self, pull_requests ):
        response = self.__request("pull-requests/{}/changes".format(pull_requests))
        return json.loads( response.content )