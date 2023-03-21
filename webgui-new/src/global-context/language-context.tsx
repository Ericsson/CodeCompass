import { createContext, useContext, useEffect, useState } from 'react';
import { ProjectContext } from './project-context';
import { createCppClient, getCppFileDiagramTypes } from 'service/cpp-service';

type LanguageContextType = {
  fileDiagramTypes: Map<string, number>;
  setFileDiagramTypes: (_val: Map<string, number>) => void;
  dirDiagramTypes: Map<string, number>;
  setDirDiagramTypes: (_val: Map<string, number>) => void;
  currentFileDiagramType: string;
  setCurrentFileDiagramType: (_val: string) => void;
  currentDirDiagramType: string;
  setCurrentDirDiagramType: (_val: string) => void;
  fileType: string;
  setFileType: (_val: string) => void;
};

export const LanguageContext = createContext<LanguageContextType>({
  fileDiagramTypes: new Map(),
  setFileDiagramTypes: (_val) => {},
  dirDiagramTypes: new Map(),
  setDirDiagramTypes: (_val) => {},
  currentFileDiagramType: '',
  setCurrentFileDiagramType: (_val) => {},
  currentDirDiagramType: '',
  setCurrentDirDiagramType: (_val) => {},
  fileType: '',
  setFileType: (_val) => {},
});

export const LanguageContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [fileDiagramTypes, setFileDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentFileDiagramType, setCurrentFileDiagramType] = useState<string>('');
  const [dirDiagramTypes, setDirDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentDirDiagramType, setCurrentDirDiagramType] = useState<string>('');
  const [fileType, setFileType] = useState<string>('');

  useEffect(() => {
    if (!projectCtx.currentWorkspace) return;
    const init = async () => {
      createCppClient(projectCtx.currentWorkspace);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!projectCtx.fileInfo) return;

    const init = async () => {
      const fType = projectCtx.fileInfo?.type as string;
      const fileDiagramTypesRes =
        fType === 'CPP' ? await getCppFileDiagramTypes(projectCtx.fileInfo?.id as string) : new Map();
      const dirDiagramTypesRes = await getCppFileDiagramTypes(projectCtx.fileInfo?.parent as string);

      setFileType(fType);
      setFileDiagramTypes(fileDiagramTypesRes);
      setDirDiagramTypes(dirDiagramTypesRes);
      setCurrentFileDiagramType(Object.keys(Object.fromEntries(fileDiagramTypesRes))[0]);
      setCurrentDirDiagramType(Object.keys(Object.fromEntries(dirDiagramTypesRes))[0]);
    };
    init();
  }, [projectCtx.fileInfo]);

  const languageContext = {
    fileDiagramTypes,
    setFileDiagramTypes,
    dirDiagramTypes,
    setDirDiagramTypes,
    currentFileDiagramType,
    setCurrentFileDiagramType,
    currentDirDiagramType,
    setCurrentDirDiagramType,
    fileType,
    setFileType,
  };

  return <LanguageContext.Provider value={languageContext}>{children}</LanguageContext.Provider>;
};
