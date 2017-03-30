import getpass

def add_db_commandline_arguments(command):
  ''' This function add database arguments such as host, port, use rname and
  password file for the command line interface'''

  command.add_argument(
    '--dbhost', type = str, default = 'localhost',
    help = 'Address of the PgSQL server. The default value is "localhost".')
  command.add_argument(
    '--dbport', type = int, default = 5432,
    help = 'Port of the PgSQL server. The default value is 5432.')
  command.add_argument(
    '--dbuser', type = str, default = getpass.getuser(),
    help = 'User name for the PgSQL server. The default value is the current user\
    name.')
  command.add_argument(
    '--dbpass', type = str,
    help = 'Password file for the PgSQL server.')

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
