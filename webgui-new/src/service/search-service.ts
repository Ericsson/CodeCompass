import thrift from 'thrift';
import { SearchService } from '@thrift-generated';
import { config } from './config';

let client: SearchService.Client | undefined;
export const createSearchClient = (workspace: string) => {
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/SearchService`,
  });
  client = thrift.createXHRClient(SearchService, connection);
  return client;
};

export const getSearchTypes = async () => {
  if (!client) {
    return [];
  }
  const searchTypes = await client.getSearchTypes();
  return searchTypes;
};
