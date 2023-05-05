import React, { SyntheticEvent, useContext } from 'react';
import { Header } from 'components/header/header';
import { AccordionMenu } from 'components/accordion-menu/accordion-menu';
import { Box, CircularProgress } from '@mui/material';
import { TabName } from 'enums/tab-enum';
import { Diagrams } from 'components/diagrams/diagrams';
import { Metrics } from 'components/metrics/metrics';
import { GitDiff } from 'components/git-diff/git-diff';
import { CodeMirrorEditor } from 'components/codemirror-editor/codemirror-editor';
import { AppContext } from 'global-context/app-context';
import { Credits } from 'components/credits/credits';
import { Welcome } from 'components/welcome/welcome';
import { UserGuide } from 'components/user-guide/user-guide';
import * as SC from 'themes/project-styles';
interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

const TabPanel = (props: TabPanelProps) => {
  const { children, value, index, ...other } = props;

  return (
    <div role={'tabpanel'} hidden={value !== index} {...other}>
      {value === index && <>{children}</>}
    </div>
  );
};

const Project = (): JSX.Element => {
  const appCtx = useContext(AppContext);

  return appCtx.loadComplete ? (
    <SC.OuterContainer>
      <Header />
      <SC.InnerContainer>
        <AccordionMenu />
        <div>
          <SC.StyledTabs
            value={appCtx.activeTab}
            onChange={(_e: SyntheticEvent, newValue: number) => appCtx.setActiveTab(newValue)}
          >
            <SC.StyledTab label={'Welcome'} />
            <SC.StyledTab label={'Code'} />
            <SC.StyledTab label={'Metrics'} />
            <SC.StyledTab label={'Diagrams'} />
            <SC.StyledTab label={'Git diff'} />
            <SC.StyledTab label={'User guide'} />
            <SC.StyledTab label={'Credits'} />
          </SC.StyledTabs>
          <TabPanel value={appCtx.activeTab} index={TabName.WELCOME}>
            <Welcome />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.CODE}>
            <CodeMirrorEditor />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.METRICS}>
            <Metrics />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.DIAGRAMS}>
            <Diagrams />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.GIT_DIFF}>
            <GitDiff />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.USER_GUIDE}>
            <UserGuide />
          </TabPanel>
          <TabPanel value={appCtx.activeTab} index={TabName.CREDITS}>
            <Credits />
          </TabPanel>
        </div>
      </SC.InnerContainer>
    </SC.OuterContainer>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
      <CircularProgress />
    </Box>
  );
};

export default Project;
