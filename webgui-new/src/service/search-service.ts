import thrift from 'thrift';
import { SearchParams, SearchRange, SearchService } from '@thrift-generated';
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

export const getSearchResults = async (options: number, query: string, start: number, maxSize: number) => {
  if (!client) {
    return [];
  }
  const searchResults = await client.search(
    new SearchParams({
      options,
      query,
      range: new SearchRange({ start, maxSize }),
    })
  );
  return searchResults;
};
