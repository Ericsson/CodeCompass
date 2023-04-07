import { createContext, useContext, useEffect, useState } from 'react';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { FileInfo, FileSearchResult, LineMatch, SearchResult, SearchResultEntry, SearchType } from '@thrift-generated';
import { ProjectContext } from './project-context';
import { enumToArray, getFileFolderPath } from 'utils/utils';
import { getStore, setStore } from 'utils/store';
import { SearchTypes } from 'enums/search-enum';

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

export const SearchContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

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
  const [expandedFileNodes, setExpandedFileNodes] = useState<FileNodesType | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createSearchClient(projectCtx.currentWorkspace);

      const searchTypes = await getSearchTypes();
      setSearchOptions(searchTypes);

      const searchTypeOptions = enumToArray(SearchTypes);

      const {
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

  const searchContext = {
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
    expandedFileNodes: expandedFileNodes as FileNodesType,
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

  return <SearchContext.Provider value={searchContext}>{children}</SearchContext.Provider>;
};
