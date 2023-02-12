import { styled } from '@mui/material/styles';
import { ArrowForwardIosSharp, Folder, Search, Info, GitHub, Construction } from '@mui/icons-material';
import MuiAccordion, { AccordionProps } from '@mui/material/Accordion';
import MuiAccordionSummary, { AccordionSummaryProps } from '@mui/material/AccordionSummary';
import MuiAccordionDetails from '@mui/material/AccordionDetails';
import Typography from '@mui/material/Typography';
import { useEffect, useState } from 'react';
import { FileTree } from '../file-tree/file-tree';
import { Checkbox, FormControlLabel } from '@mui/material';

const Container = styled('div')({
  minWidth: '280px',
});

const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
});

const FileManagerHeader = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  gap: '0.5rem',
});

const TreeSetting = styled(FormControlLabel)(({ theme }) => ({
  marginLeft: '5px',
}));

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
  padding: '0',
  height: 'calc(100vh - 81px - 4 * 48px)',
  overflowY: 'scroll',
}));

export const AccordionMenu = () => {
  const [expanded, setExpanded] = useState<string>('panel1');
  const [treeView, setTreeView] = useState<boolean>(false);

  useEffect(() => {
    const storedTreeViewSetting = localStorage.getItem('treeView');
    if (storedTreeViewSetting) {
      setTreeView(JSON.parse(storedTreeViewSetting));
    }
  }, []);

  const placeholder = (
    <IconLabel sx={{ padding: '20px' }}>
      <Construction />
      <div>{'Under construction'}</div>
    </IconLabel>
  );

  return (
    <Container>
      <Accordion expanded={expanded === 'panel1'} onChange={() => setExpanded('panel1')}>
        <AccordionSummary aria-controls={'panel1-controls'} id={'panel1-header'}>
          <IconLabel>
            <Folder />
            <FileManagerHeader>
              <Typography>{'File manager'}</Typography>
              <TreeSetting
                control={
                  <Checkbox
                    checked={treeView}
                    onChange={() => {
                      localStorage.setItem('treeView', JSON.stringify(!treeView));
                      setTreeView(!treeView);
                    }}
                    sx={{ padding: '0' }}
                  />
                }
                label={'Tree'}
              />
            </FileManagerHeader>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>
          <FileTree treeView={treeView} />
        </AccordionDetails>
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
