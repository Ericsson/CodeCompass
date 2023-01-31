import thrift from 'thrift';
import { WorkspaceService } from '../../build/workspace/cc/service/workspace/index';

const connection = thrift.createXHRConnection('localhost', 8080, {
  transport: thrift.TBufferedTransport,
  protocol: thrift.TJSONProtocol,
  path: '/WorkspaceService',
});
const client = thrift.createXHRClient(WorkspaceService, connection);

export const getWorkspaces = async () => {
  return await client.getWorkspaces();
};
