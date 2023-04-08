import { Button, styled } from '@mui/material';
import { useRouter } from 'next/router';
import { useContext } from 'react';
import { removeStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';

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
  const appCtx = useContext(AppContext);

  const loadWorkspace = (workspace: string) => {
    removeStore([
      'storedProjectFileId',
      'storedDiagramGenId',
      'storedMetricsGenId',
      'storedLanguageNodeId',
      'storedFiles',
      'storedFileTree',
      'storedFolderPath',
      'storedExpandedFileTreeNodes',
      'storedGitRepoId',
      'storedGitCommitId',
      'storedGitBranch',
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
    appCtx.setWorkspaceId(workspace);
  };

  return (
    <OuterContainer>
      <InnerContainer>
        <Title>{'Click on a project to start using CodeCompass!'}</Title>
        {appCtx.workspaces
          ? appCtx.workspaces.map((ws) => (
              <Button sx={{ textTransform: 'none' }} key={ws.id} onClick={() => loadWorkspace(ws.id as string)}>
                {ws.id}
              </Button>
            ))
          : ''}
      </InnerContainer>
    </OuterContainer>
  );
};

export default Home;
