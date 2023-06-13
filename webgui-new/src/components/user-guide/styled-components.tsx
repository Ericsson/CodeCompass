import { styled } from '@mui/material';

export const OuterContainer = styled('div')({
  padding: '10px',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px)',
  overflow: 'scroll',
});

export const StyledParagraph = styled('p')({
  width: '60%',
  marginBottom: '5xp',
});

export const StyledUl = styled('ul')({
  listStyle: 'inside',
  marginLeft: '20px',
});

export const StyledHeading1 = styled('h1')({
  fontSize: '1.5rem',
  marginBottom: '20px',
});

export const StyledHeading2 = styled('h2')({
  fontSize: '1.2rem',
  margin: '10px 0',
});
