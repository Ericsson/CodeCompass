import { Button } from '@mui/material';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { removeStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';
import * as SC from 'themes/index-styles';
import { RouterQueryType } from 'utils/types';

const Home = (): JSX.Element => {
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const loadWorkspace = (workspace: string) => {
    removeStore([
      'storedSearchProps',
      'storedSelectedSearchResult',
      'storedExpandedFileTreeNodes',
      'storedExpandedSearchFileNodes',
      'storedExpandedSearchPathNodes',
    ]);
    router.push({
      pathname: '/project',
      query: {
        workspaceId: workspace,
      } as RouterQueryType,
    });
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
