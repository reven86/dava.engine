import os;
import os.path;
import report_utils;
import platform;
import utils;
import shutil;
import subprocess;
import sys;
    
    
arguments = sys.argv[1:]

gpu = "Default"
if (len(arguments) > 0):
    gpu = arguments[0]

currentDir = os.getcwd();

data = os.path.realpath(currentDir + "/DataSource/")
input = os.path.realpath(currentDir + "/DataSource/TestData/")
output =  os.path.realpath(currentDir + "/Data/TestData")
data_folder =  os.path.realpath(currentDir + "/Data")
process = os.path.realpath(currentDir + "/DataSource/$process/")
if (platform.system() == "Darwin"):
    results = os.path.realpath(currentDir + "/Results_mac/" + gpu)
else:
    results = os.path.realpath(currentDir + "/Results/" + gpu)

tests_results = {"Tests" : {}}

print "*** DAVA AUTOTEST Cleen up working dirctories ***"

if os.path.exists(currentDir + "/Artifacts/" + gpu):
    print "Remove folder " + currentDir + "/Artifacts/" + gpu
    shutil.rmtree(currentDir + "/Artifacts/" + gpu)
    
if os.path.exists(output):
    print "Remove folder " + output
    shutil.rmtree(output)

if os.path.exists(process):
    print "Remove folder " + process
    shutil.rmtree(process)
    
if not os.path.exists(data_folder):
    print "Create folder " + data_folder
    os.mkdir(data_folder)
    
print "*** DAVA AUTOTEST Run convert_graphics.py script for %s ***" % gpu
os.chdir(data)

params = [sys.executable, 'convert_graphics.py']
if (len(arguments) > 0):
    params = params + ['-gpu', arguments[0]]

print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

print "*** DAVA AUTOTEST Check result for %s ***" % gpu
i = 0
for test in os.listdir(results):
    if(os.path.isdir(os.path.realpath(results + "/" + test))):
        i = i + 1
        result = {}
        print "*** Test#%d %s:" % (i, test)
        
        result['Name'] = test
        result['Number'] = i
        result['Success'] = True
        result['Error_msg'] = ""
        result['txt_Success'] = True
        result['tex_Success'] = True
        result['img_Success'] = True
        
        expected = os.path.realpath(results + "/" + test)
        actual = os.path.realpath(output + "/" + test)
        
        # Check TEXT files
        print "Check TXT files"
        files = filter(lambda x: x[-3:] == "txt", os.listdir(expected))
        if len(files) != 0:
            print files
        
        for file in files:
            res = utils.compare_txt(expected + "/" + file, actual + "/" + file)
            if res != None:
                result['txt_Success'] = False
                result['Error_msg'] = result['Error_msg'] + str(res) + "\n"
                print res
            
        #Check TEX files
        print "Check TEX files"
        files = filter(lambda x: x[-3:] == "tex", os.listdir(expected))
        if len(files) != 0:
            print files
        
        for file in files:
            res = utils.compare_tex(expected + "/" + file, actual + "/" + file)
            if res != None:
                result['tex_Success'] = False
                result['Error_msg'] = result['Error_msg'] + str(res) + "\n"
                print res
        
        # Check IMAGE files
        print "Check IMAGE files"
        files = filter(lambda x: x[-3:] == "png", os.listdir(expected))
        if len(files) != 0:
            print files
        
        for file in files:
            res = utils.compare_txt(expected + "/" + file, actual + "/" + file)
            if res != None:
                result['img_Success'] = False
                result['Error_msg'] = result['Error_msg'] + str(res) + "\n"
                print res
        
        files = filter(lambda x: x[-3:] == "pvr", os.listdir(expected))
        if len(files) != 0:
            print files
        
        for file in files:
            res = utils.compare_txt(expected + "/" + file, actual + "/" + file)
            if res != None:
                result['img_Success'] = False
                result['Error_msg'] = result['Error_msg'] + str(res) + "\n"
                print res

        files = filter(lambda x: x[-3:] == "dds", os.listdir(expected))
        if len(files) != 0:
            print files
        
        for file in files:
            res = utils.compare_txt(expected + "/" + file, actual + "/" + file)
            if res != None:
                result['img_Success'] = False
                result['Error_msg'] = result['Error_msg'] + str(res) + "\n"
                print res
        
        
        
        result['Success'] = result['tex_Success'] and result['txt_Success'] and result['img_Success']
        
        if result['Success']:
            print "Test passed!"
        tests_results["Tests"][test] = result
        
        #Check graphics files
        print
        print

# Make final results
test_num = 0
test_success = 0
tex_failure = 0
txt_failure = 0
img_failure = 0

for test in tests_results["Tests"].values():
    test_num = test_num + 1
        
    if test['Success']:
        test_success = test_success + 1
    
    if not test['tex_Success']:
        tex_failure = tex_failure + 1
        
    if not test['txt_Success']:
        txt_failure = txt_failure + 1
    
    if not test['img_Success']:
        img_failure = img_failure + 1

tests_results['tests'] = i
tests_results["success"] = test_success
tests_results["tex_failure"] = tex_failure
tests_results["txt_failure"] = txt_failure
tests_results["img_failure"] = img_failure
tests_results['gpu'] = gpu
        
report_utils.print_result(tests_results)
report_utils.create_html(tests_results, currentDir + "/" + gpu + ".html")

print "*** DAVA AUTOTEST Copy results for artifact storing ***"
print "Copy results for storing in TC %s -> %s" % (output, currentDir + "/Artifacts/" + gpu)
shutil.copytree(output, currentDir + "/Artifacts/" + gpu)