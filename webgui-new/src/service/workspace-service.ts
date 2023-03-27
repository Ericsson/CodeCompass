import thrift from 'thrift';
import { WorkspaceService } from '@thrift-generated';
import { config } from './config';

let client: WorkspaceService.Client | undefined;
export const createWorkspaceClient = () => {
  if (!config) return;
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/WorkspaceService`,
  });
  client = thrift.createXHRClient(WorkspaceService, connection);
  return client;
};

export const getWorkspaces = async () => {
  if (!client) {
    return [];
  }
  return await client.getWorkspaces();
};
