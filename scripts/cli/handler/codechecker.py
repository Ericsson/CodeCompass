import os
import subprocess
import sys

from os.path import join

from util.configfilehandler import ConfigFileHandler

def codechecker_handler(args):
  
  #--- Check preconditions ---#

  try:
    subprocess.call(
      join(os.environ['CODECHECKER_HOME'], 'bin', 'CodeChecker'),
      stdout = subprocess.PIPE, stderr = subprocess.PIPE)
  except (OSError, KeyError):
    sys.stderr.write('Set CODECHECKER_HOME environment variable!\n')
    sys.exit(1)

  configfilehandler = ConfigFileHandler(join(args.workdir, 'workspace.cfg'))

  if args.name not in configfilehandler.names():
    sys.stderr.write('The project has to be parsed with CodeCompass first!\n')
    sys.exit(1)

  if not args.runname:
    args.runname = args.name

  #--- Check the project ---#

  projectdir = join(args.workdir, args.name)

  command = [join(os.environ['CODECHECKER_HOME'], 'bin', 'CodeChecker'), 'check']
  command.extend(['-w', join(args.workdir, '__codechecker__')])
  command.extend(['-n', args.runname])
  command.extend(['-l', join(projectdir, 'build.json')])
  command.extend(['-j', str(args.thread)])
  command.extend(['--postgresql'])
  command.extend(['--dbaddress', args.dbhost])
  command.extend(['--dbport', str(args.dbport)])
  command.extend(['--dbusername', args.dbuser])

  if hasattr(args, 'extra') and args.extra:
    command.extend(args.extra.split(" "))

  cmd = ' '.join(command)

  print 'Start checking...'
  
  logfile = join(projectdir, 'codechecker_output')

  with open(logfile, 'a') as f:
    proc = subprocess.Popen(
      ['/bin/bash', '-c', cmd], stdout = f, stderr = f, env = os.environ)
    proc.wait()

  #--- Modify config file ---#

  configfilehandler.set_attribute(args.name, 'codecheckerrunname', args.runname)
  configfilehandler.save()

  print 'Done'
  print 'CodeChecker log file can be found at: ' + logfile
