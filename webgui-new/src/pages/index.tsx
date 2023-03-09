import { Button, styled } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { useRouter } from 'next/router';
import { useContext } from 'react';
import { WorkspaceContext } from 'global-context/workspace-context';

const OuterContainer = styled('div')({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  marginTop: '150px',
});

const InnerContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center',
  alignItems: 'center',
});

const Title = styled('div')({
  fontSize: '1.2rem',
  marginBottom: '10px',
});

const Home = () => {
  const router = useRouter();
  const workspaces = useContext(WorkspaceContext);
  const projectCtx = useContext(ProjectContext);

  const loadWorkspace = (workspace: string) => {
    localStorage.removeItem('currentFiles');
    localStorage.removeItem('currentPath');
    localStorage.removeItem('currentFileContent');
    localStorage.removeItem('currentFileInfo');
    localStorage.removeItem('currentSelectedFile');
    localStorage.removeItem('currentFileTree');
    localStorage.removeItem('currentRootFiles');
    localStorage.removeItem('expandedNodes');
    localStorage.removeItem('expandedPathNodes');
    localStorage.removeItem('expandedFileNodes');
    localStorage.removeItem('searchResults');
    localStorage.removeItem('currentSearchPage');
    localStorage.removeItem('currentSearchSize');
    localStorage.removeItem('currentSearchOption');
    localStorage.removeItem('currentSearchQuery');
    localStorage.removeItem('currentSearchFileFilterQuery');
    localStorage.removeItem('currentSearchDirFilterQuery');
    localStorage.removeItem('isFileSearch');
    localStorage.setItem('currentWorkspace', workspace);
    projectCtx.setCurrentWorkspace(workspace);
    router.push('/project');
  };

  return (
    <OuterContainer>
      <InnerContainer>
        <Title>{'Click on a project to start using CodeCompass!'}</Title>
        {workspaces.map((ws) => (
          <Button sx={{ textTransform: 'none' }} key={ws.id} onClick={() => loadWorkspace(ws.id as string)}>
            {ws.id}
          </Button>
        ))}
      </InnerContainer>
    </OuterContainer>
  );
};

export default Home;
