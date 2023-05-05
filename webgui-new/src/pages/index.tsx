import { Button } from '@mui/material';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { removeStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';
import * as SC from 'themes/index-styles';

const Home = () => {
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const loadWorkspace = (workspace: string) => {
    removeStore([
      'storedActiveAccordion',
      'storedActiveTab',
      'storedProjectFileId',
      'storedSearchProps',
      'storedDiagramGenId',
      'storedDiagramTypeId',
      'storedMetricsGenId',
      'storedLanguageNodeId',
      'storedFiles',
      'storedFileTree',
      'storedFolderPath',
      'storedEditorSelection',
      'storedSelectedSearchResult',
      'storedExpandedFileTreeNodes',
      'storedExpandedSearchFileNodes',
      'storedExpandedSearchPathNodes',
      'storedGitRepoId',
      'storedGitCommitId',
      'storedGitBranch',
    ]);
    router.replace({
      pathname: '/project',
      query: {},
    });
    appCtx.setWorkspaceId(workspace);
  };

  return (
    <SC.OuterContainer>
      <SC.InnerContainer>
        <SC.Title>{'Click on a project to start using CodeCompass!'}</SC.Title>
        {appCtx.workspaces
          ? appCtx.workspaces.map((ws) => (
              <Button key={ws.id} onClick={() => loadWorkspace(ws.id as string)}>
                {ws.id}
              </Button>
            ))
          : ''}
      </SC.InnerContainer>
    </SC.OuterContainer>
  );
};

export default Home;
