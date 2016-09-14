import argparse
import getpass
import os
import sys
import resource

import handler.codechecker
import handler.parse
import handler.server
import handler.workspace
from util.util import *

#--- Argument parser ---#

parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers()

#--- Parse handling options ---#

parse_parser = subparsers.add_parser('parse', help = 'Parser handling')

parse_parser.add_argument(
  '-w', '--workdir', type = str, required = True,
  help = 'Path of workspace directory of the projects in which config files are\
  stored.')
parse_parser.add_argument(
  '-n', '--name', type = str, required = True,
  help = 'Project name to identify the project.')
parse_parser.add_argument(
  '-d', '--description', type = str, default = '',
  help = 'Human readable description of the project which belongs to this\
  parsing. This will be displayed on the user interface. The default value is\
  the current project name.')
parse_options = parse_parser.add_mutually_exclusive_group()
parse_options.add_argument(
  '-b', '--build', type = str,
  help = 'This option is used to build the project and create build logs. This\
  flag has to be given the build command which builds the project.')
parse_options.add_argument(
  '-l', '--logfile', type = str,
  help = 'Path of the parse log which is used by the parser as input.')
parse_options.add_argument(
  '-o', '--logonly', type = str,
  help = 'Log only. In this case parser is not started after logging session.')

parse_parser.add_argument(
  '-f', '--force', action = 'store_true',
  help = 'If -l is given and the log file already exists or -b is given and the\
  project directory already exists then the commands are not executed unless\
  this flag is given. In case of forcing the log file will be overwritten and\
  project directory will be reused.')
parse_parser.add_argument(
  '-c', '--crash', action = 'store_true',
  help = 'If this flag is given then parser stops when the parser crashes.')
parse_parser.add_argument(
  '-i', '--noindex', action = 'store_true',
  help = 'If this flag is given then the database indexes won\'t be added to\
  the database.')
parse_parser.add_argument(
  '--labels', type = str, default = '',
  help = 'Name paths with labels. Some CodeCompass parser components will parse\
  only files under these paths. Furthermore on the GUI there will be shortcuts\
  on these paths. The string format has to be:\
  label1=/path/one:label2=/path/two. If not given then an interactive interface\
  will ask for the paths.')
parse_parser.add_argument(
  '-e', '--extra', type = str,
  help = 'Extra arguments passed to the CodeCompass webserver.\
  The parameter syntax should look like this: -e="extraparam1 extraparam2". \
  Allowed values of the extra parameters:\
  --skip-traverse:        Skip all traversal;\
  --skip-parser:          Skip all language parser;\
  --skip-pythonparser:    Skip Python parser;\
  --skip-version-control: Skip version control integration')
parse_parser.add_argument(
  '-t', '--thread', type = int, default = 4,
  help = 'Number of threads to parse on. The default value is 4.')
parse_parser.add_argument(
  '--dbhost', type = str, default = 'localhost',
  help = 'Address of the PgSQL server. The default value is "localhost".')
parse_parser.add_argument(
  '--dbport', type = int, default = 5432,
  help = 'Port of the PgSQL server. The default value is 5432.')
parse_parser.add_argument(
  '--dbuser', type = str, default = getpass.getuser(),
  help = 'User name for the PgSQL server. The default value is the current user\
  name.')
parse_parser.add_argument(
  '--dbpass', type = str,
  help = 'Password file for the PgSQL server.')
parse_parser.add_argument(
  '-nc', '--no-core', action = 'store_true',
  help = 'Prevent the creation of core dumps by the underlying processes.')

parse_parser.set_defaults(func = handler.parse.parse_handler)

#--- Workspace handling options ---#

workspace_parser = subparsers.add_parser(
  'workspace', help = 'Workspace handling')

workspace_parser.add_argument(
  '-w', '--workdir', type = str, required = True,
  help = 'Path of workspace directory of the projects in which config files are\
  stored.')

workspace_options = workspace_parser.add_mutually_exclusive_group(
  required = True)

workspace_options.add_argument(
  '-l', '--list', action = 'store_true',
  help = 'List all workspaces.')
workspace_options.add_argument(
  '-d', '--delete', type = str, metavar = 'NAME',
  help = 'Remove the whole workspace, including all config files and\
  databases.')
workspace_options.add_argument(
  '-r', '--rename', type = str, nargs = 2, metavar = ('FROM', 'TO'),
  help = 'Rename workspace. The effect takes place in the database too.')
workspace_options.add_argument(
  '-e', '--export', type = str, nargs = 2, metavar = ('WORKSPACE', 'FILE'),
  help = 'Export the workspace into a .tar.gz file.')
workspace_options.add_argument(
  '-i', '--import', type = str,
  help = 'Import a workspace from a project.tar.gz file.')

workspace_parser.add_argument(
  '--dbhost', type = str, default = 'localhost',
  help = 'Address of the PgSQL server. The default value is "localhost".')
