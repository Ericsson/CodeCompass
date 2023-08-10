import { Folder, Search, Info, GitHub } from '@mui/icons-material';
import Typography from '@mui/material/Typography';
import React, { useContext } from 'react';
import { FileManager } from 'components/file-manager/file-manager';
import { AccordionLabel } from 'enums/accordion-enum';
import { SearchResults } from 'components/search-results/search-results';
import { InfoTree } from 'components/info-tree/info-tree';
import { RevisionControl } from 'components/revision-control/revision-control';
import { AppContext } from 'global-context/app-context';
import * as SC from './styled-components';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';

export const AccordionMenu = (): JSX.Element => {
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const handleAccordionChange = (accordionLabel: string) => {
    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        activeAccordion: accordionLabel,
      } as RouterQueryType,
    });
  };

  return (
    <SC.Container>
      <SC.Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.FILE_MANAGER}
        onChange={() => handleAccordionChange(AccordionLabel.FILE_MANAGER)}
      >
        <SC.AccordionSummary>
          <SC.IconLabel>
            <Folder />
            <SC.FileManagerHeader>
              <Typography>{AccordionLabel.FILE_MANAGER}</Typography>
            </SC.FileManagerHeader>
          </SC.IconLabel>
        </SC.AccordionSummary>
        <SC.AccordionDetails>
          <FileManager />
        </SC.AccordionDetails>
      </SC.Accordion>
      <SC.Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.SEARCH_RESULTS}
        onChange={() => handleAccordionChange(AccordionLabel.SEARCH_RESULTS)}
      >
        <SC.AccordionSummary>
          <SC.IconLabel>
            <Search />
            <Typography>{AccordionLabel.SEARCH_RESULTS}</Typography>
          </SC.IconLabel>
        </SC.AccordionSummary>
        <SC.AccordionDetails sx={{ height: 'unset', overflow: 'hidden' }}>
          <SearchResults />
        </SC.AccordionDetails>
      </SC.Accordion>
      <SC.Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.INFO_TREE}
        onChange={() => handleAccordionChange(AccordionLabel.INFO_TREE)}
      >
        <SC.AccordionSummary>
          <SC.IconLabel>
            <Info />
            <Typography>{AccordionLabel.INFO_TREE}</Typography>
          </SC.IconLabel>
        </SC.AccordionSummary>
        <SC.AccordionDetails>
          <InfoTree />
        </SC.AccordionDetails>
      </SC.Accordion>
      <SC.Accordion
        expanded={appCtx.activeAccordion === AccordionLabel.REVISION_CONTROL_NAVIGATOR}
        onChange={() => handleAccordionChange(AccordionLabel.REVISION_CONTROL_NAVIGATOR)}
      >
        <SC.AccordionSummary>
          <SC.IconLabel>
            <GitHub />
            <Typography>{AccordionLabel.REVISION_CONTROL_NAVIGATOR}</Typography>
          </SC.IconLabel>
        </SC.AccordionSummary>
        <SC.AccordionDetails>
          <RevisionControl />
        </SC.AccordionDetails>
      </SC.Accordion>
    </SC.Container>
  );
};
