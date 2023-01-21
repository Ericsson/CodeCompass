import { Button, IconButton, InputAdornment, Menu, MenuItem, TextField, styled } from '@mui/material';
import { ProjectSelect } from '../project-select/project-select';
import { useRouter } from 'next/router';
import { Search, Settings, MoreVert, LightMode, DarkMode } from '@mui/icons-material';
import { useContext, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from '../../enums/settings-enum';
import { enumToArray } from '../../utils/array-utils';
import { SettingsModal } from '../settings-modal/settings-modal';
import { ThemeContext } from '../../themes/theme-context';
import Logo from '../../../public/logo.png';

const StyledHeader = styled('header')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

const HeaderLogo = styled('div')({
  minWidth: '280px',
  height: '76px',
  backgroundImage: `url('${Logo.src}')`,
  backgroundRepeat: 'no-repeat',
  backgroundSize: 'cover',
  backgroundPosition: 'left center',
});

const HeaderContent = styled('div')(({ theme }) => ({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  flexGrow: '1',
  padding: '10px 10px 10px 0px',
}));

const SettingsContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});

const MenuContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});

export const Header = (): JSX.Element => {
  const router = useRouter();

  const { theme, setTheme } = useContext(ThemeContext);

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

  const changeTheme = () => {
    setTheme(theme === 'dark' ? 'light' : 'dark');
  };

  return (
    <StyledHeader>
      <HeaderLogo />
      <HeaderContent>
        <SettingsContainer>
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
        </SettingsContainer>
        <MenuContainer>
          <IconButton onClick={() => changeTheme()}>{theme === 'dark' ? <LightMode /> : <DarkMode />}</IconButton>
          <IconButton onClick={(e) => handleMenuOpen(e)}>
            <MoreVert />
          </IconButton>
          <Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={handleMenuClose}>
            <MenuItem onClick={handleMenuClose}>{'About'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'User guide'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'Report a bug here'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'Credits'}</MenuItem>
          </Menu>
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
        </MenuContainer>
      </HeaderContent>
    </StyledHeader>
  );
};
