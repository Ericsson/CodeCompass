import { cpp } from '@codemirror/lang-cpp';
import { ThemeContext } from 'global-context/theme-context';
import ReactCodeMirror from '@uiw/react-codemirror';
import { githubLight, githubDark } from '@uiw/codemirror-theme-github';
import { SyntheticEvent, useContext } from 'react';
import { FileName } from 'components/file-name/file-name';
import { Header } from 'components/header/header';
import { AccordionMenu } from 'components/accordion-menu/accordion-menu';
import { Box, CircularProgress, styled, Tab, Tabs } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { Construction } from '@mui/icons-material';
import { TabName } from 'enums/tab-enum';
import { ConfigContext } from 'global-context/config-context';

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

const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
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
    <div role="tabpanel" hidden={value !== index} {...other}>
      {value === index && <>{children}</>}
    </div>
  );
};

const placeholder = (
  <IconLabel sx={{ padding: '20px' }}>
    <Construction />
    <div>{'Under construction'}</div>
  </IconLabel>
);

const Project = () => {
  const { theme } = useContext(ThemeContext);
  const configCtx = useContext(ConfigContext);
  const projectCtx = useContext(ProjectContext);

  return projectCtx.loadComplete ? (
    <OuterContainer>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <div>
          <StyledTabs
            value={configCtx.activeTab}
            onChange={(_e: SyntheticEvent, newValue: number) => configCtx.setActiveTab(newValue)}
          >
            <StyledTab label="Welcome" />
            <StyledTab label="Code" />
            <StyledTab label="Metrics" />
            <StyledTab label="Diagrams" />
            <StyledTab label="Git blame" />
            <StyledTab label="Git diff" />
            <StyledTab label="User guide" />
            <StyledTab label="Credits" />
          </StyledTabs>
          <TabPanel value={configCtx.activeTab} index={TabName.WELCOME}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.CODE}>
            <FileName
              fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
              filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
              parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
              info={projectCtx.fileInfo ?? undefined}
            />
            <ReactCodeMirror
              readOnly={true}
              extensions={[cpp()]}
              theme={theme === 'dark' ? githubDark : githubLight}
              value={projectCtx.fileContent ?? ''}
              width={'100%'}
              height={'100%'}
              minWidth={'calc(1460px - 280px)'}
              maxWidth={'calc(100vw - 280px)'}
              maxHeight={'calc(100vh - 78px - 48px - 49px)'}
              style={{ fontSize: '0.8rem' }}
            />
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.METRICS}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.DIAGRAMS}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.GIT_BLAME}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.GIT_DIFF}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.USER_GUIDE}>
            {placeholder}
          </TabPanel>
          <TabPanel value={configCtx.activeTab} index={TabName.CREDITS}>
            {placeholder}
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
