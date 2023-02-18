import { createContext } from 'react';
import { FileInfo } from '@thrift-generated/index';

type ProjectContextType = {
  currentWorkspace: string;
  setCurrentWorkspace: (_val: string) => void;
  fileContent: string | undefined;
  setFileContent: (_val: string | undefined) => void;
  fileInfo: FileInfo | undefined;
  setFileInfo: (_val: FileInfo | undefined) => void;
  projectLoadComplete: boolean;
  setProjectLoadComplete: (_val: boolean) => void;
};

export const ProjectContext = createContext<ProjectContextType>({
  currentWorkspace: '',
  setCurrentWorkspace: (_val) => {},
  fileContent: undefined,
  setFileContent: (_val) => {},
  fileInfo: undefined,
  setFileInfo: (_val) => {},
  projectLoadComplete: false,
  setProjectLoadComplete: (_val) => {},
});
