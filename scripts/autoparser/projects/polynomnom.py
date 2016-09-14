import projects
import os.path
import subprocess

class PolynomNom(projects.Project):
    Name = "Poly--Nom-nom"
    WorkspaceName = "polynomnom"
    Description = "Polynomial library (by Whisperity)"

    def __init__(self, argsArr):
        pass

    def getDownloadFolder(self):
        '''Specifies the folder where the downloaded and almost-buildable source files are located.'''
        return "polynomnom_dl"

    def download(self):
        return_code = subprocess.call(["git", "clone", "http://github.com/whisperity/polynomnom.git", self.getDownloadFolder()])
        if return_code == 0:
            # There is nothing to do when the download is successful, at least for now.
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
        subprocess.call(["cp", "-R", self.getDownloadFolder(), os.path.abspath(self.getProjectFolder())])

        return True

    def getProjectFolder(self):
        return "PNN"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])

    def build(self, parseArgs):
        parseArgs.append("-b");
        parseArgs.append('"g++ -std=c++11 src/main.cpp"')
        parseArgs.append("--labels")
        parseArgs.append('sourcedir=' + os.path.abspath(self.getProjectFolder()))

    def postBuild(self):
        pass

# This line is MANDATORY, please specify it as follows.
# Without this, the core script could not find the class.
__Project = PolynomNom
