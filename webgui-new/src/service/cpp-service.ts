import thrift from 'thrift';
import { LanguageService } from '@thrift-generated';
import { config } from './config';

let client: LanguageService.Client | undefined;
export const createCppClient = (workspace: string) => {
  if (!config) return;
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/CppService`,
  });
  client = thrift.createXHRClient(LanguageService, connection);
  return client;
};

export const getCppFileDiagramTypes = async (fileId: string) => {
  let resultMap = new Map<string, number>();
  if (!client) {
    return resultMap;
  }
  resultMap = await client.getFileDiagramTypes(fileId);
  return resultMap;
};

export const getCppFileDiagram = async (fileId: string, diagramId: number) => {
  if (!client) {
    return '';
  }
  return await client.getFileDiagram(fileId, diagramId);
};

export const getCppFileDiagramLegend = async (diagramId: number) => {
  if (!client) {
    return '';
  }
  return await client.getFileDiagramLegend(diagramId);
};
