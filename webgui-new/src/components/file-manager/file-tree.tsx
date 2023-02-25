import { FolderOpen, Folder } from '@mui/icons-material';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { FileInfo } from '@thrift-generated/FileInfo';
import { FileIcon } from '../file-icon/file-icon';
import { SyntheticEvent, useContext, useEffect, useState } from 'react';
import { getRootFiles, getChildFiles, getFileContent } from '../../service/project-service';
import { ProjectContext } from '../../global-context/project-context';

type TreeNode = {
  info: FileInfo;
  children?: TreeNode[];
};

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

const FileLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  marginLeft: '5px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

const StyledDiv = styled('div')({});

const Directory = (
  info: FileInfo,
  handleClick: (info: FileInfo) => Promise<void>,
  children?: JSX.Element[]
): JSX.Element => {
  return (
    <StyledTreeItem
      key={info.id}
      nodeId={info.id as string}
      label={<StyledDiv sx={{ fontSize: '0.8rem' }}>{info.name}</StyledDiv>}
      onClick={() => handleClick(info)}
    >
      {children}
    </StyledTreeItem>
  );
};

const File = (info: FileInfo, selected: string, handleClick: (info: FileInfo) => Promise<void>): JSX.Element => {
  return (
    <FileLabel
      key={info.id}
      data-id={info.id}
      onClick={() => handleClick(info)}
      sx={{
        backgroundColor: (theme) =>
          info.id === selected ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
      }}
    >
      <FileIcon fileName={info.name as string} />
      <StyledDiv
        sx={{
          fontSize: '0.8rem',
          color: (theme) =>
            info.parseStatus === 3
              ? theme.colors?.success
              : info.parseStatus === 2
              ? theme.colors?.warning
              : theme.colors?.primary,
        }}
      >
        {info.name}
      </StyledDiv>
    </FileLabel>
  );
};

const Loading = (): JSX.Element => {
  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
        gap: '1rem',
        paddingTop: '10px',
      }}
    >
      <div>{'Loading files...'}</div>
      <CircularProgress />
    </Box>
  );
};

export const FileTree = () => {
  const projectCtx = useContext(ProjectContext);

  const handleClick = async (info: FileInfo) => {
    if (info.isDirectory) {
      const childFiles = await getChildFiles(info.id as string);
      projectCtx.setFiles(childFiles);
      projectCtx.setFolderPath(info.path as string);
      localStorage.setItem('currentFiles', JSON.stringify(childFiles));
      localStorage.setItem('currentPath', info.path as string);
    } else {
      const children = await getChildFiles(info.parent as string);
      const fileContent = await getFileContent(info.id as string);
      projectCtx.setFiles(children);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(info);
      projectCtx.setSelectedFile(info.id as string);
      localStorage.setItem('currentFiles', JSON.stringify(children));
      localStorage.setItem('currentFileContent', fileContent);
      localStorage.setItem('currentFileInfo', JSON.stringify(info));
      localStorage.setItem('currentSelectedFile', info.id as string);
    }
  };

  const renderFileTree = (node: TreeNode | undefined): JSX.Element[] => {
    return node && node.children
      ? node.children.map((childNode) => {
          if (childNode.info.isDirectory) {
            return Directory(childNode.info, () => handleClick(childNode.info), renderFileTree(childNode));
          } else {
            return File(childNode.info, projectCtx.selectedFile, () => handleClick(childNode.info));
          }
        })
      : [];
  };

  return projectCtx.fileTree ? (
    <StyledTreeView
      defaultCollapseIcon={<FolderOpen />}
      defaultExpandIcon={<Folder />}
      defaultEndIcon={<Folder />}
      expanded={projectCtx.expandedFileTreeNodes}
      onNodeSelect={(_e: SyntheticEvent<Element, Event>, nodeId: string) => {
        const index = projectCtx.expandedFileTreeNodes.indexOf(nodeId);
        const copyExpanded = [...projectCtx.expandedFileTreeNodes];
        if (index === -1) {
          copyExpanded.push(nodeId);
        } else {
          copyExpanded.splice(index, 1);
        }
        projectCtx.setExpandedFileTreeNodes(copyExpanded);
        localStorage.setItem('expandedNodes', JSON.stringify(copyExpanded));
      }}
    >
      {projectCtx.rootFiles.map((info) =>
        info.isDirectory
          ? Directory(info, () => handleClick(info), renderFileTree(projectCtx.fileTree))
          : File(info, projectCtx.selectedFile, () => handleClick(info))
      )}
    </StyledTreeView>
  ) : (
    <Loading />
  );
};
