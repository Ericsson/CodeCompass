import copy
import os
import socket
import subprocess
import time

from os.path import join

def set_environ():
  '''This function inserts the necessary environment variables to the os.environ
  dictionary and returns the old one so that can be restored later.'''
  old = copy.copy(os.environ)

  cc = os.environ['CODECOMPASS_HOME']

  os.environ['SASL_PATH'] = join(cc, 'lib', 'sasl2')
  os.environ['MAGIC'] = join(cc, 'share', 'misc', 'magic.mgc')

  ld_lib = join(cc, 'lib') + ':' + join(cc, 'lib64')
  if 'LD_LIBRARY_PATH' in os.environ:
    ld_lib += ':' + os.environ['LD_LIBRARY_PATH']

  os.environ['LD_LIBRARY_PATH'] = ld_lib

  return old

def pgsql_cmd(projectdir, args, cmd, *db_args):
  '''This function runs the given command appended by the database connection
  options and the furhter arguments. The output (standard or error) are piped
  to the dblog file in the projectdir. The function returns the return code
  of the command.

  projectdir -- The directory of the project in which dblog file is placed.
  args -- An object containing the information for database connection.
  cmd -- The name of the PostgreSQL command to run. This command looked up in
  the CodeCompass' bin directory or in the directories of PATH environment
  variable.'''

  connection_options = []
  connection_options.extend(['-h', args.dbhost])
  connection_options.extend(['-U', args.dbuser])
  connection_options.extend(['-p', str(args.dbport)])

  old = set_environ()
  os.environ['PATH'] = join(
    os.environ['CODECOMPASS_HOME'], 'bin') + ':' + os.environ['PATH']

  with open(join(projectdir, 'dblog'), 'a') as dblog:
    try:
      proc = subprocess.Popen(
        [cmd] + connection_options + ['-w'] + list(db_args),
        cwd = projectdir,
        stdout = dblog,
        stderr = dblog,
        env = os.environ)
      proc.wait()
    except:
      raise
    finally:
      os.environ = old

  return proc.returncode

def start_codechecker_server(args):
  '''This function starts the CodeChecker server on a random free port. The
  function returns the subprocess object and the port number on which
  CodeChecker server listens.

  If the CodeChecker binary can't be found then OSError exception is raised.

  args -- An object which contains connection information, which come from
  command line arguments.'''

  import socket
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind(('', 0))
  free_port = s.getsockname()[1]
  s.close()

  command = [join(os.environ['CODECHECKER_HOME'], 'bin', 'CodeChecker')]
  command.append('server')
  command.extend(['-w', join(args.workdir, '__codechecker__')])
  command.extend(['--postgresql'])
  command.extend(['--dbport', str(args.dbport)])
  command.extend(['--dbaddress', args.dbhost])
  command.extend(['--dbusername', args.dbuser])
  command.extend(['-v', str(free_port)])

  cmd = ' '.join(command)

  proc = subprocess.Popen(
    ['/bin/bash', '-c', cmd],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    env = os.environ)
 
  while True:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex((args.dbhost, free_port))
    if result == 0: break
    time.sleep(1)

  return proc, free_port

def yes_no_question(question, default = None):
  '''This function asks a question from the user and requires a yes/no answer.
  The question is ased until the answer is correct.

  question -- A question string
  default -- The default return value. If this is None (default) then the
  function requires non-empty user input. If True or False then in case of empty
  user input the return value is True or False respectively.'''

  if default == True:
    options = ' (Yes/no) '
  elif default == False:
    options = ' (yes/No) '
  else:
    options = ' (yes/no) '

  good = ['yes', 'y']
  bad  = ['no',  'n']

  while True:
    answer = raw_input(question + options).lower()
    if default != None and not answer or answer in good or answer in bad:
      break

  if answer in good:
    return True
  elif answer in bad:
    return False
  else:
    return default

def create_connection_string(args):
  '''This function assembles a connection string from the command line
  arguments.'''
  connection_string  = 'pgsql:'
  connection_string += 'host='     + args.dbhost + ';'
  connection_string += 'database=' + args.name   + ';'
  connection_string += 'user='     + args.dbuser + ';'
  connection_string += 'port='     + str(args.dbport)

  if args.dbpass:
    connection_string += ';passwdfile=' + args.dbpass

  return connection_string
