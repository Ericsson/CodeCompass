import datetime
import glob
import os
import re
import shutil
import subprocess
import sys
import time
import signal

from os.path import join

from util.configfilehandler import ConfigFileHandler
from util import util

def __current_time():
  '''This function returns the current date and time as string.'''
  return datetime.datetime.fromtimestamp(
    time.time()).strftime('%Y-%m-%d %H:%M:%S')

def __handle_parser_crash(projectdir):
  #--- Get line containing crash message in the log file ---#

  with open(join(projectdir, 'parselog'), 'r') as f:
    for line in f:
      if 'CODECOMPASS CRASH' in line:
        crash_message = line

  #--- Check crashed build action ---#

  if not crash_message or 'UNKNOWN' in crash_message:
    sys.stderr.write('Crash caused by an unknown action! Aborting!\n')
    sys.exit(1)

  try:
    m = re.match(r'CODECOMPASS CRASH ON ACTION (\d*)', crash_message)
    build_action = int(m.group(1))
  except:
    sys.stderr.write('Crash caused by an unknown action (malformed crash\
      message)! Aborting!\n')
    sys.exit(1)

  print 'Crash caused by action ' + str(build_action)

  #--- Manage core files ---#

  core_count = len(filter(lambda x: 'crash_' in x, os.listdir(projectdir)))

  coreFilename = next(iter(glob.glob(join(projectdir, 'core*'))), None)
  dummyFilename = next(iter(glob.glob(join(projectdir, 'dummy.core*'))), None)

  if dummyFilename:
    if not coreFilename:
      coreFilename = dummyFilename
    else:
      os.remove(dummyFilename)

  if coreFilename:
    print 'A core dump was found as ', coreFilename

    if core_count > 5 and not coreFilename.startswith('dummy.core'):
      print 'Too many saved core dumps. I just delete this one...'
      os.remove(coreFilename)
    else:
      os.rename(coreFilename, join(projectdir, 'crash_' + str(build_action)))

  with open(join(projectdir, 'skippedactions'), 'a') as f:
    print 'Marking this action as skipped.'
    f.write(str(build_action) + '\n')

def interrupt_parse_handler(signum, frame):
    print "Parse stopped!"
    sys.exit(1)

def __run_parser(args):
  projectdir = join(args.workdir, args.name)

  print __current_time(), 'Parsing...'

  #--- Run parser ---#

  while True:
    with open(join(projectdir, 'parselog'), 'a') as f:
      f.write('Parser started at ' + __current_time() + '\n')

      parse_command = [join(os.environ['CODECOMPASS_HOME'], 'bin', 'parse')]
      parse_command.extend(['-d', util.create_connection_string(args)])
      parse_command.extend(['-p', args.logfile])
      parse_command.extend(['-a', 'data'])
      parse_command.extend(['-k', 'skippedactions'])
      parse_command.extend(['-n', args.name])
      parse_command.extend(['--max-parser-threads', str(args.thread)])
      parse_command.extend(['--roots', args.labels])

      if (args.extra):
        parse_command.extend((args.extra).split())
       
      old = util.set_environ()

      proc = subprocess.Popen(
        parse_command,
        stdout = f,
        stderr = f,
        cwd = projectdir,
        env = os.environ)
      proc.wait()

      os.environ = old

      f.write(
        'Parse ended with code ' + str(proc.returncode) +
        ' at ' + __current_time() + '\n')

    has_crashes = False
    if glob.glob(join(projectdir, '*core*')):
      print __current_time(), 'Parser crashed! :-('

      if not args.crash:
        __handle_parser_crash(projectdir)
        has_crashes = True
        print __current_time(), 'Crash handled, continuing...'
      else:
        break
    else:
      break

  #--- Last messages after finishing parsing ---#

  print __current_time(), 'Done',
  if has_crashes:
    print 'with crashes :\'('
  elif proc.returncode == 0:
    print 'successfully! :-D'
  else:
    print 'with error! :-('

  return proc.returncode

