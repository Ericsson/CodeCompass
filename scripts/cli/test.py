import unittest
import os
import subprocess
import time
import socket
import tarfile
import signal
from os.path import join

HOME = os.path.expanduser("~")
SCRIPT = HOME + '/trunk-install/bin/CodeCompass'
SCRIPT_TYPE = '/bin/bash'
PROJECT_DIR = 'testprojects'
AB_PROJECT_DIR_PATH = HOME + '/' + PROJECT_DIR
WORKSPACE_DIR = 'testworkdir'
AB_WORKSPACE_DIR_PATH = HOME + '/' + WORKSPACE_DIR
AB_CODECHECKER_DIR = HOME + '/Codechecker' 
PROJECTS = [
  { 'name' : 'helloworldtest',
    'type' : 'cpp',
    'build' : 'g++',
    'content': ["#include<iostream>", 
                "", 
                "int main()",
                "{",
                  "std::cout<<\"Test workspace!\"<<std::endl;",
                  "return 0;",
                "}"]
  }
]

# project index from PROJECTS
P_IND = 0 

# server datas
SERVER_PORT = None
SERVER_HOST = "localhost"

# database datas
DBUSER = None
DBPORT = None
DBPROC = None
DBHOST = "localhost"

EXP_TIME = 10
RENAME_PREFIX = "r_"
EXPORT_PREFIX = "e_"

# get eusername
def getUser():
  import getpass
  global DBUSER
  DBUSER = getpass.getuser()
  return DBUSER

# create project dir and file
def createTestProject(project):
  if not os.path.isdir(AB_PROJECT_DIR_PATH):
    os.makedirs(AB_PROJECT_DIR_PATH)
  
  testworkspace = AB_PROJECT_DIR_PATH + '/' + project['name'] + '.' + project['type']
  if not os.path.isfile(testworkspace):
    f = open(testworkspace, 'w')
    f.writelines('\n'.join(project['content']))
    f.close() 

# parsing a project
def parseTestProject(project):
  if not os.path.isdir(AB_WORKSPACE_DIR_PATH + '/' + project['name']):
    print "Parse test project (It may take a long time):"
    cmd = [SCRIPT, 'parse']
    cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
    cmd.extend(['-n', project['name']])
    cmd.extend(['-b', '"' + project['build'] + ' ' + project['name'] + '.' + project['type'] + '"'])
    cmd.extend(['--dbuser', DBUSER])
    cmd.extend(['--dbport', str(DBPORT)])
    cmd.extend(['-f'])

    os.chdir(AB_PROJECT_DIR_PATH)
  
    proc = subprocess.Popen(
      [SCRIPT_TYPE,'-c',' '.join(cmd)],
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
      env = os.environ)
    proc.wait()   

# find a free port number and return it
def findFreePort():
  import socket
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind(('', 0))
  free_port = s.getsockname()[1]
  s.close()
  return free_port

# create database into workdir and start it   
def createAndStartDatabase():
  print "Init database..."
  global DBPORT
  DBPORT = findFreePort()

  db_dir = AB_WORKSPACE_DIR_PATH + "/database"
  if not os.path.isdir(db_dir):
    os.makedirs(db_dir)
  os.chdir(db_dir)
  
  # Init database
  cmd = 'initdb -D . -E "SQL_ASCII"'
  proc = subprocess.Popen(
    ['/bin/bash', '-c' ,cmd], 
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()

  # Start database
  print "Start database on port "+ str(DBPORT) + " (It may take a long time): " 
  cmd = 'pg_ctl start -D . -o "-p '+ str(DBPORT) + "\""
  global DBPROC
  DBPROC = subprocess.Popen(
    ['/bin/bash', '-c' ,cmd], 
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)

  # wait for connection
  wait_for_checker = True
  i = 0
  while wait_for_checker:
    print "Wait for database..."
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex((DBHOST, DBPORT))
    wait_for_checker = result != 0
    time.sleep(1)
    sock.close()
    i = i + 1
    if i > EXP_TIME:
      print "Can't connect to the database! Maybe you forget to source env.sh!"
      DBPROC.terminate()
      wait_for_checker = False
      removeTempDatas()
      exit(1)
      
  print "Database started"

# close database connection
def closeDatabaseConnection():
  if DBPROC is not None:
    print "Close database connection"
    os.chdir(AB_WORKSPACE_DIR_PATH + "/database")
    cmd = "pg_ctl stop -D ."
    proc = subprocess.Popen(
      ['/bin/bash', '-c' ,cmd], 
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
    env = os.environ)
    proc.wait()

# remove projects and workdir directories   
def removeTempDatas():
  reply = raw_input("Delete test datas? [y/[n]] ")
  if reply=='y':
    import shutil
    shutil.rmtree(AB_PROJECT_DIR_PATH)
    shutil.rmtree(AB_WORKSPACE_DIR_PATH)

# list all workspaces and return it
def listWorkspaces():
  #os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'workspace']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-l'])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])

  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()
  workspaces = proc.communicate()[0] 
  return workspaces

# export a workspace
def exportWorkspace(orig_name, export_name):
  os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'workspace']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-e', orig_name, export_name])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])
 
  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()

