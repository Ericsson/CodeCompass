import { Code, DriveFolderUpload } from '@mui/icons-material';
import { Checkbox, alpha } from '@mui/material';
import React, { useContext, useState, MouseEvent, useEffect, SyntheticEvent } from 'react';
import { FileInfo } from '@thrift-generated';
import {
  getChildFiles,
  getFileInfo,
  getFileInfoByPath,
  getParentFiles,
  getParents,
  getRootFiles,
} from 'service/project-service';
import { FileIcon, FolderIcon } from 'components/custom-icon/custom-icon';
import { TabName } from 'enums/tab-enum';
import { FileContextMenu } from 'components/file-context-menu/file-context-menu';
import { RouterQueryType, TreeNode } from 'utils/types';
import { useRouter } from 'next/router';
import { getFileFolderPath } from 'utils/utils';
import { AppContext } from 'global-context/app-context';
import * as SC from './styled-components';
import { useTranslation } from 'react-i18next';

export const FileManager = (): JSX.Element => {
  const { t } = useTranslation();
  const router = useRouter();
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
      setFiles(rootFileData);
      setFileTree({ info: rootDirectory });
      setFolderPath('');
      setExpandedFileTreeNodes([]);
    };

    init();
  }, [appCtx.workspaceId]);

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
    if (!fileInfo.isDirectory) {
      if (appCtx.treeViewOption) {
        const children = await getChildFiles(fileInfo.parent as string);
        setFiles(children);
      } else {
        const parents = await getParents(folderPath as string);
        setExpandedFileTreeNodes(parents);
      }

      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          projectFileId: fileInfo.id as string,
          editorSelection: '',
          activeTab: TabName.CODE.toString(),
        } as RouterQueryType,
      });
    }
    setFileInfoForDiagram(fileInfo);
  };

  const jumpToLabel = async (label: string) => {
    const fInfo = await getFileInfoByPath(label);

    if (!fInfo) return;

    const childFiles = await getChildFiles(fInfo.id as string);
    const parents = await getParents(fInfo.path as string);

    setExpandedFileTreeNodes(parents);
    setFiles(childFiles);
    setFolderPath(fInfo.path as string);
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

      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          projectFileId: fInfo.id as string,
          editorSelection: '',
          activeTab: TabName.CODE.toString(),
        } as RouterQueryType,
      });
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

  const renderFileTree = (node: TreeNode | undefined) => {
    return node && node.children
      ? node.children.map((childNode) => {
          return childNode.info.isDirectory ? (
            <Directory key={childNode.info.id} fInfo={childNode.info}>
              {renderFileTree(childNode)}
            </Directory>
          ) : (
            <SC.StyledDiv key={childNode.info.id} sx={{ paddingLeft: '5px' }}>
              <File fInfo={childNode.info} />
            </SC.StyledDiv>
          );
        })
      : [];
  };

  const Directory = ({ fInfo, children }: { fInfo: FileInfo; children?: React.ReactNode[] }) => {
    return (
      <SC.StyledTreeItem
        nodeId={fInfo.id as string}
        label={
          <SC.StyledDiv onContextMenu={(e) => handleContextMenu(e, fInfo)} sx={{ fontSize: '0.8rem' }}>
            {fInfo.name}
          </SC.StyledDiv>
        }
        onClick={() => handleFileClick(fInfo)}
      >
        {children}
      </SC.StyledTreeItem>
    );
  };

  const File = ({ fInfo }: { fInfo: FileInfo }) => {
    return (
      <SC.FileLabel
        onClick={() => handleFileClick(fInfo)}
        onContextMenu={(e) => handleContextMenu(e, fInfo)}
        sx={{
          backgroundColor: (theme) =>
            fInfo.id === appCtx.projectFileId ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
        }}
      >
        {fInfo.isDirectory ? <FolderIcon /> : <FileIcon fileName={fInfo.name as string} />}
        <SC.StyledDiv
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
        </SC.StyledDiv>
      </SC.FileLabel>
    );
  };

  const Labels = (): JSX.Element => {
    return (
      <>
        {Array.from(appCtx.labels.keys()).map((key) => (
          <SC.FileLabel key={key} onClick={() => jumpToLabel(appCtx.labels.get(key) as string)} sx={{ padding: '5px' }}>
            <Code sx={{ width: '20px', height: '20px' }} />
            <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>{`${t('fileManager.jumpTo')} ${key}`}</SC.StyledDiv>
          </SC.FileLabel>
        ))}
      </>
    );
  };

  return (
    <>
      <SC.TreeSetting
        control={
          <Checkbox
            checked={appCtx.treeViewOption === 'true'}
            onChange={() =>
              router.push({
                pathname: '/project',
                query: {
                  ...router.query,
                  treeViewOption: appCtx.treeViewOption === 'true' ? 'false' : 'true',
                } as RouterQueryType,
              })
            }
          />
        }
        label={t('fileManager.treeView')}
      />
      {appCtx.treeViewOption === 'true' ? (
        <SC.StyledTreeView
          defaultCollapseIcon={<FolderIcon open />}
          defaultExpandIcon={<FolderIcon />}
          defaultEndIcon={<FolderIcon />}
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
        </SC.StyledTreeView>
      ) : (
        <>
          {folderPath !== undefined && folderPath === '' ? (
            <Labels />
          ) : (
            <>
              <SC.FolderName>{folderPath === '/' ? '/' : '../' + folderPath?.split('/').reverse()[0]}</SC.FolderName>
              <Labels />
              <SC.FolderUp onClick={() => navigateBack()}>
                <DriveFolderUpload sx={{ width: '20px', height: '20px' }} />
                <div>{'..'}</div>
              </SC.FolderUp>
            </>
          )}
          <SC.StyledDiv sx={{ padding: '5px', fontSize: '0.85rem' }}>
            {files ? files.map((file) => <File key={file.id} fInfo={file} />) : ''}
          </SC.StyledDiv>
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
