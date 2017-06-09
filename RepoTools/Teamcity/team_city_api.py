
import sys
import requests
import xml.etree.ElementTree as ET

__TeamCity = None

class TeamCityRequest:
    def __init__(self, teamcity_url, login, password ):

        self.__headers      = {'Content-Type': 'application/xml'}
        self.__session      = requests.Session()
        self.__session.auth = (login, password)
        self.__base_url     = ''.join((teamcity_url, "/httpAuth/app/rest/"))

    def __request(self, url, data=None):

        try:
            url = ''.join((self.__base_url, url))
            request_method = 'GET'
            if data:
                request_method = 'POST'
            response = self.__session.request(request_method, url, headers=self.__headers, data = data )
            response.raise_for_status()
            return response

        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

    def run_build( self, build_name,  branch_name = '', properties={}, triggering_options=[], agent_id='' ):

        print "Launch build {} [ {} ]".format( build_name, branch_name )

        comment = '<comment> <text>auto triggering</text> </comment>'

        if triggering_options:
            data = []
            for key  in triggering_options:
                data += [ '{}="true"'.format( key ) ]
            triggering_options = '<triggeringOptions {}/>'.format( ' '.join(data) )

        if agent_id:
            agent_id = '<agent id="{}"/>'.format( agent_id )

        if properties:
            data = ''
            for key, value in properties.iteritems():
                data += '<property name="{}" value="{}"/>'.format( key, value )
            properties = '<properties>{}</properties>'.format(data)

        if branch_name:
            branch_name = ' branchName = "{}"'.format( branch_name )

        parameter =  '<build{0}><buildType id="{1}" />{2}{3}{4}{5}</build>'.format( branch_name, build_name, properties, triggering_options,comment, agent_id )

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

        build_type = root.find( 'buildType' )


        root.attrib['project_id' ] = build_type.attrib[ 'projectId' ]
        root.attrib['config_name'] = build_type.attrib[ 'name' ]
        root.attrib['config_path'] = build_type.attrib[ 'projectName' ].replace( ' ', '' )

        root.attrib['config_path'] = '{}::{}'.format( root.attrib['config_path'] , root.attrib['config_name'] )

        return root.attrib

    def configuration_info(self, configuration_name ):
        response = self.__request("buildTypes/id:{}/".format( configuration_name ))
        root = ET.fromstring(response.content)
        root.attrib['config_path'] = root.attrib[ 'projectName' ].replace( ' ', '' )
        root.attrib['config_path'] = '{}::{}'.format( root.attrib['config_path'] , root.attrib['name'] )
        return root.attrib

    def agent_info_by_name(self, agent_name ):
        response = self.__request("agents/name:{}/".format( agent_name ))
        root = ET.fromstring( response.content )
        return root.attrib


def init( teamcity_url, login, password ):
    global __TeamCity
    __TeamCity = TeamCityRequest( teamcity_url,
                                  login,
                                  password )

def ptr():
    return __TeamCity
