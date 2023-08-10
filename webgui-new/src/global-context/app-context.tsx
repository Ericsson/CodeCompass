import React, { createContext, useEffect, useState } from 'react';
import { GitBlameHunk, WorkspaceInfo } from '@thrift-generated';
import { createWorkspaceClient, getWorkspaces } from 'service/workspace-service';
import { createProjectClient, getLabels } from 'service/project-service';
import { createSearchClient } from 'service/search-service';
import { createCppClient } from 'service/cpp-service';
import { createCppReparseClient } from 'service/cpp-reparse-service';
import { createMetricsClient } from 'service/metrics-service';
import { createGitClient } from 'service/git-service';
import { createConfig } from 'service/config';
import { SearchProps } from 'utils/types';
import { TabName } from 'enums/tab-enum';
import { AccordionLabel } from 'enums/accordion-enum';
import { useUrlState } from 'hooks/use-url-state';
import { getStore } from 'utils/store';
import { Box, CircularProgress } from '@mui/material';

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
  diagramTypeId: string;
  setDiagramTypeId: (_val: string) => void;
  diagramType: string;
  setDiagramType: (_val: string) => void;
  languageNodeId: string;
  setLanguageNodeId: (_val: string) => void;
  editorSelection: string;
  setEditorSelection: (_val: string) => void;
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
  activeTab: string;
  setActiveTab: (_val: string) => void;
  treeViewOption: string;
  setTreeViewOption: (_val: string) => void;
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
  diagramTypeId: '',
  setDiagramTypeId: (_val) => {},
  diagramType: 'file',
  setDiagramType: (_val) => {},
  languageNodeId: '',
  setLanguageNodeId: (_val) => {},
  editorSelection: '',
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
  activeTab: '0',
  setActiveTab: (_val) => {},
  treeViewOption: 'false',
  setTreeViewOption: (_val) => {},
});
/* eslint-enable no-unused-vars */
/* eslint-enable @typescript-eslint/no-empty-function */
/* eslint-enable @typescript-eslint/no-unused-vars */

export const AppContextController = ({ children }: { children: React.ReactNode }): JSX.Element => {
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);
  const [labels, setLabels] = useState<Map<string, string>>(new Map());

  const [searchProps, setSearchProps] = useState<SearchProps | undefined>(undefined);
  const [gitBlameInfo, setGitBlameInfo] = useState<GitBlameHunk[]>([]);

  const [workspaceId, setWorkspaceId] = useUrlState('workspaceId', 'CodeCompass');
  const [projectFileId, setProjectFileId] = useUrlState('projectFileId', '');
  const [metricsGenId, setMetricsGenId] = useUrlState('metricsGenId', '');
  const [diagramGenId, setDiagramGenId] = useUrlState('diagramGenId', '');
  const [diagramTypeId, setDiagramTypeId] = useUrlState('diagramTypeId', '');
  const [diagramType, setDiagramType] = useUrlState('diagramType', 'file');
  const [languageNodeId, setLanguageNodeId] = useUrlState('languageNodeId', '');
  const [editorSelection, setEditorSelection] = useUrlState('editorSelection', '');
  const [gitRepoId, setGitRepoId] = useUrlState('gitRepoId', '');
  const [gitBranch, setGitBranch] = useUrlState('gitBranch', '');
  const [gitCommitId, setGitCommitId] = useUrlState('gitCommitId', '');
  const [activeAccordion, setActiveAccordion] = useUrlState('activeAccordion', AccordionLabel.FILE_MANAGER);
  const [activeTab, setActiveTab] = useUrlState('activeTab', TabName.WELCOME.toString());
  const [treeViewOption, setTreeViewOption] = useUrlState('treeViewOption', 'false');

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

      const { storedSearchProps } = getStore();
      setSearchProps(storedSearchProps);

      setGitBlameInfo([]);
    };
    initializeApp().then(() => setLoadComplete(true));
  }, [workspaceId]);

  useEffect(() => {
    setGitBlameInfo([]);
  }, [projectFileId]);

  const appContext = {
    workspaces,
    workspaceId,
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
    gitBlameInfo,
    activeAccordion,
    activeTab,
    treeViewOption,
    labels,
    setLabels,
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
  };

  return loadComplete ? (
    <AppContext.Provider value={appContext}>{children}</AppContext.Provider>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
      <CircularProgress />
    </Box>
  );
};
