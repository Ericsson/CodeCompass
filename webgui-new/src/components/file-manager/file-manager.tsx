import { Folder, DriveFolderUpload } from '@mui/icons-material';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { useEffect, useContext } from 'react';
import { FileInfo } from '@thrift-generated/index';
import { ProjectContext } from '../../global-context/project-context';
import { getRootFiles, getChildFiles, getFileContent, getParentFiles, getParents } from '../../service/project-service';
import { FileIcon } from '../file-icon/file-icon';
import { FileTree } from './file-tree';

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

export const FileManager = ({ treeView }: { treeView: boolean }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  useEffect(() => {
    projectCtx.setProjectLoadComplete(false);
    const getData = async () => {
      const rootFileData = await getRootFiles();

      const storedRootFiles = localStorage.getItem('currentRootFiles');
      projectCtx.setRootFiles(storedRootFiles ? JSON.parse(storedRootFiles) : rootFileData);

      const storedCurrentFiles = localStorage.getItem('currentFiles');
      projectCtx.setFiles(storedCurrentFiles ? JSON.parse(storedCurrentFiles) : rootFileData);

      const storedCurrentSelectedFile = localStorage.getItem('currentSelectedFile');
      projectCtx.setSelectedFile(storedCurrentSelectedFile ?? '');

      const storedCurrentPath = localStorage.getItem('currentPath');
      projectCtx.setFolderPath(storedCurrentPath ?? '');

      const storedCurrentFileContent = localStorage.getItem('currentFileContent');
      projectCtx.setFileContent(storedCurrentFileContent ?? '');

      const storedCurrentFileInfo = localStorage.getItem('currentFileInfo');
      projectCtx.setFileInfo(storedCurrentFileInfo ? JSON.parse(storedCurrentFileInfo) : undefined);

      const storedExpandedNodes = localStorage.getItem('expandedNodes');
      projectCtx.setExpandedFileTreeNodes(storedExpandedNodes ? JSON.parse(storedExpandedNodes) : []);
    };
    getData().then(() => projectCtx.setProjectLoadComplete(true));
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    if (!projectCtx.selectedFile) {
      return;
    }
    const prevSelectedTreeViewFile = document.querySelector('[data-selected]') as HTMLDivElement;
    const selectedTreeViewFile = document.querySelector(`[data-id="${projectCtx.selectedFile}"]`) as HTMLDivElement;
    if (prevSelectedTreeViewFile) {
      prevSelectedTreeViewFile?.removeAttribute('data-selected');
    }
    if (selectedTreeViewFile) {
      selectedTreeViewFile.setAttribute('data-selected', 'true');
    }
  }, [projectCtx.selectedFile, treeView, projectCtx.expandedFileTreeNodes]);

  const navigateBack = async () => {
    const pathAsArray = projectCtx.folderPath.split('/');
    pathAsArray.pop();
    const trimmedPath = pathAsArray.join('/');
    projectCtx.setFolderPath(trimmedPath);
    localStorage.setItem('currentPath', trimmedPath);
    if (trimmedPath === '') {
      projectCtx.setFiles(projectCtx.rootFiles);
      // projectCtx.setExpandedFileTreeNodes([]);
      localStorage.setItem('currentFiles', JSON.stringify(projectCtx.rootFiles));
      // localStorage.setItem('expandedNodes', JSON.stringify([]));
      return;
    }
    // const parents = await getParents(trimmedPath);
    const parentFiles = await getParentFiles(trimmedPath);
    projectCtx.setFiles(parentFiles);
    // projectCtx.setExpandedFileTreeNodes(parents);
    localStorage.setItem('currentFiles', JSON.stringify(parentFiles));
    // localStorage.setItem('expandedNodes', JSON.stringify(parents));
  };

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      const children = await getChildFiles(file.id as string);
      // const parents = await getParents(file.path as string);
      projectCtx.setFiles(children);
      projectCtx.setFolderPath(file.path as string);
      // projectCtx.setExpandedFileTreeNodes(parents);
      localStorage.setItem('currentFiles', JSON.stringify(children));
      localStorage.setItem('currentPath', file.path as string);
      // localStorage.setItem('expandedNodes', JSON.stringify(parents));
    } else {
      // const parents = await getParents(projectCtx.folderPath);
      const fileContent = await getFileContent(file.id as string);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(file);
      projectCtx.setSelectedFile(file.id as string);
      // projectCtx.setExpandedFileTreeNodes(parents);
      localStorage.setItem('currentFileContent', fileContent);
      localStorage.setItem('currentFileInfo', JSON.stringify(file));
      localStorage.setItem('currentSelectedFile', file.id as string);
      // localStorage.setItem('expandedNodes', JSON.stringify(parents));
    }
  };

  if (!projectCtx.projectLoadComplete) {
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
  }

  return treeView ? (
    <FileTree treeView={treeView} />
  ) : (
    <>
      {projectCtx.folderPath === '' ? (
        ''
      ) : (
        <>
          <Foldername>
            {projectCtx.folderPath === '/' ? '/' : '../' + projectCtx.folderPath.split('/').reverse()[0]}
          </Foldername>
          <FolderUp onClick={() => navigateBack()}>
            <DriveFolderUpload />
            <div>{'..'}</div>
          </FolderUp>
        </>
      )}
      <Container>
        {projectCtx.files.map((file, idx) => {
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
