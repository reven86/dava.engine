
import sys
import requests
import xml.etree.ElementTree as ET


class TeamCity:
    def __init__(self, teamcity_url, login, password ):

        self.__headers      = {'Content-Type': 'application/xml'}
        self.__session      = requests.Session()
        self.__session.auth = (login, password)
        self.__base_url     = ''.join((teamcity_url, "/httpAuth/app/rest/"))

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

    def run_build( self, configuration_id,  branch_name = '', properties={} ):

        print "Launch build {} [ {} ]".format( configuration_id, branch_name )

        if properties:
            data = ''
            for key, value in properties.iteritems():
                data += '<property name="{}" value="{}"/>'.format( key, value )

            properties = '<properties>{}</properties>'.format(data)

        if branch_name:
            branch_name = ' branchName = "{}"'.format( branch_name )

        parameter =  '<build{}><buildType id="{}" />{}</build>'.format(branch_name,configuration_id, properties)

        response = self.__request("buildQueue", parameter )

        root = ET.fromstring( response.content )

        print 'Build started %s' % ( root.attrib[ 'webUrl' ] )

        return root.attrib

    def get_build_status(self, build_id ):
        response = self.__request("builds/id:{}/".format(build_id))
        root = ET.fromstring(response.content)
        statusText = None

        #statusText
        find_running_info = root.find('running-info')
        if find_running_info != None :
            find_currentStageText = find_running_info.attrib['currentStageText']
            if find_currentStageText !=  None \
               and len( find_currentStageText  ) :
                statusText = find_currentStageText

        if statusText == None :
            find_statusText = root.find('statusText')
            if find_statusText != None :
                statusText = find_statusText.text
            else:
                statusText = root.attrib[ 'state' ]

        root.attrib['statusText'] = statusText

        return root.attrib


