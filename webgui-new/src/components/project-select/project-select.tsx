import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { AppContext } from 'global-context/app-context';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { removeStore } from 'utils/store';
import { RouterQueryType } from 'utils/types';

export const ProjectSelect = (): JSX.Element => {
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

  return appCtx.workspaces ? (
    <FormControl>
      <InputLabel>{'Project'}</InputLabel>
      <Select value={appCtx.workspaceId} label={'Project'} onChange={(e) => loadWorkspace(e)}>
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
