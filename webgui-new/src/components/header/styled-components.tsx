import { styled } from '@mui/material';
import Logo from '@images/logo.png';

export const StyledHeader = styled('header')(({ theme }) => ({
  display: 'grid',
  gridTemplateColumns: '280px 1fr',
  gridTemplateRows: '1fr',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  minWidth: '1460px',
}));

export const HeaderLogo = styled('div')(({ theme }) => ({
  height: '75px',
  backgroundImage: `url('${Logo.src}')`,
  backgroundRepeat: 'no-repeat',
  backgroundSize: '90%',
  backgroundPosition: 'center center',
  borderRight: `1px solid ${theme.colors?.primary}`,
}));

export const HeaderContent = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  flexGrow: '1',
  padding: '10px 10px 10px 15px',
});

export const SettingsContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});
