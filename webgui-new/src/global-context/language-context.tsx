import { createContext, useContext, useEffect, useState } from 'react';
import { ProjectContext } from './project-context';
import { createCppClient, getCppFileDiagramTypes } from 'service/cpp-service';
import { AstNodeInfo, FileInfo, FileRange, Range } from '@thrift-generated';

type LanguageContextType = {
  diagramFileInfo: FileInfo | undefined;
  setDiagramFileInfo: (_val: FileInfo | undefined) => void;
  fileDiagramTypes: Map<string, number>;
  setFileDiagramTypes: (_val: Map<string, number>) => void;
  currentFileDiagramType: string;
  setCurrentFileDiagramType: (_val: string) => void;
  fileType: string;
  setFileType: (_val: string) => void;
  astNodeInfo: AstNodeInfo | undefined;
  setAstNodeInfo: (_val: AstNodeInfo | undefined) => void;
  nodeSelectionRange: Range | undefined;
  setNodeSelectionRange: (_val: Range | undefined) => void;
};

export const LanguageContext = createContext<LanguageContextType>({
  diagramFileInfo: undefined,
  setDiagramFileInfo: (_val) => {},
  fileDiagramTypes: new Map(),
  setFileDiagramTypes: (_val) => {},
  currentFileDiagramType: '',
  setCurrentFileDiagramType: (_val) => {},
  fileType: '',
  setFileType: (_val) => {},
  astNodeInfo: undefined,
  setAstNodeInfo: (_val) => {},
  nodeSelectionRange: undefined,
  setNodeSelectionRange: (_val) => {},
});

export const LanguageContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [diagramFileInfo, setDiagramFileInfo] = useState<FileInfo | undefined>(undefined);
  const [fileDiagramTypes, setFileDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentFileDiagramType, setCurrentFileDiagramType] = useState<string>('');
  const [fileType, setFileType] = useState<string>('');
  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [nodeSelectionRange, setNodeSelectionRange] = useState<FileRange | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) return;
    const init = async () => {
      createCppClient(projectCtx.currentWorkspace);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!diagramFileInfo) return;

    const init = async () => {
      const fType = diagramFileInfo?.type as string;
      const fileDiagramTypesRes = await getCppFileDiagramTypes(diagramFileInfo?.id as string);

      setFileType(fType);
      setFileDiagramTypes(fileDiagramTypesRes);
      setCurrentFileDiagramType(Object.keys(Object.fromEntries(fileDiagramTypesRes))[0]);
    };
    init();
  }, [diagramFileInfo]);

  const languageContext = {
    diagramFileInfo,
    setDiagramFileInfo,
    fileDiagramTypes,
    setFileDiagramTypes,
    currentFileDiagramType,
    setCurrentFileDiagramType,
    fileType,
    setFileType,
    astNodeInfo,
    setAstNodeInfo,
    nodeSelectionRange,
    setNodeSelectionRange,
  };

  return <LanguageContext.Provider value={languageContext}>{children}</LanguageContext.Provider>;
};
