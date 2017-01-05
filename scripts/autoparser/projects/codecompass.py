import projects
import os.path
import subprocess

class codecompass(projects.Project):
    Name = "CodeCompass"
    WorkspaceName = "CodeCompass_Master"
    Description = "CodeCompass master release from mainstream"

    def __init__(self, argsArr):
        self.environFile = os.path.join(argsArr.installdir, 'env.sh')

    def getDownloadFolder(self):
        return "codecompass"

    def download(self):
        return_code = subprocess.call(["git", "clone", "git@mainstream.inf.elte.hu:gsd/CodeCompass.git", self.getDownloadFolder()])
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

        configuratorFile = open("../autoparse." + self.WorkspaceName + ".configure.sh", "w")
        #print >>configuratorFile, "#!/bin/bash"
        #print >>configuratorFile, ''
        print >>configuratorFile, "source " + self.environFile
        print >>configuratorFile, ''
        print >>configuratorFile, "./autogen.sh"
        print >>configuratorFile, 'if [ $? -ne 0 ]; then exit 1; fi'
        print >>configuratorFile, ''
        print >>configuratorFile, "./configure --with-database=pgsql"
        print >>configuratorFile, 'if [ $? -ne 0 ]; then exit 1; fi'
        print >>configuratorFile, ''
        print >>configuratorFile, 'exit 0'
        configuratorFile.close()

        subprocess.call(["chmod", 'u+x', "../autoparse." + self.WorkspaceName + ".configure.sh"])
        success = subprocess.call(["bash", '-c', "../autoparse." + self.WorkspaceName + ".configure.sh"])

        if success != 0:
            return False

        builderFile = open("../autoparse." + self.WorkspaceName + ".build.sh", "w")
        #print >>builderFile, "#!/bin/bash"
        #print >>builderFile, ''
        print >>builderFile, "source " + self.environFile
        print >>builderFile, ''
        print >>builderFile, "make"
        print >>builderFile, 'if [ $? -ne 0 ]; then exit 1; fi'
        print >>builderFile, ''
        print >>builderFile, 'exit 0'
        builderFile.close()

        subprocess.call(["chmod", 'u+x', "../autoparse." + self.WorkspaceName + ".build.sh"])

        return True

    def getProjectFolder(self):
        return "codecompass_build"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])
        subprocess.call(["rm", "autoparse." + self.WorkspaceName + ".configure.sh", "autoparse." + self.WorkspaceName + ".build.sh"])

    def build(self, parseArgs):
        parseArgs.append("--force")
        parseArgs.append("--build")
        parseArgs.append('"../autoparse.' + self.WorkspaceName + '.build.sh 2>&1"')
        parseArgs.append("--labels")
        parseArgs.append('sourcedir=' + os.path.abspath(self.getProjectFolder()))

    def postBuild(self):
        pass

# This line is MANDATORY! It makes the core script realise this class!
__Project = codecompass
