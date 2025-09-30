import { TreeView, TreeItem, treeItemClasses } from '@mui/x-tree-view';
import { alpha, styled } from '@mui/material';

export const StyledDiv = styled('div')({});

export const IconLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '5px',
  paddingLeft: '15px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

export const FileLine = styled('div')({
  fontSize: '0.85rem',
  marginTop: '3px',
});

export const PaginationContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  gap: '15px',
  margin: '10px',
  height: '50px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

export const ResultsContainer = styled('div')({
  height: 'calc(100vh - 81px - 70px - 4 * 48px)',
  overflow: 'scroll',
});

export const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
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
