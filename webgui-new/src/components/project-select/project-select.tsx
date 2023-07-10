import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { AppContext } from 'global-context/app-context';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { removeStore } from 'utils/store';
import { RouterQueryType } from 'utils/types';

export const ProjectSelect = (): JSX.Element => {
  const { t } = useTranslation();
  const router = useRouter();

  const appCtx = useContext(AppContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
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
        workspaceId: e.target.value,
      } as RouterQueryType,
    });
  };

  return appCtx.workspaces.length ? (
    <FormControl>
      <InputLabel>{t('projectSelect')}</InputLabel>
      <Select value={appCtx.workspaceId} label={t('projectSelect')} onChange={(e) => loadWorkspace(e)}>
        {appCtx.workspaces.map((workspace) => (
          <MenuItem key={workspace.id} value={workspace.id}>
            {workspace.id}
          </MenuItem>
        ))}
      </Select>
    </FormControl>
  ) : (
    <></>
  );
};
