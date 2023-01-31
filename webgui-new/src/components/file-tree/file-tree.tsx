import { Folder, FolderOpen, DriveFolderUpload } from '@mui/icons-material';
import { TreeItem, TreeView, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { useState, useEffect, useContext } from 'react';
import { FileInfo } from '../../../build/project/cc/service/core';
import { ProjectContext } from '../../global-context/project-context';
import { getRootFiles, getChildFiles, getFileContent } from '../../service/project-service';
import { FileIcon } from '../file-icon/file-icon';

const Container = styled('div')({
  padding: '10px 20px',
});

const Foldername = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px 20px',
}));

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  padding: '10px 20px',
}));

const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  [`& .${treeItemClasses.group}`]: {
    marginLeft: 15,
    paddingLeft: 18,
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

const IconLabel = styled('div')({
  display: 'flex',
  gap: '0.5rem',
  cursor: 'pointer',
});

const FolderUp = styled('div')({
  padding: '10px 20px 0 20px',
  display: 'flex',
  gap: '0.5rem',
  cursor: 'pointer',
});

const project = {
  'package.json': 'package.json',
  'eslint.json': 'eslint.json',
  src: {
    pages: {
      project: {
        '[id].tsx': '[id].tsx',
        search: {
          'results.tsx': 'results.tsx',
        },
      },
      '_app.tsx': '_app.tsx',
      'index.tsx': 'index.tsx',
    },
  },
  '.gitignore': '.gitignore',
};

export const FileTree = ({ treeView }: { treeView: boolean }): JSX.Element => {
  const { setFileContent, setFileInfo } = useContext(ProjectContext);

  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [currentFileId, setCurrentFileId] = useState<string | undefined>(undefined);
  const [parentFileId, setParentFileId] = useState<string | undefined>(undefined);
  const [folderPath, setFolderPath] = useState<string>('/');
  const [fileTree, setFileTree] = useState<JSX.Element[]>([]);

  let nodeId = 1;
  let keyId = 1;
  const renderFileTree = async (files: FileInfo[]): Promise<JSX.Element[]> => {
    const tree: JSX.Element[] = [];
    files.forEach(async (file) => {
      if (file.isDirectory) {
        const children = await getChildFiles(file.id as string);
        tree.push(
          <StyledTreeItem key={keyId++} nodeId={`${nodeId++}`} label={file.name}>
            {await renderFileTree(children)}
          </StyledTreeItem>
        );
      } else {
        tree.push(
          <IconLabel key={keyId++} onClick={() => handleTreeViewFileClick(file)}>
            <FileIcon fileName={file.name as string} />
            <div>{file.name}</div>
          </IconLabel>
        );
      }
    });
    return tree;
  };

  useEffect(() => {
    const getData = async () => {
      const rootFileData = await getRootFiles();
      setRootFiles(rootFileData);
      setFiles(rootFileData);
      setFileTree(await renderFileTree(await getRootFiles()));
      setParentFileId(rootFileData[0].id);
    };
    getData();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    if (!currentFileId) {
      return;
    }
    const getData = async () => {
      const children = await getChildFiles(currentFileId);
      setFiles(children);
    };
    getData();
  }, [currentFileId]);

  const navigateBack = async () => {};

  const handleFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      setCurrentFileId(file.id);
    } else {
      const fileContent = await getFileContent(file.id as string);
      setFileContent(fileContent);
      setFileInfo(file);
    }
  };

  const handleTreeViewFileClick = async (file: FileInfo) => {
    if (file.isDirectory) {
      return;
    } else {
      const fileContent = await getFileContent(file.id as string);
      setFileContent(fileContent);
      setFileInfo(file);
    }
  };

  return treeView ? (
    <StyledTreeView defaultCollapseIcon={<FolderOpen />} defaultExpandIcon={<Folder />}>
      {fileTree}
    </StyledTreeView>
  ) : (
    <>
      <Foldername>{folderPath}</Foldername>
      {files === rootFiles ? (
        ''
      ) : (
        <FolderUp onClick={() => navigateBack()}>
          <DriveFolderUpload />
          <div>{'..'}</div>
        </FolderUp>
      )}
      <Container>
        {files.map((file, idx) => {
          return (
            <IconLabel key={idx} onClick={() => handleFileClick(file)}>
              <FileIcon fileName={file.id as string} />
              <div>{file.name}</div>
            </IconLabel>
          );
        })}
      </Container>
    </>
  );
};
