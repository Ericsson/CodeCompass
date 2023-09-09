import { Button } from '@mui/material';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { removeStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';
import * as SC from 'themes/index-styles';
import { RouterQueryType } from 'utils/types';
import { useTranslation } from 'react-i18next';

const Home = (): JSX.Element => {
  const { t } = useTranslation();
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
        <SC.Title>{t('index.title')}</SC.Title>
        {appCtx.workspaces.length
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
