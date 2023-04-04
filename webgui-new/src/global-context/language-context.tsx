import { createContext, useContext, useEffect, useState } from 'react';
import { ProjectContext } from './project-context';
import { createCppClient, getCppDiagramTypes, getCppFileDiagramTypes } from 'service/cpp-service';
import { AstNodeInfo, FileInfo, FileRange, Range } from '@thrift-generated';
import { createCppReparseClient } from 'service/cpp-reparse-service';

type LanguageContextType = {
  diagramInfo: FileInfo | AstNodeInfo | undefined;
  setDiagramInfo: (_val: FileInfo | AstNodeInfo | undefined) => void;
  diagramTypes: Map<string, number>;
  setDiagramTypes: (_val: Map<string, number>) => void;
  currentDiagramType: string;
  setCurrentDiagramType: (_val: string) => void;
  entityType: string;
  setEntityType: (_val: string) => void;
  astNodeInfo: AstNodeInfo | undefined;
  setAstNodeInfo: (_val: AstNodeInfo | undefined) => void;
  nodeSelectionRange: Range | undefined;
  setNodeSelectionRange: (_val: Range | undefined) => void;
};

export const LanguageContext = createContext<LanguageContextType>({
  diagramInfo: undefined,
  setDiagramInfo: (_val) => {},
  diagramTypes: new Map(),
  setDiagramTypes: (_val) => {},
  currentDiagramType: '',
  setCurrentDiagramType: (_val) => {},
  entityType: '',
  setEntityType: (_val) => {},
  astNodeInfo: undefined,
  setAstNodeInfo: (_val) => {},
  nodeSelectionRange: undefined,
  setNodeSelectionRange: (_val) => {},
});

export const LanguageContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [diagramInfo, setDiagramInfo] = useState<FileInfo | AstNodeInfo | undefined>(undefined);
  const [diagramTypes, setDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentDiagramType, setCurrentDiagramType] = useState<string>('');
  const [entityType, setEntityType] = useState<string>('');
  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [nodeSelectionRange, setNodeSelectionRange] = useState<FileRange | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) return;
    const init = async () => {
      createCppClient(projectCtx.currentWorkspace);
      createCppReparseClient(projectCtx.currentWorkspace);
      setAstNodeInfo(undefined);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!diagramInfo) return;

    const init = async () => {
      let initEntityType = '';
      let initDiagramTypes: typeof diagramTypes = new Map();

      if (diagramInfo instanceof FileInfo) {
        initEntityType = diagramInfo?.type as string;
        initDiagramTypes = await getCppFileDiagramTypes(diagramInfo?.id as string);
      } else if (diagramInfo instanceof AstNodeInfo) {
        initEntityType = diagramInfo.astNodeType as string;
        initDiagramTypes = await getCppDiagramTypes(diagramInfo?.id as string);
      }

      setEntityType(initEntityType);
      setDiagramTypes(initDiagramTypes);
      setCurrentDiagramType(Object.keys(Object.fromEntries(initDiagramTypes))[0]);
    };
    init();
  }, [diagramInfo]);

  const languageContext = {
    diagramInfo,
    setDiagramInfo,
    diagramTypes,
    setDiagramTypes,
    currentDiagramType,
    setCurrentDiagramType,
    entityType,
    setEntityType,
    astNodeInfo,
    setAstNodeInfo,
    nodeSelectionRange,
    setNodeSelectionRange,
  };

  return <LanguageContext.Provider value={languageContext}>{children}</LanguageContext.Provider>;
};
