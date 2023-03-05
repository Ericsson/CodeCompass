import { FolderOpen, Folder } from '@mui/icons-material';
import { TreeItem, TreeView } from '@mui/lab';
import { styled } from '@mui/material';
import { FileIcon } from 'components/file-icon/file-icon';
import { SearchContext } from 'global-context/search-context';
import { SyntheticEvent, useContext, useEffect, useState } from 'react';

const StyledDiv = styled('div')({});

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

export const SearchResults = (): JSX.Element => {
  const searchCtx = useContext(SearchContext);
  const [resultPaths, setResultPaths] = useState<string[]>([]);
  const [expandedPathNodes, setExpandedPathNodes] = useState<string[]>([]);
  // const [expandedFileNodes, setExpandedFileNodes] = useState<FileNodesType>({});

  useEffect(() => {
    if (!searchCtx.searchResult?.results) {
      return;
    }
    const paths = new Set(searchCtx.searchResult?.results?.map((entry) => entry.finfo?.path) as string[]);
    setResultPaths([...paths]);
    setExpandedPathNodes([...paths].map((_e, idx) => idx.toString()));

    // const expandedFileNodesMap: FileNodesType = {};
    // for (const path of paths) {
    //   const fileIds = searchCtx.searchResult.results
    //     .filter((entry) => entry.finfo?.path === path)
    //     .map((entry) => entry.finfo?.id) as string[];
    //   expandedFileNodesMap[path] = {
    //     expandedNodes: fileIds,
    //   };
    // }
    // setExpandedFileNodes(expandedFileNodesMap);
  }, [searchCtx.searchResult?.results]);

  const handleDirNodeSelect = () => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      const index = expandedPathNodes.indexOf(nodeId);
      const copyExpanded = [...expandedPathNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      setExpandedPathNodes(copyExpanded);
    };
  };

  // const handleFileNodeSelect = () => {
  //   return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
  //     const expandedNodes = expandedFileNodes[nodeId].expandedNodes;
  //     const index = expandedNodes.indexOf(nodeId);
  //     const copyExpanded = [...expandedNodes];
  //     if (index === -1) {
  //       copyExpanded.push(nodeId);
  //     } else {
  //       copyExpanded.splice(index, 1);
  //     }
  //     setExpandedFileNodes((prevExpanded) => {
  //       prevExpanded[nodeId].expandedNodes = copyExpanded;
  //       return {
  //         ...prevExpanded,
  //       };
  //     });
  //   };
  // };

  return (
    <TreeView
      defaultCollapseIcon={<FolderOpen />}
      defaultExpandIcon={<Folder />}
      expanded={expandedPathNodes}
      onNodeSelect={handleDirNodeSelect()}
    >
      {searchCtx.searchResult ? (
        <>
          {resultPaths.map((path, idx) => {
            return (
              <div key={idx}>
                <TreeItem nodeId={`${idx}`} label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{path}</StyledDiv>}>
                  {searchCtx.searchResult?.results
                    ?.filter((result) => result.finfo?.path === path)
                    .map((entry, idx) => {
                      return (
                        <div key={idx}>
                          <TreeView
                            defaultCollapseIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                            defaultExpandIcon={<FileIcon fileName={entry.finfo?.name as string} />}
                            // expanded={expandedFileNodes[entry.finfo?.id as string].expandedNodes}
                            // onNodeSelect={handleFileNodeSelect()}
                          >
                            <TreeItem
                              nodeId={`${entry.finfo?.id as string}`}
                              label={
                                <StyledDiv sx={{ fontSize: '0.85rem', fontWeight: 'bold' }}>
                                  {entry.finfo?.name}
                                </StyledDiv>
                              }
                            >
                              {entry.matchingLines?.map((line, idx) => {
                                return (
                                  <StyledDiv key={idx} sx={{ fontSize: '0.85rem' }}>
                                    {line.text}
                                  </StyledDiv>
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
