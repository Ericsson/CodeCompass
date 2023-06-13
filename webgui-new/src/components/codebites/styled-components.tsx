import { styled } from '@mui/material';

export const DiagramOuterContainer = styled('div')({
  padding: '10px',
  overflow: 'scroll',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px)',
});

export const NodeOuterContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  width: '500px',
  height: '349px',
  border: `1px solid ${theme.colors?.primary}`,
  cursor: 'pointer',
}));

export const NodeHeader = styled('div')(({ theme }) => ({
  width: '100%',
  height: '30px',
  overflow: 'scroll',
  padding: '5px',
  fontSize: '0.85rem',
  textAlign: 'center',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));
