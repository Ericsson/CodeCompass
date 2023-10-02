import { styled } from '@mui/material';

export const OuterContainer = styled('div')({
  display: 'grid',
  gridTemplateColumns: '400px 1fr',
});

export const GitBlameContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  overflowY: 'scroll',
  height: '100%',
  maxHeight: 'calc(100vh - 78px - 48px - 49px)',
  paddingTop: '4px',
  borderRight: `1px solid ${theme.colors?.primary}`,
}));

export const GitBlameLine = styled('div')({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  flexGrow: '1',
  gap: '5px',
  padding: '0 5px',
  height: '17.91px',
  fontSize: '0.75rem',
});
