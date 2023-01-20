import { ChevronRight, ExpandMore } from '@mui/icons-material';
import { TreeItem, TreeView } from '@mui/lab';
import { styled } from '@mui/material';

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
}));

const project = {
  'package.json': 'package.json',
  'eslint.json': 'eslint.json',
  src: {
    pages: {
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
          <TreeItem key={nodeId} nodeId={`${nodeId++}`} label={key}>
            {getFileTree(obj[key])}
          </TreeItem>
        );
      } else {
        tree.push(<TreeItem key={nodeId} nodeId={`${nodeId++}`} label={obj[key]} />);
      }
    }
    return tree;
  };

  return (
    <StyledTreeView defaultCollapseIcon={<ExpandMore />} defaultExpandIcon={<ChevronRight />}>
      {getFileTree(project).map((elem) => elem)}
    </StyledTreeView>
  );
};
