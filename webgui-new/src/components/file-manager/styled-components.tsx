import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { FormControlLabel, alpha, styled } from '@mui/material';

export const StyledDiv = styled('div')({});

export const FolderName = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px 10px',
  fontSize: '0.85rem',
  width: '100%',
}));

export const FolderUp = styled('div')(({ theme }) => ({
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

export const FileLabel = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

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

export const TreeSetting = styled(FormControlLabel)(({ theme }) => ({
  display: 'flex',
  justifyContent: 'center',
  width: '100%',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));
