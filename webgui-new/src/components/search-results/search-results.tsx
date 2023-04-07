import { FolderOpen, Folder, ChevronLeft, ChevronRight, Code } from '@mui/icons-material';
import { TreeItem, treeItemClasses, TreeView } from '@mui/lab';
import { alpha, FormControl, IconButton, InputLabel, MenuItem, Select, styled } from '@mui/material';
import { FileInfo, FileSearchResult, LineMatch, SearchResult, SearchResultEntry } from '@thrift-generated';
import { FileIcon } from 'components/file-icon/file-icon';
import { TabName } from 'enums/tab-enum';
import { ConfigContext } from 'global-context/config-context';
import { LanguageContext } from 'global-context/language-context';
import { ProjectContext } from 'global-context/project-context';
import { SearchContext } from 'global-context/search-context';
import { SyntheticEvent, useContext } from 'react';
import { getFileContent } from 'service/project-service';
import { getSearchResults } from 'service/search-service';
import { removeStore } from 'utils/store';
import { getFileFolderPath } from 'utils/utils';

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

const StyledDiv = styled('div')({});

const IconLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '5px',
  paddingLeft: '15px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

const FileLine = styled('div')({
  fontSize: '0.85rem',
  marginTop: '3px',
});

const PaginationContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  gap: '15px',
  margin: '10px',
  height: '50px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

const ResultsContainer = styled('div')({
  height: 'calc(100vh - 81px - 70px - 4 * 48px)',
  overflow: 'scroll',
});

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  fontSize: '0.85rem',
}));

