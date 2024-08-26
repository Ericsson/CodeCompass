import thrift from 'thrift';
import { LanguageService, FilePosition, Position } from '@thrift-generated';
import { config } from './config';
import { toast } from 'react-toastify';

let client: LanguageService.Client | undefined;
export const createClient = (workspace: string, fileType: string | undefined) => {
  if (!config || !fileType) return;
  
  const service = () => 
  {
    switch(fileType)
    {
      case "CPP":
        return "CppService";
      case "PY":
        return "PythonService";
    }
  };

  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/${service()}`,
  });
  client = thrift.createXHRClient(LanguageService, connection);
  return client;
};

export const getFileTypes = async () => {
  if (!client) {
    return [];
  }
  return await client.getFileTypes();
};

export const getFileDiagramTypes = async (fileId: string) => {
  let resultMap = new Map<string, number>();
  if (!client) {
    return resultMap;
  }
  try {
    resultMap = await client.getFileDiagramTypes(fileId);
  } catch (e) {
    console.error(e);
    resultMap = new Map();
  }
  return resultMap;
};

export const getFileDiagram = async (fileId: string, diagramId: number) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getFileDiagram(fileId, diagramId);
  } catch (e) {
    toast.error('Could not display diagram.');
    console.error(e);
    return '';
  }
};

export const getFileDiagramLegend = async (diagramId: number) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getFileDiagramLegend(diagramId);
  } catch (e) {
    toast.error('Could not display diagram legend.');
    console.error(e);
    return '';
  }
};

export const getDiagramTypes = async (astNodeId: string) => {
  let resultMap = new Map<string, number>();
  if (!client) {
    return resultMap;
  }
  try {
    resultMap = await client.getDiagramTypes(astNodeId);
  } catch (e) {
    console.error(e);
    resultMap = new Map();
  }
  return resultMap;
};

export const getDiagram = async (astNodeId: string, diagramId: number) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getDiagram(astNodeId, diagramId);
  } catch (e) {
    toast.error('Could not display diagram.');
    console.error(e);
    return '';
  }
};

export const getDiagramLegend = async (diagramId: number) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getDiagramLegend(diagramId);
  } catch (e) {
    toast.error('Could not display diagram legend.');
    console.error(e);
    return '';
  }
};

export const getFileReferenceTypes = async (fileId: string) => {
  let resultMap = new Map<string, number>();
  if (!client) {
    return resultMap;
  }
  try {
    resultMap = await client.getFileReferenceTypes(fileId);
  } catch (e) {
    console.error(e);
    resultMap = new Map();
  }
  return resultMap;
};

export const getFileReferences = async (fileId: string, referenceId: number) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getFileReferences(fileId, referenceId);
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getFileReferenceCount = async (fileId: string, referenceId: number) => {
  if (!client) {
    return 0;
  }
  try {
    return await client.getFileReferenceCount(fileId, referenceId);
  } catch (e) {
    console.error(e);
    return 0;
  }
};

export const getReferenceTypes = async (astNodeId: string) => {
  let resultMap = new Map<string, number>();
  if (!client) {
    return resultMap;
  }
  try {
    resultMap = await client.getReferenceTypes(astNodeId);
  } catch (e) {
    console.error(e);
    resultMap = new Map();
  }
  return resultMap;
};

export const getReferences = async (astNodeId: string, referenceId: number, tags: string[]) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getReferences(astNodeId, referenceId, tags);
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getReferenceCount = async (astNodeId: string, referenceId: number) => {
  if (!client) {
    return 0;
  }
  try {
    return await client.getReferenceCount(astNodeId, referenceId);
  } catch (e) {
    console.error(e);
    return 0;
  }
};

export const getReferencesInFile = async (
  astNodeId: string,
  referenceId: number,
  fileId: string,
  tags: string[]
) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getReferencesInFile(astNodeId, referenceId, fileId, tags);
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getReferencesPage = async (
  astNodeId: string,
  referenceId: number,
  pageSize: number,
  pageNo: number
) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getReferencesPage(astNodeId, referenceId, pageSize, pageNo);
  } catch (e) {
    console.error(e);
    return [];
  }
};

export const getSourceText = async (astNodeId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getSourceText(astNodeId);
  } catch (e) {
    console.error(e);
    return '';
  }
};

export const getProperties = async (astNodeId: string) => {
  let resultMap = new Map<string, string>();
  if (!client) {
    return resultMap;
  }
  try {
    resultMap = await client.getProperties(astNodeId);
  } catch (e) {
    console.error(e);
    resultMap = new Map();
  }
  return resultMap;
};

export const getDocumentation = async (astNodeId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getDocumentation(astNodeId);
  } catch (e) {
    toast.error('Could not get documentation about this AST node.');
    console.error(e);
    return '';
  }
};

export const getAstNodeInfo = async (astNodeId: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getAstNodeInfo(astNodeId);
  } catch (e) {
    console.error(e);
    return;
  }
};

export const getAstNodeInfoByPosition = async (fileId: string, line: number, column: number) => {
  if (!client) {
    return;
  }
  try {
    return await client.getAstNodeInfoByPosition(
      new FilePosition({
        file: fileId,
        pos: new Position({
          line,
          column,
        }),
      })
    );
  } catch (e) {
    return;
  }
};
