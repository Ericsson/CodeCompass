# -*- coding: utf-8 -*-
#
# ---- Dynamic loading every stuff under the projects/ directory ----
import sys

import importlib
from projects import __all__ as LoadableLibs

projects = importlib.import_module("projects")
AvailableProjects = []

def loadProjectByName(name):
    projectModule = importlib.import_module("projects." + project)
    clazz = getattr(projectModule, '__Project')

    return clazz

for project in LoadableLibs:
    AvailableProjects.append([project, loadProjectByName(project), None])

#
# ---- Command-line arguments ----
import argparse
import getpass

parser = argparse.ArgumentParser(description='CodeCompass auto-parser script')

parser.add_argument('-nd', '--no-download',
        dest='doDownload',
        action='store_false',
        required=False,
        help='prevents the download step from happening -- further actions will be attempted upon a previous download if available')
parser.add_argument('-nb', '--no-build',
        dest='doParse',
        action='store_false',
        required=False,
        help='prevents the building+parsing step from happening (implies --no-check)')
parser.add_argument('-nc', '--no-check',
        dest='doCheck',
        action='store_false',
        required=False,
        help='prevents the CodeChecker checking from happening')

parser.add_argument('-kd', '--keep-downloads',
        dest='keepDownload',
        action='store_true',
        required=False,
        help='don\'t delete the downloaded files when cleaning up')
parser.add_argument('-kb', '--keep-builds',
        dest='keepBuild',
        action='store_true',
        required=False,
        help='don\'t delete the built files when cleaning up')

parser.add_argument('-fc', '--force-clear',
        dest='clearAll',
        action='store_true',
        required=False,
        help='Forces the script to clear every download and build folder. Specifying this option will INHIBIT everything else. The `projects` argument will specify which projects you want to clear.')

parser.add_argument('projects', metavar='project', type=str, nargs='*',
        help='Project names to run autoparsing on - if not specified, will default to ALL projects')

class ListAction(argparse.Action):
    def __init__(self, option_strings, listing_mode = "short",
                 *args, **kwargs):
        self._Mode = listing_mode
        super(ListAction, self).__init__(option_strings=option_strings, nargs=0,
                                           *args, **kwargs)
    def __call__(self, parser, namespace, values, option_string=None):
        if (self._Mode == "short"):
            for prj, _, _ in AvailableProjects:
               print prj
        elif (self._Mode == "long"):
            print "Projects available to AutoParser (" + str(len(AvailableProjects)) + "):"

            for prj, clazz, _ in AvailableProjects:
                print prj
                print "\t" + clazz.Name
                print "\tinstalls as " + clazz.WorkspaceName
                print "\t\t" + clazz.Description
        sys.exit(0)

parser.add_argument('-l', '--list',
        action=ListAction,
            listing_mode="short",
        required=False,
        help="List available projects' names (Great for machine parsing)")

parser.add_argument('-L', '--longlist',
        action=ListAction,
            listing_mode="long",
        required=False,
        help="List available projects and their descriptions")

compassOpts = parser.add_argument_group(title='Parser options',
        description='Arguments which are passed through to `CodeCompass parse`. See `CodeCompass parse --help` for details.')
compassOpts.add_argument('--dbhost',
        dest='DBHOST',
        action='store',
        type=str,
        default='localhost',
        required=False)
compassOpts.add_argument('--dbport',
        dest='DBPORT',
        action='store',
        type=int,
        default=5432,
        required=False)
compassOpts.add_argument('--dbuser',
        dest='DBUSER',
        action='store',
        type=str,
        default=getpass.getuser(),
        required=False)
compassOpts.add_argument('--dbpass',
        dest='DBPASS',
        action='store',
        type=str,
        required=False)
compassOpts.add_argument('-i', '--installdir',
        dest='installdir',
        action='store',
        type=str,
        required=False)
compassOpts.add_argument('-w', '--workdir',
        dest='WORKDIR',
        action='store',
        type=str,
        required=False)

serverOpts = parser.add_argument_group(title='Webserver options',
        description="(The webserver inherits the appropriate \"shared\" options from parser, like --db* and --workdir.) See `CodeCompass server --help` for details.")