workspace_parser.add_argument(
  '--dbport', type = int, default = 5432,
  help = 'Port of the PgSQL server. The default value is 5432.')
workspace_parser.add_argument(
  '--dbuser', type = str, default = getpass.getuser(),
  help = 'User name for the PgSQL server. The default value is the current user\
  name.')
workspace_parser.add_argument(
  '--dbpass', type = str,
  help = 'Password file for the PgSQL server.')
workspace_parser.set_defaults(func = handler.workspace.workspace_handler)

#--- Server handling options ---#

server_parser = subparsers.add_parser(
  'server', help = 'Webserver handling')

server_parser.add_argument(
  '-w', '--workdir', type = str, required = True,
  help = 'Path of workspace directory of the projects in which config files are\
  stored.')
server_parser.add_argument(
  '-p', '--port', type = int, default = 8080,
  help = 'Port number on which webserver listens. The default value is 8080.')
server_parser.add_argument(
  '-e', '--extra', type = str,
  help = 'Extra arguments passed to the CodeCompass webserver.')
server_parser.add_argument(
  '-s', '--stop', action = 'store_true',
  help = 'If this flag is given, then the CodeCompass webserver belonging to\
  the given workspace stops.')
server_parser.add_argument(
  '--dbhost', type = str, default = 'localhost',
  help = 'Address of the PgSQL server. The default value is "localhost".')
server_parser.add_argument(
  '--dbport', type = int, default = 5432,
  help = 'Port of the PgSQL server. The default value is 5432.')
server_parser.add_argument(
  '--dbuser', type = str, default = getpass.getuser(),
  help = 'User name for the PgSQL server. The default value is the current user\
  name.')
server_parser.add_argument(
  '--dbpass', type = str,
  help = 'Password file for the PgSQL server.')
server_parser.add_argument(
  '-nc', '--no-core', action = 'store_true',
  help = 'Prevent the creation of core dumps by the underlying processes.')

server_parser.set_defaults(func = handler.server.server_handler)

#--- CodeChecker handling options ---#

codechecker_parser = subparsers.add_parser(
  'codechecker', help = 'CodeChecker handling')

codechecker_parser.add_argument(
  '-w', '--workdir', type = str, required = True,
  help = 'Path of workspace directory of the projects in which config files are\
  stored.')
codechecker_parser.add_argument(
  '-n', '--name', type = str, required = True,
  help = 'Name of the project to check.')
codechecker_parser.add_argument(
  '-r', '--runname', type = str, default = None,
  help = 'Runname to store for the project. Defaults to "name"\'s value if undefined.')

codechecker_parser.add_argument(
  '-t', '--thread', type = int, default = 4,
  help = 'Number of threads to run CodeChecker on. The default value is 4.')
codechecker_parser.add_argument(
  '-e', '--extra', type = str,
  help = 'Extra arguments passed to the CodeChecker.')
codechecker_parser.add_argument(
  '--dbhost', type = str, default = 'localhost',
  help = 'Address of the PgSQL server. The default value is "localhost".')
codechecker_parser.add_argument(
  '--dbport', type = int, default = 5432,
  help = 'Port of the PgSQL server. The default value is 5432.')
codechecker_parser.add_argument(
  '--dbuser', type = str, default = getpass.getuser(),
  help = 'User name for the PgSQL server. The default value is the current user\
  name.')
codechecker_parser.add_argument(
  '--dbpass', type = str,
  help = 'Password file for the PgSQL server.')
codechecker_parser.add_argument(
  '-nc', '--no-core', action = 'store_true',
  help = 'Prevent the creation of core dumps by the underlying processes.')

codechecker_parser.set_defaults(func = handler.codechecker.codechecker_handler)

#--- Parse arguments ---#

args = parser.parse_args()

#--- Modify args and the environment ---#

args.workdir = os.path.abspath(args.workdir)

if hasattr(args, 'logfile') and args.logfile:
  args.logfile = os.path.abspath(args.logfile)

if args.dbpass:
  os.environ['PGPASSFILE'] = os.path.abspath(args.dbpass)
elif 'PGPASSFILE' in os.environ:
  args.dbpass = os.path.abspath(os.environ['PGPASSFILE'])
elif os.path.isfile('~/.pgpass'):
  os.environ['PGPASSFILE'] = os.path.abspath('~/.pgpass')
  args.dbpass = os.environ['PGPASSFILE']
else:
  args.dbpass = ''

if hasattr(args, 'labels') and args.labels:
  abs_labels = []
  
  for label in args.labels.split(':'):
    parts = label.split('=')
    abs_labels.append(parts[0] + '=' + os.path.abspath(parts[1]))
  
  args.labels = ':'.join(abs_labels)

if hasattr(args, 'no_core') and args.no_core:
  resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

#--- Do the job ---#

args.func(args)
