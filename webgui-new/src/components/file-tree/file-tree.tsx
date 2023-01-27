import { Folder, FolderOpen } from '@mui/icons-material';
import { TreeItem, TreeView, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
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

// Example response:
// {
//   1: { str: 'id' },
//   2: { str: 'file/dirname' },
//   3: { str: 'type (Dir | Unknown)' },
//   4: { str: 'filepath' }
// }

const placeHolderFiles = [
  {
    1: { str: '1' },
    2: { str: 'index.tsx' },
    3: { str: 'Unknown' },
    4: { str: '/src/index.ts' },
  },
  {
    1: { str: '2' },
    2: { str: '_app.tsx' },
    3: { str: 'Unknown' },
    4: { str: '/src/_app.tsx' },
  },
  {
    1: { str: '3' },
    2: { str: 'project.tsx' },
    3: { str: 'Unknown' },
    4: { str: '/src/project.tsx' },
  },
];

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
  if (!treeView) {
    return (
      <>
        <Foldername>{'/src'}</Foldername>
        <Container>
          {placeHolderFiles.map((file, idx) => {
            return (
              <IconLabel key={idx}>
                <FileIcon fileName={file[2].str} />
                <div>{file[2].str}</div>
              </IconLabel>
            );
          })}
        </Container>
      </>
    );
  }

  let nodeId = 1;
  let keyId = 1;
  const getFileTree = (obj: any): JSX.Element[] => {
    const tree: JSX.Element[] = [];
    for (let key in obj) {
      if (typeof obj[key] === 'object') {
        tree.push(
          <StyledTreeItem key={keyId++} nodeId={`${nodeId++}`} label={key}>
            {getFileTree(obj[key])}
          </StyledTreeItem>
        );
      } else {
        tree.push(
          <IconLabel key={keyId++}>
            <FileIcon fileName={obj[key]} />
            <div>{obj[key]}</div>
          </IconLabel>
        );
      }
    }
    return tree;
  };

  return (
    <StyledTreeView defaultCollapseIcon={<FolderOpen />} defaultExpandIcon={<Folder />}>
      {getFileTree(project).map((elem) => elem)}
    </StyledTreeView>
  );
};
