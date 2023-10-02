import React, { SyntheticEvent, useContext } from 'react';
import { Header } from 'components/header/header';
import { AccordionMenu } from 'components/accordion-menu/accordion-menu';
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
import { useRouter } from 'next/router';
import Link from 'next/link';
import { Box } from '@mui/material';
import { useTranslation } from 'react-i18next';
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
  const { t } = useTranslation();
  const router = useRouter();
  const appCtx = useContext(AppContext);

  return appCtx.workspaceId ? (
    <SC.OuterContainer>
      <Header />
      <SC.InnerContainer>
        <AccordionMenu />
        <div>
          <SC.StyledTabs
            value={parseInt(appCtx.activeTab)}
            onChange={(_e: SyntheticEvent, newValue: number) =>
              router.push({ pathname: '/project', query: { ...router.query, activeTab: newValue.toString() } })
            }
          >
            <SC.StyledTab label={t('tabMenu.welcome')} />
            <SC.StyledTab label={t('tabMenu.code')} />
            <SC.StyledTab label={t('tabMenu.metrics')} />
            <SC.StyledTab label={t('tabMenu.diagrams')} />
            <SC.StyledTab label={t('tabMenu.gitDiff')} />
            <SC.StyledTab label={t('tabMenu.userGuide')} />
            <SC.StyledTab label={t('tabMenu.credits')} />
          </SC.StyledTabs>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.WELCOME}>
            <Welcome />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.CODE}>
            <CodeMirrorEditor />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.METRICS}>
            <Metrics />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.DIAGRAMS}>
            <Diagrams />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.GIT_DIFF}>
            <GitDiff />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.USER_GUIDE}>
            <UserGuide />
          </TabPanel>
          <TabPanel value={parseInt(appCtx.activeTab)} index={TabName.CREDITS}>
            <Credits />
          </TabPanel>
        </div>
      </SC.InnerContainer>
    </SC.OuterContainer>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh' }}>
      <div style={{ textAlign: 'center', fontSize: '1.5rem' }}>
        <div style={{ marginBottom: '10px' }}>{t('project.notFound')}</div>
        <Link href="/">{t('project.backToHome')}</Link>
      </div>
    </Box>
  );
};

export default Project;
