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
  '&[data-selected="true"]': {
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

const File = (info: FileInfo, handleClick: (info: FileInfo) => Promise<void>): JSX.Element => {
  return (
    <FileLabel key={info.id} data-id={info.id} onClick={() => handleClick(info)}>
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

const findNodeById = (node: TreeNode, nodeId: string): TreeNode | null => {
  if (node.info.id === nodeId) {
    return node;
  }
  if (node.children) {
    for (const child of node.children) {
      const foundNode = findNodeById(child, nodeId);
      if (foundNode) {
        return foundNode;
      }
    }
  }
  return null;
};

export const FileTree = () => {
  const projectCtx = useContext(ProjectContext);
  const [tree, setTree] = useState<TreeNode | undefined>(undefined);

  useEffect(() => {
    const getFileData = async () => {
      const storedRootFiles = localStorage.getItem('currentRootFiles');
      const storedFileTree = localStorage.getItem('currentFileTree');

      if (storedFileTree) {
        if (storedRootFiles) {
          projectCtx.setRootFiles(JSON.parse(storedRootFiles));
        }
        setTree(JSON.parse(storedFileTree));
      } else {
        let rootFileData: FileInfo[] = [];
        if (storedRootFiles) {
          rootFileData = JSON.parse(storedRootFiles) as FileInfo[];
        } else {
          rootFileData = await getRootFiles();
          localStorage.setItem('currentRootFiles', JSON.stringify(rootFileData));
        }
        const rootDirectory = rootFileData.find((info) => info.isDirectory) as FileInfo;
        projectCtx.setRootFiles(rootFileData);
        setTree({
          info: rootDirectory,
        });
      }

      const storedExpandedNodes = localStorage.getItem('expandedNodes');
      projectCtx.setExpandedFileTreeNodes(storedExpandedNodes ? JSON.parse(storedExpandedNodes) : []);
    };
    getFileData();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    if (!projectCtx.selectedFile) {
      return;
    }
    const prevSelectedTreeViewFile = document.querySelector('[data-selected]') as HTMLDivElement;
    const selectedTreeViewFile = document.querySelector(`[data-id="${projectCtx.selectedFile}"]`) as HTMLDivElement;
    if (prevSelectedTreeViewFile) {
      prevSelectedTreeViewFile.removeAttribute('data-selected');
    }
    if (selectedTreeViewFile) {
      selectedTreeViewFile.setAttribute('data-selected', 'true');
    }
  }, [projectCtx.selectedFile, projectCtx.expandedFileTreeNodes]);

  const handleClick = async (info: FileInfo) => {
    if (info.isDirectory) {
      const childFiles = await getChildFiles(info.id as string);
      projectCtx.setFiles(childFiles);
      projectCtx.setFolderPath(info.path as string);
      localStorage.setItem('currentFiles', JSON.stringify(childFiles));
      localStorage.setItem('currentPath', info.path as string);

      const childTreeNodes = childFiles.map((childFile) => {
        return { info: childFile } as TreeNode;
      });

      setTree((prevTree) => {
        if (!prevTree) {
          return;
        }
        const updatedNode = findNodeById(prevTree, info.id as string);
        if (!updatedNode) {
          return;
        }
        updatedNode.children = childTreeNodes;
        localStorage.setItem('currentFileTree', JSON.stringify({ ...prevTree }));
        return { ...prevTree };
      });
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

  const renderFileTree = (node: TreeNode): JSX.Element[] => {
    return node.children
      ? node.children.map((childNode) => {
          if (childNode.info.isDirectory) {
            return Directory(childNode.info, () => handleClick(childNode.info), renderFileTree(childNode));
          } else {
            return File(childNode.info, () => handleClick(childNode.info));
          }
        })
      : [];
  };

  return tree ? (
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
          ? Directory(info, () => handleClick(info), renderFileTree(tree))
          : File(info, () => handleClick(info))
      )}
    </StyledTreeView>
  ) : (
    <Loading />
  );
};
