import projects
import os.path
import subprocess
import sys
class xerces(projects.Project):
    Name = "Xerces"
    WorkspaceName = "xerces"
    Description = "Apache Xerces Project"

    def __init__(self, argsArr):
        pass

    def getDownloadFolder(self):
        '''Specifies the folder where the downloaded and almost-buildable source files are located.'''
        return "xerces-c"

    def download(self):
        return_code = subprocess.call(["git", "clone", "http://github.com/apache/xerces-c.git", "--branch", "trunk"])
        if return_code == 0:
            pass
        else:
            return False

        return True

    def cleanDownload(self):
        subprocess.call(["rm", "-fr", self.getDownloadFolder()])

    def prepare(self):
        # For example, removing the '.git' folder would be a nice thing here to do.

        # Please always make sure here that a download->build transition is happening.
        # Usually this involves copying or moving the downloaded sources.
        # (And then eg. doing automake, configure, stuff like that.)
        subprocess.call(["cp", "-R", os.path.abspath(self.getDownloadFolder()), os.path.abspath(self.getProjectFolder())])
	os.chdir(self.getProjectFolder())
	subprocess.call(["./configure"])

        return True

    def getProjectFolder(self):
        return "xerces"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])

    def build(self, parseArgs):
        parseArgs.append("--build");
        parseArgs.append('"make"')
        parseArgs.append("--labels")
        parseArgs.append('sourcedir=' + os.path.abspath(self.getProjectFolder()))

    def postBuild(self):
        pass

# This line is MANDATORY, please specify it as follows.
# Without this, the core script could not find the class.
__Project = xerces
