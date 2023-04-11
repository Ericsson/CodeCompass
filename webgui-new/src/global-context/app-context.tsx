import { createContext, useEffect, useState } from 'react';
import { Position, Range, WorkspaceInfo } from '@thrift-generated';
import { createWorkspaceClient, getWorkspaces } from 'service/workspace-service';
import { createProjectClient } from 'service/project-service';
import { createSearchClient } from 'service/search-service';
import { createCppClient } from 'service/cpp-service';
import { createCppReparseClient } from 'service/cpp-reparse-service';
import { createMetricsClient } from 'service/metrics-service';
import { createGitClient } from 'service/git-service';
import { createConfig } from 'service/config';
import { getStore, setStore } from 'utils/store';
import { RouterQueryType, SearchProps } from 'utils/types';
import { useRouter } from 'next/router';
import { TabName } from 'enums/tab-enum';
import { AccordionLabel } from 'enums/accordion-enum';

type AppContextProps = {
  workspaces: WorkspaceInfo[];
  setWorkspaces: (_val: WorkspaceInfo[]) => void;
  workspaceId: string;
  setWorkspaceId: (_val: string) => void;
  projectFileId: string;
  setProjectFileId: (_val: string) => void;
  searchProps: SearchProps | undefined;
  setSearchProps: (_val: SearchProps | undefined) => void;
  metricsGenId: string;
  setMetricsGenId: (_val: string) => void;
  diagramGenId: string;
  setDiagramGenId: (_val: string) => void;
  languageNodeId: string;
  setLanguageNodeId: (_val: string) => void;
  editorSelection: Range | undefined;
  setEditorSelection: (_val: Range | undefined) => void;
  gitRepoId: string;
  setGitRepoId: (_val: string) => void;
  gitBranch: string;
  setGitBranch: (_val: string) => void;
  gitCommitId: string;
  setGitCommitId: (_val: string) => void;
  activeAccordion: string;
  setActiveAccordion: (_val: string) => void;
  activeTab: number;
  setActiveTab: (_val: number) => void;
  treeViewOption: boolean;
  setTreeViewOption: (_val: boolean) => void;
  loadComplete: boolean;
  setLoadComplete: (_val: boolean) => void;
};

export const AppContext = createContext<AppContextProps>({
  workspaces: [],
  setWorkspaces: (_val) => {},
  workspaceId: '',
  setWorkspaceId: (_val) => {},
  projectFileId: '',
  setProjectFileId: (_val) => {},
  searchProps: undefined,
  setSearchProps: (_val) => {},
  metricsGenId: '',
  setMetricsGenId: (_val) => {},
  diagramGenId: '',
  setDiagramGenId: (_val) => {},
  languageNodeId: '',
  setLanguageNodeId: (_val) => {},
  editorSelection: undefined,
  setEditorSelection: (_val) => {},
  gitRepoId: '',
  setGitRepoId: (_val) => {},
  gitBranch: '',
  setGitBranch: (_val) => {},
  gitCommitId: '',
  setGitCommitId: (_val) => {},
  activeAccordion: '',
  setActiveAccordion: (_val) => {},
  activeTab: 0,
  setActiveTab: (_val) => {},
  treeViewOption: false,
  setTreeViewOption: (_val) => {},
  loadComplete: false,
  setLoadComplete: (_val) => {},
});

