import { styled } from '@mui/material';

export const OuterContainer = styled('div')({
  padding: '10px',
});

export const ProjectMemberContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '3rem',
  marginBottom: '5px',

  '& > div:nth-of-type(1)': {
    width: '200px',
  },
});

export const StyledHeading2 = styled('h2')({
  fontSize: '1.1rem',
  marginBottom: '10px',
});
