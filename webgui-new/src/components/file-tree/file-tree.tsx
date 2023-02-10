import { Folder, FolderOpen, DriveFolderUpload } from '@mui/icons-material';
import { TreeItem, TreeView, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { useState, useEffect, useContext } from 'react';
import { FileInfo } from '@thrift-generated/index';
import { ProjectContext } from '../../global-context/project-context';
import { getRootFiles, getChildFiles, getFileContent, getParentFiles } from '../../service/project-service';
import { FileIcon } from '../file-icon/file-icon';

const Container = styled('div')({
  padding: '5px',
  fontSize: '0.85rem',
});

const Foldername = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px 10px',
  fontSize: '0.85rem',
  width: '280px',
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
    paddingLeft: '5px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
  [`& .${treeItemClasses.label}`]: {
    fontSize: '0.85rem',
  },
}));

const IconLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.palette.text.primary, 0.3),
  },
}));

const FileLabel = styled('div')({});

const FolderUp = styled('div')(({ theme }) => ({
  padding: '5px',
  display: 'flex',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.palette.text.primary, 0.3),
  },
}));

export const FileTree = ({ treeView }: { treeView: boolean }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [folderPath, setFolderPath] = useState<string>('/');
  const [fileTree, setFileTree] = useState<JSX.Element[]>([]);
  const [selectedFile, setSelectedFile] = useState<string>();

  let nodeId = 1;
  let keyId = 1;
  const renderFileTree = async (files: FileInfo[]): Promise<JSX.Element[]> => {
    const tree: JSX.Element[] = [];
    files.forEach(async (file) => {
      if (file.isDirectory) {
        const children = await getChildFiles(projectCtx.currentWorkspace, file.id as string);
        tree.push(
          <StyledTreeItem key={keyId++} nodeId={`${nodeId++}`} label={file.name}>
            {await renderFileTree(children)}
          </StyledTreeItem>
        );
      } else {
        tree.push(
          <IconLabel key={keyId++} onClick={() => handleTreeViewFileClick(file)}>
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
    });
    return tree;
  };

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const getData = async () => {
      const rootFileData = await getRootFiles(projectCtx.currentWorkspace);
      setRootFiles(rootFileData);
      setFiles(rootFileData);
      setFileTree(await renderFileTree(await getRootFiles(projectCtx.currentWorkspace)));
    };
    getData();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [projectCtx.currentWorkspace]);

  const navigateBack = async () => {
    const pathAsArray = folderPath.split('/');
    pathAsArray.pop();
    const trimmedPath = pathAsArray.join('/');
    setFolderPath(trimmedPath);
    if (trimmedPath === '') {
      setFiles(rootFiles);
      return;
    }
    const parentFiles = await getParentFiles(projectCtx.currentWorkspace, trimmedPath);
    setFiles(parentFiles);
  };

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      const children = await getChildFiles(projectCtx.currentWorkspace, file.id as string);
      setFiles(children);
      setFolderPath(file.path as string);
    } else {
      const fileContent = await getFileContent(projectCtx.currentWorkspace, file.id as string);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(file);
      setSelectedFile(file.id);
    }
  };

  const handleTreeViewFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      return;
    } else {
      const fileContent = await getFileContent(projectCtx.currentWorkspace, file.id as string);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(file);
    }
  };

  return treeView ? (
    <StyledTreeView defaultCollapseIcon={<FolderOpen />} defaultExpandIcon={<Folder />}>
      {fileTree}
    </StyledTreeView>
  ) : (
    <>
      {files === rootFiles ? (
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
            <IconLabel
              key={idx}
              onClick={() => handleFileClick(file)}
              sx={{
                backgroundColor: (theme) =>
                  selectedFile === file.id ? alpha(theme.palette.text.primary, 0.4) : 'unset',
              }}
            >
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
