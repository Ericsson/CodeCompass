import { Box, styled } from '@mui/material';

export const AstNodeInfoHeader = styled('div')({
  fontWeight: 'bold',
});

export const ModalHeader = styled('div')({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  width: '100%',
});

export const ModalContent = styled('div')({
  padding: '10px',
  width: '100%',
  overflow: 'scroll',
});

export const ModalBox = styled(Box)({
  position: 'absolute',
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
});

export const ModalContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  width: '80vw',
  height: '80vh',
  backgroundColor: theme.backgroundColors?.primary,
}));
