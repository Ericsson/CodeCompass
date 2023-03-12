import { createContext, useContext, useEffect, useState } from 'react';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { FileInfo, FileSearchResult, SearchResult, SearchResultEntry, SearchType } from '@thrift-generated';
import { ProjectContext } from './project-context';
import { getFileFolderPath } from 'utils/utils';
import { getStore, setStore } from 'utils/store';

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
  searchFileFilterQuery: string;
  setSearchFileFilterQuery: (_val: string) => void;
  searchDirFilterQuery: string;
  setSearchDirFilterQuery: (_val: string) => void;
  searchStart: number;
  setSearchStart: (_val: number) => void;
  searchSize: number;
  setSearchSize: (_val: number) => void;
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
  searchFileFilterQuery: '',
  setSearchDirFilterQuery: (_val) => {},
  searchDirFilterQuery: '',
  setSearchFileFilterQuery: (_val) => {},
  searchStart: 0,
  setSearchStart: (_val) => {},
  searchSize: 5,
  setSearchSize: (_val) => {},
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
  const [isFileSearch, setIsFileSearch] = useState<boolean | undefined>(undefined);
  const [searchResult, setSearchResult] = useState<SearchResult | FileSearchResult | undefined>(undefined);
  const [searchQuery, setSearchQuery] = useState<string | undefined>(undefined);
  const [searchFileFilterQuery, setSearchFileFilterQuery] = useState<string | undefined>(undefined);
  const [searchDirFilterQuery, setSearchDirFilterQuery] = useState<string | undefined>(undefined);
  const [searchStart, setSearchStart] = useState<number | undefined>(undefined);
  const [searchSize, setSearchSize] = useState<number | undefined>(undefined);
  const [resultPaths, setResultPaths] = useState<string[]>([]);
  const [expandedPathNodes, setExpandedPathNodes] = useState<string[] | undefined>(undefined);
  const [expandedFileNodes, setExpandedFileNodes] = useState<FileNodesType | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createSearchClient(projectCtx.currentWorkspace);

      const searchTypes = await getSearchTypes();
      setSearchOptions(searchTypes);

      const {
        storedSearchOption,
        storedIsFileSearch,
        storedSearchResults,
        storedSearchQuery,
        storedSearchFileFilterQuery,
        storedSearchDirFilterQuery,
        storedSearchStart,
        storedSearchSize,
        storedExpandedSearchFileNodes,
        storedExpandedSearchPathNodes,
      } = getStore();

      setSearchCurrentOption(storedSearchOption ?? searchTypes[0]);
      setIsFileSearch(storedIsFileSearch ?? false);
      setSearchResult(storedSearchResults ?? undefined);
      setSearchQuery(storedSearchQuery ?? '');
      setSearchFileFilterQuery(storedSearchFileFilterQuery ?? '');
      setSearchDirFilterQuery(storedSearchDirFilterQuery ?? '');
      setSearchStart(storedSearchStart ?? 0);
      setSearchSize(storedSearchSize ?? 10);
      setExpandedPathNodes(storedExpandedSearchPathNodes ?? []);
      setExpandedFileNodes(storedExpandedSearchFileNodes ?? {});
    };
    init();
  }, [projectCtx.currentWorkspace]);

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

    const expandedFileNodesMap: FileNodesType = {};
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
      storedSearchOption: searchCurrentOption,
      storedIsFileSearch: isFileSearch,
      storedSearchResults: searchResult,
      storedSearchQuery: searchQuery,
      storedSearchFileFilterQuery: searchFileFilterQuery,
      storedSearchDirFilterQuery: searchDirFilterQuery,
      storedSearchStart: searchStart,
      storedSearchSize: searchSize,
      storedExpandedSearchPathNodes: expandedPathNodes,
      storedExpandedSearchFileNodes: expandedFileNodes,
    });
  }, [
    searchCurrentOption,
    isFileSearch,
    searchResult,
    searchQuery,
    searchFileFilterQuery,
    searchDirFilterQuery,
    searchStart,
    searchSize,
    expandedPathNodes,
    expandedFileNodes,
  ]);

  const searchContext = {
    searchOptions,
    searchCurrentOption: searchCurrentOption as SearchType,
    isFileSearch: isFileSearch as boolean,
    searchResult: searchResult as SearchResult | FileSearchResult,
    searchQuery: searchQuery as string,
    searchFileFilterQuery: searchFileFilterQuery as string,
    searchDirFilterQuery: searchDirFilterQuery as string,
    searchStart: searchStart as number,
    searchSize: searchSize as number,
    expandedPathNodes: expandedPathNodes as string[],
    expandedFileNodes: expandedFileNodes as FileNodesType,
    resultPaths,
    setSearchOptions,
    setSearchCurrentOption,
    setIsFileSearch,
    setSearchResult,
    setSearchQuery,
    setSearchFileFilterQuery,
    setSearchDirFilterQuery,
    setSearchStart,
    setSearchSize,
    setResultPaths,
    setExpandedPathNodes,
    setExpandedFileNodes,
  };

  return <SearchContext.Provider value={searchContext}>{children}</SearchContext.Provider>;
};
