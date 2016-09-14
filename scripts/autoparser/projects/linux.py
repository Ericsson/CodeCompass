import json
import projects
import os.path
import re
import subprocess
import time
from datetime import datetime

class pathFixer_Replacer:
    Pattern = "-Wp,-MD,(.+?) "

    def __init__(self, directory):
        self.directory = directory

    def __call__(self, matchObject):
        return '-Wp,-MD,' + os.path.join(self.directory, matchObject.group(1)) + ' '

def pathFixer(inputPath, outputPath):
    with open(inputPath, 'r') as f:
        data = json.load(f)

    for action in data:
        action['command'] = re.sub(
            pathFixer_Replacer.Pattern,
            pathFixer_Replacer(action['directory']),
            action['command']
        )

    with open(outputPath, 'w') as f:
        json.dump(data, f, indent = 4)

class linux(projects.Project):
    Name = "LLVMLinux"
    WorkspaceName = "llvmlinux"
    Description = "LLVMLinux kernel: Linux modified to be buildable without GNU extensions."

    def __init__(self, argsArr):
        self.argsArr = argsArr
        pass

    def getDownloadFolder(self):
        return "linuxllvm"

    def download(self):
        return_code = subprocess.call(["git", "clone", "git://git.linuxfoundation.org/llvmlinux/kernel.git", "--origin", "upstream", "--single-branch", "--depth", str(1), self.getDownloadFolder()])
        if return_code == 0:
            # There is nothing to do when the download is successful, at least for now.
            pass
        else:
            return False

        return True

    def cleanDownload(self):
        subprocess.call(["rm", "-fr", self.getDownloadFolder()])

    def prepare(self):
        subprocess.call(["cp", "-R", self.getDownloadFolder(), os.path.abspath(self.getProjectFolder())])

        os.chdir(self.getProjectFolder())

        rmscript = open('../rm', "w")
        print >>rmscript, '''#!/bin/bash

if [ "${ORIGINAL_RM}" == "" ]
then
  echo "Whoah! Cannot emulate "'`rm`'" as I was not given the 'ORIGINAL_RM' environmental variable!"
  exit 1
fi

function actually_remove()
{
  if [[ "$2" =~ ^.*\.d$ ]]
  then
    # The Linux Makefile removes dependency lists which are required by the LLVM/Clang parser - thus rendering the whole thing inapplicable.
    # We prevent this!
    echo "Not going to remove $2" >&2
    echo "# not removing $2" >> .rm.log

    #return 1
  else
    COMMAND=""
    if [ "$1" != "" ]
    then
      COMMAND=${COMMAND}" $1"
    fi
    COMMAND=${COMMAND}" $2"

    echo "rm ${COMMAND}" >&2
    echo "rm ${COMMAND}" >> .rm.log

    ${ORIGINAL_RM} ${COMMAND}

    return $?
  fi
}

if [ $# -eq 1 ]
then
  actually_remove "" $1
else
  RF_ARG=""
  if [[ "$1" =~ ^(-r|-f|-rf|-fr)$ ]]
  then
    RF_ARG="$1"
    shift 1
  fi

  RETURN_CODE=0
  for FILE in "$@"
  do
    actually_remove ${RF_ARG} "$FILE"
    
    if [ $? -ne 0 ]
    then
      RETURN_CODE=1
    fi
  done
fi

exit $RETURN_CODE'''
        rmscript.close()

        builderFile = open("../autoparse." + self.WorkspaceName + ".build.sh", "w")
        print >>builderFile, "export ORIGINAL_RM=`which rm`"
        print >>builderFile, "make defconfig"
        print >>builderFile, "make silentoldconfig"
        print >>builderFile, "export PATH=\"" + os.path.abspath("..") + ":$PATH\""
        print >>builderFile, "make HOSTCC=clang CC=clang"
        print >>builderFile, 'if [ $? -ne 0 ]; then exit 1; fi'
        print >>builderFile, ''
        print >>builderFile, 'exit 0'
        builderFile.close()

        subprocess.call(["chmod", "u+x", "../autoparse." + self.WorkspaceName + ".build.sh"])
        subprocess.call(["chmod", "u+x", "../rm"])

        return True

    def getProjectFolder(self):
        return "linux_build"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])
        subprocess.call(["rm", "autoparse." + self.WorkspaceName + ".build.sh"])
        subprocess.call(["rm", "rm"])
        subprocess.call(["rm", "autoparse." + self.WorkspaceName + ".build.log"])
        subprocess.call(["rm", "autoparse." + self.WorkspaceName + ".build.json"])

    def build(self, parseArgs):
        # Linux needs a little bit of caressing...
        # The generated build log contains relative paths to the aforementioned dependency files which scares the living shit out of Clang.
        _StartLocation = os.getcwd()

        # So we break the build into two parts.
        # First, only log the actions
        buildArgs = [
            os.path.join(self.argsArr.installdir, "bin", "CodeCompass"),
            "parse",
            "--workdir",
            self.argsArr.WORKDIR,
            "--name",
            '"' + self.WorkspaceName + "_tmp_logging" + '"',
            "--force",
            "--logonly",
            '"../autoparse.' + self.WorkspaceName + '.build.sh"'
        ]

        if self.argsArr.DBHOST:
            buildArgs.append("--dbhost")
            buildArgs.append(self.argsArr.DBHOST)
        if self.argsArr.DBPORT:
            buildArgs.append("--dbport")
            buildArgs.append(str(self.argsArr.DBPORT))
        if self.argsArr.DBUSER:
            buildArgs.append("--dbuser")
            buildArgs.append(self.argsArr.DBUSER)
        if self.argsArr.DBPASS:
            buildArgs.append("--dbpass")
            buildArgs.append(self.argsArr.DBPASS)

        os.chdir(self.getProjectFolder())
        print "BUILDING Linux kernel... Not doing a parse in this run."
        print "Build started at", datetime.now(), " ".join(["(epoch ", time.time(), ")"])
        subprocess.call(["bash", '-c', " ".join(buildArgs)])
        print "Build finished at ", datetime.now(), " ".join(["(epoch ", time.time(), ")"])
        os.chdir(_StartLocation)

        buildJSON = os.path.join(self.argsArr.WORKDIR, self.WorkspaceName + "_tmp_logging", "build.json")

        # Parse the JSON and fix the relative paths.
        pathFixer(buildJSON, "autoparse." + self.WorkspaceName + ".build.json")

        subprocess.call(["mv", os.path.abspath(os.path.join(self.argsArr.WORKDIR, self.WorkspaceName + "_tmp_logging")), \
            "autoparse." + self.WorkspaceName + ".build.log"])
        subprocess.call(["rm", "-r", os.path.join(self.argsArr.WORKDIR, self.WorkspaceName + "_tmp_logging")])

        print "PARSING Linux kernel..."
        parseArgs.append("--logfile")
        parseArgs.append(os.path.abspath("autoparse." + self.WorkspaceName + ".build.json"))
        parseArgs.append("--labels")
        parseArgs.append('sourcedir=' + os.path.abspath(self.getProjectFolder()))
        print "Starting parse at... ", datetime.now(), " ".join(["(epoch ", time.time(), ")"])

    def postBuild(self):
        # Save build files to make sure the project is CodeCheckable
        subprocess.call(["mv",
            os.path.abspath("autoparse." + self.WorkspaceName + ".build.json"),
            os.path.join(os.path.abspath(self.argsArr.WORKDIR), self.WorkspaceName, "build.json")])
        subprocess.call(["mv",
            os.path.abspath("autoparse." + self.WorkspaceName + ".build.log"),
            os.path.join(os.path.abspath(self.argsArr.WORKDIR), self.WorkspaceName, "build.log")])



__Project = linux
