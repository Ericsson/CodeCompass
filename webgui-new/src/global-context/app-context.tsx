import React, { createContext, useEffect, useState } from 'react';
import { GitBlameHunk, Position, Range, WorkspaceInfo } from '@thrift-generated';
import { createWorkspaceClient, getWorkspaces } from 'service/workspace-service';
import { createProjectClient, getLabels } from 'service/project-service';
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

/* eslint-disable no-unused-vars */
/* eslint-disable @typescript-eslint/no-empty-function */
/* eslint-disable @typescript-eslint/no-unused-vars */
type AppContextProps = {
  workspaces: WorkspaceInfo[];
  setWorkspaces: (_val: WorkspaceInfo[]) => void;
  labels: Map<string, string>;
  setLabels: (_val: Map<string, string>) => void;
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
  diagramTypeId: number;
  setDiagramTypeId: (_val: number) => void;
  diagramType: 'file' | 'ast';
  setDiagramType: (_val: 'file' | 'ast') => void;
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
  gitBlameInfo: GitBlameHunk[];
  setGitBlameInfo: (_val: GitBlameHunk[]) => void;
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
  labels: new Map(),
  setLabels: (_val) => {},
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
  diagramTypeId: -1,
  setDiagramTypeId: (_val) => {},
  diagramType: 'file',
  setDiagramType: (_val) => {},
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
  gitBlameInfo: [],
  setGitBlameInfo: (_val) => {},
  activeAccordion: '',
  setActiveAccordion: (_val) => {},
  activeTab: 0,
  setActiveTab: (_val) => {},
  treeViewOption: false,
  setTreeViewOption: (_val) => {},
  loadComplete: false,
  setLoadComplete: (_val) => {},
});
/* eslint-enable no-unused-vars */
/* eslint-enable @typescript-eslint/no-empty-function */
/* eslint-enable @typescript-eslint/no-unused-vars */

