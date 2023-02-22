import { createContext } from 'react';
import { FileInfo } from '@thrift-generated/index';

type ProjectContextType = {
  currentWorkspace: string;
  setCurrentWorkspace: (_val: string) => void;
  rootFiles: FileInfo[];
  setRootFiles: (_val: FileInfo[]) => void;
  files: FileInfo[];
  setFiles: (_val: FileInfo[]) => void;
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
