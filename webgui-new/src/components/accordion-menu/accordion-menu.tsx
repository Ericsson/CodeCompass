import { styled } from '@mui/material/styles';
import { ArrowForwardIosSharp, Folder, Search, Info, GitHub, Construction } from '@mui/icons-material';
import MuiAccordion, { AccordionProps } from '@mui/material/Accordion';
import MuiAccordionSummary, { AccordionSummaryProps } from '@mui/material/AccordionSummary';
import MuiAccordionDetails from '@mui/material/AccordionDetails';
import Typography from '@mui/material/Typography';
import { useContext, useEffect, useState } from 'react';
import { FileManager } from 'components/file-manager/file-manager';
import { Checkbox, FormControlLabel } from '@mui/material';
import { AccordionLabel } from 'enums/accordion-enum';
import { ConfigContext } from 'global-context/config-context';

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
  const configCtx = useContext(ConfigContext);
  const [treeView, setTreeView] = useState<boolean>(false);

  useEffect(() => {
    const storedTreeViewSetting = localStorage.getItem('treeView');
    if (storedTreeViewSetting) {
      setTreeView(JSON.parse(storedTreeViewSetting));
    }
  }, []);

  const setAccordion = (value: string) => {
    configCtx.setActiveAccordion(value);
    localStorage.setItem('activeAccordion', value);
  };

  const placeholder = (
    <IconLabel sx={{ padding: '20px' }}>
      <Construction />
      <div>{'Under construction'}</div>
    </IconLabel>
  );

  return (
    <Container>
      <Accordion
        expanded={configCtx.activeAccordion === AccordionLabel.FILE_MANAGER}
        onChange={() => setAccordion(AccordionLabel.FILE_MANAGER)}
      >
        <AccordionSummary>
          <IconLabel>
            <Folder />
            <FileManagerHeader>
              <Typography>{AccordionLabel.FILE_MANAGER}</Typography>
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
          <FileManager treeView={treeView} />
        </AccordionDetails>
      </Accordion>
      <Accordion
        expanded={configCtx.activeAccordion === AccordionLabel.SEARCH_RESULTS}
        onChange={() => setAccordion(AccordionLabel.SEARCH_RESULTS)}
      >
        <AccordionSummary>
          <IconLabel>
            <Search />
            <Typography>{AccordionLabel.SEARCH_RESULTS}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
      <Accordion
        expanded={configCtx.activeAccordion === AccordionLabel.INFO_TREE}
        onChange={() => setAccordion(AccordionLabel.INFO_TREE)}
      >
        <AccordionSummary>
          <IconLabel>
            <Info />
            <Typography>{AccordionLabel.INFO_TREE}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
      <Accordion
        expanded={configCtx.activeAccordion === AccordionLabel.REVISION_CONTROL_NAVIGATOR}
        onChange={() => setAccordion(AccordionLabel.REVISION_CONTROL_NAVIGATOR)}
      >
        <AccordionSummary>
          <IconLabel>
            <GitHub />
            <Typography>{AccordionLabel.REVISION_CONTROL_NAVIGATOR}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>{placeholder}</AccordionDetails>
      </Accordion>
    </Container>
  );
};
