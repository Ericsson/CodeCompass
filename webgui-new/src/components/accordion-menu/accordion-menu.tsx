import { styled } from '@mui/material/styles';
import { ArrowForwardIosSharp, Folder, Search, Info, GitHub } from '@mui/icons-material';
import MuiAccordion, { AccordionProps } from '@mui/material/Accordion';
import MuiAccordionSummary, { AccordionSummaryProps } from '@mui/material/AccordionSummary';
import MuiAccordionDetails from '@mui/material/AccordionDetails';
import Typography from '@mui/material/Typography';
import { useContext } from 'react';
import { FileManager } from 'components/file-manager/file-manager';
import { Checkbox, FormControlLabel } from '@mui/material';
import { AccordionLabel } from 'enums/accordion-enum';
import { SearchResults } from 'components/search-results/search-results';
import { InfoTree } from 'components/info-tree/info-tree';
import { RevisionControl } from 'components/revision-control/revision-control';
import { AppContext } from 'global-context/app-context';

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
  const appCtx = useContext(AppContext);

  return (
    <Container>
      <Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.FILE_MANAGER}
        onChange={() => appCtx.setActiveAccordion(AccordionLabel.FILE_MANAGER)}
      >
        <AccordionSummary>
          <IconLabel>
            <Folder />
            <FileManagerHeader>
              <Typography>{AccordionLabel.FILE_MANAGER}</Typography>
              <TreeSetting
                control={
                  <Checkbox
                    checked={appCtx.treeViewOption}
                    onChange={() => appCtx.setTreeViewOption(!appCtx.treeViewOption)}
                    sx={{ padding: '0' }}
                  />
                }
                label={'Tree'}
              />
            </FileManagerHeader>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>
          <FileManager />
        </AccordionDetails>
      </Accordion>
      <Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.SEARCH_RESULTS}
        onChange={() => appCtx.setActiveAccordion(AccordionLabel.SEARCH_RESULTS)}
      >
        <AccordionSummary>
          <IconLabel>
            <Search />
            <Typography>{AccordionLabel.SEARCH_RESULTS}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails sx={{ height: 'unset', overflow: 'hidden' }}>
          <SearchResults />
        </AccordionDetails>
      </Accordion>
      <Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.INFO_TREE}
        onChange={() => appCtx.setActiveAccordion(AccordionLabel.INFO_TREE)}
      >
        <AccordionSummary>
          <IconLabel>
            <Info />
            <Typography>{AccordionLabel.INFO_TREE}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>
          <InfoTree />
        </AccordionDetails>
      </Accordion>
      <Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.REVISION_CONTROL_NAVIGATOR}
        onChange={() => appCtx.setActiveAccordion(AccordionLabel.REVISION_CONTROL_NAVIGATOR)}
      >
        <AccordionSummary>
          <IconLabel>
            <GitHub />
            <Typography>{AccordionLabel.REVISION_CONTROL_NAVIGATOR}</Typography>
          </IconLabel>
        </AccordionSummary>
        <AccordionDetails>
          <RevisionControl />
        </AccordionDetails>
      </Accordion>
    </Container>
  );
};