const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  [`& .${treeItemClasses.group}`]: {
    marginLeft: '10px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

export const SearchResults = (): JSX.Element => {
  const configCtx = useContext(ConfigContext);
  const projectCtx = useContext(ProjectContext);
  const searchCtx = useContext(SearchContext);
  const languageCtx = useContext(LanguageContext);

  const handleDirNodeSelect = () => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      const index = searchCtx.expandedPathNodes.indexOf(nodeId);
      const copyExpanded = [...searchCtx.expandedPathNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      searchCtx.setExpandedPathNodes(copyExpanded);
    };
  };

  const handleFileNodeSelect = (pathIdx: string) => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      const expandedNodes = searchCtx.expandedFileNodes[pathIdx].expandedNodes;
      const index = expandedNodes.indexOf(nodeId);
      const copyExpanded = [...expandedNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      const expandedFileNodesCopy = { ...searchCtx.expandedFileNodes } as FileNodesType;
      expandedFileNodesCopy[pathIdx].expandedNodes = copyExpanded;
      searchCtx.setExpandedFileNodes(expandedFileNodesCopy);
    };
  };

  const handleFileLineClick = async (file: FileInfo, path: string, lineMatch?: LineMatch, idx?: string) => {
    const fileContent = await getFileContent(file.id as string);
    projectCtx.setFileInfo(file);
    projectCtx.setFileContent(fileContent);
    configCtx.setActiveTab(TabName.CODE);
    searchCtx.setSelectedSearchResult(idx ?? (file.id as string));
    languageCtx.setNodeSelectionRange(lineMatch?.range?.range);
  };

  const updateSelectResults = async (newSearchSize: number) => {
    const searchResults = (await getSearchResults(
      searchCtx.isFileSearch,
      searchCtx.searchCurrentOption?.id as number,
      searchCtx.searchQuery ?? '',
      0,
      newSearchSize,
      searchCtx.searchFileFilterQuery,
      searchCtx.searchDirFilterQuery
    )) as SearchResult | FileSearchResult;

    searchCtx.setSearchStart(0);
    searchCtx.setSearchSize(newSearchSize);
    searchCtx.setSearchResult(searchResults);

    removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
  };

  const updatePageResults = async (direction: 'left' | 'right') => {
    const start =
      direction === 'left'
        ? searchCtx.searchStart - searchCtx.searchSize
        : searchCtx.searchStart + searchCtx.searchSize;

    const searchResults = (await getSearchResults(
      searchCtx.isFileSearch,
      searchCtx.searchCurrentOption?.id as number,
      searchCtx.searchQuery ?? '',
      start,
      searchCtx.searchSize,
      searchCtx.searchFileFilterQuery,
      searchCtx.searchDirFilterQuery
    )) as SearchResult | FileSearchResult;

    searchCtx.setSearchStart(start);
    searchCtx.setSearchResult(searchResults);

    removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
  };

  return (
    <div>
      <PaginationContainer>
        <FormControl>
          <InputLabel>{'Size'}</InputLabel>
          <Select
            value={searchCtx.searchSize}
            label={'Size'}
            onChange={(e) => {
              searchCtx.setSearchStart(0);
              searchCtx.setSearchSize(e.target.value as number);
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
        <div>{`${Math.ceil(searchCtx.searchStart / 10 / (searchCtx.searchSize / 10)) + 1} of ${Math.ceil(
          searchCtx.searchResultCount / searchCtx.searchSize
        )}`}</div>
        <StyledDiv>
          <IconButton onClick={() => updatePageResults('left')} disabled={searchCtx.searchStart === 0}>
            <ChevronLeft />
          </IconButton>
          <IconButton
            onClick={() => updatePageResults('right')}
            disabled={
              Math.ceil(searchCtx.searchStart / 10 / (searchCtx.searchSize / 10)) + 1 ===
                Math.ceil(searchCtx.searchResultCount / searchCtx.searchSize) || searchCtx.searchResultCount === 0
            }
          >
            <ChevronRight />
          </IconButton>
        </StyledDiv>
      </PaginationContainer>
      <ResultsContainer>
        {searchCtx.searchResult?.results?.length ? (
          <StyledTreeView
            defaultCollapseIcon={<FolderOpen />}
            defaultExpandIcon={<Folder />}
            expanded={searchCtx.expandedPathNodes}
            onNodeSelect={handleDirNodeSelect()}
            sx={{ width: 'fit-content' }}
          >
            <StyledDiv>
              <StyledDiv>
                {searchCtx.resultPaths.map((path, pathNodeIdx) => {
                  if (!searchCtx.searchResult || !searchCtx.searchResult.results) return;
                  return (
                    <div key={pathNodeIdx}>
                      <StyledTreeItem
                        nodeId={`${pathNodeIdx}`}
                        label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{path}</StyledDiv>}
                        sx={{ marginTop: '3px' }}
                      >
                        {!searchCtx.isFileSearch
                          ? (searchCtx.searchResult?.results as SearchResultEntry[])
                              ?.filter((result) => getFileFolderPath(result.finfo?.path) === path)
                              .map((entry, fileNodeIdx) => {
                                return (
                                  <div key={fileNodeIdx}>
                                    <StyledTreeView
                                      defaultCollapseIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                                      defaultExpandIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                                      expanded={
                                        searchCtx.expandedFileNodes[pathNodeIdx.toString()]
                                          ? searchCtx.expandedFileNodes[pathNodeIdx.toString()].expandedNodes
                                          : []
                                      }
                                      onNodeSelect={handleFileNodeSelect(pathNodeIdx.toString())}
                                    >
                                      <StyledTreeItem
                                        nodeId={`${entry.finfo?.id as string}`}
                                        label={
                                          <StyledDiv
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
                                          </StyledDiv>
                                        }
                                      >
                                        {entry.matchingLines?.map((line, idx) => {
                                          return (
                                            <IconLabel
                                              key={idx}
                                              sx={{
                                                backgroundColor: (theme) =>
                                                  `${pathNodeIdx}-${idx}-${entry.finfo?.id}` ===
                                                  searchCtx.selectedSearchResult
                                                    ? alpha(theme.backgroundColors?.secondary as string, 0.3)
                                                    : '',
                                              }}
                                              onClick={() =>
                                                handleFileLineClick(
                                                  entry.finfo as FileInfo,
                                                  path,
                                                  line,
                                                  `${pathNodeIdx}-${idx}-${entry.finfo?.id}`
                                                )
                                              }
                                            >
                                              <Code />
                                              <FileLine
                                                sx={{
                                                  fontSize: '0.85rem',
                                                  marginTop: '3px',
                                                }}
                                              >
                                                {line.text}
                                              </FileLine>
                                            </IconLabel>
                                          );
                                        })}
                                      </StyledTreeItem>
                                    </StyledTreeView>
                                  </div>
                                );
                              })
                          : (searchCtx.searchResult?.results as FileInfo[])
                              ?.filter((result) => getFileFolderPath(result?.path) === path)
                              .map((entry, fileNodeIdx) => {
                                return (
                                  <IconLabel
                                    key={fileNodeIdx}
                                    sx={{
                                      backgroundColor: (theme) =>
                                        entry?.id === searchCtx.selectedSearchResult
                                          ? alpha(theme.backgroundColors?.secondary as string, 0.3)
                                          : '',
                                    }}
                                    onClick={() => handleFileLineClick(entry, path)}
                                  >
                                    <FileIcon fileName={entry.name as string} />
                                    <FileLine
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
                                    </FileLine>
                                  </IconLabel>
                                );
                              })}
                      </StyledTreeItem>
                    </div>
                  );
                })}
              </StyledDiv>
            </StyledDiv>
          </StyledTreeView>
        ) : (
          <StyledDiv sx={{ paddingLeft: '10px' }}>{'No results'}</StyledDiv>
        )}
      </ResultsContainer>
    </div>
  );
};
