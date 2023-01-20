import { Folder, FolderOpen } from '@mui/icons-material';
import { TreeItem, TreeView, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { FileIcon } from '../file-icon/file-icon';

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
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

export const FileTree = (): JSX.Element => {
  let nodeId = 1;

  const getFileTree = (obj: any): JSX.Element[] => {
    const tree: JSX.Element[] = [];
    for (let key in obj) {
      if (typeof obj[key] === 'object') {
        tree.push(
          <StyledTreeItem key={nodeId} nodeId={`${nodeId++}`} label={key}>
            {getFileTree(obj[key])}
          </StyledTreeItem>
        );
      } else {
        tree.push(
          <StyledTreeItem
            key={nodeId}
            nodeId={`${nodeId++}`}
            label={
              <IconLabel>
                <FileIcon fileName={obj[key]} />
                <div>{obj[key]}</div>
              </IconLabel>
            }
          />
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