def __collect_sql(projectdir):
  '''This function finds all .sql files under the CODECOMPASS_HOME and creates
  two other .sql files to the project directory. One is the parse.sql which is
  the concatenation of all .sql files except the comments, index creations and
  alter commands. The other one is after.sql which is the concatenation of all
  .sql files except the comments, table creations and drop commands.'''

  def remove_by_regex(s, begin, end):
    return re.sub(begin + '([\s\S]*?)' + end + '\n?', '', s)

  def sql_remove_drops(s):
    return remove_by_regex(s, 'DROP', ';')

  def sql_remove_create_tables(s):
    return remove_by_regex(s, 'CREATE TABLE', ';')

  def sql_remove_comments(s):
    return remove_by_regex(s, '\/\*', '\*\/')

  def sql_remove_constraints(s):
    return remove_by_regex(s, 'ALTER TABLE', ';')

  def sql_remove_indexes(s):
    return remove_by_regex(s, 'CREATE INDEX', ';')

  sql = ''
  for curr in os.walk(os.environ['CODECOMPASS_HOME']):
    for filename in curr[2]:
      if filename.endswith('.sql') and 'postgresql' not in curr[0]:
        with open(join(curr[0], filename), 'r') as f:
          sql += f.read()

  before = sql_remove_constraints(sql_remove_indexes(sql_remove_comments(sql)))
  after  = sql_remove_drops(sql_remove_create_tables(sql_remove_comments(sql)))

  with open(join(projectdir, 'parse.sql'), 'w') as f:
    f.write(before)

  with open(join(projectdir, 'after.sql'), 'w') as f:
    f.write(after)

def __create_database_and_clean(args):
  #--- Helper variables ---#

  projectdir = join(args.workdir, args.name)
  dblogfile = join(projectdir, 'dblog')

  #--- Database operations ---#

  print __current_time(), 'Creating new database...'

  __collect_sql(projectdir)

  util.pgsql_cmd(projectdir, args, 'dropdb', args.name)

  res = util.pgsql_cmd(
    projectdir, args,
    'createdb', '-E', 'SQL_ASCII', '-T', 'template0', '-l', 'C', args.name)

  if res != 0:
    sys.stderr.write(
      'createdb returned with ' + str(res) + '!\nSee ' + dblogfile +
      ' for details!\n')
    sys.exit(1)

  res = util.pgsql_cmd(
    projectdir, args, 'psql', '-f', 'parse.sql', '-d', args.name)

  if res != 0:
    sys.stderr.write(
      'Failed to create the database schema (psql returned with ' + str(res) +
      ')!\nSee ' + dblogfile + ' for details!\n')
    sys.exit(1)

  #--- Clean ---#

  try:
    shutil.rmtree(join(projectdir, 'data'))
    os.remove(join(projectdir, 'codechecker_output'))
    os.remove(join(projectdir, 'parselog'))
    os.remove(join(projectdir, 'buildlog'))
    for filename in os.listdir(projectdir):
      if filename.startswith('core') or filename.startswith('crash_'):
        os.remove(join(projectdir, filename))
  except:
    pass

  config_file_handler = ConfigFileHandler(join(args.workdir, 'workspace.cfg'))
  config_file_handler.remove(args.name)
  config_file_handler.save()

  print __current_time(), 'Done'

def __run_after_sql_script(args):
  print __current_time(), 'Creating indexes and constraints...'

  projectdir = join(args.workdir, args.name)

  res = util.pgsql_cmd(
    projectdir, args, 'psql', '-f', 'after.sql', '-d', args.name)

  if res == 0:
    res = util.pgsql_cmd(
      projectdir, args, 'psql', '-c', 'VACUUM ANALYZE;', '-d', args.name)

  if res != 0:
    dblogfile = join(args.workdir, args.name, 'dblog')
    sys.stderr.write(
      'Failed to create the database indexes (psql returned with ' + str(res) +
      ')!\nSee ' + dblogfile + ' for details!\n')
    sys.exit(1)

  print __current_time(), 'Done'

