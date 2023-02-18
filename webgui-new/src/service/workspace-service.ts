import thrift from 'thrift';
import { WorkspaceService } from '@thrift-generated/index';
import { config } from './config';

const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
  transport: thrift.TBufferedTransport,
  protocol: thrift.TJSONProtocol,
  https: config.webserver_https,
  path: `${config.webserver_path}/WorkspaceService`,
});
const client = thrift.createXHRClient(WorkspaceService, connection);

export const getWorkspaces = async () => {
  return await client.getWorkspaces();
};
