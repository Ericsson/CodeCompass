import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { Menu, alpha, styled } from '@mui/material';

export const StyledDiv = styled('div')({});

export const Container = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  gap: '0.6rem',
  padding: '0 15px',
  minHeight: '49px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  fontSize: '0.85rem',
}));

export const ParseStatus = styled('div')({
  borderRadius: '5px',
  padding: '5px',
});

export const StyledMenu = styled(Menu)(({ theme }) => ({
  '.MuiMenu-list': {
    border: `1px solid ${theme.colors?.primary}`,
    width: '500px',
    maxHeight: '500px',
    overflow: 'scroll',
  },
}));

export const BuildLogMenu = styled(Menu)(({ theme }) => ({
  '.MuiMenu-list': {
    border: `1px solid ${theme.colors?.primary}`,
    width: '300px',
    maxHeight: '500px',
    overflow: 'scroll',
    padding: '10px',
  },
}));

export const BuildLogHeader = styled('div')(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
  marginBottom: '5px',
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

export const Label = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  marginLeft: '5px',
  paddingLeft: '20px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));