# import workspace
def importWorkspace(name):
  os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'workspace']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-i', name])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])

  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()

# rename a workspace
def renameWorkspace(orig_name, new_name):
  os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'workspace']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-r', orig_name, new_name])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])

  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()

# delete a workspace
def deleteWorkspace(name):
  os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'workspace']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-d', name])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])
  
  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait()
    
# check code with CodeChecker
def checkCode(name):
  os.chdir(AB_WORKSPACE_DIR_PATH)
  cmd = [SCRIPT, 'codechecker']
  cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
  cmd.extend(['-n', name])
  cmd.extend(['--dbport', str(DBPORT)])
  cmd.extend(['--dbuser', DBUSER])

  proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
  proc.wait() 

# set Codechecker home variable
def exportCodeCheckerHome():
  os.environ["CODECHECKER_HOME"] = AB_CODECHECKER_DIR

# kill child processes recursively
def kill_child_processes(parent_pid, sig=signal.SIGTERM):
  ps_command = subprocess.Popen("ps -o pid --ppid %d --noheaders" % parent_pid, shell=True, stdout=subprocess.PIPE)
  ps_output = ps_command.stdout.read()
  retcode = ps_command.wait()
  for pid_str in ps_output.split("\n"):
    if pid_str != "":
      kill_child_processes(int(pid_str), sig)
      os.kill(int(pid_str), sig)

