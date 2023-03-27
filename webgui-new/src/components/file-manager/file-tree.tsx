import { FolderOpen, Folder } from '@mui/icons-material';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { FileInfo } from '@thrift-generated';
import { FileIcon } from 'components/file-icon/file-icon';
import { SyntheticEvent, useContext, useState, MouseEvent } from 'react';
import { getChildFiles, getFileContent } from 'service/project-service';
import { ProjectContext } from 'global-context/project-context';
import { ConfigContext } from 'global-context/config-context';
import { TabName } from 'enums/tab-enum';
import { FileContextMenu } from 'components/file-context-menu/file-context-menu';

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

export const FileTree = () => {
  const configCtx = useContext(ConfigContext);
  const projectCtx = useContext(ProjectContext);

  const [contextMenu, setContextMenu] = useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);
  const [fileInfoForDiagram, setFileInfoForDiagram] = useState<FileInfo | undefined>(undefined);

  const handleContextMenu = async (event: MouseEvent, fileInfo: FileInfo) => {
    event.preventDefault();
    setContextMenu(
      contextMenu === null
        ? {
            mouseX: event.clientX + 2,
            mouseY: event.clientY - 6,
          }
        : null
    );
    setFileInfoForDiagram(fileInfo);
    if (fileInfo.isDirectory) {
      const childFiles = await getChildFiles(fileInfo.id as string);
      projectCtx.setFiles(childFiles);
      projectCtx.setFolderPath(fileInfo.path as string);
    } else {
      const children = await getChildFiles(fileInfo.parent as string);
      const fileContent = await getFileContent(fileInfo.id as string);
      projectCtx.setFiles(children);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(fileInfo);
      projectCtx.setSelectedFile(fileInfo.id as string);
    }
  };

  const handleClick = async (info: FileInfo) => {
    if (info.isDirectory) {
      const childFiles = await getChildFiles(info.id as string);
      projectCtx.setFiles(childFiles);
      projectCtx.setFolderPath(info.path as string);
    } else {
      const children = await getChildFiles(info.parent as string);
      const fileContent = await getFileContent(info.id as string);
      projectCtx.setFiles(children);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(info);
      projectCtx.setSelectedFile(info.id as string);
      configCtx.setActiveTab(TabName.CODE);
    }
  };

  const renderFileTree = (node: TreeNode | undefined): JSX.Element[] => {
    return node && node.children
      ? node.children.map((childNode, idx) => {
          return childNode.info.isDirectory ? (
            <Directory key={idx} info={childNode.info}>
              {renderFileTree(childNode)}
            </Directory>
          ) : (
            <File key={idx} info={childNode.info} />
          );
        })
      : [];
  };

  const Directory = ({ info, children }: { info: FileInfo; children?: JSX.Element[] }): JSX.Element => {
    return (
      <StyledTreeItem
        key={info.id}
        nodeId={info.id as string}
        label={
          <StyledDiv onContextMenu={(e) => handleContextMenu(e, info)} sx={{ fontSize: '0.8rem' }}>
            {info.name}
          </StyledDiv>
        }
        onClick={() => handleClick(info)}
      >
        {children}
      </StyledTreeItem>
    );
  };

  const File = ({ info }: { info: FileInfo }): JSX.Element => {
    return (
      <FileLabel
        key={info.id}
        data-id={info.id}
        onClick={() => handleClick(info)}
        onContextMenu={(e) => handleContextMenu(e, info)}
        sx={{
          backgroundColor: (theme) =>
            info.id === projectCtx.selectedFile ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
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
      }}
    >
      {projectCtx.rootFiles.map((info, idx) =>
        info.isDirectory ? (
          <Directory key={idx} info={info}>
            {renderFileTree(projectCtx.fileTree)}
          </Directory>
        ) : (
          <File key={idx} info={info} />
        )
      )}
      <FileContextMenu
        contextMenu={contextMenu}
        setContextMenu={setContextMenu}
        fileInfo={fileInfoForDiagram as FileInfo}
      />
    </StyledTreeView>
  ) : (
    <Loading />
  );
};