serverOpts.add_argument('-p', '--port',
        dest='webPORT',
        action='store',
        type=int,
        required=False)

checkerOpts = parser.add_argument_group(title='CodeChecker options')
checkerOpts.add_argument('-c', '--checkerdir',
        dest='checkerdir',
        action='store',
        type=str,
        required=False,
        help='CodeChecker install directory')

args = parser.parse_args()

#
# ---- Argument handling ----
if not args.clearAll:
    if args.doParse and args.installdir is None:
        print "Will not start an autoparser run because CodeCompass installation folder is not present!"
        print "Please specify the -i/--installdir command-line argument"
        sys.exit(1)

    if args.doParse and args.WORKDIR is None:
        print "Will not start an autoparser run because workdir path is not present!"
        print "Please specify the -w/--workdir command-line argument"
        sys.exit(1)

    if not (args.doParse or args.doDownload):
        print "Both --no-download and --no-parse were specified."
        print "I've got nothing to do."
        sys.exit(2)

if args.clearAll or not args.doParse:
    if not args.installdir:
        args.installdir = ""

    if not args.WORKDIR:
        args.WORKDIR = ""

ProjectsToHandle = []
if not args.projects:
    print "Projects were not specified."
    ProjectsToHandle = AvailableProjects
else:
    print "Some projects were specified: " + ", ".join(args.projects)
    # Keep ordering!
    args.projects = [i for n, i in enumerate(args.projects) if i not in args.projects[:n]]

    for project in args.projects:
        result = filter(lambda avProject : avProject[0] == project, AvailableProjects)
        if result:
            print "Scheduling project ‘" + project + "’ for autoparser..."
            ProjectsToHandle.append(result[0])
        else:
            print "Skipping project ‘" + project + "’ - unknown project."

if not ProjectsToHandle:
    print "No known projects were specified."
    print "I've got nothing to do."
    sys.exit(2)

# A little bit of override for --force-clear to work
if args.clearAll:
    args.doDownload = args.doParse = args.doCheck = False

if args.doCheck and not args.checkerdir:
    print "--checkerdir was not specified."
    print "[STATUS] Implying --no-check from now on."
    args.doCheck = False

if not args.doParse and args.doCheck:
    print "--no-parse was specified, but --no-check was not."
    print "[STATUS] Implying --no-check from now on."
    args.doCheck = False

# Create a list of shared args
_SharedCCArgs = ["--workdir", args.WORKDIR]
if args.DBHOST:
    _SharedCCArgs.append("--dbhost")
    _SharedCCArgs.append(args.DBHOST)
if args.DBPORT:
    _SharedCCArgs.append("--dbport")
    _SharedCCArgs.append(str(args.DBPORT))
if args.DBUSER:
    _SharedCCArgs.append("--dbuser")
    _SharedCCArgs.append(args.DBUSER)
if args.DBPASS:
    _SharedCCArgs.append("--dbpass")
    _SharedCCArgs.append(args.DBPASS)

#
# ---- Action handling ----
import os, os.path
import subprocess
from datetime import datetime
import time

args.installdir = os.path.abspath(args.installdir)
args.WORKDIR = os.path.abspath(args.WORKDIR)

# Check if CodeCompass is available
_CompassBinary = ""
if args.doParse:
    _CompassBinary = os.path.join(args.installdir, 'bin/CodeCompass')
    try:
        f = open(os.devnull, 'w')
        testArgs = [_CompassBinary]

        subprocess.call(testArgs, stderr = f, stdout = f)
    except OSError:
        print "[CRITICAL] CodeCompass is not available. Did you specify `--installdir` properly?"
        sys.exit(3)

# Check if CodeChecker is available
import socket

