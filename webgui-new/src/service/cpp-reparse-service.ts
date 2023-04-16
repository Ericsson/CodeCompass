import thrift from 'thrift';
import { CppReparseService } from '@thrift-generated';
import { config } from './config';

let client: CppReparseService.Client | undefined;
export const createCppReparseClient = (workspace: string) => {
  if (!config) return;
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/CppReparseService`,
  });
  client = thrift.createXHRClient(CppReparseService, connection);
  return client;
};

export const getAsHTML = async (fileId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getAsHTML(fileId);
  } catch {
    return '';
  }
};

export const getAsHTMLForNode = async (nodeId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getAsHTMLForNode(nodeId);
  } catch {
    return '';
  }
};
