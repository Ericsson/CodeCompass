import { styled } from '@mui/material/styles';
import { ArrowForwardIosSharp, Folder, Search, Info, GitHub } from '@mui/icons-material';
import MuiAccordion, { AccordionProps } from '@mui/material/Accordion';
import MuiAccordionSummary, { AccordionSummaryProps } from '@mui/material/AccordionSummary';
import MuiAccordionDetails from '@mui/material/AccordionDetails';
import Typography from '@mui/material/Typography';
import { useState } from 'react';

const Container = styled('div')({
  minWidth: '280px',
});

const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
});

const Accordion = styled((props: AccordionProps) => <MuiAccordion disableGutters elevation={0} square {...props} />)(
  ({ theme }) => ({
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
  })
);

const AccordionSummary = styled((props: AccordionSummaryProps) => (
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

const AccordionDetails = styled(MuiAccordionDetails)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  borderTop: `1px solid ${theme.colors?.primary}`,
  padding: theme.spacing(2),
  height: 'calc(100vh - 81px - 4 * 48px)',
  overflowY: 'scroll',
  '&::-webkit-scrollbar': {
    width: '5px',
  },
  '&::-webkit-scrollbar-track': {
    boxShadow: `inset 0 0 5px ${theme.backgroundColors?.primary}`,
    borderRadius: '10px',
  },
  '&::-webkit-scrollbar-thumb': {
    background: theme.colors?.primary,
    borderRadius: '5px',
  },
}));

export const AccordionMenu = () => {
  const [expanded, setExpanded] = useState<string>('panel1');

  const placeholder = (
    <Typography>
      Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse malesuada lacus ex, sit amet blandit leo
      lobortis eget. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse malesuada lacus ex, sit amet
      blandit leo lobortis eget.
    </Typography>
  );

  return (
    <Container>
      <Accordion expanded={expanded === 'panel1'} onChange={() => setExpanded('panel1')}>
        <AccordionSummary aria-controls={'panel1-controls'} id={'panel1-header'}>
          <IconLabel>
            <Folder />
            <Typography>{'File manager'}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
      <Accordion expanded={expanded === 'panel2'} onChange={() => setExpanded('panel2')}>
        <AccordionSummary aria-controls={'panel2-controls'} id={'panel2-header'}>
          <IconLabel>
            <Search />
            <Typography>{'Search results'}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
      <Accordion expanded={expanded === 'panel3'} onChange={() => setExpanded('panel3')}>
        <AccordionSummary aria-controls={'panel3-controls'} id={'panel3-header'}>
          <IconLabel>
            <Info />
            <Typography>{'Info tree'}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
      <Accordion expanded={expanded === 'panel4'} onChange={() => setExpanded('panel4')}>
        <AccordionSummary aria-controls={'panel4-controls'} id={'panel4-header'}>
          <IconLabel>
            <GitHub />
            <Typography>{'Revision control navigator'}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
    </Container>
  );
};
