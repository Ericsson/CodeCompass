import { IconButton, InputAdornment, Menu, MenuItem, TextField, Tooltip, styled } from '@mui/material';
import { ProjectSelect } from '../project-select/project-select';
import { Search, Settings, MoreVert, LightMode, DarkMode, Info } from '@mui/icons-material';
import { useContext, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from '../../enums/settings-enum';
import { enumToArray } from '../../utils/array-utils';
import { ThemeContext } from '../../themes/theme-context';
import Logo from '../../../public/logo.png';
import { SettingsMenu } from '../settings-menu/settings-menu';
import { getTooltipText } from './get-tooltip-text';
import { WorkspaceContext } from '../../global-context/workspace-context';

const StyledHeader = styled('header')(({ theme }) => ({
  display: 'grid',
  gridTemplateColumns: '280px 1fr',
  gridTemplateRows: '1fr',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  minWidth: '1440px',
}));

const HeaderLogo = styled('div')(({ theme }) => ({
  height: '75px',
  backgroundImage: `url('${Logo.src}')`,
  backgroundRepeat: 'no-repeat',
  backgroundSize: '90%',
  backgroundPosition: 'center center',
  borderRight: `1px solid ${theme.colors?.primary}`,
}));

const HeaderContent = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  flexGrow: '1',
  padding: '10px 10px 10px 15px',
});

const SettingsContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});

const MenuContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});

export const Header = ({ workspaceName }: { workspaceName: string }): JSX.Element => {
  const workspaces = useContext(WorkspaceContext);
  const { theme, setTheme } = useContext(ThemeContext);

  const searchOptions = enumToArray(SearchOptions);
  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchOption, setSearchOption] = useState<string>(searchOptions[0]);
  const [searchLanguage, setSearchLanguage] = useState<string>(searchMainLanguages[0]);
  const [selectedTypes, setSelectedTypes] = useState<string[]>(searchTypes);
  const [menuAnchorEl, setMenuAnchorEl] = useState<null | HTMLElement>(null);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);

  const handleMenuOpen = (e: React.MouseEvent<HTMLButtonElement>) => {
    setMenuAnchorEl(e.currentTarget);
  };

  const handleMenuClose = () => {
    setMenuAnchorEl(null);
  };

  const changeTheme = () => {
    localStorage.setItem('theme', theme === 'dark' ? 'light' : 'dark');
    setTheme(theme === 'dark' ? 'light' : 'dark');
  };

  return (
    <StyledHeader>
      <HeaderLogo />
      <HeaderContent>
        <SettingsContainer>
          <ProjectSelect
            currentProject={workspaces.map((ws) => ws.id as string).filter((wsId) => wsId === workspaceName)[0]}
            projects={workspaces.map((ws) => ws.id as string)}
          />
          <TextField
            placeholder={
              searchOption === SearchOptions.FILE_NAME
                ? 'File name regex'
                : searchOption === SearchOptions.LOG
                ? 'Arbitrary log message'
                : 'Search by expression'
            }
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <IconButton onClick={(e) => setSettingsAnchorEl(e.currentTarget)}>
                    <Settings />
                  </IconButton>
                  <Tooltip title={getTooltipText(SearchMethods.EXPRESSION)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <TextField
            placeholder={'File name filter regex'}
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <Tooltip title={getTooltipText(SearchMethods.FILE_REGEX)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <TextField
            placeholder={'Path filter regex'}
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <Tooltip title={getTooltipText(SearchMethods.PATH_REGEX)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <SettingsMenu
            searchOption={searchOption}
            setSearchOption={setSearchOption}
            searchLanguage={searchLanguage}
            setSearchLanguage={setSearchLanguage}
            selectedTypes={selectedTypes}
            setSelectedTypes={setSelectedTypes}
            anchorEl={settingsAnchorEl}
            setAnchorEl={setSettingsAnchorEl}
          />
        </SettingsContainer>
        <MenuContainer>
          <IconButton onClick={() => changeTheme()}>{theme === 'dark' ? <LightMode /> : <DarkMode />}</IconButton>
          <IconButton onClick={(e) => handleMenuOpen(e)}>
            <MoreVert />
          </IconButton>
          <Menu anchorEl={menuAnchorEl} open={Boolean(menuAnchorEl)} onClose={handleMenuClose}>
            <MenuItem onClick={handleMenuClose}>{'About'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'User guide'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'Report a bug here'}</MenuItem>
            <MenuItem onClick={handleMenuClose}>{'Credits'}</MenuItem>
          </Menu>
        </MenuContainer>
      </HeaderContent>
    </StyledHeader>
  );
};
