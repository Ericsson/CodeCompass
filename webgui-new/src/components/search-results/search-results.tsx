import { ChevronLeft, ChevronRight, Code } from '@mui/icons-material';
import { alpha, FormControl, IconButton, InputLabel, MenuItem, Select } from '@mui/material';
import { FileInfo, FileSearchResult, LineMatch, SearchResult, SearchResultEntry } from '@thrift-generated';
import { FileIcon, FolderIcon } from 'components/custom-icon/custom-icon';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import React, { SyntheticEvent, useContext, useEffect, useState } from 'react';
import { getSearchResultCount, getSearchResults } from 'service/search-service';
import { getStore, removeStore, setStore } from 'utils/store';
import { FileNode } from 'utils/types';
import { getFileFolderPath } from 'utils/utils';
import * as SC from './styled-components';

export const SearchResults = () => {
  const appCtx = useContext(AppContext);

  const [searchType, setSearchType] = useState<number>(0);
  const [searchQuery, setSearchQuery] = useState<string>('');
  const [searchFileFilterQuery, setSearchFileFilterQuery] = useState<string>('');
  const [searchDirFilterQuery, setSearchDirFilterQuery] = useState<string>('');
  const [fileSearch, setFileSearch] = useState<boolean>(false);
  const [searchStart, setSearchStart] = useState<number>(0);
  const [searchSize, setSearchSize] = useState<number>(10);
  const [searchResult, setSearchResult] = useState<SearchResult | FileSearchResult | undefined>(undefined);
  const [searchResultCount, setSearchResultCount] = useState<number | undefined>(undefined);
  const [resultPaths, setResultPaths] = useState<string[]>([]);
  const [selectedSearchResult, setSelectedSearchResult] = useState<string | undefined>(undefined);
  const [expandedPathNodes, setExpandedPathNodes] = useState<string[] | undefined>(undefined);
  const [expandedFileNodes, setExpandedFileNodes] = useState<FileNode | undefined>(undefined);

  useEffect(() => {
    const { storedSelectedSearchResult } = getStore();
    setSelectedSearchResult(storedSelectedSearchResult ?? '');
  }, []);

  useEffect(() => {
    setSearchResult(undefined);
    setSearchResultCount(undefined);
  }, [appCtx.workspaceId]);

  useEffect(() => {
    const init = async () => {
      const searchProps = appCtx.searchProps;
      if (!searchProps) return;

      const initSearchResult = (await getSearchResults(
        searchProps.fileSearch,
        searchProps.type,
        searchProps.query,
        searchProps.start,
        searchProps.size,
        searchProps.fileFilter,
        searchProps.dirFilter
      )) as SearchResult | FileSearchResult;

      const initSearchResultCount = (await getSearchResultCount(
        searchProps.fileSearch,
        searchProps.type,
        searchProps.query,
        searchProps.fileFilter,
        searchProps.dirFilter
      )) as number;

      setSearchResult(initSearchResult);
      setSearchResultCount(initSearchResultCount);
      setSearchType(searchProps.type);
      setSearchQuery(searchProps.query);
      setSearchFileFilterQuery(searchProps.fileFilter);
      setSearchDirFilterQuery(searchProps.dirFilter);
      setSearchStart(searchProps.start);
      setSearchSize(searchProps.size);
      setFileSearch(searchProps.fileSearch);
    };
    init();
  }, [appCtx.searchProps]);

  useEffect(() => {
    if (!searchResult || !searchResult.results) return;

    const paths = !fileSearch
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
      const fileIds = !fileSearch
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
  }, [searchResult, fileSearch]);

  const handleDirNodeSelect = () => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      if (!expandedPathNodes) return;
      const index = expandedPathNodes.indexOf(nodeId);
      const copyExpanded = [...expandedPathNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      setExpandedPathNodes(copyExpanded);
      setStore({
        storedExpandedSearchPathNodes: copyExpanded,
      });
    };
  };

  const handleFileNodeSelect = (pathIdx: string) => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      if (!expandedFileNodes) return;
      const expandedNodes = expandedFileNodes[pathIdx].expandedNodes;
      const index = expandedNodes.indexOf(nodeId);
      const copyExpanded = [...expandedNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      const expandedFileNodesCopy = { ...expandedFileNodes } as FileNode;
      expandedFileNodesCopy[pathIdx].expandedNodes = copyExpanded;
      setExpandedFileNodes(expandedFileNodesCopy);
      setStore({
        storedExpandedSearchFileNodes: expandedFileNodesCopy,
      });
    };
  };

  const handleFileLineClick = async (file: FileInfo, lineMatch?: LineMatch, idx?: string) => {
    appCtx.setActiveTab(TabName.CODE);
    appCtx.setProjectFileId(file.id as string);
    appCtx.setEditorSelection(lineMatch?.range?.range);

    setSelectedSearchResult(idx ?? (file.id as string));
    setStore({
      storedSelectedSearchResult: idx ?? (file.id as string),
    });
  };

  const updateSelectResults = async (newSearchSize: number) => {
    const newSearchResult = (await getSearchResults(
      fileSearch,
      searchType,
      searchQuery,
      0,
      newSearchSize,
      searchFileFilterQuery,
      searchDirFilterQuery
    )) as SearchResult | FileSearchResult;

    setSearchStart(0);
    setSearchSize(newSearchSize);
    setSearchResult(newSearchResult);

    removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
  };

  const updatePageResults = async (direction: 'left' | 'right') => {
    const start = direction === 'left' ? searchStart - searchSize : searchStart + searchSize;

    const newSearchResult = (await getSearchResults(
      fileSearch,
      searchType,
      searchQuery,
      start,
      searchSize,
      searchFileFilterQuery,
      searchDirFilterQuery
    )) as SearchResult | FileSearchResult;

    setSearchStart(start);
    setSearchResult(newSearchResult);

    removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
  };

  return (
    <>
      <SC.PaginationContainer>
        <FormControl>
          <InputLabel>{'Size'}</InputLabel>
          <Select
            value={searchSize}
            label={'Size'}
            onChange={(e) => {
              setSearchStart(0);
              setSearchSize(e.target.value as number);
              updateSelectResults(e.target.value as number);
            }}
            sx={{ height: '40px' }}
          >
            {[10, 20, 30, 40, 50].map((num) => (
              <MenuItem key={num} value={num}>
                {num}
              </MenuItem>
            ))}
          </Select>
        </FormControl>
        <div>{`${Math.ceil(searchStart / 10 / (searchSize / 10)) + 1} of ${Math.ceil(
          (searchResultCount ?? 0) / searchSize
        )}`}</div>
        <SC.StyledDiv>
          <IconButton onClick={() => updatePageResults('left')} disabled={searchStart === 0}>
            <ChevronLeft />
          </IconButton>
          <IconButton
            onClick={() => updatePageResults('right')}
            disabled={
              Math.ceil(searchStart / 10 / (searchSize / 10)) + 1 ===
                Math.ceil((searchResultCount ?? 0) / searchSize) || searchResultCount === 0
            }
          >
            <ChevronRight />
          </IconButton>
        </SC.StyledDiv>
      </SC.PaginationContainer>
      <SC.ResultsContainer>
        {appCtx.searchProps && searchResult?.results?.length ? (
          <SC.StyledTreeView
            defaultCollapseIcon={<FolderIcon open />}
            defaultExpandIcon={<FolderIcon />}
            expanded={expandedPathNodes ?? []}
            onNodeSelect={handleDirNodeSelect()}
            sx={{ width: 'fit-content' }}
          >
            <SC.StyledDiv>
              <SC.StyledDiv>
                {resultPaths.map((path, pathNodeIdx) => {
                  if (!searchResult || !searchResult.results) return;
                  return (
                    <div key={pathNodeIdx}>
                      <SC.StyledTreeItem
                        nodeId={`${pathNodeIdx}`}
                        label={<SC.StyledDiv sx={{ fontSize: '0.85rem' }}>{path}</SC.StyledDiv>}
                        sx={{ marginTop: '3px' }}
                      >
                        {!fileSearch
                          ? (searchResult?.results as SearchResultEntry[])
                              ?.filter((result) => getFileFolderPath(result.finfo?.path) === path)
                              .map((entry, fileNodeIdx) => {
                                return (
                                  <div key={fileNodeIdx}>
                                    <SC.StyledTreeView
                                      defaultCollapseIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                                      defaultExpandIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                                      expanded={
                                        expandedFileNodes && expandedFileNodes[pathNodeIdx.toString()]
                                          ? expandedFileNodes[pathNodeIdx.toString()].expandedNodes
                                          : []
                                      }
                                      onNodeSelect={handleFileNodeSelect(pathNodeIdx.toString())}
                                    >
                                      <SC.StyledTreeItem
                                        nodeId={`${entry.finfo?.id as string}`}
                                        label={
                                          <SC.StyledDiv
                                            sx={{
                                              fontSize: '0.85rem',
                                              marginTop: '3px',
                                              fontWeight: 'bold',
                                              color: (theme) =>
                                                entry.finfo?.parseStatus === 3
                                                  ? theme.colors?.success
                                                  : entry.finfo?.parseStatus === 2
                                                  ? theme.colors?.warning
                                                  : theme.colors?.primary,
                                            }}
                                          >
                                            {entry.finfo?.name}
                                          </SC.StyledDiv>
                                        }
                                      >
                                        {entry.matchingLines?.map((line, idx) => {
                                          return (
                                            <SC.IconLabel
                                              key={idx}
                                              sx={{
                                                backgroundColor: (theme) =>
                                                  `${pathNodeIdx}-${idx}-${entry.finfo?.id}` === selectedSearchResult
                                                    ? alpha(theme.backgroundColors?.secondary as string, 0.3)
                                                    : '',
                                              }}
                                              onClick={() =>
                                                handleFileLineClick(
                                                  entry.finfo as FileInfo,
                                                  line,
                                                  `${pathNodeIdx}-${idx}-${entry.finfo?.id}`
                                                )
                                              }
                                            >
                                              <Code />
                                              <SC.FileLine
                                                sx={{
                                                  fontSize: '0.85rem',
                                                  marginTop: '3px',
                                                }}
                                              >
                                                {line.text}
                                              </SC.FileLine>
                                            </SC.IconLabel>
                                          );
                                        })}
                                      </SC.StyledTreeItem>
                                    </SC.StyledTreeView>
                                  </div>
                                );
                              })
                          : (searchResult?.results as FileInfo[])
                              ?.filter((result) => getFileFolderPath(result?.path) === path)
                              .map((entry, fileNodeIdx) => {
                                return (
                                  <SC.IconLabel
                                    key={fileNodeIdx}
                                    sx={{
                                      backgroundColor: (theme) =>
                                        entry?.id === selectedSearchResult
                                          ? alpha(theme.backgroundColors?.secondary as string, 0.3)
                                          : '',
                                    }}
                                    onClick={() => handleFileLineClick(entry)}
                                  >
                                    <FileIcon fileName={entry.name as string} />
                                    <SC.FileLine
                                      sx={{
                                        color: (theme) =>
                                          entry.parseStatus === 3
                                            ? theme.colors?.success
                                            : entry.parseStatus === 2
                                            ? theme.colors?.warning
                                            : theme.colors?.primary,
                                      }}
                                    >
                                      {entry.name}
                                    </SC.FileLine>
                                  </SC.IconLabel>
                                );
                              })}
                      </SC.StyledTreeItem>
                    </div>
                  );
                })}
              </SC.StyledDiv>
            </SC.StyledDiv>
          </SC.StyledTreeView>
        ) : (
          <SC.Placeholder>{'No results'}</SC.Placeholder>
        )}
      </SC.ResultsContainer>
    </>
  );
};
