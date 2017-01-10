import os
import signal
import socket
import subprocess
import sys
import time
import copy

from os.path import join

from util.configfilehandler import ConfigFileHandler
from util import util
from util import dbutil

SERVER_PID = None

def handler(signum, frame):
  os.kill(SERVER_PID, signal.SIGTERM)

def server_handler(args):
  config_file = join(args.workdir, 'workspace.cfg')
  pid_file = join(args.workdir, 'pids')

  #--- Stop the webserver ---#

  if args.stop:
    if not os.path.isfile(pid_file):
      return

    with open(pid_file) as f:
      for line in f:
        try:
          os.kill(int(line), signal.SIGTERM)
        except:
          print 'No process number ' + line

    os.remove(pid_file)

    return

  #--- Start the webserver ---#

  with open(pid_file, 'w') as f:
    # The parent's PID is stored because the CodeCompass.py is wrapped by a
    # bash script.
    f.write(str(os.getppid()) + '\n')

  configfilehandler = ConfigFileHandler(config_file)

  for name in configfilehandler.names():
    connArgs = copy.copy(args)
    connArgs.name = name
    connection = dbutil.create_connection_string(connArgs)
    configfilehandler.set_attribute(name, 'connection', connection)

  configfilehandler.save()

  command = [join(os.environ['CODECOMPASS_HOME'], 'bin', 'CodeCompass_webserver')]
  command.extend(['-p', str(args.port)])
  command.extend(['-w', args.workdir])

  wslog = join(args.workdir, 'webserver.log')

  while True:
    print '(Re)Starting web server with log file: {}'.format(config_file)
    print 'Web server log files can be found at: ' + wslog

    with open(wslog, 'a') as f:
      old = util.set_environ()

      proc = subprocess.Popen(command, stdout = f, stderr = f, env = os.environ)

      with open(pid_file, 'w') as f:
        f.write(str(proc.pid) + '\n')

      global SERVER_PID
      SERVER_PID = proc.pid
      signal.signal(signal.SIGTERM, handler)
      proc.wait()

      if proc.returncode & 126 == 0:
        print 'Web server exited with code: ' + str(proc.returncode)
        sys.exit(proc.returncode)

      os.environ = old
