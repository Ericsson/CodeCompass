import { Select, MenuItem, Box, CircularProgress, FormControl, InputLabel, SelectChangeEvent } from '@mui/material';
import { useRouter } from 'next/router';

export const ProjectSelect = ({
  currentProject,
  projects,
}: {
  currentProject: string;
  projects: string[];
}): JSX.Element => {
  const router = useRouter();

  const loadWorkspace = (e: SelectChangeEvent<string>) => {
    localStorage.removeItem('currentFiles');
    localStorage.removeItem('currentPath');
    localStorage.removeItem('currentFileContent');
    localStorage.removeItem('currentFileInfo');
    localStorage.removeItem('currentSelectedFile');
    router.push(`/project/${e.target.value}`);
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