_CheckerBinary = ""
_CheckerPID = -1
_CheckerSrvPort = -1
if args.doCheck:
    args.checkerdir = os.path.abspath(args.checkerdir)
    _CheckerBinary = os.path.join(args.checkerdir, 'bin/CodeChecker')

    f = open(os.devnull, 'w')
    try:
        testArgs = [_CheckerBinary]

        subprocess.call(testArgs, stderr = f, stdout = f)
    except OSError:
        print "\n[ERROR] CodeChecker was not found properly, but `--no-check` was not supplied."
        print "Did you specify `--checkerdir` correctly?"
        print "\n[STATUS] Implying `--no-check` from now on..."
        args.doCheck = False

    f.close()

    if args.checkerdir and not os.environ.get("CODECHECKER_HOME"):
        print "CODECHECKER_HOME environmental variable was missing. Setting it to --checkerdir ", args.checkerdir
        os.environ["CODECHECKER_HOME"] = args.checkerdir

    print "[CodeChecker] Starting server..."
    __dummy_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    __dummy_socket.bind(("", 0))
    __dummy_socket.listen(1)
    _CheckerSrvPort = str(__dummy_socket.getsockname()[1])
    __dummy_socket.close()      # Close the server here to make sure the port did not get bound in the meantime

    checkerArgs = [_CheckerBinary, "server", "--postgresql", "--view-port ", _CheckerSrvPort]
    # CodeChecker uses --dbaddress not --dbhost...
    checkerArgs = checkerArgs + [
        "--dbaddress" if x == "--dbhost"
        else "--workspace" if x == "--workdir"
        else x for x in _SharedCCArgs
    ] + [
          # We need to save the checker's child's PID as that process is the one we are able to kill later
          "</dev/null &>/dev/null & echo \"$!\" > .pidfile"
    ]

    command = " ".join(checkerArgs)
    processArgs = ["bash", "-c", command]
    subprocess.call(processArgs)

    with open(".pidfile") as f:
        _CheckerPID = f.read().replace("\n", "")
    subprocess.call(["rm", ".pidfile"])


_StartLocation = os.getcwd()

#
# Wrapper functions

def serverStop():
    os.chdir(args.WORKDIR)
    serverArgs = [_CompassBinary, "server", "--stop"]
    serverArgs = serverArgs + _SharedCCArgs
    subprocess.call(serverArgs)
    os.chdir(_StartLocation)
    print "[SERVER] Server killed."

def serverStart():
    os.chdir(args.WORKDIR)
    serverArgs = [_CompassBinary, "server", "--port", str(args.webPORT)]
    serverArgs = serverArgs + _SharedCCArgs
    serverArgs = serverArgs + ["</dev/null &>/dev/null &!"]

    command = " ".join(serverArgs)

    processArgs = ["bash", "-c", command]

    subprocess.call(processArgs)
    os.chdir(_StartLocation)
    print "[SERVER] Server started on port ", args.webPORT

