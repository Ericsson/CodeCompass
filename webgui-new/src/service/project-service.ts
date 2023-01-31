import thrift from 'thrift';
import { ProjectService } from '../../build/project/cc/service/core/index';

const connection = thrift.createXHRConnection('localhost', 8080, {
  transport: thrift.TBufferedTransport,
  protocol: thrift.TJSONProtocol,
  // TODO: dynamic path based on workspace
  path: '/cjson/ProjectService',
});
const client = thrift.createXHRClient(ProjectService, connection);

export const getRootFiles = async () => {
  return await client.getRootFiles();
};

export const getParent = async (fileId: string) => {
  return await client.getParent(fileId);
};

export const getChildFiles = async (fileId: string) => {
  return await client.getChildFiles(fileId);
};

export const getFileContent = async (fileId: string) => {
  return await client.getFileContent(fileId);
};
