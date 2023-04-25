import { SyntheticEvent, useContext } from 'react';
import { Header } from 'components/header/header';
import { AccordionMenu } from 'components/accordion-menu/accordion-menu';
import { Box, CircularProgress, styled, Tab, Tabs } from '@mui/material';
import { TabName } from 'enums/tab-enum';
import { Diagrams } from 'components/diagrams/diagrams';
import { Metrics } from 'components/metrics/metrics';
import { GitDiff } from 'components/git-diff/git-diff';
import { CodeMirrorEditor } from 'components/codemirror-editor/codemirror-editor';
import { AppContext } from 'global-context/app-context';
import { Credits } from 'components/credits/credits';
import { Welcome } from 'components/welcome/welcome';
import { UserGuide } from 'components/user-guide/user-guide';

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

const OuterContainer = styled('div')({
  display: 'grid',
  gridTemplateColumns: '1fr',
  gridTemplateRows: '76px 1fr',
  height: '100vh',
});

const InnerContainer = styled('div')({
  display: 'grid',
  gridTemplateColumns: '280px 1fr',
  gridTemplateRows: '1fr',
});

const StyledTabs = styled(Tabs)(({ theme }) => ({
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

const StyledTab = styled(Tab)({
  textTransform: 'none',
});

const TabPanel = (props: TabPanelProps) => {
  const { children, value, index, ...other } = props;

  return (
    <div role={'tabpanel'} hidden={value !== index} {...other}>
      {value === index && <>{children}</>}
    </div>
  );
};

const Project = () => {
  const appCtx = useContext(AppContext);

  return appCtx.loadComplete ? (
    <OuterContainer>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <div>
          <StyledTabs
            value={appCtx.activeTab}
            onChange={(_e: SyntheticEvent, newValue: number) => appCtx.setActiveTab(newValue)}
          >
            <StyledTab label={'Welcome'} />
            <StyledTab label={'Code'} />
            <StyledTab label={'Metrics'} />
            <StyledTab label={'Diagrams'} />
            <StyledTab label={'Git diff'} />
            <StyledTab label={'User guide'} />
            <StyledTab label={'Credits'} />
          </StyledTabs>
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
      </InnerContainer>
    </OuterContainer>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
      <CircularProgress />
    </Box>
  );
};

export default Project;
