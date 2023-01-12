import { Button, IconButton, InputAdornment, Menu, MenuItem, TextField, styled } from '@mui/material';
import { ProjectSelect } from '../project-select/project-select';
import { useRouter } from 'next/router';
import { Search, Settings, MoreVert, MoreHoriz } from '@mui/icons-material';
import Logo from '../../../../public/logo.png';
import Image from 'next/image';
import { useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from '../../../enums/settings-enum';
import { enumToArray } from '../../../utils/array-utils';
import { SettingsModal } from '../settings-modal/settings-modal';

const StyledHeader = styled('header')(({ theme }) => ({
  padding: '10px',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  gap: '1rem',
  borderBottom: `1px solid ${theme.colors?.primary}`,

  [theme.breakpoints.down('md')]: {
    flexDirection: 'column',
  },
}));

const HeaderContent = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',

  [theme.breakpoints.down('md')]: {
    flexDirection: 'column',
  },
}));

const Title = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
});

export const Header = (): JSX.Element => {
  const router = useRouter();

  const searchOptions = enumToArray(SearchOptions);
  const searchMethods = enumToArray(SearchMethods);
  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchOption, setSearchOption] = useState<string>(searchOptions[0]);
  const [searchMethod, setSearchMethod] = useState<string>(searchMethods[0]);
  const [searchLanguage, setSearchLanguage] = useState<string>(searchMainLanguages[0]);
  const [selectedTypes, setSelectedTypes] = useState<string[]>(searchTypes);
  const [modalOpen, setModalOpen] = useState<boolean>(false);

  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);

  const handleMenuOpen = (e: React.MouseEvent<HTMLButtonElement>) => {
    setAnchorEl(e.currentTarget);
  };

  const handleMenuClose = () => {
    setAnchorEl(null);
  };

  return (
    <StyledHeader>
      <HeaderContent>
        <Title>
          <Image src={Logo} width={'50'} height={'50'} alt={'logo'} />
          <h1>{'CodeCompass'}</h1>
        </Title>
        <ProjectSelect currentProject={router.query.id as string} projects={['CodeCompass', 'cJSON']} />
        <TextField
          placeholder={searchMethod}
          InputProps={{
            startAdornment: (
              <InputAdornment position={'start'}>
                <Search />
              </InputAdornment>
            ),
          }}
        />
        <Button sx={{ textTransform: 'none' }} startIcon={<Settings />} onClick={() => setModalOpen(true)}>
          {'Search settings'}
        </Button>
        <SettingsModal
          modalOpen={modalOpen}
          setModalOpen={setModalOpen}
          searchOption={searchOption}
          setSearchOption={setSearchOption}
          searchMethod={searchMethod}
          setSearchMethod={setSearchMethod}
          searchLanguage={searchLanguage}
          setSearchLanguage={setSearchLanguage}
          selectedTypes={selectedTypes}
          setSelectedTypes={setSelectedTypes}
        />
      </HeaderContent>
      <IconButton onClick={(e) => handleMenuOpen(e)}>
        <MoreVert />
      </IconButton>
      <Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={handleMenuClose}>
        <MenuItem onClick={handleMenuClose}>{'About'}</MenuItem>
        <MenuItem onClick={handleMenuClose}>{'User guide'}</MenuItem>
        <MenuItem onClick={handleMenuClose}>{'Report a bug here'}</MenuItem>
        <MenuItem onClick={handleMenuClose}>{'Credits'}</MenuItem>
      </Menu>
    </StyledHeader>
  );
};
