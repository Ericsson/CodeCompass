import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { AppContext } from 'global-context/app-context';
import { useRouter } from 'next/router';
import React, { useContext } from 'react';
import { removeStore } from 'utils/store';

export const ProjectSelect = () => {
  const router = useRouter();

  const appCtx = useContext(AppContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
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
      pathname: router.pathname,
      query: {},
    });
    appCtx.setWorkspaceId(e.target.value);
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
