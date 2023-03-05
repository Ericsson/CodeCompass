import thrift from 'thrift';
import { ProjectService } from '@thrift-generated';
import { config } from './config';

let client: ProjectService.Client | undefined;
export const createProjectClient = (workspace: string) => {
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/ProjectService`,
  });
  client = thrift.createXHRClient(ProjectService, connection);
  return client;
};

export const getRootFiles = async () => {
  if (!client) {
    return [];
  }
  const files = await client.getRootFiles();
  return files;
};

export const getParentFiles = async (filePath: string) => {
  if (!client) {
    return [];
  }
  const parentFileInfo = await client.getFileInfoByPath(filePath);
  const parentFiles = await client.getChildFiles(parentFileInfo.id as string);
  return parentFiles;
};

export const getParents = async (filePath: string) => {
  if (!client) {
    return [];
  }
  const parents: string[] = [];
  let currentPath = filePath;
  while (currentPath) {
    let parent = await client.getFileInfoByPath(currentPath);
    parents.push(parent.id as string);
    const pathAsArray = currentPath.split('/');
    pathAsArray.pop();
    currentPath = pathAsArray.join('/');
  }
  const root = await client.getFileInfoByPath('/');
  parents.push(root.id as string);
  return parents;
};

export const getChildFiles = async (fileId: string) => {
  if (!client) {
    return [];
  }
  const files = await client.getChildFiles(fileId);
  return files;
};

export const getFileContent = async (fileId: string) => {
  if (!client) {
    return '';
  }
  const fileContent = await client.getFileContent(fileId);
  return fileContent;
};
