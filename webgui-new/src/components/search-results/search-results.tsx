import { FolderOpen, Folder } from '@mui/icons-material';
import { TreeItem, TreeView } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { FileInfo } from '@thrift-generated';
import { FileIcon } from 'components/file-icon/file-icon';
import { TabName } from 'enums/tab-enum';
import { OtherContext } from 'global-context/other-context';
import { ProjectContext } from 'global-context/project-context';
import { SearchContext } from 'global-context/search-context';
import { SyntheticEvent, useContext } from 'react';
import { getParents, getFileContent } from 'service/project-service';

const StyledDiv = styled('div')({});

const FileLine = styled('div')(({ theme }) => ({
  fontSize: '0.85rem',
  marginTop: '3px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

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

  return (
    <TreeView
      defaultCollapseIcon={<FolderOpen />}
      defaultExpandIcon={<Folder />}
      expanded={searchCtx.expandedPathNodes}
      onNodeSelect={handleDirNodeSelect()}
      sx={{ padding: '10px 5px', width: 'fit-content' }}
    >
      {searchCtx.searchResult ? (
        <>
          {searchCtx.resultPaths.map((path, pathNodeIdx) => {
            return (
              <div key={pathNodeIdx}>
                <TreeItem
                  nodeId={`${pathNodeIdx}`}
                  label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{path}</StyledDiv>}
                  sx={{ marginTop: '3px' }}
                >
                  {searchCtx.searchResult?.results
                    ?.filter((result) => result.finfo?.path === path)
                    .map((entry, fileNodeIdx) => {
                      return (
                        <div key={fileNodeIdx}>
                          <TreeView
                            defaultCollapseIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                            defaultExpandIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                            expanded={searchCtx.expandedFileNodes[pathNodeIdx.toString()].expandedNodes}
                            onNodeSelect={handleFileNodeSelect(pathNodeIdx.toString())}
                          >
                            <TreeItem
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
                                    sx={{ fontSize: '0.85rem', marginTop: '3px' }}
                                    onClick={() => handleFileLineClick(entry.finfo as FileInfo)}
                                  >
                                    {line.text}
                                  </FileLine>
                                );
                              })}
                            </TreeItem>
                          </TreeView>
                        </div>
                      );
                    })}
                </TreeItem>
              </div>
            );
          })}
        </>
      ) : (
        'No results'
      )}
    </TreeView>
  );
};
