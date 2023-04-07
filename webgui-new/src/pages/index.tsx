import { Button, styled } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { useRouter } from 'next/router';
import { useContext } from 'react';
import { WorkspaceContext } from 'global-context/workspace-context';
import { removeStore } from 'utils/store';

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
    removeStore([
      'storedFiles',
      'storedFolderPath',
      'storedFileContent',
      'storedFileInfo',
      'storedSelectedFile',
      'storedFileTree',
      'storedExpandedFileTreeNodes',
      'storedSearchResults',
      'storedSearchResultCount',
      'storedSearchStart',
      'storedSearchSize',
      'storedSearchOption',
      'storedSearchQuery',
      'storedSearchFileFilterQuery',
      'storedSearchDirFilterQuery',
      'storedIsFileSearch',
      'storedSelectedSearchResult',
      'storedExpandedSearchFileNodes',
      'storedExpandedSearchPathNodes',
    ]);
    router.replace({
      pathname: '/project',
      query: {},
    });
    projectCtx.setCurrentWorkspace(workspace);
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
