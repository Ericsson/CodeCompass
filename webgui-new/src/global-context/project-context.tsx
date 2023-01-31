import { createContext } from 'react';
import { FileInfo } from '../../build/project/cc/service/core';

type ContextType = {
  fileContent: string | undefined;
  setFileContent: (_val: string | undefined) => void;
  fileInfo: FileInfo | undefined;
  setFileInfo: (_val: FileInfo | undefined) => void;
};

export const ProjectContext = createContext<ContextType>({
  fileContent: undefined,
  setFileContent: (_val) => {},
  fileInfo: undefined,
  setFileInfo: (_val) => {},
});
