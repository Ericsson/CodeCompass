import { Menu, styled } from '@mui/material';

export const Container = styled('div')(({ theme }) => ({
  padding: '10px',
  border: `1px solid ${theme.colors?.primary}`,
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  borderRadius: '5px',
}));

export const StyledMenu = styled(Menu)({
  '& ul': {
    padding: '0',
  },
});

export const RadioWithInfo = styled('div')({
  display: 'flex',
  alignItems: 'center',
});

export const ExpressionSearchSettings = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  gap: '2rem',
});

export const OtherLanguagesContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  marginTop: '10px',
  paddingTop: '20px',
  borderTop: `1px solid ${theme.colors?.primary}`,
}));
