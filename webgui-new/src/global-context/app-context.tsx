import { createContext, useEffect, useState } from 'react';
import {
  FileInfo,
  FileSearchResult,
  Position,
  Range,
  SearchResult,
  SearchResultEntry,
  SearchType,
  WorkspaceInfo,
} from '@thrift-generated';
import { createWorkspaceClient, getWorkspaces } from 'service/workspace-service';
import { createProjectClient } from 'service/project-service';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { createCppClient } from 'service/cpp-service';
import { createCppReparseClient } from 'service/cpp-reparse-service';
import { createMetricsClient } from 'service/metrics-service';
import { createGitClient } from 'service/git-service';
import { createConfig } from 'service/config';
import { getStore, setStore } from 'utils/store';
import { FileNode, RouterQueryType } from 'utils/types';
import { SearchTypes } from 'enums/search-enum';
import { enumToArray, getFileFolderPath } from 'utils/utils';
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
  searchOptions: SearchType[];
  setSearchOptions: (_val: SearchType[]) => void;
  searchCurrentOption: SearchType | undefined;
  setSearchCurrentOption: (_val: SearchType | undefined) => void;
  isFileSearch: boolean;
  setIsFileSearch: (_val: boolean) => void;
  searchResult: SearchResult | FileSearchResult | undefined;
  setSearchResult: (_val: SearchResult | FileSearchResult | undefined) => void;
  searchResultCount: number;
  setSearchResultCount: (_val: number) => void;
  searchQuery: string;
  setSearchQuery: (_val: string) => void;
  searchFileFilterQuery: string;
  setSearchFileFilterQuery: (_val: string) => void;
  searchDirFilterQuery: string;
  setSearchDirFilterQuery: (_val: string) => void;
  searchLanguage: string;
  setSearchLanguage: (_val: string) => void;
  selectedSearchTypes: string[];
  setSelectedSearchTypes: (_val: string[]) => void;
  searchStart: number;
  setSearchStart: (_val: number) => void;
  searchSize: number;
  setSearchSize: (_val: number) => void;
  resultPaths: string[];
  setResultPaths: (_val: string[]) => void;
  selectedSearchResult: string;
  setSelectedSearchResult: (_val: string) => void;
  expandedPathNodes: string[];
  setExpandedPathNodes: (_val: string[]) => void;
  expandedFileNodes: FileNode;
  setExpandedFileNodes: (_val: FileNode) => void;
};

export const AppContext = createContext<AppContextProps>({
  workspaces: [],
  setWorkspaces: (_val) => {},
  workspaceId: '',
  setWorkspaceId: (_val) => {},
  projectFileId: '',
  setProjectFileId: (_val) => {},
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
  searchOptions: [],
  setSearchOptions: (_val) => {},
  searchCurrentOption: undefined,
  setSearchCurrentOption: (_val) => {},
  isFileSearch: false,
  setIsFileSearch: (_val) => {},
  searchResult: undefined,
  setSearchResult: (_val) => {},
  searchResultCount: 0,
  setSearchResultCount: (_val) => {},
  searchQuery: '',
  setSearchQuery: (_val) => {},
  searchFileFilterQuery: '',
  setSearchDirFilterQuery: (_val) => {},
  searchDirFilterQuery: '',
  setSearchFileFilterQuery: (_val) => {},
  searchLanguage: '',
  setSearchLanguage: (_val) => {},
  selectedSearchTypes: [],
  setSelectedSearchTypes: (_val) => {},
  searchStart: 0,
  setSearchStart: (_val) => {},
  searchSize: 5,
  setSearchSize: (_val) => {},
  resultPaths: [],
  setResultPaths: (_val) => {},
  selectedSearchResult: '',
  setSelectedSearchResult: (_val) => {},
  expandedPathNodes: [],
  setExpandedPathNodes: (_val) => {},
  expandedFileNodes: {},
  setExpandedFileNodes: (_val) => {},
});

