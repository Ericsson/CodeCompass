import projects
import os.path
import subprocess

class helloworld(projects.Project):
    Name = "Hello World!"
    WorkspaceName = "helloworld"
    Description = "Runtime-generated 'Hello World!' project for testing autoparser."

    def __init__(self, argsArr):
        pass

    def getDownloadFolder(self):
        return "helloworld"

    def download(self):
        subprocess.call(["mkdir", '-p', self.getDownloadFolder()])
        maincpp = open(os.path.join(self.getDownloadFolder(), "hello.cpp"), "w")
        print >>maincpp, '''#include <iostream>

    int main()
    {
        std::cout << "Hello World!" << std::endl;
        return 0;
    }'''

        maincpp.close()

        return True

    def cleanDownload(self):
        subprocess.call(["rm", "-fr", self.getDownloadFolder()])

    def prepare(self):
        subprocess.call(["cp", "-R", self.getDownloadFolder(), os.path.abspath(self.getProjectFolder())])

        os.chdir(self.getProjectFolder())
        makefile = open("Makefile", "w")
        print >>makefile, '''all:
\tg++ -std=c++11 -Wall -pedantic hello.cpp'''
        makefile.close()

        builderFile = open("../autoparse." + self.WorkspaceName + ".build.sh", "w")
        #print >>builderFile, "#!/bin/bash"
        #print >>builderFile, ''
        print >>builderFile, "make --always-make"
        print >>builderFile, 'if [ $? -ne 0 ]; then exit 1; fi'
        print >>builderFile, ''
        print >>builderFile, 'exit 0'
        builderFile.close()

        subprocess.call(["chmod", 'u+x', "../autoparse." + self.WorkspaceName + ".build.sh"])

        return True

    def getProjectFolder(self):
        return "helloworld_build"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])
        subprocess.call(["rm", "autoparse." + self.WorkspaceName + ".build.sh"])

    def build(self, parseArgs):
        parseArgs.append("--force")
        parseArgs.append("--build")
        parseArgs.append('"../autoparse.' + self.WorkspaceName + '.build.sh 2>&1"')
        parseArgs.append("--labels")
        parseArgs.append('sourcedir=' + os.path.abspath(self.getProjectFolder()))

    def postBuild(self):
        pass

__Project = helloworld
