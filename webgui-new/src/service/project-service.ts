import thrift from 'thrift';
import { ProjectService } from '@thrift-generated/index';

const getClient = (workspace: string) => {
  const connection = thrift.createXHRConnection('localhost', 8080, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    path: `/${workspace}/ProjectService`,
  });
  const client = thrift.createXHRClient(ProjectService, connection);
  return { client, connection };
};

export const getRootFiles = async (workspace: string) => {
  const { client, connection } = getClient(workspace);
  const files = await client.getRootFiles();
  connection.close();
  return files;
};

export const getParentFiles = async (workspace: string, filePath: string) => {
  const { client, connection } = getClient(workspace);
  const parentFileInfo = await client.getFileInfoByPath(filePath);
  const parentFiles = await client.getChildFiles(parentFileInfo.id as string);
  connection.close();
  return parentFiles;
};

export const getChildFiles = async (workspace: string, fileId: string) => {
  const { client, connection } = getClient(workspace);
  const files = await client.getChildFiles(fileId);
  connection.close();
  return files;
};

export const getFileContent = async (workspace: string, fileId: string) => {
  const { client, connection } = getClient(workspace);
  const fileContent = await client.getFileContent(fileId);
  connection.close();
  return fileContent;
};
