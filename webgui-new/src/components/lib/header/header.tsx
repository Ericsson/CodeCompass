import { styled } from '@mui/material';
import { ProjectSelect } from '../project-select/project-select';
import { useRouter } from 'next/router';

const StyledHeader = styled('header')({
  padding: '10px',
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
});

export const Header = (): JSX.Element => {
  const router = useRouter();
  return (
    <StyledHeader>
      <div>{'CodeCompass'}</div>
      <ProjectSelect currentProject={router.query.id as string} projects={['CodeCompass', 'cJSON']} />
    </StyledHeader>
  );
};
