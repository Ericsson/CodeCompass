import thrift from 'thrift';
import { WorkspaceServiceClient } from '@thrift-generated/WorkspaceService';

const connection = thrift.createXHRConnection('localhost', 8080, {
  transport: thrift.TBufferedTransport,
  protocol: thrift.TJSONProtocol,
  path: '/WorkspaceService',
});
const client = thrift.createXHRClient(WorkspaceServiceClient, connection);

export const getWorkspaces = async () => {
  return await client.getWorkspaces();
};
