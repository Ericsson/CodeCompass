import { createContext, useContext, useEffect, useState } from 'react';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { SearchResult } from '@thrift-generated';
import { ProjectContext } from './project-context';

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

type SearchContextType = {
  searchOption: number;
  setSearchOption: (_val: number) => void;
  searchResult: SearchResult | undefined;
  setSearchResult: (_val: SearchResult | undefined) => void;
  resultPaths: string[];
  setResultPaths: (_val: string[]) => void;
  expandedPathNodes: string[];
  setExpandedPathNodes: (_val: string[]) => void;
  expandedFileNodes: FileNodesType;
  setExpandedFileNodes: (_val: FileNodesType) => void;
};

export const SearchContext = createContext<SearchContextType>({
  searchOption: 1,
  setSearchOption: (_val) => {},
  searchResult: undefined,
  setSearchResult: (_val) => {},
  resultPaths: [],
  setResultPaths: (_val) => {},
  expandedPathNodes: [],
  setExpandedPathNodes: (_val) => {},
  expandedFileNodes: {},
  setExpandedFileNodes: (_val) => {},
});

export const SearchContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const [searchOption, setSearchOption] = useState<number>(1);
  const [searchResult, setSearchResult] = useState<SearchResult | undefined>(undefined);
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
      setSearchOption(searchTypes[0].id as number);

      const storedSearchResults = localStorage.getItem('searchResults');
      setSearchResult(storedSearchResults ? JSON.parse(storedSearchResults) : undefined);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!searchResult) {
      return;
    }

    const paths = new Set(searchResult?.results?.map((entry) => entry.finfo?.path) as string[]);
    setResultPaths([...paths]);

    const storedExpandedPathNodes = localStorage.getItem('expandedPathNodes');
    const pathNodes = [...paths].map((_e, idx) => idx.toString());
    setExpandedPathNodes(storedExpandedPathNodes ? JSON.parse(storedExpandedPathNodes) : pathNodes);

    const storedExpandedFileNodes = localStorage.getItem('expandedFileNodes');
    const expandedFileNodesMap: FileNodesType = {};
    let idx = 0;
    for (const path of paths) {
      const fileIds = searchResult?.results
        ?.filter((entry) => entry.finfo?.path === path)
        .map((entry) => entry.finfo?.id) as string[];
      expandedFileNodesMap[idx.toString()] = {
        expandedNodes: fileIds,
      };
      ++idx;
    }
    setExpandedFileNodes(storedExpandedFileNodes ? JSON.parse(storedExpandedFileNodes) : expandedFileNodesMap);
  }, [searchResult]);

  const searchContext = {
    searchOption,
    setSearchOption,
    searchResult,
    setSearchResult,
    resultPaths,
    setResultPaths,
    expandedPathNodes,
    setExpandedPathNodes,
    expandedFileNodes,
    setExpandedFileNodes,
  };

  return <SearchContext.Provider value={searchContext}>{children}</SearchContext.Provider>;
};
