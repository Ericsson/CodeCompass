import { cpp } from '@codemirror/lang-cpp';
import { ThemeContext } from '../../themes/theme-context';
import ReactCodeMirror from '@uiw/react-codemirror';
import { githubLight, githubDark } from '@uiw/codemirror-theme-github';
import { useContext, useEffect } from 'react';
import { FileName } from '../../components/file-name/file-name';
import { Header } from '../../components/header/header';
import { AccordionMenu } from '../../components/accordion-menu/accordion-menu';
import { Box, CircularProgress, styled } from '@mui/material';
import { ProjectContext } from '../../global-context/project-context';
import { useRouter } from 'next/router';
import { WorkspaceContext } from 'global-context/workspace-context';

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
  const router = useRouter();
  const { theme } = useContext(ThemeContext);
  const projectCtx = useContext(ProjectContext);
  const workspaces = useContext(WorkspaceContext);

  useEffect(() => {
    if (
      router.query.id &&
      projectCtx.currentWorkspace !== '' &&
      (router.query.id as string) !== projectCtx.currentWorkspace
    ) {
      localStorage.removeItem('currentFiles');
      localStorage.removeItem('currentPath');
      localStorage.removeItem('currentFileContent');
      localStorage.removeItem('currentFileInfo');
      localStorage.removeItem('currentSelectedFile');
    }
    projectCtx.setCurrentWorkspace(router.query.id as string);
  }, [router.query.id, projectCtx]);

  return workspaces.map((ws) => ws.id).includes(router.query.id as string) ? (
    <OuterContainer>
      <Header workspaceName={router.query.id as string} />
      <InnerContainer>
        <AccordionMenu />
        <div>
          <FileName
            fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
            filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
            parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
            info={projectCtx.fileInfo}
          />
          <ReactCodeMirror
            readOnly={true}
            extensions={[cpp()]}
            theme={theme === 'dark' ? githubDark : githubLight}
            value={projectCtx.fileContent ?? ''}
            width={'100%'}
            height={'100%'}
            maxWidth={'calc(100vw - 280px)'}
            maxHeight={'calc(100vh - 78px - 49px)'}
            style={{ fontSize: '0.8rem' }}
          />
        </div>
      </InnerContainer>
    </OuterContainer>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
      <CircularProgress />
    </Box>
  );
};

export default Project;
