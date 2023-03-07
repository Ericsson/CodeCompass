import { FolderOpen, Folder, ChevronLeft, ChevronRight } from '@mui/icons-material';
import { TreeItem, treeItemClasses, TreeView } from '@mui/lab';
import { alpha, FormControl, IconButton, InputLabel, MenuItem, Select, styled } from '@mui/material';
import { FileInfo, SearchResult } from '@thrift-generated';
import { FileIcon } from 'components/file-icon/file-icon';
import { TabName } from 'enums/tab-enum';
import { OtherContext } from 'global-context/other-context';
import { ProjectContext } from 'global-context/project-context';
import { SearchContext } from 'global-context/search-context';
import { SyntheticEvent, useContext } from 'react';
import { getParents, getFileContent } from 'service/project-service';
import { getSearchResults } from 'service/search-service';

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

const StyledDiv = styled('div')({});

const FileLine = styled('div')(({ theme }) => ({
  fontSize: '0.85rem',
  marginTop: '3px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

const PaginationContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '15px',
  padding: '0 10px',
  margin: '5px 0px 15px 0px',
  width: '280px',
});

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  padding: '5px',
  fontSize: '0.85rem',
}));

const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  [`& .${treeItemClasses.group}`]: {
    marginLeft: '10px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

export const SearchResults = (): JSX.Element => {
  const otherCtx = useContext(OtherContext);
  const projectCtx = useContext(ProjectContext);
  const searchCtx = useContext(SearchContext);

  const handleDirNodeSelect = () => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      const index = searchCtx.expandedPathNodes.indexOf(nodeId);
      const copyExpanded = [...searchCtx.expandedPathNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      localStorage.setItem('expandedPathNodes', JSON.stringify(copyExpanded));
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
      localStorage.setItem('expandedFileNodes', JSON.stringify({ ...expandedFileNodesCopy }));
      searchCtx.setExpandedFileNodes(expandedFileNodesCopy);
    };
  };

  const handleFileLineClick = async (file: FileInfo) => {
    const parents = await getParents(projectCtx.folderPath);
    const fileContent = await getFileContent(file.id as string);
    projectCtx.setFileContent(fileContent);
    projectCtx.setFileInfo(file);
    projectCtx.setSelectedFile(file.id as string);
    projectCtx.setExpandedFileTreeNodes(parents);
    otherCtx.setActiveTab(TabName.CODE);
    localStorage.setItem('activeTab', JSON.stringify(TabName.CODE));
    localStorage.setItem('currentFileContent', fileContent);
    localStorage.setItem('currentFileInfo', JSON.stringify(file));
    localStorage.setItem('currentSelectedFile', file.id as string);
    localStorage.setItem('expandedNodes', JSON.stringify(parents));
  };

  const handlePageChange = (direction?: 'left' | 'right') => {
    const newPage = direction === 'left' ? searchCtx.searchPage - 1 : searchCtx.searchPage + 1;
    searchCtx.setSearchPage(newPage);
    updateResults(newPage);
  };

  const updateResults = async (newPage?: number, newSearchSize?: number) => {
    if (!searchCtx.searchQuery) {
      return;
    }
    searchCtx.setSearchStart(newPage ?? searchCtx.searchPage);
    const searchResults = (await getSearchResults(
      searchCtx.searchOption,
      searchCtx.searchQuery,
      newPage ?? searchCtx.searchPage,
      newSearchSize ?? searchCtx.searchSize
    )) as SearchResult;
    searchCtx.setSearchResult(searchResults);
    localStorage.setItem('searchResults', JSON.stringify(searchResults as SearchResult));
    localStorage.setItem('currentSearchPage', JSON.stringify(newPage ?? searchCtx.searchPage));
    localStorage.setItem('currentSearchSize', JSON.stringify(newSearchSize ?? searchCtx.searchSize));
    localStorage.removeItem('expandedPathNodes');
    localStorage.removeItem('expandedFileNodes');
  };

  return (
    <StyledTreeView
      defaultCollapseIcon={<FolderOpen />}
      defaultExpandIcon={<Folder />}
      expanded={searchCtx.expandedPathNodes}
      onNodeSelect={handleDirNodeSelect()}
      sx={{ padding: '10px 5px', width: 'fit-content' }}
    >
      {searchCtx.searchResult ? (
        <StyledDiv>
          <PaginationContainer>
            <FormControl>
              <InputLabel>{'Size'}</InputLabel>
              <Select
                value={searchCtx.searchSize}
                label={'Size'}
                onChange={(e) => {
                  searchCtx.setSearchPage(0);
                  searchCtx.setSearchSize(e.target.value as number);
                  updateResults(0, e.target.value as number);
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
            <div>{`${searchCtx.searchPage + 1} of ${Math.ceil(100 / searchCtx.searchSize)}`}</div>
            <StyledDiv>
              <IconButton onClick={() => handlePageChange('left')} disabled={searchCtx.searchPage === 0}>
                <ChevronLeft />
              </IconButton>
              <IconButton
                onClick={() => handlePageChange('right')}
                disabled={searchCtx.searchPage === Math.ceil(100 / searchCtx.searchSize) - 1}
              >
                <ChevronRight />
              </IconButton>
            </StyledDiv>
          </PaginationContainer>
          <StyledDiv>
            {searchCtx.resultPaths.map((path, pathNodeIdx) => {
              return (
                <div key={pathNodeIdx}>
                  <StyledTreeItem
                    nodeId={`${pathNodeIdx}`}
                    label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{path}</StyledDiv>}
                    sx={{ marginTop: '3px' }}
                  >
                    {searchCtx.searchResult?.results
                      ?.filter((result) => result.finfo?.path === path)
                      .map((entry, fileNodeIdx) => {
                        return (
                          <div key={fileNodeIdx}>
                            <StyledTreeView
                              defaultCollapseIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                              defaultExpandIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                              expanded={searchCtx.expandedFileNodes[pathNodeIdx.toString()].expandedNodes}
                              onNodeSelect={handleFileNodeSelect(pathNodeIdx.toString())}
                            >
                              <StyledTreeItem
                                nodeId={`${entry.finfo?.id as string}`}
                                label={
                                  <StyledDiv sx={{ fontSize: '0.85rem', marginTop: '3px', fontWeight: 'bold' }}>
                                    {entry.finfo?.name}
                                  </StyledDiv>
                                }
                              >
                                {entry.matchingLines?.map((line, idx) => {
                                  return (
                                    <FileLine
                                      key={idx}
                                      sx={{ fontSize: '0.85rem', marginTop: '3px', paddingLeft: '5px' }}
                                      onClick={() => handleFileLineClick(entry.finfo as FileInfo)}
                                    >
                                      {line.text}
                                    </FileLine>
                                  );
                                })}
                              </StyledTreeItem>
                            </StyledTreeView>
                          </div>
                        );
                      })}
                  </StyledTreeItem>
                </div>
              );
            })}
          </StyledDiv>
        </StyledDiv>
      ) : (
        'No results'
      )}
    </StyledTreeView>
  );
};
