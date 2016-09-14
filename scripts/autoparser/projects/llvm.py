import projects
import os.path
import subprocess

class llvm(projects.Project):
    Name = "LLVM"
    WorkspaceName = "llvm"
    Description = "LLVM"

    def __init__(self, argsArr):
        pass

    def getDownloadFolder(self):
        return "llvm"

    def download(self):
        return_code = subprocess.call(["git", "clone", "http://github.com/llvm-mirror/llvm", self.getDownloadFolder()])
        if return_code == 0:
            return_code = subprocess.call(["git", "clone", "http://github.com/llvm-mirror/clang", os.path.join(self.getDownloadFolder(), "tools", "clang")])

            if return_code == 0:
                return_code = subprocess.call(["git", "clone", "http://github.com/llvm-mirror/clang-tools-extra", os.path.join(self.getDownloadFolder(), "tools", "clang", "tools", "extra")])
                
                if return_code == 0:
                    pass
                else:
                    return False
            else:
                return False
        else:
            return False

        return True

    def cleanDownload(self):
        subprocess.call(["rm", "-fr", self.getDownloadFolder()])

    def prepare(self):
        subprocess.call(["mkdir", "-p", self.getProjectFolder()])
        os.chdir(self.getProjectFolder())

        cmake = subprocess.call(["cmake", os.path.join("..", self.getDownloadFolder())])
        
        return cmake == 0

    def getProjectFolder(self):
        return "llvm_build"

    def cleanBuild(self):
        subprocess.call(["rm", "-fr", self.getProjectFolder()])

    def build(self, parseArgs):
        parseArgs.append("--build")
        parseArgs.append('"cmake --build ."')
        parseArgs.append("--labels")
        parseArgs.append('builddir=' + os.path.abspath(self.getProjectFolder()) + ':sourcedir=' + os.path.abspath(self.getDownloadFolder()))

    def postBuild(self):
        pass

__Project = llvm