export const AppContextController = ({ children }: { children: JSX.Element }): JSX.Element => {
  const router = useRouter();
  const routerQuery = router.query as RouterQueryType;

  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[] | undefined>(undefined);
  const [workspaceId, setWorkspaceId] = useState<string | undefined>(undefined);
  const [projectFileId, setProjectFileId] = useState<string | undefined>(undefined);
  const [searchProps, setSearchProps] = useState<SearchProps | undefined>(undefined);
  const [metricsGenId, setMetricsGenId] = useState<string | undefined>(undefined);
  const [diagramGenId, setDiagramGenId] = useState<string | undefined>(undefined);
  const [languageNodeId, setLanguageNodeId] = useState<string | undefined>(undefined);
  const [editorSelection, setEditorSelection] = useState<Range | undefined>(undefined);
  const [gitRepoId, setGitRepoId] = useState<string | undefined>(undefined);
  const [gitBranch, setGitBranch] = useState<string | undefined>(undefined);
  const [gitCommitId, setGitCommitId] = useState<string | undefined>(undefined);
  const [activeAccordion, setActiveAccordion] = useState<string | undefined>(undefined);
  const [activeTab, setActiveTab] = useState<number | undefined>(undefined);
  const [treeViewOption, setTreeViewOption] = useState<boolean | undefined>(undefined);
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    const init = async () => {
      const wHost = window.location.hostname;
      const wPort = window.location.port;
      const wHTTPS = window.location.protocol === 'https:';
      const wPath = window.location.pathname;

      createConfig({
        webserver_host: wHost,
        webserver_port: wHTTPS && !wPort ? 443 : parseInt(wPort),
        webserver_https: wHTTPS,
        webserver_path: wPath === '/new' ? '' : wPath.slice(0, wPath.lastIndexOf('/new')),
      });

      createWorkspaceClient();
      const initWorkspaces = await getWorkspaces();
      setWorkspaces(initWorkspaces);
    };
    init();
  }, []);

  useEffect(() => {
    if (!workspaces) return;
    const { storedWorkspaceId } = getStore();
    setWorkspaceId(storedWorkspaceId ?? (workspaces[0].id as string));
  }, [workspaces]);

  useEffect(() => {
    if (!routerQuery.wsId) return;
    setWorkspaceId(routerQuery.wsId);
  }, [routerQuery.wsId]);

  useEffect(() => {
    if (!workspaceId) return;
    setLoadComplete(false);
    const initializeApp = async () => {
      await new Promise((resolve) => resolve(''));

      createProjectClient(workspaceId);
      createSearchClient(workspaceId);
      createCppClient(workspaceId);
      createCppReparseClient(workspaceId);
      createMetricsClient(workspaceId);
      createGitClient(workspaceId);

      const {
        storedProjectFileId,
        storedSearchProps,
        storedMetricsGenId,
        storedDiagramGenId,
        storedLanguageNodeId,
        storedEditorSelection,
        storedGitRepoId,
        storedGitBranch,
        storedGitCommitId,
        storedActiveAccordion,
        storedActiveTab,
        storedTreeViewOption,
      } = getStore();

      setProjectFileId(storedProjectFileId);
      setSearchProps(storedSearchProps ?? undefined);
      setMetricsGenId(storedMetricsGenId);
      setDiagramGenId(storedDiagramGenId);
      setLanguageNodeId(storedLanguageNodeId);
      setEditorSelection(storedEditorSelection);
      setGitRepoId(storedGitRepoId);
      setGitBranch(storedGitBranch);
      setGitCommitId(storedGitCommitId);
      setActiveAccordion(storedActiveAccordion ?? AccordionLabel.FILE_MANAGER);
      setActiveTab(storedActiveTab ?? 0);
      setTreeViewOption(storedTreeViewOption ?? false);

      setStore({
        storedWorkspaceId: workspaceId,
      });

      const routerQuery = router.query as RouterQueryType;
      if (routerQuery) {
        if (!routerQuery.projFileId) return;
        setProjectFileId(routerQuery.projFileId);
        setActiveAccordion(AccordionLabel.FILE_MANAGER);
        setActiveTab(TabName.CODE);

        if (!routerQuery.selection) return;
        const selection = routerQuery.selection.split('|');
        const startLine = parseInt(selection[0]);
        const startCol = parseInt(selection[1]);
        const endLine = parseInt(selection[2]);
        const endCol = parseInt(selection[3]);

        const startpos = new Position({
          line: startLine,
          column: startCol,
        });
        const endpos = new Position({
          line: endLine,
          column: endCol,
        });
        const range = new Range({
          startpos,
          endpos,
        });

        setEditorSelection(range);

        router.replace({
          pathname: '/project',
          query: {},
        });
      }
    };
    initializeApp().then(() => setLoadComplete(true));
  }, [workspaceId, router]);

  useEffect(() => {
    setStore({
      storedProjectFileId: projectFileId,
      storedSearchProps: searchProps,
      storedMetricsGenId: metricsGenId,
      storedDiagramGenId: diagramGenId,
      storedLanguageNodeId: languageNodeId,
      storedEditorSelection: editorSelection,
      storedGitRepoId: gitRepoId,
      storedGitBranch: gitBranch,
      storedGitCommitId: gitCommitId,
      storedActiveAccordion: activeAccordion,
      storedActiveTab: activeTab,
      storedTreeViewOption: treeViewOption,
    });
  }, [
    projectFileId,
    searchProps,
    metricsGenId,
    diagramGenId,
    languageNodeId,
    editorSelection,
    gitRepoId,
    gitBranch,
    gitCommitId,
    activeAccordion,
    activeTab,
    treeViewOption,
  ]);

  const appContext = {
    workspaces: workspaces as WorkspaceInfo[],
    workspaceId: workspaceId as string,
    projectFileId: projectFileId as string,
    searchProps: searchProps,
    metricsGenId: metricsGenId as string,
    diagramGenId: diagramGenId as string,
    languageNodeId: languageNodeId as string,
    editorSelection: editorSelection as Range,
    gitRepoId: gitRepoId as string,
    gitBranch: gitBranch as string,
    gitCommitId: gitCommitId as string,
    activeAccordion: activeAccordion as string,
    activeTab: activeTab as number,
    treeViewOption: treeViewOption as boolean,
    loadComplete,
    setWorkspaces,
    setWorkspaceId,
    setProjectFileId,
    setSearchProps,
    setMetricsGenId,
    setDiagramGenId,
    setLanguageNodeId,
    setEditorSelection,
    setGitRepoId,
    setGitBranch,
    setGitCommitId,
    setActiveAccordion,
    setActiveTab,
    setTreeViewOption,
    setLoadComplete,
  };

  return <AppContext.Provider value={appContext}>{children}</AppContext.Provider>;
};
