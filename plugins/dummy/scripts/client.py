import sys

sys.path.append('gen-py')
sys.path.append('thrift-0.9.3/lib/py/build/lib.linux-x86_64-2.7')

from thrift import Thrift
from thrift.transport import THttpClient
from thrift.protocol import TJSONProtocol

from dummy import DummyService

#--- Server information ---#

host = 'md-mtas2.tsp.eth.ericsson.se'
port = 8080
workspace = 'proba'

#--- Create client objects ---#

def create_client(service, service_name):
  '''This function initializes the Thrift client and returns the client objects
  to the API.'''

  path = '/' + workspace + '/' + service_name

  transport = THttpClient.THttpClient(host, port, path)
  protocol = TJSONProtocol.TJSONProtocol(transport)

  return service.Client(protocol)

dummyservice = create_client(DummyService, 'DummyService')

#--- Do the job ---#

def main():
  print dummyservice.getDummyString()

if __name__ == "__main__":
  main()