export const AppContextController = ({ children }: { children: React.ReactNode }): JSX.Element => {
  const router = useRouter();
  const routerQuery = router.query as RouterQueryType;

  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[] | undefined>(undefined);
  const [labels, setLabels] = useState<Map<string, string>>(new Map());
  const [workspaceId, setWorkspaceId] = useState<string | undefined>(undefined);
  const [projectFileId, setProjectFileId] = useState<string | undefined>(undefined);
  const [searchProps, setSearchProps] = useState<SearchProps | undefined>(undefined);
  const [metricsGenId, setMetricsGenId] = useState<string | undefined>(undefined);
  const [diagramGenId, setDiagramGenId] = useState<string | undefined>(undefined);
  const [diagramTypeId, setDiagramTypeId] = useState<number | undefined>(undefined);
  const [diagramType, setDiagramType] = useState<'file' | 'ast' | undefined>(undefined);
  const [languageNodeId, setLanguageNodeId] = useState<string | undefined>(undefined);
  const [editorSelection, setEditorSelection] = useState<Range | undefined>(undefined);
  const [gitRepoId, setGitRepoId] = useState<string | undefined>(undefined);
  const [gitBranch, setGitBranch] = useState<string | undefined>(undefined);
  const [gitCommitId, setGitCommitId] = useState<string | undefined>(undefined);
  const [gitBlameInfo, setGitBlameInfo] = useState<GitBlameHunk[]>([]);
  const [activeAccordion, setActiveAccordion] = useState<string | undefined>(undefined);
  const [activeTab, setActiveTab] = useState<number | undefined>(undefined);
  const [treeViewOption, setTreeViewOption] = useState<boolean | undefined>(undefined);
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    const init = async () => {
      const url = window.location;

      const wHost = url.hostname;
      const wPort = url.port;
      const wHTTPS = url.protocol === 'https:';
      const wPath = url.pathname;

      createConfig({
        webserver_host: wHost,
        webserver_port: wHTTPS && !wPort ? 443 : parseInt(wPort),
        webserver_https: wHTTPS,
        webserver_path: wPath.includes('/new') ? wPath.slice(0, wPath.lastIndexOf('/new')) : wPath,
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
    setWorkspaceId(routerQuery.workspaceId ?? storedWorkspaceId ?? (workspaces[0].id as string));
  }, [workspaces, routerQuery.workspaceId]);

  useEffect(() => {
    if (!workspaceId) return;
    setLoadComplete(false);
    const initializeApp = async () => {
      createProjectClient(workspaceId);
      createSearchClient(workspaceId);
      createCppClient(workspaceId);
      createCppReparseClient(workspaceId);
      createMetricsClient(workspaceId);
      createGitClient(workspaceId);

      const initLabels = (await getLabels()) ?? new Map();
      setLabels(initLabels);

      const {
        storedProjectFileId,
        storedSearchProps,
        storedMetricsGenId,
        storedDiagramGenId,
        storedDiagramTypeId,
        storedDiagramType,
        storedLanguageNodeId,
        storedEditorSelection,
        storedGitRepoId,
        storedGitBranch,
        storedGitCommitId,
        storedActiveAccordion,
        storedActiveTab,
        storedTreeViewOption,
      } = getStore();

      setProjectFileId(routerQuery.projectFileId ?? storedProjectFileId);
      setMetricsGenId(routerQuery.metricsGenId ?? storedMetricsGenId);
      setDiagramGenId(routerQuery.diagramGenId ?? storedDiagramGenId);
      setDiagramTypeId(routerQuery.diagramTypeId ? parseInt(routerQuery.diagramTypeId) : storedDiagramTypeId);
      setDiagramType(storedDiagramType);
      setLanguageNodeId(routerQuery.languageNodeId ?? storedLanguageNodeId);
      setEditorSelection(storedEditorSelection);
      setSearchProps(storedSearchProps);
      setGitRepoId(routerQuery.gitRepoId ?? storedGitRepoId);
      setGitBranch(routerQuery.gitBranch ?? storedGitBranch);
      setGitCommitId(routerQuery.gitCommitId ?? storedGitCommitId);
      setGitBlameInfo([]);
      setActiveAccordion(routerQuery.activeAccordion ?? storedActiveAccordion ?? AccordionLabel.FILE_MANAGER);
      setActiveTab(routerQuery.activeTab ? parseInt(routerQuery.activeTab) : storedActiveTab ?? TabName.WELCOME);
      setTreeViewOption(Boolean(routerQuery.treeViewOption ?? storedTreeViewOption ?? false));

      if (routerQuery.editorSelection) {
        const selection = routerQuery.editorSelection.split('|');
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
      }

      setStore({
        storedWorkspaceId: workspaceId,
      });

      if (routerQuery) {
        router.replace({
          pathname: '/project',
          query: {},
        });
      }
    };
    initializeApp().then(() => setLoadComplete(true));
    // This is required so the 'router' does not need to be passed as a dependecy, because it would cause an infinite loop.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [workspaceId]);

  useEffect(() => {
    setStore({
      storedProjectFileId: projectFileId,
      storedSearchProps: searchProps,
      storedMetricsGenId: metricsGenId,
      storedDiagramGenId: diagramGenId,
      storedDiagramTypeId: diagramTypeId,
      storedDiagramType: diagramType,
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
    diagramTypeId,
    diagramType,
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
    diagramTypeId: diagramTypeId as number,
    diagramType: diagramType as 'file' | 'ast',
    languageNodeId: languageNodeId as string,
    editorSelection: editorSelection as Range,
    gitRepoId: gitRepoId as string,
    gitBranch: gitBranch as string,
    gitCommitId: gitCommitId as string,
    gitBlameInfo,
    activeAccordion: activeAccordion as string,
    activeTab: activeTab as number,
    treeViewOption: treeViewOption as boolean,
    labels,
    setLabels,
    loadComplete,
    setWorkspaces,
    setWorkspaceId,
    setProjectFileId,
    setSearchProps,
    setMetricsGenId,
    setDiagramGenId,
    setDiagramTypeId,
    setDiagramType,
    setLanguageNodeId,
    setEditorSelection,
    setGitRepoId,
    setGitBranch,
    setGitCommitId,
    setGitBlameInfo,
    setActiveAccordion,
    setActiveTab,
    setTreeViewOption,
    setLoadComplete,
  };

  return <AppContext.Provider value={appContext}>{children}</AppContext.Provider>;
};
