import { Folder, DriveFolderUpload, FolderOpen } from '@mui/icons-material';
import { alpha, styled } from '@mui/material';
import { useContext, useState, MouseEvent, useEffect, SyntheticEvent } from 'react';
import { FileInfo } from '@thrift-generated';
import { getChildFiles, getFileInfo, getParentFiles, getParents, getRootFiles } from 'service/project-service';
import { FileIcon } from 'components/file-icon/file-icon';
import { TabName } from 'enums/tab-enum';
import { FileContextMenu } from 'components/file-context-menu/file-context-menu';
import { RouterQueryType, TreeNode } from 'utils/types';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { getStore, setStore } from 'utils/store';
import { useRouter } from 'next/router';
import { getFileFolderPath } from 'utils/utils';
import { AppContext } from 'global-context/app-context';

const StyledDiv = styled('div')({});

const FolderName = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px 10px',
  fontSize: '0.85rem',
  width: '100%',
}));

const FolderUp = styled('div')(({ theme }) => ({
  padding: '5px',
  display: 'flex',
  alignItems: 'center',
  fontSize: '0.8rem',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

const FileLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

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

export const FileManager = (): JSX.Element => {
  const router = useRouter();
  const routerQuery = router.query as RouterQueryType;

  const appCtx = useContext(AppContext);

  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);

  const [files, setFiles] = useState<FileInfo[] | undefined>(undefined);
  const [fileTree, setFileTree] = useState<TreeNode | undefined>(undefined);
  const [folderPath, setFolderPath] = useState<string | undefined>(undefined);
  const [expandedFileTreeNodes, setExpandedFileTreeNodes] = useState<string[] | undefined>(undefined);

  const [contextMenu, setContextMenu] = useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);
  const [fileInfoForDiagram, setFileInfoForDiagram] = useState<FileInfo | undefined>(undefined);

  useEffect(() => {
    if (!appCtx.workspaceId) return;

    const init = async () => {
      const rootFileData = await getRootFiles();
      const rootDirectory = rootFileData.find((info) => info.isDirectory) as FileInfo;
      setRootFiles(rootFileData);

      const { storedFiles, storedFileTree, storedFolderPath, storedExpandedFileTreeNodes } = getStore();

      setFiles(storedFiles ?? rootFileData);
      setFileTree(storedFileTree ?? { info: rootDirectory });
      setFolderPath(storedFolderPath ?? '');
      setExpandedFileTreeNodes(storedExpandedFileTreeNodes ?? []);

      if (routerQuery.projFileId) {
        try {
          const initFileInfo = await getFileInfo(routerQuery.projFileId);
          const children = await getChildFiles(initFileInfo?.parent as string);
          const parents = await getParents(initFileInfo?.path as string);

          setFiles(children);
          setExpandedFileTreeNodes(parents);
        } catch {}
      }
    };

    init();
  }, [appCtx.workspaceId, routerQuery.projFileId]);

  useEffect(() => {
    if (!appCtx.projectFileId) return;

    const init = async () => {
      const initFileInfo = await getFileInfo(appCtx.projectFileId);
      const initFolderPath = getFileFolderPath(initFileInfo?.path as string);
      const parents = await getParents(initFolderPath);
      const children = await getChildFiles(parents[0]);

      if (children.length) {
        setFiles(children);
        setFolderPath(initFolderPath);
        setExpandedFileTreeNodes((prevNodes) =>
          prevNodes && prevNodes.length ? [...prevNodes.filter((node) => !parents.includes(node)), ...parents] : parents
        );
      }
    };

    init();
  }, [appCtx.projectFileId]);

  useEffect(() => {
    if (!rootFiles?.length) return;

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

    const getChildNodes = async (childFiles: FileInfo[]): Promise<TreeNode[]> => {
      const childNodes: TreeNode[] = [];
      for (const childFile of childFiles) {
        if (childFile.isDirectory && expandedFileTreeNodes?.includes(childFile.id as string)) {
          const children = await getChildFiles(childFile.id as string);
          childNodes.push({ info: childFile, children: await getChildNodes(children) });
        } else {
          childNodes.push({ info: childFile } as TreeNode);
        }
      }
      return childNodes;
    };

    const updateTree = async () => {
      const rootDirectory = rootFiles.find((info) => info.isDirectory) as FileInfo;
      const childFiles = await getChildFiles(rootDirectory.id as string);
      const childTreeNodes = await getChildNodes(childFiles);
      setFileTree((prevFileTree) => {
        if (!prevFileTree) {
          return;
        }
        const updatedNode = findNodeById(prevFileTree, rootDirectory.id as string);
        if (!updatedNode) {
          return;
        }
        updatedNode.children = childTreeNodes;
        return { ...prevFileTree };
      });
    };

    updateTree();
  }, [rootFiles, expandedFileTreeNodes]);

  useEffect(() => {
    setStore({
      storedFiles: files,
      storedFileTree: fileTree,
      storedFolderPath: folderPath,
      storedExpandedFileTreeNodes: expandedFileTreeNodes,
    });
  }, [files, fileTree, folderPath, expandedFileTreeNodes]);

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
    handleFileClick(fileInfo);
    setFileInfoForDiagram(fileInfo);
  };

  const navigateBack = async () => {
    if (folderPath === '/') {
      setFolderPath('');
      setFiles(rootFiles);
      setExpandedFileTreeNodes([]);
      return;
    }
    const pathAsArray = folderPath?.split('/') as string[];
    pathAsArray.pop();
    if (pathAsArray.length === 1) {
      pathAsArray.push('');
    }
    const trimmedPath = pathAsArray.join('/');
    const parents = await getParents(trimmedPath);
    const parentFiles = await getParentFiles(trimmedPath);
    setFolderPath(trimmedPath);
    setFiles(parentFiles);
    setExpandedFileTreeNodes(parents);
  };

  const handleFileClick = async (fInfo: FileInfo) => {
    if (fInfo.isDirectory) {
      const childFiles = await getChildFiles(fInfo.id as string);

      if (!appCtx.treeViewOption) {
        const parents = await getParents(fInfo.path as string);
        setExpandedFileTreeNodes(parents);
      }

      setFiles(childFiles);
      setFolderPath(fInfo.path as string);
    } else {
      if (appCtx.treeViewOption) {
        const children = await getChildFiles(fInfo.parent as string);
        setFiles(children);
      } else {
        const parents = await getParents(folderPath as string);
        setExpandedFileTreeNodes(parents);
      }

      appCtx.setProjectFileId(fInfo.id as string);
      appCtx.setActiveTab(TabName.CODE);
    }
  };

  const handleTreeNodeSelect = () => {
    return (_e: SyntheticEvent<Element, Event>, nodeId: string) => {
      if (!expandedFileTreeNodes) return;

      const index = expandedFileTreeNodes.indexOf(nodeId);
      const copyExpanded = [...expandedFileTreeNodes];
      if (index === -1) {
        copyExpanded.push(nodeId);
      } else {
        copyExpanded.splice(index, 1);
      }
      setExpandedFileTreeNodes(copyExpanded);
    };
  };

  const renderFileTree = (node: TreeNode | undefined): JSX.Element[] => {
    return node && node.children
      ? node.children.map((childNode) => {
          return childNode.info.isDirectory ? (
            <Directory key={childNode.info.id} fInfo={childNode.info}>
              {renderFileTree(childNode)}
            </Directory>
          ) : (
            <StyledDiv key={childNode.info.id} sx={{ paddingLeft: '5px' }}>
              <File fInfo={childNode.info} />
            </StyledDiv>
          );
        })
      : [];
  };

  const Directory = ({ fInfo, children }: { fInfo: FileInfo; children?: JSX.Element[] }): JSX.Element => {
    return (
      <StyledTreeItem
        nodeId={fInfo.id as string}
        label={
          <StyledDiv onContextMenu={(e) => handleContextMenu(e, fInfo)} sx={{ fontSize: '0.8rem' }}>
            {fInfo.name}
          </StyledDiv>
        }
        onClick={() => handleFileClick(fInfo)}
      >
        {children}
      </StyledTreeItem>
    );
  };

  const File = ({ fInfo }: { fInfo: FileInfo }): JSX.Element => {
    return (
      <FileLabel
        onClick={() => handleFileClick(fInfo)}
        onContextMenu={(e) => handleContextMenu(e, fInfo)}
        sx={{
          backgroundColor: (theme) =>
            fInfo.id === appCtx.projectFileId ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
        }}
      >
        {fInfo.isDirectory ? (
          <Folder sx={{ width: '20px', height: '20px' }} />
        ) : (
          <FileIcon fileName={fInfo.name as string} />
        )}
        <StyledDiv
          sx={{
            fontSize: '0.8rem',
            color: (theme) =>
              fInfo.parseStatus === 3
                ? theme.colors?.success
                : fInfo.parseStatus === 2
                ? theme.colors?.warning
                : theme.colors?.primary,
          }}
        >
          {fInfo.name}
        </StyledDiv>
      </FileLabel>
    );
  };

  return (
    <>
      {appCtx.treeViewOption ? (
        <StyledTreeView
          defaultCollapseIcon={<FolderOpen />}
          defaultExpandIcon={<Folder />}
          defaultEndIcon={<Folder />}
          expanded={expandedFileTreeNodes ?? []}
          onNodeSelect={handleTreeNodeSelect()}
        >
          {fileTree
            ? rootFiles.map((fInfo) =>
                fInfo.isDirectory ? (
                  <Directory key={fInfo.id} fInfo={fInfo}>
                    {renderFileTree(fileTree)}
                  </Directory>
                ) : (
                  <File key={fInfo.id} fInfo={fInfo} />
                )
              )
            : ''}
        </StyledTreeView>
      ) : (
        <>
          {folderPath !== undefined && folderPath === '' ? (
            ''
          ) : (
            <>
              <FolderName>{folderPath === '/' ? '/' : '../' + folderPath?.split('/').reverse()[0]}</FolderName>
              <FolderUp onClick={() => navigateBack()}>
                <DriveFolderUpload sx={{ width: '20px', height: '20px' }} />
                <div>{'..'}</div>
              </FolderUp>
            </>
          )}
          <StyledDiv sx={{ padding: '5px', fontSize: '0.85rem' }}>
            {files ? files.map((file) => <File key={file.id} fInfo={file} />) : ''}
          </StyledDiv>
        </>
      )}
      <FileContextMenu
        contextMenu={contextMenu}
        setContextMenu={setContextMenu}
        fileInfo={fileInfoForDiagram as FileInfo}
      />
    </>
  );
};
