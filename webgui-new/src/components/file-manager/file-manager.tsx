import { Folder, DriveFolderUpload } from '@mui/icons-material';
import { alpha, styled } from '@mui/material';
import { useContext, useState, MouseEvent } from 'react';
import { FileInfo } from '@thrift-generated';
import { ProjectContext } from 'global-context/project-context';
import { getChildFiles, getFileContent, getParentFiles, getParents } from 'service/project-service';
import { FileIcon } from 'components/file-icon/file-icon';
import { FileTree } from './file-tree';
import { ConfigContext } from 'global-context/config-context';
import { TabName } from 'enums/tab-enum';
import { FileContextMenu } from 'components/file-context-menu/file-context-menu';

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

export const FileManager = (): JSX.Element => {
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

  const navigateBack = async () => {
    if (projectCtx.folderPath === '/') {
      projectCtx.setFolderPath('');
      projectCtx.setFiles(projectCtx.rootFiles);
      projectCtx.setExpandedFileTreeNodes([]);
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
  };

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      const children = await getChildFiles(file.id as string);
      const parents = await getParents(file.path as string);
      projectCtx.setFiles(children);
      projectCtx.setFolderPath(file.path as string);
      projectCtx.setExpandedFileTreeNodes(parents);
    } else {
      const parents = await getParents(projectCtx.folderPath);
      const fileContent = await getFileContent(file.id as string);
      projectCtx.setFileContent(fileContent);
      projectCtx.setFileInfo(file);
      projectCtx.setSelectedFile(file.id as string);
      projectCtx.setExpandedFileTreeNodes(parents);
      configCtx.setActiveTab(TabName.CODE);
    }
  };

  return configCtx.treeViewOption ? (
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
              onContextMenu={(e) => handleContextMenu(e, file)}
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
        <FileContextMenu
          contextMenu={contextMenu}
          setContextMenu={setContextMenu}
          fileInfo={fileInfoForDiagram as FileInfo}
        />
      </Container>
    </>
  );
};
