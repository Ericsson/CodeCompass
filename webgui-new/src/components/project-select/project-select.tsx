import { Select, MenuItem, Box, CircularProgress, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { ProjectContext } from 'global-context/project-context';
import { useRouter } from 'next/router';
import { useContext } from 'react';

export const ProjectSelect = ({
  currentProject,
  projects,
}: {
  currentProject: string;
  projects: string[];
}): JSX.Element => {
  const router = useRouter();
  const projectCtx = useContext(ProjectContext);

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
    router.push(`/project/${e.target.value}`);
    projectCtx.setFileContent(undefined);
    projectCtx.setFileInfo(undefined);
  };

  return currentProject && projects ? (
    <FormControl>
      <InputLabel>{'Project'}</InputLabel>
      <Select value={currentProject} label={'Project'} onChange={(e) => loadWorkspace(e)}>
        {projects.map((project) => (
          <MenuItem key={project} value={project}>
            {project}
          </MenuItem>
        ))}
      </Select>
    </FormControl>
  ) : (
    <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
      <CircularProgress />
    </Box>
  );
};