def parse_handler(args):
  signal.signal(signal.SIGINT, interrupt_parse_handler)

  #--- Print database connection data ---#

  print '--- Database connection ---'
  print 'Database host: ' +     args.dbhost
  print 'Database port: ' + str(args.dbport)
  print 'Database user: ' +     args.dbuser
  print 'Database pass: ' +     args.dbpass
  print '---------------------------'

  #--- Create workdir and project directory ---#

  projectdir = join(args.workdir, args.name)

  if not os.path.exists(projectdir):
    try:
      os.makedirs(projectdir)
    except OSError:
      sys.stderr.write('Permission denied to create ' + projectdir + '\n')
      sys.exit(1)
  elif not args.force:
    sys.stderr.write(projectdir + ' already exists. Use -f to force reparsing!\n')
    sys.exit(1)
  else:
    if args.logfile:
      for entry in os.listdir(projectdir):
        fullentry = os.path.join(projectdir, entry)
        if (os.path.isdir(fullentry)):
          shutil.rmtree(fullentry)
        elif (entry != "build.json"):
          os.remove(fullentry)
    else:
      shutil.rmtree(projectdir)
      os.makedirs(projectdir)

  #--- Create build log ---#

  print __current_time(), 'Logger started...'

  if args.build or args.logonly:
    param = args.build if args.build else args.logonly
    try:
      args.logfile = join(projectdir, 'build.json')

      if not args.force and os.path.exists(args.logfile):
        sys.stderr.write('The project has already been logged\n')
        sys.exit(1)

      cmd = 'source {} {}; bash -c "{}"'.format(
        join(os.environ['CODECOMPASS_HOME'], 'bin', 'setldlogenv.sh'),
        args.logfile,
        param)

      with open(join(projectdir, 'buildlog'), 'a') as f:
        os.environ['LDLOGGER_HOME']=join(os.environ['CODECOMPASS_HOME'], 'logger')
        proc = subprocess.Popen(
          ['/bin/bash', '-c', cmd],
          stderr = f,
          stdout = f,
          env = os.environ)
        proc.wait()

      if not os.path.exists(args.logfile):
        sys.exit("Failed to log the build commands!")
      elif os.path.getsize(args.logfile) <= 5:
        sys.exit("Failed to log the build commands: the build log is empty!")

    except RuntimeError as err:
      sys.stderr.write(str(err) + '\n')
      sys.exit(1)

  if not args.build and not args.logfile:
    args.logfile = join(projectdir, 'build.json')

    with open(args.logfile, 'w') as logfile:
      logfile.write('[]')

  print __current_time(), 'Done'

  #--- Parse the project ---#

  if not args.logonly:
    if not os.path.isfile(args.logfile):
      sys.exit('The given build log file not found!')

    __create_database_and_clean(args)
    returncode = __run_parser(args)

    if not args.logfile.startswith(projectdir):
      shutil.copy(args.logfile, projectdir)

    if not args.noindex:
      __run_after_sql_script(args)
 
    #--- Add workspace to config file ---#

    if returncode == 0:
      desc = args.description if hasattr(args, 'description') and args.description <> "" else args.name

      config_file_handler = ConfigFileHandler(join(args.workdir, 'workspace.cfg'))
      connection_string = util.create_connection_string(args)

      config_file_handler.add(args.name)
      config_file_handler.set_attribute(args.name, 'connection', connection_string)
      config_file_handler.set_attribute(args.name, 'description', desc)

      config_file_handler.save()

    print 'Parse log files can be found at: ' + join(projectdir, 'parselog')
    print 'Database log files can be found at: ' + join(projectdir, 'dblog')
