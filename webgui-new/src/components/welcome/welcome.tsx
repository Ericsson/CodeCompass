import { styled } from '@mui/material';

const OuterContainer = styled('div')({
  padding: '10px',
});

const StyledHeading1 = styled('h1')({
  fontSize: '1.5rem',
  marginBottom: '20px',
});

export const Welcome = (): JSX.Element => {
  return (
    <OuterContainer>
      <StyledHeading1>{'CodeCompass'}</StyledHeading1>
      <div>{'CodeCompass is a pluginable code comprehension tool which helps you to understand large code bases.'}</div>
    </OuterContainer>
  );
};
