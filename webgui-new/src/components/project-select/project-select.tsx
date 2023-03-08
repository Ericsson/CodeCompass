import { Select, MenuItem, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { WorkspaceContext } from 'global-context/workspace-context';
import { useContext } from 'react';

export const ProjectSelect = (): JSX.Element => {
  const workspaces = useContext(WorkspaceContext);
  const projectCtx = useContext(ProjectContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
    localStorage.removeItem('currentFiles');
    localStorage.removeItem('currentPath');
    localStorage.removeItem('currentFileContent');
    localStorage.removeItem('currentFileInfo');
    localStorage.removeItem('currentSelectedFile');
    localStorage.removeItem('currentFileTree');
    localStorage.removeItem('currentRootFiles');
    localStorage.removeItem('expandedNodes');
    localStorage.removeItem('expandedPathNodes');
    localStorage.removeItem('expandedFileNodes');
    localStorage.removeItem('searchResults');
    localStorage.removeItem('currentSearchPage');
    localStorage.removeItem('currentSearchSize');
    localStorage.removeItem('currentSearchOption');
    localStorage.removeItem('currentSearchQuery');
    localStorage.removeItem('isFileSearch');
    localStorage.setItem('currentWorkspace', e.target.value);
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
