import { styled } from '@mui/material/styles';
import { ArrowForwardIosSharp } from '@mui/icons-material';
import MuiAccordion, { AccordionProps } from '@mui/material/Accordion';
import MuiAccordionSummary, { AccordionSummaryProps } from '@mui/material/AccordionSummary';
import MuiAccordionDetails from '@mui/material/AccordionDetails';
import React from 'react';
import { FormControlLabel } from '@mui/material';

export const Container = styled('div')({
  minWidth: '280px',
});

export const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
});

export const FileManagerHeader = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  gap: '0.5rem',
});

export const TreeSetting = styled(FormControlLabel)({
  marginLeft: '5px',
});

export const Accordion = styled((props: AccordionProps) => (
  <MuiAccordion disableGutters elevation={0} square {...props} />
))(({ theme }) => ({
  border: `1px solid ${theme.colors?.primary}`,
  '&:nth-of-type(1)': {
    borderTop: '0',
  },
  '&:not(:last-child)': {
    borderBottom: 0,
  },
  '&:before': {
    display: 'none',
  },
  '&:nth-of-type(1) > div': {
    borderTop: '0',
  },
  '&:last-child > div': {
    borderBottom: '0',
  },
}));

export const AccordionSummary = styled((props: AccordionSummaryProps) => (
  <MuiAccordionSummary expandIcon={<ArrowForwardIosSharp sx={{ fontSize: '0.9rem' }} />} {...props} />
))(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  flexDirection: 'row-reverse',
  '& .MuiAccordionSummary-expandIconWrapper.Mui-expanded': {
    transform: 'rotate(90deg)',
  },
  '& .MuiAccordionSummary-content': {
    marginLeft: theme.spacing(1),
  },
}));

export const AccordionDetails = styled(MuiAccordionDetails)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  borderTop: `1px solid ${theme.colors?.primary}`,
  padding: '0',
  height: 'calc(100vh - 81px - 4 * 48px)',
  overflowY: 'scroll',
}));
