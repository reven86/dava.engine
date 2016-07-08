import sys
import os.path
from optparse import OptionParser
from sets import Set
import xml.etree.ElementTree


class TeamCityOutput:
    @staticmethod
    def log_test_start(message):
        print '##teamcity[blockOpened name=\'' + message + '\']'

    @staticmethod
    def log_test_end(message):
        print '##teamcity[blockClosed name=\'' + message + '\']'

    @staticmethod
    def log(message):
        print '##teamcity[testFailed name=\'\' message=\'' + message + '\' details=\'\']'


class LogEntry:
    """Log Message Entry Representer"""

    error_status_list = ['HIGH', 'MEDIUM', 'LOW']

    def __init__(self):
        self.error_level = 0
        self.line_no = 0
        self.error_code = ''
        self.file_path = ''
        self.message = ''

    def get_status(self):
        return LogEntry.error_status_list[self.error_level - 1]

    def print_self(self):
        print 'line ' + str(self.line_no) + '. Priority ' + self.get_status() + ' . PVS Code ' + self.error_code
        print self.message
        print ' '

    @staticmethod
    def less_than_by_file(one, other):
        return one.file_path <= other.file_path

    @staticmethod
    def less_than_by_error_level(one, other):
        return one.error_level <= other.error_level

    @staticmethod
    def less_than_by_error_code(one, other):
        return one.error_code <= other.error_code


g_folders_to_report = ['sources\\internal', 'tools', 'projects']


def get_entries(log_file):
    global g_folders_to_report

    log_entries = []

    root = xml.etree.ElementTree.parse(log_file).getroot()
    for log_entry in root.iter('PVS-Studio_Analysis_Log'):
        entry = LogEntry()

        for entry_level in log_entry.iter('Level'):
            entry.error_level = int(entry_level.text)
            break
        for line_no in log_entry.iter('Line'):
            entry.line_no = int(line_no.text)
            break
        for message in log_entry.iter('Message'):
            entry.message = message.text
            break
        for error_code in log_entry.iter('ErrorCode'):
            entry.error_code = error_code.text
            break
        for file_path in log_entry.iter('File'):
            entry.file_path = file_path.text
            break

        # add only log entries related dava framework src
        for folder in g_folders_to_report:
            if folder in entry.file_path:
                log_entries.append(entry)
                break

    return log_entries


def get_error_level_count(level, log_entries):
    count = 0
    for entry in log_entries:
        if level == entry.error_level-1:
            count += 1

    return count


def report_status_to_teamcity(log_entries):
    counters = [0, 0, 0]
    for error_level in range(len(LogEntry.error_status_list)):
        error_level_entries_count = get_error_level_count(error_level, log_entries)
        counters[error_level] = error_level_entries_count

    description_string = ''
    for i in range(len(LogEntry.error_status_list)):
        description_string += LogEntry.error_status_list[i] + ': ' + str(counters[i]) + ' '

    TeamCityOutput.log('PVS Check Failed. ' + str(len(log_entries)) + ' Errors found:')
    TeamCityOutput.log(description_string)


def report_errors_to_teamcity(log_entries):
        current_file = ''

        for entry in log_entries:
            if current_file != entry.file_path:
                if current_file != '':
                    TeamCityOutput.log_test_end('')

                current_file = entry.file_path
                # log filename with one or couple of errors
                TeamCityOutput.log_test_start('In file ' + current_file)

            entry.print_self()

        TeamCityOutput.log_test_end('')


def main():
    options = OptionParser()
    options.add_option("--plog", dest="log_xml_file", default="Log/Log.plog", help="PVS studio report file.")

    (options, args) = options.parse_args(sys.argv)

    log_entries = get_entries(options.log_xml_file)

    if len(log_entries) > 0:
        # report error and some details
        report_status_to_teamcity(log_entries)

        # sort entries by filename to group them
        log_entries.sort(LogEntry.less_than_by_file)

        # report logs grouped by filename
        report_errors_to_teamcity(log_entries)

        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == '__main__':
    main()
