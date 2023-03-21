import { createContext, useContext, useEffect, useState } from 'react';
import { FileInfo } from '@thrift-generated';
import { WorkspaceContext } from './workspace-context';
import { createProjectClient, getChildFiles, getRootFiles } from 'service/project-service';
import { getStore, setStore } from 'utils/store';
import { createCppClient } from 'service/cpp-service';

type TreeNode = {
  info: FileInfo;
  children?: TreeNode[];
};

type ProjectContextType = {
  currentWorkspace: string;
  setCurrentWorkspace: (_val: string) => void;
  rootFiles: FileInfo[];
  setRootFiles: (_val: FileInfo[]) => void;
  files: FileInfo[];
  setFiles: (_val: FileInfo[]) => void;
  fileTree: TreeNode | undefined;
  setFileTree: (_val: TreeNode | undefined) => void;
  fileContent: string;
  setFileContent: (_val: string) => void;
  fileInfo: FileInfo | undefined;
  setFileInfo: (_val: FileInfo | undefined) => void;
  selectedFile: string;
  setSelectedFile: (_val: string) => void;
  folderPath: string;
  setFolderPath: (_val: string) => void;
  expandedFileTreeNodes: string[];
  setExpandedFileTreeNodes: (_val: string[]) => void;
  loadComplete: boolean;
  setLoadComplete: (_val: boolean) => void;
};

export const ProjectContext = createContext<ProjectContextType>({
  currentWorkspace: '',
  setCurrentWorkspace: (_val) => {},
  rootFiles: [],
  setRootFiles: (_val) => {},
  files: [],
  setFiles: (_val) => {},
  fileTree: undefined,
  setFileTree: (_val) => {},
  fileContent: '',
  setFileContent: (_val) => {},
  fileInfo: undefined,
  setFileInfo: (_val) => {},
  selectedFile: '',
  setSelectedFile: (_val) => {},
  folderPath: '',
  setFolderPath: (_val) => {},
  expandedFileTreeNodes: [],
  setExpandedFileTreeNodes: (_val) => {},
  loadComplete: false,
  setLoadComplete: (_val) => {},
});

export const ProjectContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const workspaces = useContext(WorkspaceContext);

  const [currentWorkspace, setCurrentWorkspace] = useState<string | undefined>(undefined);
  const [rootFiles, setRootFiles] = useState<FileInfo[] | undefined>(undefined);
  const [files, setFiles] = useState<FileInfo[] | undefined>(undefined);
  const [fileTree, setFileTree] = useState<TreeNode | undefined>(undefined);
  const [fileContent, setFileContent] = useState<string | undefined>(undefined);
  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [selectedFile, setSelectedFile] = useState<string | undefined>(undefined);
  const [folderPath, setFolderPath] = useState<string | undefined>(undefined);
  const [expandedFileTreeNodes, setExpandedFileTreeNodes] = useState<string[] | undefined>(undefined);
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    if (!workspaces.length) {
      return;
    }
    const initWorkspace = async () => {
      const { storedWorkspace } = getStore();
      setCurrentWorkspace(storedWorkspace ?? (workspaces[0].id as string));
    };
    initWorkspace();
  }, [workspaces]);

  useEffect(() => {
    if (!currentWorkspace) {
      return;
    }
    setLoadComplete(false);
    const init = async () => {
      createProjectClient(currentWorkspace);

      const rootFileData = await getRootFiles();
      const rootDirectory = rootFileData.find((info) => info.isDirectory) as FileInfo;
      setRootFiles(rootFileData);

      const {
        storedFiles,
        storedFileTree,
        storedSelectedFile,
        storedFolderPath,
        storedFileContent,
        storedFileInfo,
        storedExpandedFileTreeNodes,
      } = getStore();

      setFiles(storedFiles ?? rootFileData);
      setFileTree(storedFileTree ?? { info: rootDirectory });
      setSelectedFile(storedSelectedFile ?? '');
      setFolderPath(storedFolderPath ?? '');
      setFileContent(storedFileContent ?? '');
      setFileInfo(storedFileInfo ?? undefined);
      setExpandedFileTreeNodes(storedExpandedFileTreeNodes ?? []);

      setStore({
        storedWorkspace: currentWorkspace,
      });
    };
    init().then(() => setLoadComplete(true));
  }, [currentWorkspace]);

  useEffect(() => {
    if (!rootFiles?.length) {
      return;
    }
    const findNodeById = (node: TreeNode, nodeId: string): TreeNode | null => {
      if (node.info.id === nodeId) {
        return node;
      }
      if (node.children) {
        for (const child of node.children) {
          const foundNode = findNodeById(child, nodeId);
          if (foundNode) {
            return foundNode;
          }
        }
      }
      return null;
    };
    const getChildNodes = async (childFiles: FileInfo[]): Promise<TreeNode[]> => {
      const childNodes: TreeNode[] = [];
      for (const childFile of childFiles) {
        if (childFile.isDirectory && expandedFileTreeNodes?.includes(childFile.id as string)) {
          const children = await getChildFiles(childFile.id as string);
          childNodes.push({ info: childFile, children: await getChildNodes(children) });
        } else {
          childNodes.push({ info: childFile } as TreeNode);
        }
      }
      return childNodes;
    };
    const updateTree = async () => {
      const rootDirectory = rootFiles.find((info) => info.isDirectory) as FileInfo;
      const childFiles = await getChildFiles(rootDirectory.id as string);
      const childTreeNodes = await getChildNodes(childFiles);
      setFileTree((prevFileTree) => {
        if (!prevFileTree) {
          return;
        }
        const updatedNode = findNodeById(prevFileTree, rootDirectory.id as string);
        if (!updatedNode) {
          return;
        }
        updatedNode.children = childTreeNodes;
        return { ...prevFileTree };
      });
    };
    updateTree();
  }, [rootFiles, expandedFileTreeNodes]);

  useEffect(() => {
    setStore({
      storedFiles: files,
      storedFileTree: fileTree,
      storedFileInfo: fileInfo,
      storedFileContent: fileContent,
      storedSelectedFile: selectedFile,
      storedFolderPath: folderPath,
      storedExpandedFileTreeNodes: expandedFileTreeNodes,
    });
  }, [files, fileTree, fileContent, fileInfo, selectedFile, folderPath, expandedFileTreeNodes]);

  const projectContext = {
    currentWorkspace: currentWorkspace as string,
    rootFiles: rootFiles as FileInfo[],
    files: files as FileInfo[],
    fileTree: fileTree as TreeNode,
    fileContent: fileContent as string,
    fileInfo: fileInfo as FileInfo,
    selectedFile: selectedFile as string,
    folderPath: folderPath as string,
    expandedFileTreeNodes: expandedFileTreeNodes as string[],
    loadComplete,
    setCurrentWorkspace,
    setRootFiles,
    setFiles,
    setFileTree,
    setFileContent,
    setFileInfo,
    setSelectedFile,
    setFolderPath,
    setExpandedFileTreeNodes,
    setLoadComplete,
  };

  return <ProjectContext.Provider value={projectContext}>{children}</ProjectContext.Provider>;
};
