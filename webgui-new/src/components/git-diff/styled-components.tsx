import { TreeView, TreeItem, treeItemClasses } from '@mui/x-tree-view';
import { alpha, styled } from '@mui/material';

export const StyledSpan = styled('span')({});

export const CommitSummary = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  fontSize: '0.85rem',
  height: '87px',
  overflow: 'scroll',

  '& > div:nth-of-type(1)': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
    padding: '10px',
    fontSize: '1.1rem',
    fontWeight: 'bold',
  },

  '& > div:nth-of-type(2)': {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: '10px',
  },
}));

export const FileNameContainer = styled('div')({
  padding: '10px',
  fontSize: '0.85rem',
});

export const ChangeLine = styled('div')(({ theme }) => ({
  padding: '10px',
  color: theme.colors?.primary,
  backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.2),
}));

export const DiffViewOptions = styled('div')({
  display: 'flex',
  alignItems: 'center',
  height: '60px',
  padding: '10px',
});

export const DiffViewContainer = styled('div')({
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 87px - 60px)',
  overflow: 'scroll',
});

export const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  padding: '5px',
  fontSize: '0.85rem',
}));

export const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  [`& .${treeItemClasses.group}`]: {
    marginLeft: '10px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

export const Placeholder = styled('div')({
  padding: '10px',
});
