import thrift from 'thrift';
import { SearchFilter, SearchParams, SearchRange, SearchService } from '@thrift-generated';
import { config } from './config';
import { toast } from 'react-toastify';

let client: SearchService.Client | undefined;
export const createSearchClient = (workspace: string) => {
  if (!config) return;
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
  try {
    return await client.getSearchTypes();
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getSearchResultCount = async (
  file: boolean,
  options: number,
  query: string,
  fileFilter = '',
  dirFilter = ''
) => {
  if (!client) {
    return [];
  }
  try {
    const searchResults = file
      ? await client.searchFile(
          new SearchParams({
            options,
            query,
            filter: new SearchFilter({ fileFilter, dirFilter }),
          })
        )
      : await client.search(
          new SearchParams({
            options,
            query,
            filter: new SearchFilter({ fileFilter, dirFilter }),
          })
        );
    return searchResults.results?.length;
  } catch (e) {
    console.error(e);
    return 0;
  }
};

export const getSearchResults = async (
  file: boolean,
  options: number,
  query: string,
  start: number,
  maxSize: number,
  fileFilter = '',
  dirFilter = ''
) => {
  if (!client) {
    return [];
  }
  try {
    return file
      ? await client.searchFile(
          new SearchParams({
            options,
            query,
            range: new SearchRange({ start, maxSize }),
            filter: new SearchFilter({ fileFilter, dirFilter }),
          })
        )
      : await client.search(
          new SearchParams({
            options,
            query,
            range: new SearchRange({ start, maxSize }),
            filter: new SearchFilter({ fileFilter, dirFilter }),
          })
        );
  } catch (e) {
    toast.error('Could not display search results.');
    console.error(e);
    return;
  }
};
