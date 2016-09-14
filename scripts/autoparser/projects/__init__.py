# Define a base class for projects
class Project:
    '''The name of the project - used in the command-line to specify which projects to use'''
    Name = "__init__"
    
    # WorkspaceName and Description CAN be modified before build() is called.
    # This means that download() and prepare() MAY modify these variables.
    # For example: to add version control description, tag name, release name, etc.
    
    '''The name of the workspace where the project will be stored'''
    WorkspaceName = "Dummy"

    '''The long description of the project which will be visible on the webpage.'''
    Description = "Dummy project interface"
    
    def __init__(self, argsArr):
        pass
    
    def getDownloadFolder(self):
        '''Specifies the folder where the downloaded and almost-buildable source files are located.'''
        pass

    def download(self):
        '''Supposed to retrieve the files from the remote source.'''
        pass

    def cleanDownload(self):
        '''Cleans the download folder'''
        pass

    def prepare(self):
        '''Prepare the build folder after a download and before a build+parse.'''
        # For example, removing the '.git' folder would be a nice thing here to do.

        # Please always make sure here that a download->build transition is happening.
        # Usually this involves copying or moving the downloaded sources.
        # (And then eg. doing automake, configure, stuff like that.)
        pass

    def getProjectFolder(self):
        '''Specifies the folder where the project files will be for the build to happen.
        This may or may not be the same as the download folder. Due to the
        ability to "cache" or "keep" downloads, this SHOULD NOT be the same as the download folder.'''
        pass

    def cleanBuild(self):
        '''Supposed to clean the project files after a successful parsing run.'''
        pass

    def build(self, parseArgs):
        '''The build command should do the final preparation of the build, most importantly
        setting arguments like --build command, sourcedir for --labels for the CodeCompass parser.'''
        pass

    def postBuild(self):
        '''Execute arbitrary action after build but before deploy.'''
        pass


# Load all projects in the current folder
import os
__all__ = []

for root, dirs, files in os.walk("./projects"):
    for file in files:
        if file != "__init__.py" and file.endswith(".py"):
             file = file.replace('./projects/', '').replace('.py', '')
             __all__.append(file)