# Tests
class CodeCompassTest(unittest.TestCase):
  
  # destructor
  @classmethod
  def tearDownClass(self):
    closeDatabaseConnection() 
    removeTempDatas()

  # test parse
  def testParse(self):
    print "-- Parse test --"
    parseTestProject(PROJECTS[P_IND])
    workspace_dir = AB_WORKSPACE_DIR_PATH + '/' + PROJECTS[P_IND]['name']
    self.assertTrue(
      os.path.isdir(workspace_dir) and
      os.path.isfile(workspace_dir + '/after.sql' ) and
      os.path.isfile(workspace_dir + '/parse.sql' )
    )

  # test server
  def testRunServer(self):
    print "-- Server test --"
    parseTestProject(PROJECTS[P_IND])

    global SERVER_PORT
    SERVER_PORT = findFreePort()

    cmd = [SCRIPT, 'server']
    cmd.extend(['-w', AB_WORKSPACE_DIR_PATH])
    cmd.extend(['-p', str(SERVER_PORT)])
    cmd.extend(['--dbport', str(DBPORT)])
    cmd.extend(['--dbuser', DBUSER])
    
    print "Run webserver on port: " + str(SERVER_PORT) 
    proc = subprocess.Popen(
    [SCRIPT_TYPE,'-c',' '.join(cmd)], 
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
      env = os.environ)
    wait_for_checker = True
    i = 0
    while wait_for_checker:
      print "Wait for server answer..."
      sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      result = sock.connect_ex((SERVER_HOST, SERVER_PORT))
      wait_for_checker = result != 0
      time.sleep(1)
      sock.close()
      i= i + 1
      if i > EXP_TIME:
        print "Can't start the webserver!"
        wait_for_checker = False

    kill_child_processes(proc.pid)
    proc.terminate()
    self.assertTrue(i<=EXP_TIME)
    
  def testListWorkspaceIsNotEmptyList(self):
    print "-- List workspace test --"
    parseTestProject(PROJECTS[P_IND])
    workspaces = listWorkspaces()
    import re
    pattern = re.compile("(.*\n)*" + PROJECTS[P_IND]['name'] + "\n*")
    self.assertTrue(workspaces != "" and pattern.match(workspaces))

  def testExportWorkspace(self):
    print "-- Export workspace test --"
    parseTestProject(PROJECTS[P_IND])
    exportWorkspace(PROJECTS[P_IND]['name'], EXPORT_PREFIX + PROJECTS[P_IND]['name'])

    self.assertTrue(os.path.isfile(
      AB_WORKSPACE_DIR_PATH + '/' + EXPORT_PREFIX + PROJECTS[P_IND]['name']))

  # test rename workspace
  def testRenameWorkspace(self):
    print "-- Rename workspace test --"
    parseTestProject(PROJECTS[P_IND])
    renameWorkspace(PROJECTS[P_IND]['name'], RENAME_PREFIX + PROJECTS[P_IND]['name'])
 
    rw_dir = AB_WORKSPACE_DIR_PATH + '/' + RENAME_PREFIX + PROJECTS[P_IND]['name']
    rename_was_succes = os.path.isdir(rw_dir)
    renameWorkspace(RENAME_PREFIX + PROJECTS[P_IND]['name'], PROJECTS[P_IND]['name'])
    
    self.assertTrue(rename_was_succes)

  # delete a workspace
  def testDeleteWorkspace(self):
    print "-- Delete workspace test --"
    parseTestProject(PROJECTS[P_IND])
    
    deleteWorkspace(PROJECTS[P_IND]['name'])

    del_was_succes = os.path.isdir(AB_WORKSPACE_DIR_PATH + PROJECTS[P_IND]['name'])
    parseTestProject(PROJECTS[P_IND])
    checkCode(PROJECTS[P_IND]['name'])
    self.assertFalse(del_was_succes)
  
  # import a workspace
  def testImportWorkspace(self):
    print "-- Import workspace test --"
    import_name = "i_" + PROJECTS[P_IND]['name']
    parseTestProject(PROJECTS[P_IND])
    exportWorkspace(PROJECTS[P_IND]['name'], import_name)
    deleteWorkspace(PROJECTS[P_IND]['name'])
    checkCode(PROJECTS[P_IND]['name'])

    project_name = None
    with tarfile.open(AB_WORKSPACE_DIR_PATH + '/' + import_name) as tar:
      files = tar.getmembers()
      project_name = files[0].name
      
    importWorkspace(import_name)

    iw_dir = AB_WORKSPACE_DIR_PATH + '/' + project_name
    import_was_succes = os.path.isdir(iw_dir) and os.path.getsize(iw_dir) > 0
    
    import shutil
    shutil.rmtree(iw_dir)
    
    parseTestProject(PROJECTS[P_IND])
    checkCode(PROJECTS[P_IND]['name'])

    self.assertTrue(import_was_succes)

  # CodeCheker test
  def testCodeChecker(self):
    print "-- CodeChecker test --"
    if os.path.isdir(AB_CODECHECKER_DIR):
      exportCodeCheckerHome()
              
      parseTestProject(PROJECTS[P_IND])
      checkCode(PROJECTS[P_IND]['name'])
      
      cc_dir = AB_WORKSPACE_DIR_PATH + '/__codechecker__'
    
      self.assertTrue(os.path.isdir(cc_dir) and 
        os.path.getsize(cc_dir) > 0)
    else:
      print AB_CODECHECKER_DIR + " directory is not exists!"
      self.assertTrue(False)

if __name__ == '__main__':
  print "Run tests"
  print "Create test projects:"
  createTestProject(PROJECTS[P_IND])
  getUser()
  createAndStartDatabase()
  parseTestProject(PROJECTS[P_IND]) 
  
  unittest.main() 