def download(project, obj):
    os.chdir(_StartLocation)
    print datetime.now(), time.time(), "Downloading ‘" + project + "’..."

    if os.path.isfile(os.path.join(obj.getDownloadFolder(), ".cc-autoparser")):
        print "\tDownload folder already exists!"
        print "\t# Executing cleanup/download..."
        cleanup(project, obj, ["download"])

    success = obj.download()

    if not success:
        print "Downloading ‘" + project + "’ FAILED"
    else:
        # Mark the download folder downladed :)
        hndl = open(os.path.join(obj.getDownloadFolder(), ".cc-autoparser"), 'wb')
        hndl.write(datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S'))
        hndl.close()

    obj.canPrepareBuild = success

def prepareBuild(project, obj):
    os.chdir(_StartLocation)
    print datetime.now(), time.time(), "Preparing ‘" + project + "’ for build..."
    success = obj.prepare()

    if not success:
        print "Preparing the build for ‘" + project + "’ FAILED ..."

    obj.isBuildPrepared = success


def build(project, obj):
    os.chdir(_StartLocation)
    print datetime.now(), time.time(), "Building ‘" + project + "’ ..."
    parseArgs = []
    obj.build(parseArgs)

    parseArgsReal = [_CompassBinary, "parse", "--name", '"' + obj.WorkspaceName + "_tmp" + '"', "--description", '"' + obj.Description + '"']
    parseArgsReal = parseArgsReal + _SharedCCArgs + parseArgs

    os.chdir(obj.getProjectFolder())
    print "Running parse command: ", parseArgsReal

    success = subprocess.call(["bash", '-c', " ".join(parseArgsReal)])
    if success == 0:
        obj.isSuccessfullyParsed = True

    os.chdir(os.path.join(_StartLocation, obj.getProjectFolder()))
    print datetime.now(), time.time(), "Executing post-build actions..."
    obj.postBuild()

    os.chdir(_StartLocation)

def deploy(project, obj):
    '''Makes sure CodeCompass can actually see the project after parsing'''
    print datetime.now(), time.time(), "Deploying ‘" + project + "’ ..."

    p = subprocess.Popen([_CompassBinary, "workspace", "--workdir", args.WORKDIR, "--list"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = p.communicate()
    rc = p.returncode

    if rc == 0:
        if obj.WorkspaceName in output.split('\n'):
            # Need to remove the old workspace...
            remArgs = [_CompassBinary, "workspace", "--delete", obj.WorkspaceName]
            remArgs = remArgs + _SharedCCArgs
            subprocess.call(remArgs)
    else:
        print "Communication with CodeCompass failed! Did you set the environment properly?!"
        print err

    renArgs = [_CompassBinary, "workspace", "--rename", obj.WorkspaceName + "_tmp", obj.WorkspaceName]
    renArgs = renArgs + _SharedCCArgs
    subprocess.call(renArgs)

    # Check if the database was really renamed (or created)
    p = subprocess.Popen([_CompassBinary, "workspace", "--workdir", args.WORKDIR, "--list"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = p.communicate()
    rc = p.returncode

    if args.doCheck and obj.isChecked and obj.checkerRunnameOld:
        remArgs = [_CheckerBinary, "cmd", "del", "--port", _CheckerSrvPort, "-n", obj.checkerRunnameOld, "</dev/null &>/dev/null"]
        subprocess.call(["bash", "-c", " ".join(remArgs)])

    obj.isDeployedProperly = (rc == 0 and obj.WorkspaceName in output.split('\n') and str(obj.WorkspaceName + "_tmp") not in output.split('\n'))

def _calc_checker_runnames(json, projectList):
    # For optimisation reasons we don't want the server to be dead for the ENTIRE checking process...
    # But we can't allow CodeChecker to remove or modify the data which is used by a fully concluded parse
    #
    # So we do a little magic of always checking the current "_tmp" parse with an alternating name and hot-wiring CodeCompass
    # to use this as a runname. After every successful parse the alternate run is destroyed.
    if not args.doCheck:
        return

    runNames = []
    for runs in json:
        runNames.append(next(iter(runs.keys())))

    for _, _, obj in projectList:
        if obj.WorkspaceName + "foo" in runNames:
            obj.checkerRunnameOld = obj.WorkspaceName + "foo"
            obj.checkerRunname = obj.WorkspaceName + "bar"
        elif obj.WorkspaceName + "bar" in runNames:
            obj.checkerRunnameOld = obj.WorkspaceName + "bar"
            obj.checkerRunname = obj.WorkspaceName + "foo"
        else:
            # Maybe this is the first checking for the project?
            obj.checkerRunnameOld = None
            obj.checkerRunname = obj.WorkspaceName + "foo"

def checker(project, obj):
    if not args.doCheck or not obj.isSuccessfullyParsed:
        return

    print datetime.now(), time.time(), "Running CodeChecker for ‘" + project + "’ ..."
    checkArgs = [_CompassBinary, "codechecker", "--name", '"' + obj.WorkspaceName + '_tmp"', "--runname", obj.checkerRunname, ""'--extra "\-\-force"']
    checkArgs = checkArgs + _SharedCCArgs

    print "Executing CodeChecker: ", checkArgs

    return_code = subprocess.call(["bash", "-c", " ".join(checkArgs)])
    obj.isChecked = return_code == 0

def cleanup(project, obj, stages):
    if not stages: return

    for stage in stages:
        print datetime.now(), time.time(), "Cleaning up ‘" + project + "’/" + stage + " ..."

        if stage == "download":
            obj.cleanDownload()

        if stage == "build":
            obj.cleanBuild()

#
# Real handling.
import json
_AnySuccessInParsing = False

for arr in ProjectsToHandle:
        project = arr[0]
        obj = arr[1](args)

        # Initialise some default values
        obj.canPrepareBuild = obj.isBuildPrepared = obj.isSuccessfullyParsed = obj.isChecked = obj.isDeployedProperly = False
        obj.checkerRunnameOld = obj.checkerRunname = None

        arr[2] = obj

        if not args.clearAll:
            print
            print "# Selecting previously unselected project ‘" + project + "’ for downloading ..."

            if args.doDownload:
                download(project, obj)
            else:
                try:
                    hndl = open(os.path.join(obj.getDownloadFolder(), ".cc-autoparser"), 'rb')
                except IOError:
                    print "‘" + project + "’ has no downloaded contents available!... Skipping."
                    continue

                print 'Using previous download cache of ' + hndl.read() + '...'
                obj.canPrepareBuild = True

            print "# Unselecting project ‘" + project + "’."

if not args.clearAll:
    print "\n"
    if args.doParse:
        for project, _, obj in ProjectsToHandle:
            print
            print "# Selecting project ‘" + project + "’ for parsing ..."

            if obj.canPrepareBuild:
                prepareBuild(project, obj)
            else:
                print "‘" + project + "’ did not enter prebuildable state. Skipping prebuild..."
                continue

            if obj.isBuildPrepared:
                build(project, obj)

                if obj.isSuccessfullyParsed:
                    _AnySuccessInParsing = True
            else:
                print "‘" + project + "’ could not be prepared for build. Skipping parse..."
                continue

            print "# Unselecting project ‘" + project + "’."

    print "\n"
    if _AnySuccessInParsing and args.doCheck:
        print "# Executing CodeChecker runs..."

        # Try to wait for CodeChecker to stand up...
        iterCount = 0
        _CheckerJSON = None
        while iterCount < 20:
            p = subprocess.Popen([_CheckerBinary, "cmd", "runs", "--port", _CheckerSrvPort, "-o", "json"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output, err = p.communicate()
            rc = p.returncode

            if rc == 0:
                _CheckerJSON = output
                iterCount = 21         # Kill the loop and make sure we have the JSON
            else:
                iterCount = iterCount + 1
                time.sleep(0.5)          # Wait for CodeChecker to come alive
                print "[CodeChecker] Attempt", iterCount, "of communicating with server..."

        if not _CheckerJSON:
            print "[CodeChecker] Server did not fire up or give a usable response in an expectable timeframe."
            print "Implying `--no-check` from now on..."
            args.doCheck = False
            _CheckerPID = _CheckerSrvPort = -1

        _calc_checker_runnames(json.loads(_CheckerJSON), ProjectsToHandle)

        for project, _, obj in ProjectsToHandle:
            if obj.isSuccessfullyParsed and args.doCheck:
                checker(project, obj)

    print "\n"
    if _AnySuccessInParsing:
        serverStop()

        for project, _, obj in ProjectsToHandle:
            print "# Selecting project ‘" + project + "’ ..."
            if obj.isSuccessfullyParsed:
                deploy(project, obj)
                if not obj.isDeployedProperly:
                    print "‘" + project + "’ deployment failed!"
            else:
                print "‘" + project + "’ did not parse successfuly. Skipping..."

            print "# Unselecting project ‘" + project + "’"

        if args.doCheck and _CheckerPID != -1:
            subprocess.call(["bash", "-c", "kill " + _CheckerPID])
            print "[CodeChecker] Server killed"

        serverStart()

    print "\n"

if _AnySuccessInParsing or args.clearAll:
    print "# Cleanup..."

    for project, _, obj in ProjectsToHandle:
        stages = ["download", "build"]

        if args.keepDownload:
            stages.remove("download")

        if args.keepBuild:
            stages.remove("build")

        cleanup(project, obj, stages)

    print "# Cleanup over."
