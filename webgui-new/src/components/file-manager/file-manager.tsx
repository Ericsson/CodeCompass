import { Folder, DriveFolderUpload } from '@mui/icons-material';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { useContext } from 'react';
import { FileInfo } from '@thrift-generated/cc/service/core';
import { ProjectContext } from 'global-context/project-context';
import { getChildFiles, getFileContent, getParentFiles, getParents } from 'service/project-service';
import { FileIcon } from 'components/file-icon/file-icon';
import { FileTree } from './file-tree';
import { OtherContext } from 'global-context/other-context';
import { TabName } from 'enums/tab-enum';

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
}));

const FileLabel = styled('div')({
  fontSize: '0.8rem',
});

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

export const FileManager = ({ treeView }: { treeView: boolean }): JSX.Element => {
  const otherCtx = useContext(OtherContext);
  const projectCtx = useContext(ProjectContext);

  const navigateBack = async () => {
    if (projectCtx.folderPath === '/') {
      projectCtx.setFolderPath('');
      projectCtx.setFiles(projectCtx.rootFiles);
      projectCtx.setExpandedFileTreeNodes([]);
      localStorage.setItem('currentPath', '');
      localStorage.setItem('currentFiles', JSON.stringify(projectCtx.rootFiles));
      localStorage.setItem('expandedNodes', JSON.stringify([]));
      return;
    }
    const pathAsArray = projectCtx.folderPath.split('/');
    pathAsArray.pop();
    if (pathAsArray.length === 1) {
      pathAsArray.push('');
    }
    const trimmedPath = pathAsArray.join('/');
    const parents = await getParents(trimmedPath);
    const parentFiles = await getParentFiles(trimmedPath);
    projectCtx.setFolderPath(trimmedPath);
    projectCtx.setFiles(parentFiles);
    projectCtx.setExpandedFileTreeNodes(parents);
    localStorage.setItem('currentPath', trimmedPath);
    localStorage.setItem('currentFiles', JSON.stringify(parentFiles));
    localStorage.setItem('expandedNodes', JSON.stringify(parents));
  };

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      const children = await getChildFiles(file.id as string);
      const parents = await getParents(file.path as string);
      projectCtx.setFiles(children);
      projectCtx.setFolderPath(file.path as string);
      projectCtx.setExpandedFileTreeNodes(parents);
      localStorage.setItem('currentFiles', JSON.stringify(children));
      localStorage.setItem('currentPath', file.path as string);
      localStorage.setItem('expandedNodes', JSON.stringify(parents));
    } else {
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
    <FileTree />
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
            <DriveFolderUpload sx={{ width: '20px', height: '20px' }} />
            <div>{'..'}</div>
          </FolderUp>
        </>
      )}
      <Container>
        {projectCtx.files.map((file, idx) => {
          return (
            <IconLabel
              key={idx}
              data-id={file.id}
              onClick={() => handleFileClick(file)}
              sx={{
                backgroundColor: (theme) =>
                  file.id === projectCtx.selectedFile ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
              }}
            >
              {file.isDirectory ? (
                <Folder sx={{ width: '20px', height: '20px' }} />
              ) : (
                <FileIcon fileName={file.name as string} />
              )}
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
