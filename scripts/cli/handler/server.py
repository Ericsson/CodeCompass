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

  #--- Start codechecker server ---#

  checker_proc = None
  checker_sourced = False
  wait_for_checker = False

  try:
    subprocess.call(
      join(os.environ['CODECHECKER_HOME'], 'bin', 'CodeChecker'),
      stdout = subprocess.PIPE, stderr = subprocess.PIPE)
  except (OSError, KeyError):
    sys.stderr.write('Set CODECHECKER_HOME environment variable!\n')
  else:
    checker_sourced = True

  if checker_sourced:
    print 'Start CodeChecker server...'
    checker_proc, checker_port = util.start_codechecker_server(args)

    configfilehandler = ConfigFileHandler(config_file)

    for name in configfilehandler.names():
      if configfilehandler.get_attribute(name, 'codecheckerrunname'):
        configfilehandler.set_attribute(
          name, 'codecheckerurl', '{}:{}/CodeCheckerService'.format(
            'http://localhost', str(checker_port)))

    configfilehandler.save()

    wait_for_checker = True

  while wait_for_checker:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex((args.dbhost, checker_port))
    wait_for_checker = result != 0
    time.sleep(1)

  #--- Start the webserver ---#

  with open(pid_file, 'w') as f:
    # The parent's PID is stored because the CodeCompass.py is wrapped by a
    # bash script.
    f.write(str(os.getppid()) + '\n')

  configfilehandler = ConfigFileHandler(config_file)

  for name in configfilehandler.names():
    connArgs = copy.copy(args)
    connArgs.name = name
    connection = util.create_connection_string(connArgs)
    configfilehandler.set_attribute(name, 'connection', connection)

  configfilehandler.save()

  command = [join(os.environ['CODECOMPASS_HOME'], 'bin', 'pluginablewebserver')]
  command.extend(
    ['-P', join(os.environ['CODECOMPASS_HOME'], 'lib', 'codecompass', 'webplugin')])
  command.extend(
    ['-d', join(os.environ['CODECOMPASS_HOME'], 'share', 'codecompass', 'html')])
  command.extend(['-p', str(args.port)])
  command.extend(['--workspaces', config_file])

  if args.extra:
    command.extend(args.extra.split(" "))

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
        if checker_proc:
          os.killpg(os.getpgid(checker_proc.pid), signal.SIGTERM)
        sys.exit(proc.returncode)

      os.environ = old
