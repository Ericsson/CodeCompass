import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { AppContext } from 'global-context/app-context';
import { useRouter } from 'next/router';
import { useContext } from 'react';
import { removeStore } from 'utils/store';

export const ProjectSelect = (): JSX.Element => {
  const router = useRouter();

  const appCtx = useContext(AppContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
    removeStore([
      'storedProjectFileId',
      'storedSearchProps',
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
      'storedFileSearch',
      'storedSelectedSearchResult',
      'storedExpandedSearchFileNodes',
      'storedExpandedSearchPathNodes',
    ]);
    router.replace({
      pathname: router.pathname,
      query: {},
    });
    appCtx.setWorkspaceId(e.target.value);
  };

  return (
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
  );
};
