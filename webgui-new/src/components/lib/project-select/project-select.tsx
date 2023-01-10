import { Select, MenuItem, Box, CircularProgress, FormControl, InputLabel } from '@mui/material';
import { useRouter } from 'next/router';

export const ProjectSelect = ({
  currentProject,
  projects,
}: {
  currentProject: string;
  projects: string[];
}): JSX.Element => {
  const router = useRouter();

  return currentProject ? (
    <FormControl>
      <InputLabel>{'Project'}</InputLabel>
      <Select value={currentProject} label={'Project'} onChange={(e) => router.push(`/project/${e.target.value}`)}>
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
