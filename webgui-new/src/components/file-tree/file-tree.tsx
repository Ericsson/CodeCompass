import { Folder, FolderOpen, DriveFolderUpload } from '@mui/icons-material';
import { TreeItem, TreeView, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { useState, useEffect, useContext, SyntheticEvent, useCallback } from 'react';
import { FileInfo } from '@thrift-generated/index';
import { ProjectContext } from '../../global-context/project-context';
import { getRootFiles, getChildFiles, getFileContent, getParentFiles, getParents } from '../../service/project-service';
import { FileIcon } from '../file-icon/file-icon';

const Container = styled('div')({
  padding: '5px',
  fontSize: '0.85rem',
});

const Foldername = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px 10px',
  fontSize: '0.85rem',
  width: '100%',
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
    paddingLeft: '1px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

const IconLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
  '&[data-selected="true"]': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

const FileLabel = styled('div')({
  fontSize: '0.8rem',
});

const FolderUp = styled('div')(({ theme }) => ({
  padding: '5px',
  display: 'flex',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

export const FileTree = ({ treeView }: { treeView: boolean }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [selectedFile, setSelectedFile] = useState<string | undefined>(undefined);
  const [fileTree, setFileTree] = useState<JSX.Element[]>([]);
  const [folderPath, setFolderPath] = useState<string>('');
  const [expanded, setExpanded] = useState<string[]>([]);

  const renderFileTree = useCallback(async (files: FileInfo[]): Promise<JSX.Element[]> => {
    const tree: JSX.Element[] = [];
    for (const file of files) {
      if (file.isDirectory) {
        const children = await getChildFiles(file.id as string);
        tree.push(
          <StyledTreeItem
            key={file.id}
            nodeId={`${file.id}`}
            label={<FileLabel>{file.name}</FileLabel>}
            onClick={async () => {
              const children = await getChildFiles(file.id as string);
              setFiles(children);
              setFolderPath(file.path as string);
              localStorage.setItem('currentFiles', JSON.stringify(children));
              localStorage.setItem('currentPath', file.path as string);
            }}
          >
            {await renderFileTree(children)}
          </StyledTreeItem>
        );
      } else {
        tree.push(
          <IconLabel
            key={file.id}
            data-id={file.id}
            onClick={async () => {
              const children = await getChildFiles(file.parent as string);
              setFiles(children);
              const fileContent = await getFileContent(file.id as string);
              projectCtx.setFileContent(fileContent);
              projectCtx.setFileInfo(file);
              setSelectedFile(file.id);
              localStorage.setItem('currentFileContent', fileContent);
              localStorage.setItem('currentFileInfo', JSON.stringify(file));
              localStorage.setItem('currentSelectedFile', file.id as string);
              localStorage.setItem('currentFiles', JSON.stringify(children));
            }}
            sx={{ marginLeft: '5px' }}
          >
            <FileIcon fileName={file.name as string} />
            <FileLabel
              sx={{
                color: (theme) =>
                  file.parseStatus === 3
                    ? theme.colors?.success
                    : file.parseStatus === 2
                    ? theme.colors?.warning
                    : theme.colors?.primary,
              }}
            >
              {file.name}
            </FileLabel>
          </IconLabel>
        );
      }
    }
    return tree;
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    const getData = async () => {
      const rootFileData = await getRootFiles();
      setRootFiles(rootFileData);

      const renderedFileTree = await renderFileTree(rootFileData);
      setFileTree(renderedFileTree);

      const storedCurrentFiles = localStorage.getItem('currentFiles');
      setFiles(storedCurrentFiles ? JSON.parse(storedCurrentFiles) : rootFileData);

      const storedCurrentSelectedFile = localStorage.getItem('currentSelectedFile');
      setSelectedFile(storedCurrentSelectedFile ?? undefined);

      const storedCurrentPath = localStorage.getItem('currentPath');
      setFolderPath(storedCurrentPath ?? '');

      const storedCurrentFileContent = localStorage.getItem('currentFileContent');
      projectCtx.setFileContent(storedCurrentFileContent ?? undefined);

      const storedCurrentFileInfo = localStorage.getItem('currentFileInfo');
      projectCtx.setFileInfo(storedCurrentFileInfo ? JSON.parse(storedCurrentFileInfo) : undefined);

      const storedExpandedNodes = localStorage.getItem('expandedNodes');
      setExpanded(storedExpandedNodes ? JSON.parse(storedExpandedNodes) : []);
    };
    getData();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!selectedFile) {
      return;
    }
    const prevSelectedTreeViewFile = document.querySelector('[data-selected]') as HTMLDivElement;
    const selectedTreeViewFile = document.querySelector(`[data-id="${selectedFile}"]`) as HTMLDivElement;
    if (prevSelectedTreeViewFile) {
      prevSelectedTreeViewFile?.removeAttribute('data-selected');
    }
    if (selectedTreeViewFile) {
      selectedTreeViewFile.setAttribute('data-selected', 'true');
    }
  }, [selectedFile, treeView, expanded]);

  const navigateBack = async () => {
    const pathAsArray = folderPath.split('/');
    pathAsArray.pop();
    const trimmedPath = pathAsArray.join('/');
    setFolderPath(trimmedPath);
    localStorage.setItem('currentPath', trimmedPath);
    if (trimmedPath === '') {
      setFiles(rootFiles);
      setExpanded([]);
      localStorage.setItem('currentFiles', JSON.stringify(rootFiles));
      localStorage.setItem('expandedNodes', JSON.stringify([]));
      return;
    }
    const parentFiles = await getParentFiles(trimmedPath);
    setFiles(parentFiles);
    const parents = await getParents(trimmedPath);
    setExpanded(parents);
    localStorage.setItem('currentFiles', JSON.stringify(parentFiles));
    localStorage.setItem('expandedNodes', JSON.stringify(parents));
  };

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      const children = await getChildFiles(file.id as string);
      setFiles(children);
      setFolderPath(file.path as string);
      const parents = await getParents(file.path as string);
      setExpanded(parents);
      localStorage.setItem('currentFiles', JSON.stringify(children));
      localStorage.setItem('currentPath', file.path as string);
      localStorage.setItem('expandedNodes', JSON.stringify(parents));
    } else {
      const fileContent = await getFileContent(file.id as string);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(file);
      setSelectedFile(file.id);
      const parents = await getParents(folderPath);
      setExpanded(parents);
      localStorage.setItem('currentFileContent', fileContent);
      localStorage.setItem('currentFileInfo', JSON.stringify(file));
      localStorage.setItem('currentSelectedFile', file.id as string);
      localStorage.setItem('expandedNodes', JSON.stringify(parents));
    }
  };

  return treeView ? (
    <StyledTreeView
      defaultCollapseIcon={<FolderOpen />}
      defaultExpandIcon={<Folder />}
      expanded={expanded}
      onNodeSelect={(_e: SyntheticEvent<Element, Event>, nodeId: string) => {
        const index = expanded.indexOf(nodeId);
        const copyExpanded = [...expanded];
        if (index === -1) {
          copyExpanded.push(nodeId);
        } else {
          copyExpanded.splice(index, 1);
        }
        setExpanded(copyExpanded);
        localStorage.setItem('expandedNodes', JSON.stringify(copyExpanded));
      }}
    >
      {fileTree}
    </StyledTreeView>
  ) : (
    <>
      {folderPath === '' ? (
        ''
      ) : (
        <>
          <Foldername>{folderPath === '/' ? '/' : '../' + folderPath.split('/').reverse()[0]}</Foldername>
          <FolderUp onClick={() => navigateBack()}>
            <DriveFolderUpload />
            <div>{'..'}</div>
          </FolderUp>
        </>
      )}
      <Container>
        {files.map((file, idx) => {
          return (
            <IconLabel key={idx} data-id={file.id} onClick={() => handleFileClick(file)}>
              {file.isDirectory ? <Folder /> : <FileIcon fileName={file.name as string} />}
              <FileLabel
                sx={{
                  color: (theme) =>
                    file.parseStatus === 3
                      ? theme.colors?.success
                      : file.parseStatus === 2
                      ? theme.colors?.warning
                      : theme.colors?.primary,
                }}
              >
                {file.name}
              </FileLabel>
            </IconLabel>
          );
        })}
      </Container>
    </>
  );
};
