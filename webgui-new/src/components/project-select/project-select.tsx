import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { WorkspaceContext } from 'global-context/workspace-context';
import { useContext } from 'react';
import { removeStore } from 'utils/store';

export const ProjectSelect = (): JSX.Element => {
  const workspaces = useContext(WorkspaceContext);
  const projectCtx = useContext(ProjectContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
    removeStore([
      'storedFiles',
      'storedFolderPath',
      'storedFileContent',
      'storedFileInfo',
      'storedSelectedFile',
      'storedFileTree',
      'storedExpandedFileTreeNodes',
      'storedSearchResults',
      'storedSearchStart',
      'storedSearchSize',
      'storedSearchOption',
      'storedSearchQuery',
      'storedSearchFileFilterQuery',
      'storedSearchDirFilterQuery',
      'storedIsFileSearch',
      'storedExpandedSearchFileNodes',
      'storedExpandedSearchPathNodes',
    ]);
    projectCtx.setCurrentWorkspace(e.target.value);
  };

  return (
    <FormControl>
      <InputLabel>{'Project'}</InputLabel>
      <Select value={projectCtx.currentWorkspace} label={'Project'} onChange={(e) => loadWorkspace(e)}>
        {workspaces.map((workspace) => (
          <MenuItem key={workspace.id} value={workspace.id}>
            {workspace.id}
          </MenuItem>
        ))}
      </Select>
    </FormControl>
  );
};