export const AppContextController = ({ children }: { children: JSX.Element }): JSX.Element => {
  const router = useRouter();
  const routerQuery = router.query as RouterQueryType;

  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[] | undefined>(undefined);
  const [workspaceId, setWorkspaceId] = useState<string | undefined>(undefined);
  const [projectFileId, setProjectFileId] = useState<string | undefined>(undefined);
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

  const [searchOptions, setSearchOptions] = useState<SearchType[]>([]);
  const [searchCurrentOption, setSearchCurrentOption] = useState<SearchType | undefined>(undefined);
  const [isFileSearch, setIsFileSearch] = useState<boolean | undefined>(undefined);
  const [searchResult, setSearchResult] = useState<SearchResult | FileSearchResult | undefined>(undefined);
  const [searchResultCount, setSearchResultCount] = useState<number | undefined>(undefined);
  const [searchQuery, setSearchQuery] = useState<string | undefined>(undefined);
  const [searchFileFilterQuery, setSearchFileFilterQuery] = useState<string | undefined>(undefined);
  const [searchDirFilterQuery, setSearchDirFilterQuery] = useState<string | undefined>(undefined);
  const [searchLanguage, setSearchLanguage] = useState<string | undefined>(undefined);
  const [selectedSearchTypes, setSelectedSearchTypes] = useState<string[] | undefined>(undefined);
  const [searchStart, setSearchStart] = useState<number | undefined>(undefined);
  const [searchSize, setSearchSize] = useState<number | undefined>(undefined);
  const [resultPaths, setResultPaths] = useState<string[]>([]);
  const [selectedSearchResult, setSelectedSearchResult] = useState<string | undefined>(undefined);
  const [expandedPathNodes, setExpandedPathNodes] = useState<string[] | undefined>(undefined);
  const [expandedFileNodes, setExpandedFileNodes] = useState<FileNode | undefined>(undefined);

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
    const init = async () => {
      createProjectClient(workspaceId);
      createSearchClient(workspaceId);
      createCppClient(workspaceId);
      createCppReparseClient(workspaceId);
      createMetricsClient(workspaceId);
      createGitClient(workspaceId);

      const {
        storedProjectFileId,
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
        storedSearchOption,
        storedIsFileSearch,
        storedSearchResults,
        storedSearchResultCount,
        storedSearchQuery,
        storedSearchFileFilterQuery,
        storedSearchDirFilterQuery,
        storedSearchLanguage,
        storedSelectedSearchTypes,
        storedSearchStart,
        storedSearchSize,
        storedSelectedSearchResult,
        storedExpandedSearchFileNodes,
        storedExpandedSearchPathNodes,
      } = getStore();

      setProjectFileId(storedProjectFileId);
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

      const searchTypes = await getSearchTypes();
      setSearchOptions(searchTypes);

      const searchTypeOptions = enumToArray(SearchTypes);

      setSearchCurrentOption(storedSearchOption ?? searchTypes[0]);
      setIsFileSearch(storedIsFileSearch ?? false);
      setSearchResult(storedSearchResults ?? undefined);
      setSearchResultCount(storedSearchResultCount ?? 0);
      setSearchQuery(storedSearchQuery ?? '');
      setSearchFileFilterQuery(storedSearchFileFilterQuery ?? '');
      setSearchDirFilterQuery(storedSearchDirFilterQuery ?? '');
      setSearchLanguage(storedSearchLanguage ?? 'Any');
      setSelectedSearchTypes(storedSelectedSearchTypes ?? searchTypeOptions);
      setSearchStart(storedSearchStart ?? 0);
      setSearchSize(storedSearchSize ?? 10);
      setSelectedSearchResult(storedSelectedSearchResult ?? '');
      setExpandedPathNodes(storedExpandedSearchPathNodes ?? []);
      setExpandedFileNodes(storedExpandedSearchFileNodes ?? {});

      setStore({
        storedWorkspaceId: workspaceId,
      });

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
      }
    };
    init().then(() => setLoadComplete(true));
  }, [workspaceId, routerQuery]);

  useEffect(() => {
    if (!searchResult || !searchResult.results) return;

    const paths = !isFileSearch
      ? new Set(
          (searchResult?.results as SearchResultEntry[])?.map((entry) =>
            getFileFolderPath(entry.finfo?.path)
          ) as string[]
        )
      : new Set((searchResult?.results as FileInfo[])?.map((entry) => getFileFolderPath(entry.path)) as string[]);
    setResultPaths([...paths]);

    const { storedExpandedSearchPathNodes, storedExpandedSearchFileNodes } = getStore();

    const pathNodes = [...paths].map((_e, idx) => idx.toString());
    setExpandedPathNodes(storedExpandedSearchPathNodes ?? pathNodes);

    const expandedFileNodesMap: FileNode = {};
    let idx = 0;
    for (const path of paths) {
      const fileIds = !isFileSearch
        ? ((searchResult?.results as SearchResultEntry[])
            ?.filter((entry) => getFileFolderPath(entry.finfo?.path) === path)
            .map((entry) => entry.finfo?.id) as string[])
        : ((searchResult?.results as FileInfo[])
            ?.filter((entry) => getFileFolderPath(entry.path) === path)
            .map((entry) => entry.id) as string[]);
      expandedFileNodesMap[idx.toString()] = {
        expandedNodes: fileIds,
      };
      ++idx;
    }
    setExpandedFileNodes(storedExpandedSearchFileNodes ?? expandedFileNodesMap);
  }, [searchResult, isFileSearch]);

  useEffect(() => {
    setStore({
      storedProjectFileId: projectFileId,
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
      storedSearchOption: searchCurrentOption,
      storedIsFileSearch: isFileSearch,
      storedSearchResults: searchResult,
      storedSearchResultCount: searchResultCount,
      storedSearchQuery: searchQuery,
      storedSearchFileFilterQuery: searchFileFilterQuery,
      storedSearchDirFilterQuery: searchDirFilterQuery,
      storedSearchLanguage: searchLanguage,
      storedSelectedSearchTypes: selectedSearchTypes,
      storedSearchStart: searchStart,
      storedSearchSize: searchSize,
      storedSelectedSearchResult: selectedSearchResult,
      storedExpandedSearchPathNodes: expandedPathNodes,
      storedExpandedSearchFileNodes: expandedFileNodes,
    });
  }, [
    projectFileId,
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
    searchCurrentOption,
    isFileSearch,
    searchResult,
    searchResultCount,
    searchQuery,
    searchFileFilterQuery,
    searchDirFilterQuery,
    searchLanguage,
    selectedSearchTypes,
    searchStart,
    searchSize,
    selectedSearchResult,
    expandedPathNodes,
    expandedFileNodes,
  ]);

  const appContext = {
    workspaces: workspaces as WorkspaceInfo[],
    workspaceId: workspaceId as string,
    projectFileId: projectFileId as string,
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
    searchOptions,
    searchCurrentOption: searchCurrentOption as SearchType,
    isFileSearch: isFileSearch as boolean,
    searchResult: searchResult as SearchResult | FileSearchResult,
    searchResultCount: searchResultCount as number,
    searchQuery: searchQuery as string,
    searchFileFilterQuery: searchFileFilterQuery as string,
    searchDirFilterQuery: searchDirFilterQuery as string,
    searchLanguage: searchLanguage as string,
    selectedSearchTypes: selectedSearchTypes as string[],
    searchStart: searchStart as number,
    searchSize: searchSize as number,
    expandedPathNodes: expandedPathNodes as string[],
    expandedFileNodes: expandedFileNodes as FileNode,
    resultPaths,
    selectedSearchResult: selectedSearchResult as string,
    setSearchOptions,
    setSearchCurrentOption,
    setIsFileSearch,
    setSearchResult,
    setSearchResultCount,
    setSearchQuery,
    setSearchFileFilterQuery,
    setSearchDirFilterQuery,
    setSearchLanguage,
    setSelectedSearchTypes,
    setSearchStart,
    setSearchSize,
    setResultPaths,
    setSelectedSearchResult,
    setExpandedPathNodes,
    setExpandedFileNodes,
  };

  return <AppContext.Provider value={appContext}>{children}</AppContext.Provider>;
};
