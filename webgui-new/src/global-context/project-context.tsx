import { createContext, useContext, useEffect, useState } from 'react';
import { FileInfo } from '@thrift-generated/index';
import { WorkspaceContext } from './workspace-context';
import { createProjectClient, getChildFiles, getRootFiles } from 'service/project-service';

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
  projectLoadComplete: boolean;
  setProjectLoadComplete: (_val: boolean) => void;
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
  projectLoadComplete: false,
  setProjectLoadComplete: (_val) => {},
});

export const ProjectContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const workspaces = useContext(WorkspaceContext);

  const [currentWorkspace, setCurrentWorkspace] = useState<string>('');
  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [fileTree, setFileTree] = useState<TreeNode | undefined>(undefined);
  const [fileContent, setFileContent] = useState<string>('');
  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [selectedFile, setSelectedFile] = useState<string>('');
  const [folderPath, setFolderPath] = useState<string>('');
  const [expandedFileTreeNodes, setExpandedFileTreeNodes] = useState<string[]>([]);
  const [projectLoadComplete, setProjectLoadComplete] = useState<boolean>(true);

  useEffect(() => {
    if (!workspaces.length) {
      return;
    }
    const initWorkspace = async () => {
      const storedCurrentWorkspace = localStorage.getItem('currentWorkspace');
      setCurrentWorkspace(storedCurrentWorkspace ?? (workspaces[0].id as string));
    };
    initWorkspace();
  }, [workspaces]);

  useEffect(() => {
    if (!currentWorkspace) {
      return;
    }
    setProjectLoadComplete(false);
    const init = async () => {
      const rootFileData = await getRootFiles();
      const rootDirectory = rootFileData.find((info) => info.isDirectory) as FileInfo;

      const storedRootFiles = localStorage.getItem('currentRootFiles');
      setRootFiles(() => (storedRootFiles ? JSON.parse(storedRootFiles) : rootFileData));

      const storedCurrentFiles = localStorage.getItem('currentFiles');
      setFiles(storedCurrentFiles ? JSON.parse(storedCurrentFiles) : rootFileData);

      const storedFileTree = localStorage.getItem('currentFileTree');
      setFileTree(storedFileTree ? JSON.parse(storedFileTree) : { info: rootDirectory });

      const storedCurrentSelectedFile = localStorage.getItem('currentSelectedFile');
      setSelectedFile(storedCurrentSelectedFile ?? '');

      const storedCurrentPath = localStorage.getItem('currentPath');
      setFolderPath(storedCurrentPath ?? '');

      const storedCurrentFileContent = localStorage.getItem('currentFileContent');
      setFileContent(storedCurrentFileContent ?? '');

      const storedCurrentFileInfo = localStorage.getItem('currentFileInfo');
      setFileInfo(storedCurrentFileInfo ? JSON.parse(storedCurrentFileInfo) : undefined);

      const storedExpandedNodes = localStorage.getItem('expandedNodes');
      setExpandedFileTreeNodes(storedExpandedNodes ? JSON.parse(storedExpandedNodes) : []);

      createProjectClient(currentWorkspace);
    };
    init().then(() => setProjectLoadComplete(true));
  }, [currentWorkspace]);

  useEffect(() => {
    if (!rootFiles.length) {
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
        if (childFile.isDirectory && expandedFileTreeNodes.includes(childFile.id as string)) {
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
        localStorage.setItem('currentFileTree', JSON.stringify({ ...prevFileTree }));
        return { ...prevFileTree };
      });
    };
    updateTree();
  }, [rootFiles, expandedFileTreeNodes]);

  const projectContext = {
    currentWorkspace,
    setCurrentWorkspace,
    rootFiles,
    setRootFiles,
    files,
    setFiles,
    fileTree,
    setFileTree,
    fileContent,
    setFileContent,
    fileInfo,
    setFileInfo,
    selectedFile,
    setSelectedFile,
    folderPath,
    setFolderPath,
    expandedFileTreeNodes,
    setExpandedFileTreeNodes,
    projectLoadComplete,
    setProjectLoadComplete,
  };

  return <ProjectContext.Provider value={projectContext}>{children}</ProjectContext.Provider>;
};
