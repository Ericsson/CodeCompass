import { Breadcrumbs, styled } from '@mui/material';

export const StyledDiv = styled('div')({});

export const StyledRect = styled('rect')({});

export const OuterContainer = styled('div')({
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px)',
  overflow: 'scroll',
});

export const MetricsOptionsContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
});

export const MetricsContainer = styled('div')({
  overflow: 'scroll',
  padding: '10px',
});

export const StyledBreadcrumbs = styled(Breadcrumbs)(({ theme }) => ({
  padding: '5px',
  margin: '10px',
  border: `1px solid ${theme.colors?.primary}`,
  borderRadius: '5px',
  width: 'max-content',
}));

export const Placeholder = styled('div')({
  padding: '10px',
});
