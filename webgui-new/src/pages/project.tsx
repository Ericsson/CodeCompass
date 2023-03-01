import { cpp } from '@codemirror/lang-cpp';
import { ThemeContext } from '../global-context/theme-context';
import ReactCodeMirror from '@uiw/react-codemirror';
import { githubLight, githubDark } from '@uiw/codemirror-theme-github';
import { useContext, useEffect, useState } from 'react';
import { FileName } from '../components/file-name/file-name';
import { Header } from '../components/header/header';
import { AccordionMenu } from '../components/accordion-menu/accordion-menu';
import { Box, CircularProgress, styled } from '@mui/material';
import { ProjectContext } from '../global-context/project-context';
import { createProjectClient } from '../service/project-service';
import { ProjectService } from '@thrift-generated/cc/service/core';

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

const Project = () => {
  const { theme } = useContext(ThemeContext);
  const projectCtx = useContext(ProjectContext);
  const [projectClient, setProjectClient] = useState<ProjectService.Client | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const projectClient = createProjectClient(projectCtx.currentWorkspace);
    setProjectClient(projectClient);
  }, [projectCtx.currentWorkspace]);

  return projectClient ? (
    <OuterContainer>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <div>
          <FileName
            fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
            filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
            parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
            info={projectCtx.projectLoadComplete ? projectCtx.fileInfo : undefined}
          />
          <ReactCodeMirror
            readOnly={true}
            extensions={[cpp()]}
            theme={theme === 'dark' ? githubDark : githubLight}
            value={projectCtx.projectLoadComplete ? projectCtx.fileContent : ''}
            width={'100%'}
            height={'100%'}
            minWidth={'calc(1460px - 280px)'}
            maxWidth={'calc(100vw - 280px)'}
            maxHeight={'calc(100vh - 78px - 49px)'}
            style={{ fontSize: '0.8rem' }}
          />
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
