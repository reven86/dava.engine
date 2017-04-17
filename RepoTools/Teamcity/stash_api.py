
import sys
import requests
import json

class Stash:
    def __init__(self, stash_url, login, password ):

        self.__headers      = {'Content-Type': 'application/json'}
        self.__session      = requests.Session()
        self.__session.auth = (login, password)
        self.__base_url     = ''.join((stash_url, "/rest/api/1.0/projects/DF/repos/dava.framework/"))

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