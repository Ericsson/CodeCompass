import thrift from 'thrift';
import { WorkspaceService } from '../../build/cc/service/workspace';

export const getWorkspaces = async () => {
  const connection = thrift.createXHRConnection('localhost', 8080, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    path: '/WorkspaceService',
  });
  const client = thrift.createXHRClient(WorkspaceService, connection);
  const workspaces = await client.getWorkspaces();
  return workspaces;
};
