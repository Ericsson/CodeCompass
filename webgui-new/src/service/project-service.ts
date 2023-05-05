import thrift from 'thrift';
import { ProjectService } from '@thrift-generated';
import { config } from './config';

let client: ProjectService.Client | undefined;
export const createProjectClient = (workspace: string) => {
  if (!config) return;
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
  try {
    return await client.getRootFiles();
  } catch (e) {
    console.error(e);
    return [];
  }
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
    const parent = await client.getFileInfoByPath(currentPath);
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
  try {
    return await client.getChildFiles(fileId);
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getFileContent = async (fileId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getFileContent(fileId);
  } catch (e) {
    console.error(e);
    return '';
  }
};

export const getFileInfo = async (fileId: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getFileInfo(fileId);
  } catch (e) {
    console.error(e);
    return;
  }
};

export const getFileInfoByPath = async (filePath: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getFileInfoByPath(filePath);
  } catch (e) {
    console.error(e);
    return;
  }
};

export const getLabels = async () => {
  if (!client) {
    return;
  }
  try {
    return await client.getLabels();
  } catch (e) {
    console.error(e);
    return;
  }
};
