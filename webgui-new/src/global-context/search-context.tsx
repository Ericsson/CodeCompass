import { createContext, useContext, useEffect, useState } from 'react';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { FileInfo, FileSearchResult, SearchResult, SearchResultEntry, SearchType } from '@thrift-generated';
import { ProjectContext } from './project-context';

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

type SearchContextType = {
  searchOptions: SearchType[];
  setSearchOptions: (_val: SearchType[]) => void;
  searchCurrentOption: SearchType | undefined;
  setSearchCurrentOption: (_val: SearchType | undefined) => void;
  isFileSearch: boolean;
  setIsFileSearch: (_val: boolean) => void;
  searchResult: SearchResult | FileSearchResult | undefined;
  setSearchResult: (_val: SearchResult | FileSearchResult | undefined) => void;
  searchQuery: string;
  setSearchQuery: (_val: string) => void;
  searchStart: number;
  setSearchStart: (_val: number) => void;
  searchSize: number;
  setSearchSize: (_val: number) => void;
  searchPage: number;
  setSearchPage: (_val: number) => void;
  resultPaths: string[];
  setResultPaths: (_val: string[]) => void;
  expandedPathNodes: string[];
  setExpandedPathNodes: (_val: string[]) => void;
  expandedFileNodes: FileNodesType;
  setExpandedFileNodes: (_val: FileNodesType) => void;
};

export const SearchContext = createContext<SearchContextType>({
  searchOptions: [],
  setSearchOptions: (_val) => {},
  searchCurrentOption: undefined,
  setSearchCurrentOption: (_val) => {},
  isFileSearch: false,
  setIsFileSearch: (_val) => {},
  searchResult: undefined,
  setSearchResult: (_val) => {},
  searchQuery: '',
  setSearchQuery: (_val) => {},
  searchStart: 0,
  setSearchStart: (_val) => {},
  searchSize: 5,
  setSearchSize: (_val) => {},
  searchPage: 0,
  setSearchPage: (_val) => {},
  resultPaths: [],
  setResultPaths: (_val) => {},
  expandedPathNodes: [],
  setExpandedPathNodes: (_val) => {},
  expandedFileNodes: {},
  setExpandedFileNodes: (_val) => {},
});

export const SearchContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [searchOptions, setSearchOptions] = useState<SearchType[]>([]);
  const [searchCurrentOption, setSearchCurrentOption] = useState<SearchType | undefined>(undefined);
  const [isFileSearch, setIsFileSearch] = useState<boolean>(false);
  const [searchResult, setSearchResult] = useState<SearchResult | FileSearchResult | undefined>(undefined);
  const [searchQuery, setSearchQuery] = useState<string>('');
  const [searchStart, setSearchStart] = useState<number>(0);
  const [searchSize, setSearchSize] = useState<number>(10);
  const [searchPage, setSearchPage] = useState<number>(0);
  const [resultPaths, setResultPaths] = useState<string[]>([]);
  const [expandedPathNodes, setExpandedPathNodes] = useState<string[]>([]);
  const [expandedFileNodes, setExpandedFileNodes] = useState<FileNodesType>({});

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createSearchClient(projectCtx.currentWorkspace);

      const searchTypes = await getSearchTypes();
      setSearchOptions(searchTypes);

      const storedCurrentOption = localStorage.getItem('currentSearchOption');
      setSearchCurrentOption(storedCurrentOption ? JSON.parse(storedCurrentOption) : searchTypes[0]);

      const storedIsFileSearch = localStorage.getItem('isFileSearch');
      setIsFileSearch(storedIsFileSearch ? JSON.parse(storedIsFileSearch) : false);

      const storedSearchResults = localStorage.getItem('searchResults');
      setSearchResult(storedSearchResults ? JSON.parse(storedSearchResults) : undefined);

      const storedSearchQuery = localStorage.getItem('currentSearchQuery');
      setSearchQuery(storedSearchQuery ? storedSearchQuery : '');

      const storedSearchSize = localStorage.getItem('currentSearchSize');
      setSearchSize(storedSearchSize ? JSON.parse(storedSearchSize) : 10);

      const storedSearchPage = localStorage.getItem('currentSearchPage');
      setSearchPage(storedSearchPage ? JSON.parse(storedSearchPage) : 0);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!searchResult || !searchResult.results) return;

    const paths = !isFileSearch
      ? new Set((searchResult?.results as SearchResultEntry[])?.map((entry) => entry.finfo?.path) as string[])
      : new Set((searchResult?.results as FileInfo[])?.map((entry) => entry.path) as string[]);
    setResultPaths([...paths]);

    const storedExpandedPathNodes = localStorage.getItem('expandedPathNodes');
    const pathNodes = [...paths].map((_e, idx) => idx.toString());
    setExpandedPathNodes(storedExpandedPathNodes ? JSON.parse(storedExpandedPathNodes) : pathNodes);

    const storedExpandedFileNodes = localStorage.getItem('expandedFileNodes');
    const expandedFileNodesMap: FileNodesType = {};
    let idx = 0;
    for (const path of paths) {
      const fileIds = !isFileSearch
        ? ((searchResult?.results as SearchResultEntry[])
            ?.filter((entry) => entry.finfo?.path === path)
            .map((entry) => entry.finfo?.id) as string[])
        : ((searchResult?.results as FileInfo[])
            ?.filter((entry) => entry.path === path)
            .map((entry) => entry.id) as string[]);
      expandedFileNodesMap[idx.toString()] = {
        expandedNodes: fileIds,
      };
      ++idx;
    }
    setExpandedFileNodes(storedExpandedFileNodes ? JSON.parse(storedExpandedFileNodes) : expandedFileNodesMap);
  }, [searchResult, isFileSearch]);

  const searchContext = {
    searchOptions,
    setSearchOptions,
    searchCurrentOption,
    setSearchCurrentOption,
    isFileSearch,
    setIsFileSearch,
    searchResult,
    setSearchResult,
    searchQuery,
    setSearchQuery,
    searchStart,
    setSearchStart,
    searchSize,
    setSearchSize,
    searchPage,
    setSearchPage,
    resultPaths,
    setResultPaths,
    expandedPathNodes,
    setExpandedPathNodes,
    expandedFileNodes,
    setExpandedFileNodes,
  };

  return <SearchContext.Provider value={searchContext}>{children}</SearchContext.Provider>;
};
