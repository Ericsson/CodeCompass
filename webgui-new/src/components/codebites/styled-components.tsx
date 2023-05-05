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
  justifyContent: 'center',
  width: '500px',
  height: '349px',
  border: `1px solid ${theme.colors?.primary}`,
}));
